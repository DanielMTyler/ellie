/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#include "app.hpp"
#include <filesystem>

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
    // Assuming that our ProcessManager is for initialization only.
    while (m_processManager.Count() > 0)
        m_processManager.Update(0.0f);

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
    if (!m_view.Init())
        return false;

    LogInfo("Initialized.");
    return true;
}

void App::Cleanup()
{
    LogInfo("Cleaning up.");
    m_view.Cleanup();
    SDL_Quit();
    ForceSingleInstanceCleanup();
}

int App::Loop()
{
    LogInfo("Main loop.");

    bool quit = false;
    uint64 dtNow = SDL_GetPerformanceCounter();
    uint64 dtLast;
    DeltaTime dt;
    while (!quit)
    {
        dtLast = dtNow;
        dtNow = SDL_GetPerformanceCounter();
        dt = (DeltaTime)(dtNow - dtLast) * 1000.0f / (DeltaTime)SDL_GetPerformanceFrequency();
        if (!m_view.Update(dt))
            quit = true;
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
#include <iostream>

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
