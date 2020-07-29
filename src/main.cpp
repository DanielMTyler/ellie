/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#include "global.hpp"



global_variable std::string g_prefPath;



internal bool AppInitSavePath()
{
    // Must SDL_Init before SDL_GetPrefPath.
    if (SDL_Init(0) < 0)
    {
        std::cerr << "Failed to initialze minimal SDL: " << SDL_GetError() << "." << std::endl;
        return false;
    }
    
    char* prefPath = SDL_GetPrefPath(ORGANIZATION_NAME, APPLICATION_NAME);
    if (!prefPath)
    {
        std::cerr << "Failed to get save path: " << SDL_GetError() << "." << std::endl;
        return false;
    }
    g_prefPath = prefPath;
    SDL_free(prefPath);
    prefPath = nullptr;
    
    return true;
}

internal void AppSDLLogOutputFunction(void* userdata, int category, SDL_LogPriority priority, const char* message)
{
    // @todo Log to file and console; maybe cerr (works even with GUIs) and cout?
    std::cout << message << std::endl;
}

internal bool AppInitLog()
{
    SDL_LogSetOutputFunction(AppSDLLogOutputFunction, nullptr);
    
    #ifndef NDEBUG
        LogWarning("Debug Build.");
    #endif
    
    return true;
}

internal bool AppInitSDL()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        LogFatal("Failed to initialize SDL: %s", SDL_GetError());
        return false;
    }
    LogInfo("Initalized SDL.");
    
    return true;
}

internal bool AppInitPaths()
{
    // exePathBuf will end with a path separator, which is what we want.
    char* exePathBuf = SDL_GetBasePath();
    if (!exePathBuf)
    {
        LogFatal("Failed to get executable path: %s.", SDL_GetError());
        return false;
    }
    std::string exePath = exePathBuf;
    SDL_free(exePathBuf);
    exePathBuf = nullptr;
    
    std::string cwdPath;
    if (!RetrieveCWD(cwdPath))
    {
        LogFatal("Failed to get the current working directory: %s.", AppGetLastError().c_str());
        return false;
    }
    
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
                LogFatal("The data folder wasn't found in the current working directory (%s) or the executable directory (%s).", cwdPath, exePath);
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
                    LogFatal("The data folder wasn't found in the current working directory (%s), the executable directory (%s), or \"<cwd>../../release/\" (%s).", cwdPath.c_str(), exePath.c_str(), releasePath.c_str());
                    return false;
                }
            #endif
        }
    }
    
    LogInfo("Save path: %s.", g_prefPath.c_str());
    LogInfo("Data path: %s.", dataPath.c_str());
    if (!ResManInit(dataPath, g_prefPath))
        return false;
    
    return true;
}

internal bool AppInit()
{
    if (!VerifySingleInstanceInit())
        return false;
    
    if (!AppInitSavePath())
        return false;
    if (!AppInitLog())
        return false;
    if (!AppInitSDL())
        return false;
    if (!AppInitPaths())
        return false;
    
    return true;
}

internal void AppCleanup()
{
    ResManCleanup();
    SDL_Quit();
    VerifySingleInstanceCleanup();
}

internal int AppLoop()
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


// @warning SDL 2 requires this function signature to avoid SDL_main linker errors.
int main(int argc, char* argv[])
{
    int ret = 0;
    
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
