/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#include "global.hpp"



global_variable std::string g_prefPath;
global_variable SDL_Window* g_window = nullptr;
global_variable SDL_GLContext g_glContext = nullptr;



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
    std::string prefix;
    switch (priority)
    {
    case SDL_LOG_PRIORITY_DEBUG:    prefix = "[Debug] "; break;
    case SDL_LOG_PRIORITY_WARN:     prefix = "[Warning] "; break;
    case SDL_LOG_PRIORITY_CRITICAL: prefix = "[Fatal] "; break;
    default: break;
    }
    std::cout << prefix << message << std::endl;
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

    if (SDL_GL_LoadLibrary(nullptr))
    {
        LogFatal("SDL failed to load OpenGL: %s.", SDL_GetError());
        return false;
    }
    
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

internal bool AppInitWindow()
{
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, MINIMUM_OPENGL_MAJOR) ||
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, MINIMUM_OPENGL_MINOR) ||
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE))
    {
        LogFatal("Failed to set desired OpenGL version and profile attributes: %s.", SDL_GetError());
        return false;
    }
    
    g_window = SDL_CreateWindow(APPLICATION_NAME, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if (!g_window)
    {
        LogFatal("Failed to create window: %s.", SDL_GetError());
        return false;
    }
    
    g_glContext = SDL_GL_CreateContext(g_window);
    if (!g_glContext)
    {
        LogFatal("Failed to create OpenGL context: %s.", SDL_GetError());
        return false;
    }
    
#if ENABLE_VSYNC
    if (!SDL_GL_SetSwapInterval(1))
        LogWarning("Failed to enable VSync: %s.", SDL_GetError());
    else
        LogInfo("Enabled VSync.");
#else
    LogInfo("Not setting VSync.");
#endif // ENABLE_VSYNC.

    return true;
}

internal bool AppInitOpenGL()
{
    if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
    {
        LogFatal("Failed to load OpenGL.");
        return false;
    }
    
    if (GLVersion.major < MINIMUM_OPENGL_MAJOR ||
        (GLVersion.major == MINIMUM_OPENGL_MAJOR && GLVersion.minor < MINIMUM_OPENGL_MINOR))
    {
        LogFatal("Loaded OpenGL v%i.%i, but need v%i.%i.", GLVersion.major, GLVersion.minor,
                 MINIMUM_OPENGL_MAJOR, MINIMUM_OPENGL_MINOR);
        return false;
    }
    
    return true;
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
    
    LogSystemInformation();

    if (!AppInitWindow())
        return false;
    if (!AppInitOpenGL())
        return false;
    
    return true;
}

internal void AppCleanup()
{
    if (g_glContext)
    {
        SDL_GL_DeleteContext(g_glContext);
        g_glContext = nullptr;
    }
    
    if (g_window)
    {
        SDL_DestroyWindow(g_window);
        g_window = nullptr;
    }
    
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
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        SDL_GL_SwapWindow(g_window);
        SDL_Delay(3000);
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
