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
    
    virtual bool Init() override
    {
        if (!BaseApp::Init())
            return false;
        
        
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
        m_prefPath = sdlPrefPath;
        SDL_free(sdlPrefPath);
        sdlPrefPath = nullptr;
        
        
        try
        {
            auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(m_prefPath + PROJECT_NAME + ".log", true);
            
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
            m_logger = std::make_shared<spdlog::logger>("default", loggerSinks.begin(), loggerSinks.end());
            spdlog::register_logger(m_logger);
            spdlog::set_default_logger(m_logger);
        }
        catch (const spdlog::spdlog_ex& ex)
        {
            std::cerr << "Failed to initialize loggers: " << ex.what() << "." << std::endl;
            return false;
        }
        
        
        #ifndef NDEBUG
            SPDLOG_LOGGER_WARN(m_logger, "Debug Build.");
        #endif
        
        SPDLOG_LOGGER_INFO(m_logger, "User preferences path: {}.", GetPrefPath());
        
        
        if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
        {
            SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to initialize SDL: %s", SDL_GetError());
            return false;
        }
        SPDLOG_LOGGER_INFO(m_logger, "Initalized SDL.");
        
        
        // exePathBuf will end with a path separator, which is what we want.
        char* exePathBuf = SDL_GetBasePath();
        if (!exePathBuf)
        {
            SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to get executable path: {}.", SDL_GetError());
            return false;
        }
        std::string exePath = exePathBuf;
        SDL_free(exePathBuf);
        exePathBuf = nullptr;
        SPDLOG_LOGGER_INFO(m_logger, "EXE path: {}.", exePath);
        
        std::string cwdPath;
        ResultBool r = GetCWD(cwdPath);
        if (!r)
        {
            SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to get the current working directory: {}.", r.error);
            return false;
        }
        SPDLOG_LOGGER_INFO(m_logger, "CWD: {}.", cwdPath);
        
        // Find the data folder in cwdPath, exePath, or "<cwdPath>/../../release/" in debug builds.
        std::string releasePath = cwdPath;
        m_dataPath = releasePath + "data" + GetPathSeparator();
        if (!FolderExists(m_dataPath))
        {
            releasePath = exePath;
            m_dataPath = releasePath + "data" + GetPathSeparator();
            if (!FolderExists(m_dataPath))
            {
                #ifdef NDEBUG
                    SPDLOG_LOGGER_CRITICAL(m_logger, "The data folder wasn't found in the current working directory ({}) or the executable directory ({}).", cwdPath, exePath);
                    return false;
                #else
                    // Move cwdPath up 2 directories.
                    releasePath = cwdPath.substr(0, cwdPath.size()-1);
                    releasePath = releasePath.substr(0, releasePath.find_last_of(GetPathSeparator()));
                    releasePath = releasePath.substr(0, releasePath.find_last_of(GetPathSeparator()));
                    releasePath += GetPathSeparator() + "release" + GetPathSeparator();
                    m_dataPath = releasePath + "data" + GetPathSeparator();
                    if (!FolderExists(m_dataPath))
                    {
                        SPDLOG_LOGGER_CRITICAL(m_logger, "The data folder wasn't found in the current working directory ({}), the executable directory ({}), or \"<cwd>../../release/\" ({}).", cwdPath, exePath, releasePath);
                        return false;
                    }
                #endif
            }
        }
        SPDLOG_LOGGER_INFO(m_logger, "Data path: {}.", m_dataPath);
        
        
        r = m_taskManager.Init(this);
        if (!r)
        {
            SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to initalize primary task manager: {}.", r.error);
            return false;
        }
        SPDLOG_LOGGER_INFO(m_logger, "Initialized primary task manager.");
        
        
        return true;
    }
    
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
            //m_taskManager.Add(std::make_shared<TestTask>());
            m_taskManager.Update(dt);
            if (!m_taskManager.GetSize())
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
    StrongLoggerPtr m_logger;
    std::string m_dataPath;
    std::string m_prefPath;
    TaskManager m_taskManager;
    DeltaTime m_timeDilation = 1.0f;
};



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
