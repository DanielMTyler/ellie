/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef VIEW_OPENGL_HPP
#define VIEW_OPENGL_HPP

#include "global.hpp"
#include <SDL.h>
#include <glad/glad.h>
#include <KHR/khrplatform.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
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
    typedef uint32 Texture;

    const uint32 MINIMUM_OPENGL_MAJOR = 3;
    const uint32 MINIMUM_OPENGL_MINOR = 3;

    const bool ENABLE_VYSNC = true;
    const bool ADAPTIVE_VSYNC = true; // Classic or Adaptive VSync?

    const bool   MULTISAMPLING = true;
    const uint32 MULTISAMPLING_NUMSAMPLES = 4; // 2 or 4.

    const float32 m_cameraSpeed = 0.01f;
    const float32 m_cameraSensitivityYaw   = 0.1f;
    const float32 m_cameraSensitivityPitch = 0.1f;
    const float32 m_cameraZoomMin = 1.0f;
    const float32 m_cameraZoomMax = 45.0f;
    const float32 m_cameraZoomStep = 1.0f;
    const bool m_cameraInvertedYaw   = false;
    const bool m_cameraInvertedPitch = false;

    IApp* m_app = nullptr;
    std::string m_shaderPath;
    std::string m_texturePath;

    SDL_Window* m_window = nullptr;
    SDL_GLContext m_glContext = nullptr;

    uint64 m_fpsLastTime = 0;
    uint32 m_fpsCounter = 0;

    uint32 m_windowWidth  = 800;
    uint32 m_windowHeight = 600;

    float32 m_cameraZoom = m_cameraZoomMax;
    float32 m_nearPlane = 0.1f;
    float32 m_farPlane  = 100.0f;

    glm::vec3 m_cameraPosition = glm::vec3(0.0f, 0.0f,  3.0f);
    glm::vec3 m_cameraFront;
    glm::vec3 m_cameraUp;
    glm::vec3 m_cameraRight;
    glm::vec3 m_cameraWorldUp  = glm::vec3(0.0f, 1.0f,  0.0f);
    float32 m_cameraYaw   = -90.0f;
    float32 m_cameraPitch =   0.0f;

    void UpdateCamera();

    bool InitWindowAndGLContext_();
    bool InitGLFunctions_();
    void InitLogGraphicsInfo_();

    std::map<std::string, Shader> m_shaders;
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
};

#endif // VIEW_OPENGL_HPP
