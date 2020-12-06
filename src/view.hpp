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
#include <string>
#include <vector>

class View
{
public:
    bool Init();
    void Cleanup();
    bool Update(DeltaTime dt); // Returns false when time to exit.

private:
    const uint MINIMUM_OPENGL_MAJOR = 3;
    const uint MINIMUM_OPENGL_MINOR = 3;

    const uint DESIRED_WINDOW_WIDTH  = 1280;
    const uint DESIRED_WINDOW_HEIGHT = 720;

    const bool ENABLE_VYSNC = true;
    const bool ADAPTIVE_VSYNC = true; // Classic or Adaptive VSync?

    const bool MULTISAMPLING = true;
    const uint MULTISAMPLING_NUMSAMPLES = 4; // 2 or 4.

    std::string m_shaderPath;

    SDL_Window* m_window = nullptr;
    SDL_GLContext m_glContext = nullptr;

    uint64 m_fpsTimer = 0;
    uint32 m_fpsCounter = 0;

    bool InitWindowAndGLContext_();
    bool InitGLFunctions_();
    // @todo Log Graphics Information.

    struct Shader {
        std::string name;
        uint program;
    };

    std::vector<Shader> m_shaders;
    typedef std::vector<std::string> ShaderList;

    bool CreateShader(std::string name, ShaderList vertices, ShaderList fragments);
    void DeleteShader(std::string name);
    bool UseShader(std::string name);
    bool ShaderSetBool  (std::string shader, std::string name, bool    value) const;
    bool ShaderSetInt   (std::string shader, std::string name, int     value) const;
    bool ShaderSetFloat (std::string shader, std::string name, float32 value) const;
    bool ShaderSet3Float(std::string shader, std::string name, float32 x, float32 y, float32 z) const;

    bool LoadShader_(std::string name, bool vertex, uint32& shader);
};

#endif // VIEW_HPP
