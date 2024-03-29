/*
    ==================================
    Copyright (C) 2021 Daniel Tyler.
      This file is part of Ellie.
    ==================================
*/

#include "view_opengl.hpp"
#include "app.hpp"
#include "events.hpp"
#include "logic.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <memory> // make_shared

global_variable uint32 g_cubeVBO  = 0;
global_variable uint32 g_cubeVAO  = 0;
global_variable uint32 g_lightVAO = 0;
global_variable uint32 g_cubeEBO  = 0;
global_variable glm::vec3 g_lightPos(1.2f, 1.0f, 2.0f);

bool ViewOpenGL::Init()
{
    m_app   = &App::Get();
    m_logic = m_app->Logic();

    if (SDL_GL_LoadLibrary(nullptr))
    {
        LogFatal("Failed to load OpenGL library: %s.", SDL_GetError());
        return false;
    }
    LogInfo("Loaded OpenGL library.");

    if (!InitWindowAndGLContext_())
        return false;
    // @warning Requires active OpenGL Context.
    if (!InitGLFunctions_())
        return false;

    InitLogGraphicsInfo_();

    stbi_set_flip_vertically_on_load(true);

    if (SDL_SetRelativeMouseMode(SDL_TRUE))
    {
        LogFatal("Failed to set SDL relative mouse mode: %s.", SDL_GetError());
        return false;
    }

    glViewport(0, 0, m_app->m_options.graphics.windowWidth, m_app->m_options.graphics.windowHeight);
    glEnable(GL_DEPTH_TEST);

    float32 cubeVertices[] = {
        // back
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,   // 0
         0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,   // 1
         0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f,   // 2
         //0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, // 2
        -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f,   // 3
        //-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, // 0

        // front
        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,   // 4
         0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,   // 5
         0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,   // 6
         //0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, // 6
        -0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,   // 7
        //-0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, // 4

        // left
        -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f,   // 8
        -0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f,   // 9
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,   // 10
        //-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, // 10
        //-0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, // 4
        //-0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, // 8

        // right
         0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f,   // 11
         //0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, // 2
         0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,   // 12
         //0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, // 12
         0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f,   // 13
         //0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, // 11

        // bottom
        //-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, // 10
         0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,   // 14
         //0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, // 5
         //0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, // 5
        //-0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, // 4
        //-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, // 10

        // top
        //-0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // 3
         //0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // 2
         //0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // 11
         //0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // 11
        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f    // 15
        //-0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // 3
    };
    uint32 cubeIndices[] = {
        // back
        0, 1, 2,
        2, 3, 0,

        // front
        4, 5, 6,
        6, 7, 4,

        // left
        8, 9, 10,
        10, 4, 8,

        // right
        11, 2, 12,
        12, 13, 11,

        // bottom
        10, 14, 5,
        5, 4, 10,

        // top
        3, 2, 11,
        11, 15, 3
    };

    glGenVertexArrays(1, &g_cubeVAO);
    glGenVertexArrays(1, &g_lightVAO);
    glGenBuffers(1, &g_cubeVBO);
    glGenBuffers(1, &g_cubeEBO);

    glBindVertexArray(g_cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, g_cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_cubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);
    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float32), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float32), (void*)(3 * sizeof(float32)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(g_lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, g_cubeVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_cubeEBO);
    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float32), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float32), (void*)(3 * sizeof(float32)));
    glEnableVertexAttribArray(1);

    if (!CreateShader("default", "default", "default"))
        return false;

    m_fpsLastTime = App::Time();

    return true;
}

