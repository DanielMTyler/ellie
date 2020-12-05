/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef APP_HPP
#define APP_HPP

#include "global.hpp"
#include "SDL.h"
#include <string>

class App {
public:
    // These are used to name the saves folder among other things, so ASCII without spaces is probably best.
    const char* ORGANIZATION_NAME = "DanielMTyler";
    const char* APPLICATION_NAME  = "Ellie";

    const uint MINIMUM_OPENGL_MAJOR = 3;
    const uint MINIMUM_OPENGL_MINOR = 3;

    const uint WINDOW_WIDTH  = 1280;
    const uint WINDOW_HEIGHT = 720;

    const bool ENABLE_VYSNC = true;
    const bool ADAPTIVE_VSYNC = true; // Classic or Adaptive VSync?

    const bool MULTISAMPLING = true;
    const uint MULTISAMPLING_NUMSAMPLES = 4; // 2 or 4.

    const std::string PATH_SEP = PATH_SEPARATOR;

    std::string SavePath() const { return m_savePath; }
    std::string DataPath() const { return m_dataPath; }
    std::string ExecutablePath() const { return m_executablePath; }
    std::string CWD() const { return m_cwd; }

    static App& Get();

    // Returns the last error (may be "") and clears it.
    std::string LastError();
    void SetError(std::string e);
    bool HasError();
    void ClearError();

    bool FolderExists(std::string folder);

    bool Init();
    void Cleanup();
    int Loop();

private:
    std::string m_lastError;

    std::string m_savePath;
    std::string m_dataPath;
    std::string m_executablePath;
    std::string m_cwd;

    SDL_Window* m_window = nullptr;
    SDL_GLContext m_glContext = nullptr;

    App() {};

    // Returns true if we're the only running instance or false if we're not; returns false after SetError() on failure.
    bool ForceSingleInstanceInit();
    void ForceSingleInstanceCleanup();

    bool InitLog();
    bool InitSavePath();
    bool InitCWD();
    bool InitExecutablePath();
    bool InitDataPath();
    bool InitSDL();
    bool InitWindow();
    bool InitOpenGL();

    void LogSystemInfoPower();
    void LogSystemInfoSDLVersion();
    void LogSystemInfoWindowManager();
    void LogSystemInfoRAM();
    void LogSystemInfoCPU();
    void LogSystemInfoGraphics();
    void LogSystemInfo();
};

#endif // APP_HPP
