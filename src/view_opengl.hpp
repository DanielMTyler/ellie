/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef VIEW_OPENGL_HPP
#define VIEW_OPENGL_HPP

// @todo This should really be split into HumanView and OpenGLRenderer.

#include "global.hpp"
#include <glad/glad.h>
#include <KHR/khrplatform.h>
#include <glm/glm.hpp>
#include <SDL.h>
#include <map>
#include <string>
#include "events.hpp"
#include "logic.hpp"
#include "view_interface.hpp"

class App;

class ViewOpenGL : public IView
{
public:
    bool Init()               override;
    void Cleanup()            override;
    bool ProcessEvents(DeltaTime dt) override;
    bool Render(DeltaTime dt) override;

private:
    // @todo View-specific Resource Manager (shaders, textures, audio, etc)?

    typedef uint32 Shader;
    typedef uint32 Texture;

    const uint32 MINIMUM_OPENGL_MAJOR = 3;
    const uint32 MINIMUM_OPENGL_MINOR = 3;

    App*   m_app   = nullptr;
    Logic* m_logic = nullptr;

    SDL_Window* m_window = nullptr;
    SDL_GLContext m_glContext = nullptr;

    uint64 m_fpsLastTime = 0;
    uint32 m_fpsCounter = 0;

    bool InitWindowAndGLContext_();
    bool InitGLFunctions_();
    void InitLogGraphicsInfo_();

    std::map<std::string, Shader>  m_shaders;
    std::map<std::string, Texture> m_textures;

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
    bool ShaderSetVec2(std::string shader, std::string name, glm::vec2 v) const;
    bool ShaderSetVec3(std::string shader, std::string name, glm::vec3 v) const;
    bool ShaderSetVec4(std::string shader, std::string name, glm::vec4 v) const;
    bool ShaderSetMat4(std::string shader, std::string name, glm::mat4 m) const;
    bool LoadShader_(std::string name, bool vertex, Shader& shader);

    bool CreateTexture(std::string name,
                       bool hasAlpha,
                       uint32 wrapS = GL_REPEAT,
                       uint32 wrapT = GL_REPEAT,
                       uint32 minFilter = GL_LINEAR_MIPMAP_LINEAR,
                       uint32 magFilter = GL_LINEAR,
                       const float32* rgbaBorderColor = nullptr);
    void DeleteTexture(std::string name);
    bool UseTexture   (std::string name);

    void OnWindowResized(IEventDataPtr e);
};

#endif // VIEW_OPENGL_HPP
