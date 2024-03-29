/*
    ==================================
    Copyright (C) 2021 Daniel Tyler.
      This file is part of Ellie.
    ==================================
*/

#include "app.hpp"
#include "event_bus.hpp"
#include "logic.hpp"
#include "view_interface.hpp"
#include "view_opengl.hpp"

#include <SDL.h>
#include <SDL_syswm.h>

#include <cstdio>
#include <filesystem>
#include <new>

App& App::Get()
{
    static App a;
    return a;
}

bool App::FolderExists(std::string folder)
{
    std::error_code ec; // ignored; set if the folder doesn't exist.
    if (std::filesystem::is_directory(folder, ec))
        return true;
    else
        return false;
}

bool App::LoadFile(std::string file, std::string& contents)
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
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                 APPLICATION_NAME,
                                 "Another instance is already running.",
                                 nullptr);
        return false;
    }

    #ifndef NDEBUG
        // Show all messages; debug/verbose are hidden by default.
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

    m_options.core.shaderPath  = m_options.core.dataPath + "shaders" + PATH_SEPARATOR;
    m_options.core.texturePath = m_options.core.dataPath + "textures" + PATH_SEPARATOR;

    m_events = new (std::nothrow) EventBus;
    if (!m_events)
    {
        LogFatal("Failed to allocate memory for event bus.");
        return false;
    }

    m_logic = new (std::nothrow) class Logic;
    if (!m_logic)
    {
        LogFatal("Failed to allocate memory for logic.");
        return false;
    }
    else if (!m_logic->Init())
    {
        return false;
    }

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

    LogInfo("Initialized.");
    return true;
}

void App::Cleanup()
{
    LogInfo("Cleaning up.");

    if (m_view)
    {
        m_view->Cleanup();
        delete m_view;
        m_view = nullptr;
    }

    if (m_logic)
    {
        m_logic->Cleanup();
        delete m_logic;
        m_logic = nullptr;
    }

    if (m_events)
    {
        delete m_events;
        m_events = nullptr;
    }

    SDL_Quit();
    ForceSingleInstanceCleanup_();
}

int App::Loop()
{
    TimeStamp dtNow = Time();
    TimeStamp dtLast;
    DeltaTime dt;
    while (true)
    {
        dtLast = dtNow;
        dtNow = Time();
        dt = App::MillisecondsBetween(dtLast, dtNow);

        if (!m_view->ProcessEvents(dt))
            break;
        // @todo Dynamically adjust timelimit and or drop events if needed to
        //       maintain framerate and avoid the backlog growing forever.
        m_events->Update(true, 1000.0f/30.0f/2.0f);

        if (!m_logic->Update(dt))
            break;
        if (!m_view->Render(dt))
            break;
    }

    return 0;
}

#if defined(OS_WINDOWS)

    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>

    global_variable HANDLE g_windowsSingleInstanceMutex = nullptr;

    bool App::ForceSingleInstanceInit_()
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

    void App::ForceSingleInstanceCleanup_()
    {
        if (g_windowsSingleInstanceMutex)
        {
            ReleaseMutex(g_windowsSingleInstanceMutex);
            g_windowsSingleInstanceMutex = nullptr;
        }
    }

#elif defined(OS_LINUX)

    bool App::ForceSingleInstanceInit_()
    {
        // @todo ForceSingleInstanceInit_
        LogWarning("TODO: ForceSingleInstanceInit_.");
        return true;
    }

    void App::ForceSingleInstanceCleanup_()
    {
        // @todo ForceSingleInstanceCleanup_
        LogWarning("TODO: ForceSingleInstanceCleanup_.");
    }

#else

    #error Unknown OS.

#endif // OS_WINDOWS.

void App::InitLogSystemInfo_()
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
    m_options.core.savePath = prefPath;
    SDL_free(prefPath);
    prefPath = nullptr;
    LogInfo("Save path: %s.", m_options.core.savePath.c_str());
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

    m_options.core.cwdPath = p.string() + PATH_SEPARATOR;
    LogInfo("CWD: %s.", m_options.core.cwdPath.c_str());
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
    m_options.core.executablePath = exePathBuf;
    SDL_free(exePathBuf);
    exePathBuf = nullptr;
    LogInfo("Executable path: %s.", m_options.core.executablePath.c_str());
    return true;
}

bool App::InitDataPath_()
{
    // Find the data folder in cwd, executable path, or "<cwd>/../../release/".
    std::string releasePath = m_options.core.cwdPath;
    m_options.core.dataPath = releasePath + "data" + PATH_SEPARATOR;
    if (!FolderExists(m_options.core.dataPath))
    {
        releasePath = m_options.core.executablePath;
        m_options.core.dataPath = releasePath + "data" + PATH_SEPARATOR;
        if (!FolderExists(m_options.core.dataPath))
        {
            // Move cwd up 2 directories.
            releasePath = m_options.core.cwdPath.substr(0, m_options.core.cwdPath.size()-1);
            releasePath = releasePath.substr(0, releasePath.find_last_of(PATH_SEPARATOR));
            releasePath = releasePath.substr(0, releasePath.find_last_of(PATH_SEPARATOR));
            releasePath += std::string(PATH_SEPARATOR) + "release" + PATH_SEPARATOR;
            m_options.core.dataPath = releasePath + "data" + PATH_SEPARATOR;
            if (!FolderExists(m_options.core.dataPath))
            {
                LogFatal("The data folder wasn't found in the current working directory (%s), the executable directory (%s), or \"<cwd>../../release/\" (%s).", m_options.core.cwdPath.c_str(), m_options.core.executablePath.c_str(), releasePath.c_str());
                return false;
            }
        }
    }

    LogInfo("Data path: %s.", m_options.core.dataPath.c_str());
    return true;
}