void ViewOpenGL::Cleanup()
{
    m_processes.AbortAll(true);

    if (g_cubeEBO)
    {
        glDeleteBuffers(1, &g_cubeEBO);
        g_cubeEBO = 0;
    }

    if (g_cubeVBO)
    {
        glDeleteBuffers(1, &g_cubeVBO);
        g_cubeVBO = 0;
    }

    if (g_lightVAO)
    {
        glDeleteVertexArrays(1, &g_lightVAO);
        g_lightVAO = 0;
    }

    if (g_cubeVAO)
    {
        glDeleteVertexArrays(1, &g_cubeVAO);
        g_cubeVAO = 0;
    }

    if (!m_textures.empty())
    {
        for (auto it = m_textures.begin(); it != m_textures.end(); it++)
            glDeleteTextures(1, &it->second);

        m_textures.clear();
    }

    if (!m_shaders.empty())
    {
        for (auto it = m_shaders.begin(); it != m_shaders.end(); it++)
            glDeleteProgram(it->second);

        m_shaders.clear();
    }

    if (m_glContext)
    {
        SDL_GL_DeleteContext(m_glContext);
        m_glContext = nullptr;
    }

    if (m_window)
    {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
}

bool ViewOpenGL::ProcessEvents(DeltaTime dt)
{
    // @todo Deal with being minimized, toggling fullscreen, etc.

    SDL_Event e;
    while (SDL_PollEvent(&e) != 0)
    {
        if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE))
        {
            return false;
        }
        else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED)
        {
            // @note SDL_WINDOWEVENT_RESIZED only fires if the window size
            //       changed due to an external event, i.e., not an SDL call;
            //       Also, initial window creation doesn't cause this either.
            m_app->m_options.graphics.windowWidth  = e.window.data1;
            m_app->m_options.graphics.windowHeight = e.window.data2;
            glViewport(0, 0, m_app->m_options.graphics.windowWidth, m_app->m_options.graphics.windowHeight);
            LogInfo("Window resized to %ux%u; viewport set.", m_app->m_options.graphics.windowWidth, m_app->m_options.graphics.windowHeight);
        }
        else if (e.type == SDL_KEYDOWN)
        {
            if (e.key.keysym.scancode == SDL_SCANCODE_T)
            {
                static bool wireframe = false;
                wireframe = !wireframe;

                if (wireframe)
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                else
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

                LogInfo("Wireframe: %s.", OnOffBoolToStr(wireframe));
            }
        }
        else if (e.type == SDL_MOUSEMOTION)
        {
            m_app->Events()->Publish(std::make_shared<EventRotateCamera>(e.motion.xrel, e.motion.yrel));
        }
        else if (e.type == SDL_MOUSEWHEEL)
        {
            // scroll up == zoom in
            bool in = (e.wheel.y > 0 ? true : false);
            if (e.wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
                in = -in;
            m_app->Events()->Publish(std::make_shared<EventZoomCamera>(in));
        }
    }

    bool moveCameraForward  = false;
    bool moveCameraBackward = false;
    bool moveCameraLeft     = false;
    bool moveCameraRight    = false;
    const uint8* kbState = SDL_GetKeyboardState(nullptr);
    if (kbState[SDL_SCANCODE_W])
        moveCameraForward = true;
    if (kbState[SDL_SCANCODE_S])
        moveCameraBackward = true;
    if (kbState[SDL_SCANCODE_A])
        moveCameraLeft = true;
    if (kbState[SDL_SCANCODE_D])
        moveCameraRight = true;

    if (moveCameraForward || moveCameraBackward || moveCameraLeft || moveCameraRight)
        m_app->Events()->Publish(std::make_shared<EventMoveCamera>(dt, moveCameraForward, moveCameraBackward, moveCameraLeft, moveCameraRight));

    if (kbState[SDL_SCANCODE_I])
        g_lightPos += glm::vec3(0.0f, 0.0f, -1.0f) * m_app->m_options.camera.speed * dt;
    else if (kbState[SDL_SCANCODE_K])
        g_lightPos -= glm::vec3(0.0f, 0.0f, -1.0f) * m_app->m_options.camera.speed * dt;
    if (kbState[SDL_SCANCODE_J])
        g_lightPos -= glm::vec3(1.0f, 0.0f, 0.0f) * m_app->m_options.camera.speed * dt;
    else if (kbState[SDL_SCANCODE_L])
        g_lightPos += glm::vec3(1.0f, 0.0f, 0.0f) * m_app->m_options.camera.speed * dt;
    if (kbState[SDL_SCANCODE_U])
        g_lightPos -= glm::vec3(0.0f, 1.0f, 0.0f) * m_app->m_options.camera.speed * dt;
    else if (kbState[SDL_SCANCODE_O])
        g_lightPos += glm::vec3(0.0f, 1.0f, 0.0f) * m_app->m_options.camera.speed * dt;

    return true;
}

