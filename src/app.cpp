/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#include "app.hpp"
#include "SDL_syswm.h"
#include <cstdio>
#include <filesystem>
#include <new>
#include "view_opengl.hpp"

App& App::Get()
{
    static App a;
    return a;
}

bool App::FolderExists(std::string folder) const
{
    std::error_code ec; // @note ignored; set if the folder doesn't exist.
    if (std::filesystem::is_directory(folder, ec))
        return true;
    else
        return false;
}

bool App::LoadFile(std::string file, std::string& contents) const
{
    // Failing to load a file might not be fatal, but we do provide warning
    // messages to provide information on the actual failure.

    std::FILE* f = std::fopen(file.c_str(), "rb");
    if (!f)
    {
        LogWarning("Failed to open file: %s.", file.c_str());
        return false;
    }

    if (std::fseek(f, 0, SEEK_END))
    {
        LogWarning("Failed to seek to end of file: %s.", file.c_str());
        std::fclose(f);
        return false;
    }

    int32 t = std::ftell(f);
    if (t == -1)
    {
        LogWarning("Failed to get current position in file: %s.", file.c_str());
        std::fclose(f);
        return false;
    }

    contents.clear();
    contents.resize(t);
    std::rewind(f);

    std::fread(&contents[0], 1, contents.size(), f);
    if (std::ferror(f))
    {
        LogWarning("Failed to read file: %s.", file.c_str());
        std::fclose(f);
        return false;
    }

    std::fclose(f);
    return true;
}

bool App::Init()
{
    if (!ForceSingleInstanceInit_())
    {
        LogFatal("Another instance is already running.");
        return false;
    }

    #ifndef NDEBUG
        // Set app log priority to show all messages; debug/verbose are hidden by default.
        SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE);
        LogWarning("Debug Build.");
    #endif

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        LogFatal("Failed to initialize SDL: %s", SDL_GetError());
        return false;
    }
    LogInfo("Initalized SDL.");

    InitLogSystemInfo_();

    if (!InitSavePath_())
        return false;
    if (!InitCWD_())
        return false;
    if (!InitExecutablePath_())
        return false;
    if (!InitDataPath_())
        return false;

    m_view = new (std::nothrow) ViewOpenGL;
    if (!m_view)
    {
        LogFatal("Failed to allocate memory for view.");
        return false;
    }
    else if (!m_view->Init())
    {
        return false;
    }

    if (!m_logic.Init())
        return false;

    LogInfo("Initialized.");
    return true;
}

void App::Cleanup()
{
    LogInfo("Cleaning up.");

    m_logic.Cleanup();

    if (m_view)
    {
        m_view->Cleanup();
        delete m_view;
        m_view = nullptr;
    }

    SDL_Quit();
    ForceSingleInstanceCleanup_();
}

int App::Loop()
{
    bool quit = false;
    uint64 dtNow = SDL_GetPerformanceCounter();
    uint64 dtLast;
    DeltaTime dt;
    while (!quit)
    {
        dtLast = dtNow;
        dtNow = SDL_GetPerformanceCounter();
        dt = (DeltaTime)(dtNow - dtLast) * 1000.0f / (DeltaTime)SDL_GetPerformanceFrequency();
        if (!m_logic.Update(dt) || !m_view->Update(dt))
            quit = true;
    }

    return 0;
}

#if defined(OS_WINDOWS)

    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>

    global_variable HANDLE g_windowsSingleInstanceMutex = nullptr;

    bool App::ForceSingleInstanceInit_() const
    {
        SDL_assert(!g_windowsSingleInstanceMutex);

        const char* name = ORGANIZATION_NAME "/" APPLICATION_NAME "/ForceSingleInstance";

        g_windowsSingleInstanceMutex = CreateMutex(nullptr, true, name);
        if (g_windowsSingleInstanceMutex && GetLastError() != ERROR_SUCCESS)
        {
            // GetLastError() should be ERROR_ALREADY_EXISTS || ERROR_ACCESS_DENIED.
            g_windowsSingleInstanceMutex = nullptr;
            return false;
        }

        return true;
    }

    void App::ForceSingleInstanceCleanup_() const
    {
        if (g_windowsSingleInstanceMutex)
        {
            ReleaseMutex(g_windowsSingleInstanceMutex);
            g_windowsSingleInstanceMutex = nullptr;
        }
    }

#elif defined(OS_LINUX)

    bool App::ForceSingleInstanceInit_() const
    {
        // @todo
        LogWarning("TODO: ForceSingleInstanceInit_.");
        return true;
    }

    void App::ForceSingleInstanceCleanup_() const
    {
        // @todo
        LogWarning("TODO: ForceSingleInstanceCleanup_.");
    }

#else

    #error Unknown OS.

#endif // OS_WINDOWS.

