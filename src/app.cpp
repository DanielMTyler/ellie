/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#include "global.hpp"
#include <SDL.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <exception> // exception
#include <iostream>
#include <memory> // make_shared, shared_ptr
#include <string>
#include <vector>



class App : public BaseApp {
public:
    virtual ~App() {}
    
    // Init() has to be defined outside the class so that it can create the Init
    // tasks while allowing the Init tasks to have a complete definition of App.
    virtual bool Init() override;
    
    virtual void Cleanup() override
    {
        m_taskManager.Cleanup();
        SDL_Quit();
        spdlog::shutdown();
        BaseApp::Cleanup();
    }
    
    virtual int Run() override
    {
        bool quit = false;
        uint64 dtNow = SDL_GetPerformanceCounter();
        uint64 dtLast = 0;
        DeltaTime dtReal = 0.0f;
        DeltaTime dt = 0.0f;
        while (!quit)
        {
            dtLast = dtNow;
            dtNow = SDL_GetPerformanceCounter();
            dtReal = (DeltaTime)(dtNow - dtLast) * 1000.0f / (DeltaTime)SDL_GetPerformanceFrequency();
            dt = dtReal * m_timeDilation;
            m_taskManager.Update(dt);
            if (!m_taskManager.GetCount())
                quit = true;
            SDL_Delay(1);
        }
        
        return 0;
    }
    
    virtual StrongLoggerPtr GetLogger()      override { return m_logger;       }
    virtual std::string     GetDataPath()    override { return m_dataPath;     }
    virtual std::string     GetPrefPath()    override { return m_prefPath;     }
    virtual TaskManager&    GetTaskManager() override { return m_taskManager;  }
    
    virtual DeltaTime GetTimeDilation()             override { return m_timeDilation; }
    virtual void      SetTimeDilation(DeltaTime td) override { m_timeDilation = td;   }
    
    
private:
    friend class InitPrefPathTask;
    friend class InitLogTask;
    friend class InitSDLTask;
    friend class InitPathsTask;
    
    StrongLoggerPtr m_logger;
    std::string m_dataPath;
    std::string m_prefPath;
    TaskManager m_taskManager;
    DeltaTime m_timeDilation = 1.0f;
};


class InitPrefPathTask : public Task {
public:
    virtual bool OnInit()
    {
        App& app = (App&)GetApp();
        
        // Must SDL_Init before SDL_GetPrefPath.
        if (SDL_Init(0) < 0)
        {
            std::cerr << "Failed to initialze minimal SDL: " << SDL_GetError() << "." << std::endl;
            return false;
        }
        
        char* sdlPrefPath = SDL_GetPrefPath(ORGANIZATION, PROJECT_NAME);
        if (!sdlPrefPath)
        {
            std::cerr << "Failed to get user preferences path: " << SDL_GetError() << "." << std::endl;
            return false;
        }
        app.m_prefPath = sdlPrefPath;
        SDL_free(sdlPrefPath);
        sdlPrefPath = nullptr;
        
        Succeed();
        return true;
    }
};


class InitLogTask : public Task {
public:
    virtual bool OnInit()
    {
        App& app = (App&)GetApp();
        StrongLoggerPtr logger;
        
        try
        {
            auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(app.m_prefPath + PROJECT_NAME + ".log", true);
            
            consoleSink->set_pattern("[%^%L%$] %v");
            fileSink->set_pattern("%T [%L] %v");
            
            #ifdef NDEBUG
                consoleSink->set_level(spdlog::level::info);
                fileSink->set_level(spdlog::level::info);
            #else
                consoleSink->set_level(spdlog::level::trace);
                fileSink->set_level(spdlog::level::trace);
            #endif
            
            std::vector<spdlog::sink_ptr> loggerSinks;
            loggerSinks.push_back(consoleSink);
            loggerSinks.push_back(fileSink);
            logger = app.m_logger = std::make_shared<spdlog::logger>("default", loggerSinks.begin(), loggerSinks.end());
            spdlog::register_logger(logger);
            spdlog::set_default_logger(logger);
        }
        catch (const spdlog::spdlog_ex& ex)
        {
            std::cerr << "Failed to initialize loggers: " << ex.what() << "." << std::endl;
            return false;
        }
        
        
        #ifndef NDEBUG
            SPDLOG_LOGGER_WARN(logger, "Debug Build.");
        #endif
        
        SPDLOG_LOGGER_INFO(logger, "User preferences path: {}.", app.GetPrefPath());
        
        
        Succeed();
        return true;
    }
};


