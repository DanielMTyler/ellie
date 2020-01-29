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
#include "platform.hpp"
#include "resources.hpp"
#include "tasks.hpp"



bool InitSavePathTaskOnInit(StrongTaskPtr t)
{
    // Must SDL_Init before SDL_GetPrefPath.
    if (SDL_Init(0) < 0)
    {
        std::cerr << "Failed to initialze minimal SDL: " << SDL_GetError() << "." << std::endl;
        return false;
    }
    
    char* prefPath = SDL_GetPrefPath(ORGANIZATION, PROJECT_NAME);
    if (!prefPath)
    {
        std::cerr << "Failed to get save path: " << SDL_GetError() << "." << std::endl;
        return false;
    }
    t->child->userData = new std::string(prefPath);
    SDL_free(prefPath);
    prefPath = nullptr;
    
    TaskSucceed(t);
    return true;
}

bool InitLogTaskOnInit(StrongTaskPtr t)
{
    std::string& savePath = *((std::string*)t->userData);
    t->child->userData = t->userData;
    
    try
    {
        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(savePath + PROJECT_NAME + ".log", true);
        
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
        delete (std::string*)t->userData;
        std::cerr << "Failed to initialize logger: " << ex.what() << "." << std::endl;
        return false;
    }
    
    #ifndef NDEBUG
        SPDLOG_WARN("Debug Build.");
    #endif
    
    SPDLOG_INFO("Save path: {}.", savePath);
    
    TaskSucceed(t);
    return true;
}

bool InitSDLTaskOnInit(StrongTaskPtr t)
{
    t->child->userData = t->userData;
    
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        delete (std::string*)t->userData;
        SPDLOG_CRITICAL("Failed to initialize SDL: %s", SDL_GetError());
        return false;
    }
    SPDLOG_INFO("Initalized SDL.");
    
    TaskSucceed(t);
    return true;
}

bool InitPathsTaskOnInit(StrongTaskPtr t)
{
    std::string savePath = *((std::string*)t->userData);
    delete (std::string*)t->userData;
    t->userData = nullptr;
    
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
    std::string pathSep = PATH_SEPARATOR;
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
    SPDLOG_INFO("Data path: {}.", dataPath);
    ResManInit(dataPath, savePath);
    
    TaskSucceed(t);
    return true;
}

bool AppInit()
{
    if (!VerifySingleInstanceInit())
        return false;
    
    TaskManager tm;
    TaskManInit(tm);
    StrongTaskPtr t = TaskManAttachTask(tm, TaskCreate(InitSavePathTaskOnInit));
    t = TaskAttachChild(t, TaskCreate(InitLogTaskOnInit));
    t = TaskAttachChild(t, TaskCreate(InitSDLTaskOnInit));
    t = TaskAttachChild(t, TaskCreate(InitPathsTaskOnInit));
    // Run all tasks.
    while (TaskManNumTasks(tm))
        TaskManUpdate(tm, 0.0f);
    TaskCount numFailed = TaskManNumFailed(tm);
    TaskManCleanup(tm);
    if (numFailed)
        return false;
    
    return true;
}

void AppCleanup()
{
    SDL_Quit();
    spdlog::shutdown();
    VerifySingleInstanceCleanup();
}

int AppLoop()
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


// WARNING: SDL 2 requires this function signature; changing it will give "undefined reference to SDL_main" linker errors.
int main(int argc, char* argv[])
{
    int ret = 1;
    
    try
    {
        if (AppInit())
            ret = AppLoop();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    
    AppCleanup();
    return ret;
}