bool ViewOpenGL::Render(DeltaTime dt)
{
    if (App::SecondsElapsed(m_fpsLastTime) >= 1.0f)
    {
        LogDebug("FPS: %u, DT: %f.", m_fpsCounter, dt);
        m_fpsCounter = 0;
        m_fpsLastTime = App::Time();
    }

    m_processes.Update(dt);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static glm::mat4 identity = glm::mat4(1.0f);
    glm::mat4 view = identity;
    view = glm::lookAt(m_app->m_options.camera.position, m_app->m_options.camera.position + m_app->m_options.camera.front, m_app->m_options.camera.up);
    glm::mat4 projection = glm::perspective(glm::radians(m_app->m_options.camera.fov), (float32)m_app->m_options.graphics.windowWidth/(float32)m_app->m_options.graphics.windowHeight, m_app->m_options.graphics.planeNear, m_app->m_options.graphics.planeFar);
    glm::mat4 model = identity;

    if (!UseShader("default"))
        return false;
    if (!ShaderSetMat4("default", "view", view))
        return false;
    if (!ShaderSetMat4("default", "projection", projection))
        return false;
    if (!ShaderSetMat4("default", "model", model))
        return false;
    if (!ShaderSetVec3f("default", "objectColor", 1.0f, 0.5f, 0.31f))
        return false;
    if (!ShaderSetVec3f("default", "lightColor", 1.0f, 1.0f, 1.0f))
        return false;
    if (!ShaderSetBool("default", "isLightSource", false))
        return false;
    if (!ShaderSetVec3("default", "lightPos", g_lightPos))
        return false;
    if (!ShaderSetVec3("default", "viewPos", m_app->m_options.camera.position))
        return false;

    glBindVertexArray(g_cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);

    model = identity;
    model = glm::translate(model, g_lightPos);
    model = glm::scale(model, glm::vec3(0.2f));
    if (!ShaderSetMat4("default", "model", model))
        return false;
    if (!ShaderSetBool("default", "isLightSource", true))
        return false;
    glBindVertexArray(g_lightVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);

    //glBindVertexArray(0);
    SDL_GL_SwapWindow(m_window);
    SDL_Delay(1);

    m_fpsCounter++;

    return true;
}

bool ViewOpenGL::CreateShader(std::string name, std::string vertex, std::string fragment)
{
    if (name.empty())
    {
        LogFatal("Tried to create shader with no name.");
        return false;
    }

    LogInfo("Creating shader: %s.", name.c_str());

    if (vertex.empty() && fragment.empty())
    {
        LogFatal("No vertex or fragment shader provided.");
        return false;
    }

    auto exists = m_shaders.find(name);
    if (exists != m_shaders.end())
    {
        LogFatal("Shader already exists.");
        return false;
    }

    Shader s = glCreateProgram();
    Shader v;
    Shader f;

    if (!LoadShader_(vertex, true, v))
    {
        glDeleteShader(v);
        glDeleteProgram(s);
        return false;
    }
    glAttachShader(s, v);

    if (!LoadShader_(fragment, false, f))
    {
        glDeleteShader(f);
        glDeleteShader(v);
        glDeleteProgram(s);
        return false;
    }
    glAttachShader(s, f);

    glLinkProgram(s);

    int success;
    const uint32 infoLogSize = KIBIBYTES(1);
    char infoLog[infoLogSize];
    glGetProgramiv(s, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(s, infoLogSize, nullptr, infoLog);
        LogFatal("Failed to link shader: %s.", infoLog);

        glDeleteShader(f);
        glDeleteShader(v);
        glDeleteProgram(s);
        return false;
    }

    glDeleteShader(f);
    glDeleteShader(v);
    m_shaders[name] = s;
    return true;
}

void ViewOpenGL::DeleteShader(std::string name)
{
    LogInfo("Deleting shader if exists: %s.", name.c_str());

    auto s = m_shaders.find(name);
    if (s != m_shaders.end())
    {
        glDeleteProgram(s->second);
        m_shaders.erase(name);
    }
}

bool ViewOpenGL::UseShader(std::string name)
{
    if (name.empty())
    {
        LogFatal("Tried to use shader with no name.");
        return false;
    }

    auto s = m_shaders.find(name);
    if (s == m_shaders.end())
    {
        LogFatal("Tried to use non-existant shader: %s.", name.c_str());
        return false;
    }

    glUseProgram(s->second);
    return true;
}

