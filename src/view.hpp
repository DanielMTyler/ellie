/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef VIEW_HPP
#define VIEW_HPP

#include "global.hpp"
#include "SDL.h"
#include "process_manager.hpp"

class View
{
public:
    const uint MINIMUM_OPENGL_MAJOR = 3;
    const uint MINIMUM_OPENGL_MINOR = 3;

    const uint WINDOW_WIDTH  = 1280;
    const uint WINDOW_HEIGHT = 720;

    const bool ENABLE_VYSNC = true;
    const bool ADAPTIVE_VSYNC = true; // Classic or Adaptive VSync?

    const bool MULTISAMPLING = true;
    const uint MULTISAMPLING_NUMSAMPLES = 4; // 2 or 4.

    bool Init();
    void Cleanup();
    bool Update(DeltaTime dt); // Returns false when time to exit.

private:
    ProcessManager m_processManager;

    SDL_Window* m_window = nullptr;
    SDL_GLContext m_glContext = nullptr;

    bool InitSDL();
    bool InitWindow();
    bool InitOpenGL();

    void LogSystemInfoPower() const;
    void LogSystemInfoSDLVersion() const;
    void LogSystemInfoWindowManager() const;
    void LogSystemInfoRAM() const;
    void LogSystemInfoCPU() const;
    void LogSystemInfoGraphics() const;
    void LogSystemInfo() const;
};

#endif // VIEW_HPP
