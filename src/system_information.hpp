/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef SYSTEM_INFORMATION_HPP_INCLUDED
#define SYSTEM_INFORMATION_HPP_INCLUDED

internal void LogSysInfoPower_()
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

internal void LogSysInfoSDLVersion_()
{
    SDL_version c;
    SDL_version l;
    SDL_VERSION(&c);
    SDL_GetVersion(&l);
    LogInfo("- SDL version: %u.%u.%u compiled & %u.%u.%u linked.",
            c.major, c.minor, c.patch, l.major, l.minor, l.patch);
}

internal void LogSysInfoWindowManager_()
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

internal void LogSysInfoRAM_()
{
    LogInfo("- RAM: %i MiB.", SDL_GetSystemRAM());
}

internal void LogSysInfoCPU_()
{
    LogInfo("- CPU: %i logical cores, L1 cache: %i bytes, 3DNow!: %s, AVX: %s, AVX2: %s, AltiVec: %s, MMX: %s, RDTSC: %s, SSE: %s, SSE2: %s, SSE3: %s, SSE4.1: %s, SSE4.2: %s.",
            SDL_GetCPUCount(), SDL_GetCPUCacheLineSize(), YesNoBoolToStr(SDL_Has3DNow()), YesNoBoolToStr(SDL_HasAVX()), YesNoBoolToStr(SDL_HasAVX2()),
            YesNoBoolToStr(SDL_HasAltiVec()), YesNoBoolToStr(SDL_HasMMX()), YesNoBoolToStr(SDL_HasRDTSC()),
            YesNoBoolToStr(SDL_HasSSE()), YesNoBoolToStr(SDL_HasSSE2()), YesNoBoolToStr(SDL_HasSSE3()), YesNoBoolToStr(SDL_HasSSE41()), YesNoBoolToStr(SDL_HasSSE42()));
}

internal void LogSysInfoGraphics_()
{
    // @todo
    LogInfo("- Graphics: TODO.");
}

internal void LogSystemInformation()
{
    LogInfo("System Information:");
    LogSysInfoPower_();
    LogInfo("- Platform: %s.", SDL_GetPlatform());
    LogSysInfoSDLVersion_();
    LogSysInfoWindowManager_();
    LogSysInfoRAM_();
    LogSysInfoCPU_();
    LogSysInfoGraphics_();
}

#endif // SYSTEM_INFORMATION_HPP_INCLUDED.