bool ViewOpenGL::ShaderSetBool(std::string shader, std::string name, bool value) const
{
    return ShaderSetInt(shader, name, (int)value);
}

bool ViewOpenGL::ShaderSetInt(std::string shader, std::string name, int value) const
{
    if (shader.empty())
    {
        LogFatal("Tried to set shader int with no shader name.");
        return false;
    }
    if (name.empty())
    {
        LogFatal("Tried to set unnamed shader (%s) int.", shader.c_str());
        return false;
    }

    auto s = m_shaders.find(shader);
    if (s == m_shaders.end())
    {
        LogFatal("Tried to set shader (%s) int (%s), but shader doesn't exist.", shader.c_str(), name.c_str());
        return false;
    }

    glUniform1i(glGetUniformLocation(s->second, name.c_str()), value);
    return true;
}

bool ViewOpenGL::ShaderSetFloat(std::string shader, std::string name, float32 value) const
{
    if (shader.empty())
    {
        LogFatal("Tried to set shader float with no shader name.");
        return false;
    }
    if (name.empty())
    {
        LogFatal("Tried to set unnamed shader (%s) float.", shader.c_str());
        return false;
    }

    auto s = m_shaders.find(shader);
    if (s == m_shaders.end())
    {
        LogFatal("Tried to set shader (%s) float (%s), but shader doesn't exist.", shader.c_str(), name.c_str());
        return false;
    }

    glUniform1f(glGetUniformLocation(s->second, name.c_str()), value);
    return true;
}

bool ViewOpenGL::ShaderSetVec2f(std::string shader, std::string name, float32 x, float32 y) const
{
    if (shader.empty())
    {
        LogFatal("Tried to set shader Vec2f with no shader name.");
        return false;
    }
    if (name.empty())
    {
        LogFatal("Tried to set unnamed shader (%s) Vec2f.", shader.c_str());
        return false;
    }

    auto s = m_shaders.find(shader);
    if (s == m_shaders.end())
    {
        LogFatal("Tried to set shader (%s) Vec2f (%s), but shader doesn't exist.", shader.c_str(), name.c_str());
        return false;
    }

    glUniform2f(glGetUniformLocation(s->second, name.c_str()), x, y);
    return true;
}

bool ViewOpenGL::ShaderSetVec3f(std::string shader, std::string name, float32 x, float32 y, float32 z) const
{
    if (shader.empty())
    {
        LogFatal("Tried to set shader Vec3f with no shader name.");
        return false;
    }
    if (name.empty())
    {
        LogFatal("Tried to set unnamed shader (%s) Vec3f.", shader.c_str());
        return false;
    }

    auto s = m_shaders.find(shader);
    if (s == m_shaders.end())
    {
        LogFatal("Tried to set shader (%s) Vec3f (%s), but shader doesn't exist.", shader.c_str(), name.c_str());
        return false;
    }

    glUniform3f(glGetUniformLocation(s->second, name.c_str()), x, y, z);
    return true;
}

bool ViewOpenGL::ShaderSetVec4f(std::string shader, std::string name, float32 x, float32 y, float32 z, float32 w) const
{
    if (shader.empty())
    {
        LogFatal("Tried to set shader Vec4f with no shader name.");
        return false;
    }
    if (name.empty())
    {
        LogFatal("Tried to set unnamed shader (%s) Vec4f.", shader.c_str());
        return false;
    }

    auto s = m_shaders.find(shader);
    if (s == m_shaders.end())
    {
        LogFatal("Tried to set shader (%s) Vec4f (%s), but shader doesn't exist.", shader.c_str(), name.c_str());
        return false;
    }

    glUniform4f(glGetUniformLocation(s->second, name.c_str()), x, y, z, w);
    return true;
}

bool ViewOpenGL::ShaderSetVec2(std::string shader, std::string name, glm::vec2 v) const
{
    if (shader.empty())
    {
        LogFatal("Tried to set shader glm::vec2 with no shader name.");
        return false;
    }
    if (name.empty())
    {
        LogFatal("Tried to set unnamed shader (%s) glm::vec2.", shader.c_str());
        return false;
    }

    auto s = m_shaders.find(shader);
    if (s == m_shaders.end())
    {
        LogFatal("Tried to set shader (%s) glm::vec2 (%s), but shader doesn't exist.", shader.c_str(), name.c_str());
        return false;
    }

    glUniform2fv(glGetUniformLocation(s->second, name.c_str()), 1, glm::value_ptr(v));
    return true;
}

