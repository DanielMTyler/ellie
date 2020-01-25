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
        
        try
        {
            auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            consoleSink->set_pattern("[%^%l%$] %v");
            #ifdef NDEBUG
                consoleSink->set_level(spdlog::level::info);
            #else
                consoleSink->set_level(spdlog::level::trace);
            #endif
            
            auto consoleLogger = std::make_shared<spdlog::logger>("console", consoleSink);
            
            
            if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
            {
                SPDLOG_LOGGER_CRITICAL(consoleLogger, "Failed to initialize SDL: %s", SDL_GetError());
                return false;
            }
            
            
            char* sdlPrefPath = SDL_GetPrefPath(ORGANIZATION, PROJECT_NAME);
            if (!sdlPrefPath)
            {
                SPDLOG_LOGGER_CRITICAL(consoleLogger, "Failed to get user preferences path: {}.", SDL_GetError());
                return false;
            }
            m_prefPath = sdlPrefPath;
            SDL_free(sdlPrefPath);
            sdlPrefPath = nullptr;
            
            
            auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(m_prefPath + PROJECT_NAME + ".log", true);
            fileSink->set_pattern("%@ %T.%e [%l] %v");
            #ifdef NDEBUG
                fileSink->set_level(spdlog::level::info);
            #else
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
            std::cerr << "Failed to initialize logger: " << ex.what() << "." << std::endl;
            return false;
        }
        
        
        #ifndef NDEBUG
            SPDLOG_LOGGER_WARN(m_logger, "Debug Build.");
        #endif
        
        SPDLOG_LOGGER_INFO(m_logger, "User preferences path: {}.", m_prefPath);
        
        
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
        m_dataPath = releasePath + "data" + PathSeparator();
        if (!FolderExists(m_dataPath))
        {
            releasePath = exePath;
            m_dataPath = releasePath + "data" + PathSeparator();
            if (!FolderExists(m_dataPath))
            {
                #ifdef NDEBUG
                    SPDLOG_LOGGER_CRITICAL(m_logger, "The data folder wasn't found in the current working directory ({}) or the executable directory ({}).", cwdPath, exePath);
                    return false;
                #else
                    // Move cwdPath up 2 directories.
                    releasePath = cwdPath.substr(0, cwdPath.size()-1);
                    releasePath = releasePath.substr(0, releasePath.find_last_of(PathSeparator()));
                    releasePath = releasePath.substr(0, releasePath.find_last_of(PathSeparator()));
                    releasePath += PathSeparator() + "release" + PathSeparator();
                    m_dataPath = releasePath + "data" + PathSeparator();
                    if (!FolderExists(m_dataPath))
                    {
                        SPDLOG_LOGGER_CRITICAL(m_logger, "The data folder wasn't found in the current working directory ({}), the executable directory ({}), or \"<cwd>../../release/\" ({}).", cwdPath, exePath, releasePath);
                        return false;
                    }
                #endif
            }
        }
        SPDLOG_LOGGER_INFO(m_logger, "Data path: {}.", m_dataPath);
        
        return true;
    }
    
    virtual void Cleanup() override
    {
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
            
            quit = true;
            SPDLOG_INFO("We ran.");
            SDL_Delay(1);
        }
        
        return 0;
    }
    
    virtual std::shared_ptr<spdlog::logger> Logger() override { return m_logger; }
    virtual std::string DataPath() override { return m_dataPath; }
    virtual std::string PrefPath() override { return m_prefPath; }
    
    virtual DeltaTime TimeDilation() override { return m_timeDilation; }
    virtual void TimeDilation(DeltaTime td) override { m_timeDilation = td; }
    
    
    
private:
    std::shared_ptr<spdlog::logger> m_logger;
    std::string m_dataPath;
    std::string m_prefPath;
    DeltaTime m_timeDilation = 1.0f;
};



// WARNING: SDL 2 requires this function signature; changing it will give "undefined reference to SDL_main" linker errors.
int main(int /*argc*/, char* /*argv*/[])
{
    App app;
    int ret = 1;
    if (app.Init())
        ret = app.Run();
    
    app.Cleanup();
    return ret;
}