class InitSDLTask : public Task {
public:
    virtual bool OnInit()
    {
        App& app = (App&)GetApp();
        
        if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
        {
            SPDLOG_LOGGER_CRITICAL(app.m_logger, "Failed to initialize SDL: %s", SDL_GetError());
            return false;
        }
        SPDLOG_LOGGER_INFO(app.m_logger, "Initalized SDL.");
        
        Succeed();
        return true;
    }
};


class InitPathsTask : public Task {
public:
    virtual bool OnInit()
    {
        App& app = (App&)GetApp();
        StrongLoggerPtr logger   = app.m_logger;
        std::string     pathSep  = app.GetPathSeparator();
        std::string     dataPath;
        
        // exePathBuf will end with a path separator, which is what we want.
        char* exePathBuf = SDL_GetBasePath();
        if (!exePathBuf)
        {
            SPDLOG_LOGGER_CRITICAL(logger, "Failed to get executable path: {}.", SDL_GetError());
            return false;
        }
        std::string exePath = exePathBuf;
        SDL_free(exePathBuf);
        exePathBuf = nullptr;
        SPDLOG_LOGGER_INFO(logger, "EXE path: {}.", exePath);
        
        std::string cwdPath;
        ResultBool r = app.GetCWD(cwdPath);
        if (!r)
        {
            SPDLOG_LOGGER_CRITICAL(logger, "Failed to get the current working directory: {}.", r.error);
            return false;
        }
        SPDLOG_LOGGER_INFO(logger, "CWD: {}.", cwdPath);
        
        // Find the data folder in cwdPath, exePath, or "<cwdPath>/../../release/" in debug builds.
        std::string releasePath = cwdPath;
        dataPath = releasePath + "data" + pathSep;
        if (!app.FolderExists(dataPath))
        {
            releasePath = exePath;
            dataPath = releasePath + "data" + pathSep;
            if (!app.FolderExists(dataPath))
            {
                #ifdef NDEBUG
                    SPDLOG_LOGGER_CRITICAL(logger, "The data folder wasn't found in the current working directory ({}) or the executable directory ({}).", cwdPath, exePath);
                    return false;
                #else
                    // Move cwdPath up 2 directories.
                    releasePath = cwdPath.substr(0, cwdPath.size()-1);
                    releasePath = releasePath.substr(0, releasePath.find_last_of(pathSep));
                    releasePath = releasePath.substr(0, releasePath.find_last_of(pathSep));
                    releasePath += pathSep + "release" + pathSep;
                    dataPath = releasePath + "data" + pathSep;
                    if (!app.FolderExists(dataPath))
                    {
                        SPDLOG_LOGGER_CRITICAL(logger, "The data folder wasn't found in the current working directory ({}), the executable directory ({}), or \"<cwd>../../release/\" ({}).", cwdPath, exePath, releasePath);
                        return false;
                    }
                #endif
            }
        }
        app.m_dataPath = dataPath;
        SPDLOG_LOGGER_INFO(logger, "Data path: {}.", dataPath);
        
        Succeed();
        return true;
    }
};


bool App::Init()
{
    if (!BaseApp::Init())
        return false;
    
    m_taskManager.Init(this);
    
    m_taskManager.AttachTask(std::make_shared<InitPrefPathTask>())
        ->AttachChild(std::make_shared<InitLogTask>())
        ->AttachChild(std::make_shared<InitSDLTask>())
        ->AttachChild(std::make_shared<InitPathsTask>());
    // Run all init tasks.
    while (m_taskManager.GetCount())
        m_taskManager.Update(0.0f);
    if (m_taskManager.GetFailed())
        return false;
    
    return true;
}



// WARNING: SDL 2 requires this function signature; changing it will give "undefined reference to SDL_main" linker errors.
int main(int argc, char* argv[])
{
    App app;
    int ret = 1;
    
    try
    {
        if (app.Init())
            ret = app.Run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    
    app.Cleanup();
    return ret;
}