bool ViewOpenGL::ShaderSetVec3(std::string shader, std::string name, glm::vec3 v) const
{
    if (shader.empty())
    {
        LogFatal("Tried to set shader glm::vec3 with no shader name.");
        return false;
    }
    if (name.empty())
    {
        LogFatal("Tried to set unnamed shader (%s) glm::vec3.", shader.c_str());
        return false;
    }

    auto s = m_shaders.find(shader);
    if (s == m_shaders.end())
    {
        LogFatal("Tried to set shader (%s) glm::vec3 (%s), but shader doesn't exist.", shader.c_str(), name.c_str());
        return false;
    }

    glUniform3fv(glGetUniformLocation(s->second, name.c_str()), 1, glm::value_ptr(v));
    return true;
}

bool ViewOpenGL::ShaderSetVec4(std::string shader, std::string name, glm::vec4 v) const
{
    if (shader.empty())
    {
        LogFatal("Tried to set shader glm::vec4 with no shader name.");
        return false;
    }
    if (name.empty())
    {
        LogFatal("Tried to set unnamed shader (%s) glm::vec4.", shader.c_str());
        return false;
    }

    auto s = m_shaders.find(shader);
    if (s == m_shaders.end())
    {
        LogFatal("Tried to set shader (%s) glm::vec4 (%s), but shader doesn't exist.", shader.c_str(), name.c_str());
        return false;
    }

    glUniform4fv(glGetUniformLocation(s->second, name.c_str()), 1, glm::value_ptr(v));
    return true;
}

bool ViewOpenGL::ShaderSetMat4(std::string shader, std::string name, glm::mat4 m) const
{
    if (shader.empty())
    {
        LogFatal("Tried to set shader glm::mat4 with no shader name.");
        return false;
    }
    if (name.empty())
    {
        LogFatal("Tried to set unnamed shader (%s) glm::mat4.", shader.c_str());
        return false;
    }

    auto s = m_shaders.find(shader);
    if (s == m_shaders.end())
    {
        LogFatal("Tried to set shader (%s) glm::mat4 (%s), but shader doesn't exist.", shader.c_str(), name.c_str());
        return false;
    }

    glUniformMatrix4fv(glGetUniformLocation(s->second, name.c_str()), 1, GL_FALSE, glm::value_ptr(m));
    return true;
}

bool ViewOpenGL::CreateTexture(std::string name,
                               bool hasAlpha,
                               uint32 wrapS,
                               uint32 wrapT,
                               uint32 minFilter,
                               uint32 magFilter,
                               const float32* rgbaBorderColor)
{
    if (name.empty())
    {
        LogFatal("Tried to create texture with no filename.");
        return false;
    }

    LogInfo("Creating texture from image: %s.", name.c_str());

    auto exists = m_textures.find(name);
    if (exists != m_textures.end())
    {
        LogFatal("Texture already exists.");
        return false;
    }

    int width;
    int height;
    int numChannels;
    unsigned char* data = stbi_load((m_app->m_options.core.texturePath + name).c_str(), &width, &height, &numChannels, 0);
    if (!data)
    {
        LogFatal("Failed to load image: %s.", stbi_failure_reason());
        return false;
    }

    Texture texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    if (rgbaBorderColor)
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, rgbaBorderColor);

    int internalFormat = GL_RGB;
    if (hasAlpha)
        internalFormat = GL_RGBA;
    int imageFormat = internalFormat;
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, imageFormat, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    data = nullptr;

    m_textures[name] = texture;
    return true;
}

void ViewOpenGL::DeleteTexture(std::string name)
{
    LogInfo("Deleting texture if exists: %s.", name.c_str());

    auto s = m_textures.find(name);
    if (s != m_textures.end())
    {
        glDeleteTextures(1, &s->second);
        m_textures.erase(name);
    }
}

bool ViewOpenGL::UseTexture(std::string name)
{
    if (name.empty())
    {
        LogFatal("Tried to use texture with no name.");
        return false;
    }

    auto s = m_textures.find(name);
    if (s == m_textures.end())
    {
        LogFatal("Tried to use non-existant texture: %s.", name.c_str());
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, s->second);
    return true;
}