void App::InitLogSystemInfo_() const
{
    // SDL version
    {
        SDL_version c;
        SDL_version l;
        SDL_VERSION(&c);
        SDL_GetVersion(&l);
        LogInfo("SDL version: %u.%u.%u compiled & %u.%u.%u linked.",
                c.major, c.minor, c.patch, l.major, l.minor, l.patch);
    }

    LogInfo("Platform: %s.", SDL_GetPlatform());
    LogInfo("RAM: %i MiB.", SDL_GetSystemRAM());
    LogInfo("CPU: %i logical cores, L1 cache: %i bytes, 3DNow!: %s, AVX: %s, AVX2: %s, AltiVec: %s, MMX: %s, RDTSC: %s, SSE: %s, SSE2: %s, SSE3: %s, SSE4.1: %s, SSE4.2: %s.",
            SDL_GetCPUCount(), SDL_GetCPUCacheLineSize(), YesNoBoolToStr(SDL_Has3DNow()), YesNoBoolToStr(SDL_HasAVX()), YesNoBoolToStr(SDL_HasAVX2()),
            YesNoBoolToStr(SDL_HasAltiVec()), YesNoBoolToStr(SDL_HasMMX()), YesNoBoolToStr(SDL_HasRDTSC()),
            YesNoBoolToStr(SDL_HasSSE()), YesNoBoolToStr(SDL_HasSSE2()), YesNoBoolToStr(SDL_HasSSE3()), YesNoBoolToStr(SDL_HasSSE41()), YesNoBoolToStr(SDL_HasSSE42()));

    // Power
    {
        int secondsLeft;
        int batteryPercentage;
        SDL_PowerState s = SDL_GetPowerInfo(&secondsLeft, &batteryPercentage);

        if (s == SDL_POWERSTATE_ON_BATTERY)
        {
            std::string time = "unknown time";
            std::string charge = "unknown charge";

            if (batteryPercentage != -1)
                charge = std::to_string(batteryPercentage) + "%";

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

            LogInfo("Power: Battery with %s and %s remaining.", charge.c_str(), time.c_str());
        }
        else if (s == SDL_POWERSTATE_NO_BATTERY)
        {
            LogInfo("Power: AC with no battery.");
        }
        else if (s == SDL_POWERSTATE_CHARGING)
        {
            LogInfo("Power: AC with a charging battery.");
        }
        else if (s == SDL_POWERSTATE_CHARGED)
        {
            LogInfo("Power: AC with a fully charged battery.");
        }
        else
        {
            LogInfo("Power: Unknown.");
        }
    }

    // Window Manager
    {
        SDL_Window* w = nullptr;
        SDL_SysWMinfo i;
        SDL_VERSION(&i.version);

        w = SDL_CreateWindow("App::InitLogSystemInfo_",
                             SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                             1, 1, SDL_WINDOW_HIDDEN);
        if (!w)
        {
            LogInfo("Window Manager: Unknown (failed to create window: %s).", SDL_GetError());
            return;
        }
        else
        {
            if (!SDL_GetWindowWMInfo(w, &i))
            {
                LogInfo("Window Manager: Unknown (%s).", SDL_GetError());
                SDL_DestroyWindow(w);
                w = nullptr;
                return;
            }
            else
            {
                SDL_DestroyWindow(w);
                w = nullptr;
            }
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

        LogInfo("Window Manager: %s.", wm);
    }
}

bool App::InitSavePath_()
{
    char* prefPath = SDL_GetPrefPath(ORGANIZATION_NAME, APPLICATION_NAME);
    if (!prefPath)
    {
        LogFatal("Failed to get save path: %s.", SDL_GetError());
        return false;
    }
    m_savePath = prefPath;
    SDL_free(prefPath);
    prefPath = nullptr;
    LogInfo("Save path: %s.", m_savePath.c_str());
    return true;
}

bool App::InitCWD_()
{
    std::error_code ec;
    std::filesystem::path p = std::filesystem::current_path(ec);
    if (ec)
    {
        LogFatal("Failed to get the current working directory: %s.", ec.message().c_str());
        return false;
    }

    m_cwd = p.string() + PATH_SEPARATOR;
    LogInfo("CWD: %s.", m_cwd.c_str());
    return true;
}

bool App::InitExecutablePath_()
{
    // exePathBuf will end with a path separator, which is what we want.
    char* exePathBuf = SDL_GetBasePath();
    if (!exePathBuf)
    {
        LogFatal("Failed to get executable path: %s.", SDL_GetError());
        return false;
    }
    m_executablePath = exePathBuf;
    SDL_free(exePathBuf);
    exePathBuf = nullptr;
    LogInfo("Executable path: %s.", m_executablePath.c_str());
    return true;
}

bool App::InitDataPath_()
{
    // Find the data folder in cwd, executable path, or "<cwd>/../../release/".
    std::string releasePath = m_cwd;
    m_dataPath = releasePath + "data" + PATH_SEPARATOR;
    if (!FolderExists(m_dataPath))
    {
        releasePath = m_executablePath;
        m_dataPath = releasePath + "data" + PATH_SEPARATOR;
        if (!FolderExists(m_dataPath))
        {
            // Move cwd up 2 directories.
            releasePath = m_cwd.substr(0, m_cwd.size()-1);
            releasePath = releasePath.substr(0, releasePath.find_last_of(PATH_SEPARATOR));
            releasePath = releasePath.substr(0, releasePath.find_last_of(PATH_SEPARATOR));
            releasePath += std::string(PATH_SEPARATOR) + "release" + PATH_SEPARATOR;
            m_dataPath = releasePath + "data" + PATH_SEPARATOR;
            if (!FolderExists(m_dataPath))
            {
                LogFatal("The data folder wasn't found in the current working directory (%s), the executable directory (%s), or \"<cwd>../../release/\" (%s).", m_cwd.c_str(), m_executablePath.c_str(), releasePath.c_str());
                return false;
            }
        }
    }

    LogInfo("Data path: %s.", m_dataPath.c_str());
    return true;
}
