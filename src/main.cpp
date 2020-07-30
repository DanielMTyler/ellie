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
        LogFatal("Failed to initialze minimal SDL: %s.", SDL_GetError());
        return false;
    }
    
    char* prefPath = SDL_GetPrefPath(ORGANIZATION_NAME, APPLICATION_NAME);
    if (!prefPath)
    {
        LogFatal("Failed to get save path: %s.", SDL_GetError());
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
        LogFatal("Failed to get the current working directory: %s.", AppGetError().c_str());
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
    
    return true;
}

internal void ReportPowerState()
{
    int secondsLeft;
    int batteryPercentage;
    SDL_PowerState s = SDL_GetPowerInfo(&secondsLeft, &batteryPercentage);
    
    if (s == SDL_POWERSTATE_ON_BATTERY)
    {
        std::string time = "unknown time";
        std::string charge = "unknown charge";
        
        if (batteryPercentage != -1)
        {
            charge = std::to_string(batteryPercentage) + "%";
        }
        
        if (secondsLeft != -1)
        {
            int s = secondsLeft;
            int m = 60;
            int h = 60*m;
            int hours = s/h;
            s -= hours*h;
            int minutes = s/m;
            s -= minutes*m;
            int seconds = s;
            
            if (hours > 0)
                time = std::to_string(hours) + "h" + std::to_string(minutes) + "m" + std::to_string(seconds) + "s";
            else if (minutes > 0)
                time = std::to_string(minutes) + "m" + std::to_string(seconds) + "s";
            else
                time = std::to_string(seconds) + "s";
        }
        
        LogInfo("- Power Source: Battery with %s and %s remaining.", charge.c_str(), time.c_str());
    }
    else if (s == SDL_POWERSTATE_NO_BATTERY)
    {
        LogInfo("- Power Source: AC with no battery.");
    }
    else if (s == SDL_POWERSTATE_CHARGING)
    {
        LogInfo("- Power Source: AC with a charging battery.");
    }
    else if (s == SDL_POWERSTATE_CHARGED)
    {
        LogInfo("- Power Source: AC with a fully charged battery.");
    }
    else
    {
        LogInfo("- Power Source: Unknown.");
    }
}

internal void ReportSDLVersion()
{
    SDL_version c;
    SDL_version l;
    SDL_VERSION(&c);
    SDL_GetVersion(&l);
    LogInfo("- SDL version: %u.%u.%u compiled & %u.%u.%u linked.",
            c.major, c.minor, c.patch, l.major, l.minor, l.patch);
}

internal void ReportWindowManager()
{
    SDL_Window* w = nullptr;
    SDL_SysWMinfo i;
    w = SDL_CreateWindow("", 0, 0, 0, 0, SDL_WINDOW_HIDDEN);
    if (!w || !SDL_GetWindowWMInfo(w, &i))
    {
        // SDL_GetWindowWMInfo fails if the compiled and linked SDL versions don't match.
        LogInfo("- Window Manager: Unknown.");
        return;
    }
    else
    {
        SDL_DestroyWindow(w);
        w = nullptr;
    }
    
    const char* wm = "Unknown";
    switch (i.subsystem)
    {
    case SDL_SYSWM_WINDOWS:  wm = "Microsoft Windows"; break;
    case SDL_SYSWM_X11:      wm = "X Window System"; break;
    case SDL_SYSWM_WINRT:    wm = "WinRT"; break;
    case SDL_SYSWM_DIRECTFB: wm = "DirectFB"; break;
    case SDL_SYSWM_COCOA:    wm = "Apple OS X"; break;
    case SDL_SYSWM_UIKIT:    wm = "UIKit"; break;
    case SDL_SYSWM_WAYLAND:  wm = "Wayland"; break;
    case SDL_SYSWM_MIR:      wm = "Mir"; break;
    case SDL_SYSWM_ANDROID:  wm = "Android"; break;
    case SDL_SYSWM_VIVANTE:  wm = "Vivante"; break;
    default: break;
    }
    
    LogInfo("- Window Manager: %s.", wm);
}

internal void ReportRAM()
{
    LogInfo("- RAM: %i MiB.", SDL_GetSystemRAM());
}

internal void ReportCPU()
{
    #define YNS(b) YesNoBoolToStr(b)
    LogInfo("- CPU: %i logical cores, L1 cache: %i bytes, 3DNow!: %s, AVX: %s, AVX2: %s, AltiVec: %s, MMX: %s, RDTSC: %s, SSE: %s, SSE2: %s, SSE3: %s, SSE4.1: %s, SSE4.2: %s.",
            SDL_GetCPUCount(), SDL_GetCPUCacheLineSize(), YNS(SDL_Has3DNow()), YNS(SDL_HasAVX()), YNS(SDL_HasAVX2()), YNS(SDL_HasAltiVec()), YNS(SDL_HasMMX()), YNS(SDL_HasRDTSC()),
            YNS(SDL_HasSSE()), YNS(SDL_HasSSE2()), YNS(SDL_HasSSE3()), YNS(SDL_HasSSE41()), YNS(SDL_HasSSE42()));
    #undef YNS
}

internal void ReportGraphics()
{
    // @todo
    LogInfo("- Graphics: Implement.");
}

internal void ReportSystemInformation()
{
    LogInfo("System Information:");
    ReportPowerState();
    LogInfo("- Platform: %s.", SDL_GetPlatform());
    ReportSDLVersion();
    ReportWindowManager();
    ReportRAM();
    ReportCPU();
    ReportGraphics();
}

internal bool AppInit()
{
    if (!CheckSingleInstanceInit())
    {
        if (AppHasError())
        {
            LogWarning("Failed while checking for other running instances: %s.", AppGetError().c_str());
        }
        else
        {
            LogWarning("Another instance is already running.");
            // @todo Maybe use SDL_ShowMessageBox to ask the user if they want to run another instance.
        }

        return false;
    }
    
    if (!AppInitSavePath())
        return false;
    if (!AppInitLog())
        return false;
    if (!AppInitSDL())
        return false;
    if (!AppInitPaths())
        return false;

    ReportSystemInformation();
    
    return true;
}

internal void AppCleanup()
{
    SDL_Quit();
    CheckSingleInstanceCleanup();
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
        LogFatal("%s.", e.what());
    }
    
    AppCleanup();
    return ret;
}
