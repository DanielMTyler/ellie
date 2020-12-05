/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#include "app.hpp"
#include "SDL_syswm.h"
#include "../thirdparty/glad/include/glad/glad.h"
#include "../thirdparty/glad/include/KHR/khrplatform.h"
#include <filesystem>
#include <iostream>

App& App::Get()
{
    static App a;
    return a;
}

std::string App::LastError()
{
    std::string r = m_lastError;
    m_lastError.clear();
    return r;
}

void App::SetError(std::string e)
{
    m_lastError = e;
}

bool App::HasError()
{
    return m_lastError.empty();
}

void App::ClearError()
{
    m_lastError.clear();
}

bool App::FolderExists(std::string folder)
{
    std::error_code ec;
    if (std::filesystem::is_directory(folder, ec))
    {
        return true;
    }
    else
    {
        if (ec)
            SetError(ec.message());

        return false;
    }
}

bool App::Init()
{
    if (!ForceSingleInstanceInit())
    {
        if (HasError())
            LogWarning("Failed while checking for other running instances: %s.", LastError().c_str());
        else
            LogWarning("Another instance is already running.");

        return false;
    }

    if (!InitLog())
        return false;
    if (!InitSavePath())
        return false;
    if (!InitCWD())
        return false;
    if (!InitExecutablePath())
        return false;
    if (!InitDataPath())
        return false;
    if (!InitSDL())
        return false;

    LogSystemInfo();

    if (!InitWindow())
        return false;
    if (!InitOpenGL())
        return false;

    LogInfo("Initialized.");
    return true;
}

void App::Cleanup()
{
    LogInfo("Cleaning up.");

    if (m_glContext)
    {
        SDL_GL_DeleteContext(m_glContext);
        m_glContext = nullptr;
    }

    if (m_window)
    {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    SDL_Quit();
    ForceSingleInstanceCleanup();
}

int App::Loop()
{
    // @todo This whole function.
    LogInfo("Main loop.");

    // @todo Deal with being minimized, toggling fullscreen, exiting, etc.
    // @todo dt and timeDilation should be in the logic system.
    bool quit = false;
    // Time Dilation is how fast time moves, e.g., 0.75f means everything moves at 75% speed.
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
        m_processManager.Update(dt);
        quit = true;
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        SDL_GL_SwapWindow(m_window);
        SDL_Delay(3000);
    }

    return 0;
}

#if defined(OS_WINDOWS)

    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>

    global_variable HANDLE g_windowsSingleInstanceMutex = nullptr;

    bool App::ForceSingleInstanceInit() const
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

    void App::ForceSingleInstanceCleanup() const
    {
        if (g_windowsSingleInstanceMutex)
        {
            ReleaseMutex(g_windowsSingleInstanceMutex);
            g_windowsSingleInstanceMutex = nullptr;
        }
    }

#elif defined(OS_LINUX)

    bool App::ForceSingleInstanceInit() const
    {
        // @todo
        LogWarning("TODO: ForceSingleInstanceInit.");
        return true;
    }

    void App::ForceSingleInstanceCleanup() const
    {
        // @todo
        LogWarning("TODO: ForceSingleInstanceCleanup.");
    }

#else

    #error Unknown OS.

#endif // OS_WINDOWS.

#if 0
void LogOutputFunction(void* /*userdata*/, int /*category*/, SDL_LogPriority priority, const char* message)
{
    std::string prefix;
    switch (priority)
    {
    case SDL_LOG_PRIORITY_DEBUG:    prefix = "DEBUG: "; break;
    case SDL_LOG_PRIORITY_WARN:     prefix = "WARNING: "; break;
    case SDL_LOG_PRIORITY_CRITICAL: prefix = "FATAL: "; break;
    default: break;
    }
    std::cerr << prefix << message << std::endl;
}
#endif // 0

bool App::InitLog()
{
    //SDL_LogSetOutputFunction(LogOutputFunction, nullptr);

    #ifndef NDEBUG
        // Set app log priority to show all messages; debug/verbose are hidden by default.
        SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE);
        LogWarning("Debug Build.");
    #endif

    return true;
}

bool App::InitSavePath()
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
    m_savePath = prefPath;
    SDL_free(prefPath);
    prefPath = nullptr;
    LogInfo("Save path: %s.", m_savePath.c_str());
    return true;
}

bool App::InitCWD()
{
    std::error_code ec;
    std::filesystem::path p = std::filesystem::current_path(ec);
    if (ec)
    {
        LogFatal("Failed to get the current working directory: %s.", ec.message().c_str());
        return false;
    }

    m_cwd = p.string() + PATH_SEP;
    LogInfo("CWD: %s.", m_cwd.c_str());
    return true;
}

bool App::InitExecutablePath()
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

