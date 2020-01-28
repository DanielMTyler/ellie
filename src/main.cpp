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
#include "platform.cpp"



std::string g_dataPath;
std::string g_prefPath;



class InitPrefPathTask : public Task {
public:
    virtual bool OnInit()
    {
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
        g_prefPath = sdlPrefPath;
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
        try
        {
            auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(g_prefPath + PROJECT_NAME + ".log", true);
            
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
            StrongLoggerPtr logger = std::make_shared<Logger>("default", loggerSinks.begin(), loggerSinks.end());
            spdlog::register_logger(logger);
            spdlog::set_default_logger(logger);
        }
        catch (const spdlog::spdlog_ex& ex)
        {
            std::cerr << "Failed to initialize loggers: " << ex.what() << "." << std::endl;
            return false;
        }
        
        
        #ifndef NDEBUG
            SPDLOG_WARN("Debug Build.");
        #endif
        
        SPDLOG_INFO("User preferences path: {}.", g_prefPath);
        
        
        Succeed();
        return true;
    }
};


class InitSDLTask : public Task {
public:
    virtual bool OnInit()
    {
        if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
        {
            SPDLOG_CRITICAL("Failed to initialize SDL: %s", SDL_GetError());
            return false;
        }
        SPDLOG_INFO("Initalized SDL.");
        
        Succeed();
        return true;
    }
};


class InitPathsTask : public Task {
public:
    virtual bool OnInit()
    {
        // exePathBuf will end with a path separator, which is what we want.
        char* exePathBuf = SDL_GetBasePath();
        if (!exePathBuf)
        {
            SPDLOG_CRITICAL("Failed to get executable path: {}.", SDL_GetError());
            return false;
        }
        std::string exePath = exePathBuf;
        SDL_free(exePathBuf);
        exePathBuf = nullptr;
        SPDLOG_INFO("EXE path: {}.", exePath);
        
        std::string cwdPath;
        ResultBool r = RetrieveCWD(cwdPath);
        if (!r)
        {
            SPDLOG_CRITICAL("Failed to get the current working directory: {}.", r.error);
            return false;
        }
        SPDLOG_INFO("CWD: {}.", cwdPath);
        
        // Find the data folder in cwdPath, exePath, or "<cwdPath>/../../release/" in debug builds.
        std::string pathSep = GetPathSeparator();
        std::string releasePath = cwdPath;
        std::string dataPath = releasePath + "data" + pathSep;
        if (!FolderExists(dataPath))
        {
            releasePath = exePath;
            dataPath = releasePath + "data" + pathSep;
            if (!FolderExists(dataPath))
            {
                #ifdef NDEBUG
                    SPDLOG_CRITICAL("The data folder wasn't found in the current working directory ({}) or the executable directory ({}).", cwdPath, exePath);
                    return false;
                #else
                    // Move cwdPath up 2 directories.
                    releasePath = cwdPath.substr(0, cwdPath.size()-1);
                    releasePath = releasePath.substr(0, releasePath.find_last_of(pathSep));
                    releasePath = releasePath.substr(0, releasePath.find_last_of(pathSep));
                    releasePath += pathSep + "release" + pathSep;
                    dataPath = releasePath + "data" + pathSep;
                    if (!FolderExists(dataPath))
                    {
                        SPDLOG_CRITICAL("The data folder wasn't found in the current working directory ({}), the executable directory ({}), or \"<cwd>../../release/\" ({}).", cwdPath, exePath, releasePath);
                        return false;
                    }
                #endif
            }
        }
        g_dataPath = dataPath;
        SPDLOG_INFO("Data path: {}.", g_dataPath);
        
        Succeed();
        return true;
    }
};


bool MainInit()
{
    if (!VerifySingleInstanceInit())
    {
        return false;
    }
    
    // @todo We don't need a TaskManager here, but I want it as a test for
    //       the TaskManager refactor later.
    TaskManager tm;
    tm.Init();
    
    tm.AttachTask(std::make_shared<InitPrefPathTask>())
        ->AttachChild(std::make_shared<InitLogTask>())
        ->AttachChild(std::make_shared<InitSDLTask>())
        ->AttachChild(std::make_shared<InitPathsTask>());
    // Run all tasks.
    while (tm.GetCount())
        tm.Update(0.0f);
    if (tm.GetFailed())
        return false;
    
    tm.Cleanup();
    return true;
}



int MainLoop()
{
    // @todo dt and timeDilation should be in the logic system.
    bool quit = false;
    // Time Dilation is how fast time moves, e.g., 0.5f means time is 50% slower than normal.
    DeltaTime timeDilation = 1.0f;
    uint64 dtNow = SDL_GetPerformanceCounter();
    uint64 dtLast = 0;
    DeltaTime dtReal = 0.0f;
    DeltaTime dt = 0.0f;
    while (!quit)
    {
        dtLast = dtNow;
        dtNow = SDL_GetPerformanceCounter();
        dtReal = (DeltaTime)(dtNow - dtLast) * 1000.0f / (DeltaTime)SDL_GetPerformanceFrequency();
        dt = dtReal * timeDilation;
        quit = true;
        SDL_Delay(1);
    }
    
    return 0;
}


void MainCleanup()
{
    SDL_Quit();
    spdlog::shutdown();
    VerifySingleInstanceCleanup();
}


// WARNING: SDL 2 requires this function signature; changing it will give "undefined reference to SDL_main" linker errors.
int main(int argc, char* argv[])
{
    int ret = 1;
    
    // While I don't use exceptions, many libraries (including the STL) do.
    try
    {
        if (MainInit())
            ret = MainLoop();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    
    MainCleanup();
    return ret;
}
