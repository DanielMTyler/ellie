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
    std::string m_shaderPath;

    SDL_Window* m_window = nullptr;
    SDL_GLContext m_glContext = nullptr;

    uint64 m_fpsTimer;
    uint32 m_fpsCounter = 0;

    bool InitOpenGLLibrary_();
    bool InitWindowAndGLContext_();
    bool InitOpenGL_();

    void LogSystemInfoPower_() const;
    void LogSystemInfoSDLVersion_() const;
    void LogSystemInfoWindowManager_() const;
    void LogSystemInfoRAM_() const;
    void LogSystemInfoCPU_() const;
    void LogSystemInfoGraphics_() const;
    void LogSystemInfo() const;

    struct Shader {
        enum class Type { Vertex, Fragment };

        std::string name;
        enum Type type;
        uint shader;
    };

    struct ShaderProgram {
        std::string name;
        uint program;
    };

    std::vector<Shader> m_shaders;
    std::vector<ShaderProgram> m_shaderPrograms;

    typedef std::vector<std::string> ShaderList;

    bool IsShaderLoaded(std::string name, enum Shader::Type type);
    bool RetrieveShader(std::string name, enum Shader::Type type, uint& shader);
    void DeleteShader(std::string name, enum Shader::Type type);

    bool LoadShaderFile_(std::string name, enum Shader::Type type, std::string& contents);
    bool LoadShader_(std::string name, enum Shader::Type type, bool required);

    bool LoadShader(std::string name, enum Shader::Type type) { return LoadShader_(name, type, false); }
    bool RequireShader(std::string name, enum Shader::Type type) { return LoadShader_(name, type, true); }

    bool IsShaderProgramCreated(std::string name);
    bool RetrieveShaderProgram(std::string name, uint& program);
    void DeleteShaderProgram(std::string name);
    bool CreateShaderProgram(std::string name, ShaderList vertexShaders, ShaderList fragmentShaders, bool deleteShadersOnSuccess = true);
    bool UseShaderProgram(std::string name);
};

#endif // VIEW_HPP