bool ViewOpenGL::InitWindowAndGLContext_()
{
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, MINIMUM_OPENGL_MAJOR))
    {
        LogFatal("Failed to request OpenGL major version: %s.", SDL_GetError());
        return false;
    }
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, MINIMUM_OPENGL_MINOR))
    {
        LogFatal("Failed to request OpenGL minor version: %s.", SDL_GetError());
        return false;
    }
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE))
    {
        LogFatal("Failed to request OpenGL core profile: %s.", SDL_GetError());
        return false;
    }
    LogInfo("OpenGL requested: v%i.%i Core Profile.", MINIMUM_OPENGL_MAJOR, MINIMUM_OPENGL_MINOR);

    if (SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1))
    {
        LogFatal("Failed to request OpenGL doublebuffering: %s.", SDL_GetError());
        return false;
    }
    LogInfo("Double Buffering requested.");

    if (m_app->m_options.graphics.multisampling)
    {
        if (SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1))
        {
            LogFatal("Failed to request OpenGL multisampling: %s.", SDL_GetError());
            return false;
        }
        if (SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, m_app->m_options.graphics.multisamplingNumSamples))
        {
            LogFatal("Failed to request OpenGL multisampling num samples: %s.", SDL_GetError());
            return false;
        }
        LogInfo("Multisampling requested: On with %i samples.", m_app->m_options.graphics.multisamplingNumSamples);
    }
    else
    {
        LogInfo("Multisampling requested: No.");
    }

    m_window = SDL_CreateWindow(APPLICATION_NAME,
                                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                m_app->m_options.graphics.windowWidth, m_app->m_options.graphics.windowHeight,
                                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!m_window)
    {
        LogFatal("Failed to create window: %s.", SDL_GetError());
        return false;
    }
    LogInfo("Created window.");

    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext)
    {
        LogFatal("Failed to create OpenGL Context: %s.", SDL_GetError());
        return false;
    }
    LogInfo("Created OpenGL Context.");
    if (SDL_GL_MakeCurrent(m_window, m_glContext))
    {
        LogFatal("Failed to make OpenGL Context current: %s.", SDL_GetError());
        return false;
    }
    LogInfo("Made OpenGL Context current.");

    int glMajor;
    if (SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &glMajor))
    {
        LogFatal("Failed to retrieve actual OpenGL major version: %s.", SDL_GetError());
        return false;
    }
    int glMinor;
    if (SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &glMinor))
    {
        LogFatal("Failed to retrieve actual OpenGL minor version: %s.", SDL_GetError());
        return false;
    }
    int glProfile;
    if (SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &glProfile))
    {
        LogFatal("Failed to retrieve actual OpenGL profile: %s.", SDL_GetError());
        return false;
    }
    std::string glProfileStr = "Unknown";
    if (glProfile == SDL_GL_CONTEXT_PROFILE_CORE)
        glProfileStr = "Core";
    else if (glProfile == SDL_GL_CONTEXT_PROFILE_COMPATIBILITY)
        glProfileStr = "Compatibility";
    else if (glProfile == SDL_GL_CONTEXT_PROFILE_ES)
        glProfileStr = "ES";
    LogInfo("OpenGL: v%i.%i %s Profile.", glMajor, glMinor, glProfileStr.c_str());

    int doublebuffering;
    if (SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &doublebuffering))
    {
        LogFatal("Failed to retrieve actual OpenGL doublebuffering: %s.", SDL_GetError());
        return false;
    }
    LogInfo("Double Buffering: %s.", OnOffBoolToStr(doublebuffering));

    int multisampling;
    if (SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &multisampling))
    {
        LogFatal("Failed to retrieve actual OpenGL multisampling: %s.", SDL_GetError());
        return false;
    }
    int multisampling_numSamples;
    if (SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &multisampling_numSamples))
    {
        LogFatal("Failed to retrieve actual OpenGL multisampling num samples: %s.", SDL_GetError());
        return false;
    }
    LogInfo("Multisampling: %s with %i samples.", OnOffBoolToStr(multisampling), multisampling_numSamples);

    if (m_app->m_options.graphics.vsync)
    {
        if (m_app->m_options.graphics.vsyncAdaptive)
        {
            if (SDL_GL_SetSwapInterval(-1))
            {
                LogWarning("Failed to set Adaptive VSync: %s.", SDL_GetError());
                if (SDL_GL_SetSwapInterval(1))
                    LogWarning("Failed to set Standard VSync: %s.", SDL_GetError());
                else
                    LogInfo("Set VSync: On.");
            }
            else
            {
                LogInfo("Set VSync: Adaptive.");
            }
        }
        else
        {
            if (SDL_GL_SetSwapInterval(1))
                LogWarning("Failed to set Standard VSync: %s.", SDL_GetError());
            else
                LogInfo("Set VSync: On.");
        }
    }
    else
    {
        if (SDL_GL_SetSwapInterval(0))
            LogWarning("Failed to set VSync off: %s.", SDL_GetError());
        LogInfo("Set VSync: Off.");
    }

    int vsync = SDL_GL_GetSwapInterval();
    if (vsync == 0)
        LogInfo("VSync: Off.");
    else if (vsync == 1)
        LogInfo("VSync: On.");
    else
        LogInfo("VSync: Adaptive.");

    return true;
}