bool App::InitDataPath()
{
    // Find the data folder in cwd, executable path, or "<cwd>/../../release/".
    std::string releasePath = m_cwd;
    m_dataPath = releasePath + "data" + PATH_SEP;
    if (!FolderExists(m_dataPath))
    {
        releasePath = m_executablePath;
        m_dataPath = releasePath + "data" + PATH_SEP;
        if (!FolderExists(m_dataPath))
        {
            // Move cwd up 2 directories.
            releasePath = m_cwd.substr(0, m_cwd.size()-1);
            releasePath = releasePath.substr(0, releasePath.find_last_of(PATH_SEP));
            releasePath = releasePath.substr(0, releasePath.find_last_of(PATH_SEP));
            releasePath += PATH_SEP + "release" + PATH_SEP;
            m_dataPath = releasePath + "data" + PATH_SEP;
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

bool App::InitSDL()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        LogFatal("Failed to initialize SDL: %s", SDL_GetError());
        return false;
    }
    LogInfo("Initalized SDL.");

    if (SDL_GL_LoadLibrary(nullptr))
    {
        LogFatal("SDL failed to load OpenGL: %s.", SDL_GetError());
        return false;
    }
    LogInfo("Loaded OpenGL library.");

    return true;
}

bool App::InitWindow()
{
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, MINIMUM_OPENGL_MAJOR))
    {
        LogFatal("Failed to request OpenGL major version: %s.", SDL_GetError());
        return false;
    }
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, MINIMUM_OPENGL_MINOR))
    {
        LogFatal("Failed to request OpenGL minor version: %s.", SDL_GetError());
        return false;
    }
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE))
    {
        LogFatal("Failed to request OpenGL core profile: %s.", SDL_GetError());
        return false;
    }
    LogInfo("OpenGL requested: v%i.%i Core Profile.", MINIMUM_OPENGL_MAJOR, MINIMUM_OPENGL_MINOR);

    if (SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1))
    {
        LogFatal("Failed to request OpenGL doublebuffering: %s.", SDL_GetError());
        return false;
    }
    LogInfo("Double Buffering requested.");

    if (MULTISAMPLING)
    {
        if (SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1))
        {
            LogFatal("Failed to request OpenGL multisampling: %s.", SDL_GetError());
            return false;
        }
        if (SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, MULTISAMPLING_NUMSAMPLES))
        {
            LogFatal("Failed to request OpenGL multisampling num samples: %s.", SDL_GetError());
            return false;
        }
        LogInfo("Multisampling requested: On with %i samples.", MULTISAMPLING_NUMSAMPLES);
    }
    else
    {
        LogInfo("Multisampling requested: No.");
    }

    // @note It looks like VSync has to be set prior to window creation.
    if (ENABLE_VYSNC)
    {
        if (ADAPTIVE_VSYNC)
        {
            if (!SDL_GL_SetSwapInterval(-1))
            {
                LogWarning("Failed to request Adaptive VSync: %s.", SDL_GetError());
                if (!SDL_GL_SetSwapInterval(1))
                    LogWarning("Failed to request Standard VSync: %s.", SDL_GetError());
                else
                    LogInfo("VSync Request: On.");
            }
            else
            {
                LogInfo("VSync Request: Adaptive.");
            }
        }
        else
        {
            if (!SDL_GL_SetSwapInterval(1))
                LogWarning("Failed to request Standard VSync: %s.", SDL_GetError());
            else
                LogInfo("VSync Request: On.");
        }
    }
    else
    {
        if (!SDL_GL_SetSwapInterval(0))
            LogWarning("Failed to request VSync off: %s.", SDL_GetError());
        LogInfo("VSync Request: Off.");
    }

    // @note Cleanup is handled by Cleanup().
    m_window = SDL_CreateWindow(APPLICATION_NAME, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if (!m_window)
    {
        LogFatal("Failed to create window: %s.", SDL_GetError());
        return false;
    }
    LogInfo("Created window.");

    // @note Cleanup is handled by Cleanup().
    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext)
    {
        LogFatal("Failed to create OpenGL context: %s.", SDL_GetError());
        return false;
    }
    LogInfo("Created OpenGL Context.");

    int glMajor;
    if (SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &glMajor))
    {
        LogFatal("Failed to retrieve actual OpenGL major version: %s.", SDL_GetError());
        return false;
    }
    int glMinor;
    if (SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &glMinor))
    {
        LogFatal("Failed to retrieve actual OpenGL minor version: %s.", SDL_GetError());
        return false;
    }
    int glProfile;
    if (SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &glProfile))
    {
        LogFatal("Failed to retrieve actual OpenGL profile: %s.", SDL_GetError());
        return false;
    }
    std::string glProfileStr = "Unknown";
    if (glProfile == SDL_GL_CONTEXT_PROFILE_CORE)
        glProfileStr = "Core";
    else if (glProfile == SDL_GL_CONTEXT_PROFILE_COMPATIBILITY)
        glProfileStr = "Compatibility";
    else if (glProfile == SDL_GL_CONTEXT_PROFILE_ES)
        glProfileStr = "ES";
    LogInfo("OpenGL: v%i.%i %s Profile.", glMajor, glMinor, glProfileStr.c_str());

    int doublebuffering;
    if (SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &doublebuffering))
    {
        LogFatal("Failed to retrieve actual OpenGL doublebuffering: %s.", SDL_GetError());
        return false;
    }
    LogInfo("Double Buffering: %s.", OnOffBoolToStr(doublebuffering));

    int multisampling;
    if (SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &multisampling))
    {
        LogFatal("Failed to retrieve actual OpenGL multisampling: %s.", SDL_GetError());
        return false;
    }
    int multisampling_numSamples;
    if (SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &multisampling_numSamples))
    {
        LogFatal("Failed to retrieve actual OpenGL multisampling num samples: %s.", SDL_GetError());
        return false;
    }
    LogInfo("Multisampling: %s with %i samples.", OnOffBoolToStr(multisampling), multisampling_numSamples);

    int vsync = SDL_GL_GetSwapInterval();
    if (vsync == 0)
        LogInfo("VSync: Off.");
    else if (vsync == 1)
        LogInfo("VSync: On.");
    else
        LogInfo("VSync: Adaptive.");

    return true;
}

