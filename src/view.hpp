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

    uint64 m_fpsTimer;
    uint32 m_fpsCounter = 0;

    bool InitOpenGLLibrary_();
    bool InitWindowAndGLContext_();
    bool InitOpenGL_();

    // @todo Log Graphics Information.

    // @todo Everything below this really needs to be re-thought out.
    //       Too much individual functionality is being wrapped up, e.g., VBOs.
    //       It'd be best to only wrap difficult things like Shader *Programs* or
    //       hide the implementation details behind a DrawableEntity or something.

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

    bool IsShaderLoaded(std::string name, enum Shader::Type type);
    bool RetrieveShader(std::string name, enum Shader::Type type, uint& shader);
    void DeleteShader(std::string name, enum Shader::Type type);

    bool LoadShaderFile_(std::string name, enum Shader::Type type, std::string& contents);
    bool LoadShader_(std::string name, enum Shader::Type type, bool required);

    bool LoadShader(std::string name, enum Shader::Type type) { return LoadShader_(name, type, false); }
    bool RequireShader(std::string name, enum Shader::Type type) { return LoadShader_(name, type, true); }

    std::vector<ShaderProgram> m_shaderPrograms;
    typedef std::vector<std::string> ShaderList;

    bool IsShaderProgramCreated(std::string name);
    bool RetrieveShaderProgram(std::string name, uint& program);
    void DeleteShaderProgram(std::string name);
    bool CreateShaderProgram(std::string name, ShaderList vertexShaders, ShaderList fragmentShaders, bool deleteShadersOnSuccess = true);
    bool UseShaderProgram(std::string name);

    struct VBO {
        std::string name;
        uint vbo;
        uint32 usage;
    };

    std::vector<VBO> m_vbos;

    bool IsVBOCreated(std::string name);
    bool RetrieveVBO(std::string name, uint& vbo);
    void DeleteVBO(std::string name);
    // usage:
    //     GL_STREAM_DRAW:  the data is set only once and used by the GPU at most a few times.
    //     GL_STATIC_DRAW:  the data is set only once and used many times.
    //     GL_DYNAMIC_DRAW: the data is changed a lot and used many times.
    bool CreateVBO(std::string name, float32* vertices, uint32 usage);
    bool UseVBO(std::string name);
};

#endif // VIEW_HPP