bool ViewOpenGL::InitGLFunctions_()
{
    // @warning This requires an OpenGL Context.
    if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
    {
        LogFatal("Failed to load OpenGL functions.");
        return false;
    }

    if (GLVersion.major < (int)MINIMUM_OPENGL_MAJOR ||
        (GLVersion.major == (int)MINIMUM_OPENGL_MAJOR && GLVersion.minor < (int)MINIMUM_OPENGL_MINOR))
    {
        LogFatal("Loaded OpenGL v%i.%i functions, but need v%i.%i+.", GLVersion.major, GLVersion.minor,
                 MINIMUM_OPENGL_MAJOR, MINIMUM_OPENGL_MINOR);
        return false;
    }
    else
    {
        LogInfo("Loaded OpenGL v%i.%i functions.", GLVersion.major, GLVersion.minor);
        return true;
    }
}

void ViewOpenGL::InitLogGraphicsInfo_()
{
    int v;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &v);
    LogInfo("OpenGL max vertex attributes supported: %i.", v);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &v);
    LogInfo("OpenGL max 1D/2D texture size: %ix%i.", v, v);
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &v);
    LogInfo("OpenGL max 3D texture size: %ix%ix%i.", v, v, v);
    glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &v);
    LogInfo("OpenGL max cube map texture size: %ix%ix%i.", v, v, v);
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &v);
    LogInfo("OpenGL max fragment texture image units: %i.", v);
    glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &v);
    LogInfo("OpenGL max vertex texture image units: %i.", v);
    glGetIntegerv(GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS, &v);
    LogInfo("OpenGL max geometry texture image units: %i.", v);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &v);
    LogInfo("OpenGL max combined texture image units: %i.", v);

    // @todo Log more, like VRAM usage.
    // @todo Certain things (like VRAM usage) should queryable for real-time display.
}

bool ViewOpenGL::LoadShader_(std::string name, bool vertex, Shader& shader)
{
    if (name.empty())
    {
        LogFatal("Tried to load %s shader with no name.", (vertex ? "vertex" : "fragment"));
        return false;
    }

    LogInfo("Loading %s shader: %s.", (vertex ? "vertex" : "fragment"), name.c_str());

    std::string file = m_app->m_options.core.shaderPath + name + (vertex ? ".vert" : ".frag");
    std::string shaderStr;
    if (!m_app->LoadFile(file, shaderStr))
    {
        LogFatal("Failed to load shader file.");
        return false;
    }

    if (vertex)
        shader = glCreateShader(GL_VERTEX_SHADER);
    else
        shader = glCreateShader(GL_FRAGMENT_SHADER);

    const char* shaderCStr = shaderStr.c_str();
    glShaderSource(shader, 1, &shaderCStr, nullptr);
    glCompileShader(shader);

    int success;
    const uint32 infoLogSize = KIBIBYTES(1);
    char infoLog[infoLogSize];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, infoLogSize, nullptr, infoLog);
        LogFatal("Failed to compile shader: %s.", infoLog);
        glDeleteShader(shader);
        shader = 0;
        return false;
    }

    return true;
}