bool App::InitOpenGL()
{
    if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
    {
        LogFatal("Failed to load OpenGL library functions.");
        return false;
    }
    LogInfo("Loaded OpenGL library functions.");

    if (GLVersion.major < (int)MINIMUM_OPENGL_MAJOR ||
        (GLVersion.major == (int)MINIMUM_OPENGL_MAJOR && GLVersion.minor < (int)MINIMUM_OPENGL_MINOR))
    {
        LogFatal("Loaded OpenGL v%i.%i, but need v%i.%i+.", GLVersion.major, GLVersion.minor,
                 MINIMUM_OPENGL_MAJOR, MINIMUM_OPENGL_MINOR);
        return false;
    }

    return true;
}

void App::LogSystemInfoPower() const
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

void App::LogSystemInfoSDLVersion() const
{
    SDL_version c;
    SDL_version l;
    SDL_VERSION(&c);
    SDL_GetVersion(&l);
    LogInfo("- SDL version: %u.%u.%u compiled & %u.%u.%u linked.",
            c.major, c.minor, c.patch, l.major, l.minor, l.patch);
}

void App::LogSystemInfoWindowManager() const
{
    SDL_Window* w = nullptr;
    SDL_SysWMinfo i;
    SDL_VERSION(&i.version);

    w = SDL_CreateWindow(APPLICATION_NAME, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                         WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_HIDDEN);
    if (!w)
    {
        LogInfo("- Window Manager: Unknown (failed to create window: %s).", SDL_GetError());
        return;
    }
    else
    {
        if (!SDL_GetWindowWMInfo(w, &i))
        {
            LogInfo("- Window Manager: Unknown (%s).", SDL_GetError());
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

    LogInfo("- Window Manager: %s.", wm);
}

void App::LogSystemInfoRAM() const
{
    LogInfo("- RAM: %i MiB.", SDL_GetSystemRAM());
}

void App::LogSystemInfoCPU() const
{
    LogInfo("- CPU: %i logical cores, L1 cache: %i bytes, 3DNow!: %s, AVX: %s, AVX2: %s, AltiVec: %s, MMX: %s, RDTSC: %s, SSE: %s, SSE2: %s, SSE3: %s, SSE4.1: %s, SSE4.2: %s.",
            SDL_GetCPUCount(), SDL_GetCPUCacheLineSize(), YesNoBoolToStr(SDL_Has3DNow()), YesNoBoolToStr(SDL_HasAVX()), YesNoBoolToStr(SDL_HasAVX2()),
            YesNoBoolToStr(SDL_HasAltiVec()), YesNoBoolToStr(SDL_HasMMX()), YesNoBoolToStr(SDL_HasRDTSC()),
            YesNoBoolToStr(SDL_HasSSE()), YesNoBoolToStr(SDL_HasSSE2()), YesNoBoolToStr(SDL_HasSSE3()), YesNoBoolToStr(SDL_HasSSE41()), YesNoBoolToStr(SDL_HasSSE42()));
}

void App::LogSystemInfoGraphics() const
{
    // @todo
    LogInfo("- Graphics: TODO.");
}

void App::LogSystemInfo() const
{
    LogInfo("System Information:");
    LogSystemInfoPower();
    LogInfo("- Platform: %s.", SDL_GetPlatform());
    LogSystemInfoSDLVersion();
    LogSystemInfoWindowManager();
    LogSystemInfoRAM();
    LogSystemInfoCPU();
    LogSystemInfoGraphics();
}
