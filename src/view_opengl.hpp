/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef VIEW_OPENGL_HPP
#define VIEW_OPENGL_HPP

#include "global.hpp"
#include "SDL.h"
#include <map>
#include <string>
#include "app_interface.hpp"
#include "view_interface.hpp"

class ViewOpenGL : public IView
{
public:
    bool Init()               override;
    void Cleanup()            override;
    bool Update(DeltaTime dt) override;

private:
    // @todo Resource Manager.

    typedef uint32 Shader;

    const uint MINIMUM_OPENGL_MAJOR = 3;
    const uint MINIMUM_OPENGL_MINOR = 3;

    const uint DESIRED_WINDOW_WIDTH  = 1280;
    const uint DESIRED_WINDOW_HEIGHT = 720;

    const bool ENABLE_VYSNC = true;
    const bool ADAPTIVE_VSYNC = true; // Classic or Adaptive VSync?

    const bool MULTISAMPLING = true;
    const uint MULTISAMPLING_NUMSAMPLES = 4; // 2 or 4.

    IApp* m_app = nullptr;
    std::string m_shaderPath;

    SDL_Window* m_window = nullptr;
    SDL_GLContext m_glContext = nullptr;

    uint64 m_fpsLastTime = 0;
    uint32 m_fpsCounter = 0;

    bool InitWindowAndGLContext_();
    bool InitGLFunctions_();

    std::map<std::string, Shader> m_shaders;

    // @note Sometimes multiple vertex or multiple fragment shaders can be used in
    //       a single program, but OpenGL ES and some others don't support it, so
    //       just don't allow it. Use preprocessing for shader source combination.
    bool CreateShader(std::string name, std::string vertex, std::string fragment);
    void DeleteShader(std::string name);
    bool UseShader   (std::string name);
    bool ShaderSetBool (std::string shader, std::string name, bool    value) const;
    bool ShaderSetInt  (std::string shader, std::string name, int     value) const;
    bool ShaderSetFloat(std::string shader, std::string name, float32 value) const;
    bool ShaderSetVec2f(std::string shader, std::string name, float32 x, float32 y) const;
    bool ShaderSetVec3f(std::string shader, std::string name, float32 x, float32 y, float32 z) const;
    bool ShaderSetVec4f(std::string shader, std::string name, float32 x, float32 y, float32 z, float32 w) const;
    bool LoadShader_(std::string name, bool vertex, Shader& shader);
};

#endif // VIEW_OPENGL_HPP
