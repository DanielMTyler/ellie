/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#include "view.hpp"
#include "SDL.h"
#include "../thirdparty/glad/include/glad/glad.h"
#include "../thirdparty/glad/include/KHR/khrplatform.h"
#include "app.hpp"

global_variable uint32 g_vbo = 0;
global_variable uint32 g_vao = 0;
global_variable uint32 g_ebo = 0;

bool View::Init()
{
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

    // @todo Log Graphics Information.

    int glNumVertexAttribs;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &glNumVertexAttribs);
    LogInfo("OpenGL maximum number of vertex attributes supported: %i.", glNumVertexAttribs);

    m_shaderPath = App::Get().DataPath() + "shaders" + PATH_SEPARATOR;
    m_fpsLastTime = SDL_GetPerformanceCounter();

    glViewport(0, 0, DESIRED_WINDOW_WIDTH, DESIRED_WINDOW_HEIGHT);

    if (!CreateShader("default", "default", "default"))
        return false;

    float32 vertices[] = {
         0.5f,  0.5f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left
    };
    uint32 indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };

    glGenVertexArrays(1, &g_vao);
    glBindVertexArray(g_vao);

    glGenBuffers(1, &g_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &g_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float32), nullptr);
    glEnableVertexAttribArray(0);
    return true;
}

void View::Cleanup()
{
    if (g_ebo)
    {
        glDeleteBuffers(1, &g_ebo);
        g_ebo = 0;
    }

    if (g_vao)
    {
        glDeleteVertexArrays(1, &g_vao);
        g_vao = 0;
    }

    if (g_vbo)
    {
        glDeleteBuffers(1, &g_vbo);
        g_vbo = 0;
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

bool View::Update(DeltaTime dt)
{
    // @todo Deal with being minimized, toggling fullscreen, etc.

    bool quit = false;
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0)
    {
        if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE))
        {
            quit = true;
            LogInfo("User requested quit.");
        }
        else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED)
        {
            // @note SDL_WINDOWEVENT_RESIZED only fires if the window size
            //       changed due to an external event, i.e., not an SDL call;
            //       Also, initial window creation doesn't cause this either.
            uint32 w = e.window.data1;
            uint32 h = e.window.data2;
            glViewport(0, 0, w, h);
            LogInfo("Window resized to %ux%u; viewport set.", w, h);
        }
        else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_w)
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

    if (((DeltaTime)(SDL_GetPerformanceCounter() - m_fpsLastTime) / (DeltaTime)SDL_GetPerformanceFrequency()) >= 1.0f)
    {
        LogDebug("FPS: %u, DT: %f.", m_fpsCounter, dt);
        m_fpsCounter = 0;
        m_fpsLastTime = SDL_GetPerformanceCounter();
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (!UseShader("default"))
        return false;

    glBindVertexArray(g_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    SDL_GL_SwapWindow(m_window);
    SDL_Delay(1);

    m_fpsCounter++;

    return !quit;
}

bool View::InitWindowAndGLContext_()
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

    if (MULTISAMPLING)
    {
        if (SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1))
        {
            LogFatal("Failed to request OpenGL multisampling: %s.", SDL_GetError());
            return false;
        }
        if (SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, MULTISAMPLING_NUMSAMPLES))
        {
            LogFatal("Failed to request OpenGL multisampling num samples: %s.", SDL_GetError());
            return false;
        }
        LogInfo("Multisampling requested: On with %i samples.", MULTISAMPLING_NUMSAMPLES);
    }
    else
    {
        LogInfo("Multisampling requested: No.");
    }

    m_window = SDL_CreateWindow(App::Get().APPLICATION_NAME,
                                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                DESIRED_WINDOW_WIDTH, DESIRED_WINDOW_HEIGHT,
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

    // @note Sometimes SDL_GL_SetSwapInterval isn't allowed, but SDL_GL_GetSwapInterval works fine.
    if (ENABLE_VYSNC)
    {
        if (ADAPTIVE_VSYNC)
        {
            if (!SDL_GL_SetSwapInterval(-1))
            {
                LogWarning("Failed to set Adaptive VSync: %s.", SDL_GetError());
                if (!SDL_GL_SetSwapInterval(1))
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
            if (!SDL_GL_SetSwapInterval(1))
                LogWarning("Failed to set Standard VSync: %s.", SDL_GetError());
            else
                LogInfo("Set VSync: On.");
        }
    }
    else
    {
        if (!SDL_GL_SetSwapInterval(0))
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

bool View::InitGLFunctions_()
{
    // @warning This requires an OpenGL Context.
    if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
    {
        LogFatal("Failed to load OpenGL functions.");
        return false;
    }
    LogInfo("Loaded OpenGL functions.");

    if (GLVersion.major < (int)MINIMUM_OPENGL_MAJOR ||
        (GLVersion.major == (int)MINIMUM_OPENGL_MAJOR && GLVersion.minor < (int)MINIMUM_OPENGL_MINOR))
    {
        LogFatal("Loaded OpenGL v%i.%i functions, but need v%i.%i+.", GLVersion.major, GLVersion.minor,
                 MINIMUM_OPENGL_MAJOR, MINIMUM_OPENGL_MINOR);
        return false;
    }

    return true;
}

bool View::CreateShader(std::string name, std::string vertex, std::string fragment)
{
    LogInfo("Creating shader: %s.", name.c_str());

    if (name.empty())
    {
            LogFatal("Shader name is empty.");
            return false;
    }
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

void View::DeleteShader(std::string name)
{
    LogInfo("Deleting shader if exists: %s.", name.c_str());

    auto s = m_shaders.find(name);
    if (s != m_shaders.end())
    {
        glDeleteProgram(s->second);
        m_shaders.erase(name);
    }
}

bool View::UseShader(std::string name)
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

bool View::ShaderSetBool(std::string shader, std::string name, bool value) const
{
    return ShaderSetInt(shader, name, (int)value);
}

bool View::ShaderSetInt(std::string shader, std::string name, int value) const
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

    auto s = m_shaders.find(name);
    if (s == m_shaders.end())
    {
        LogFatal("Tried to set shader (%s) int (%s), but shader doesn't exist.", shader.c_str(), name.c_str());
        return false;
    }

    glUniform1i(glGetUniformLocation(s->second, name.c_str()), value);
    return true;
}

bool View::ShaderSetFloat(std::string shader, std::string name, float32 value) const
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

    auto s = m_shaders.find(name);
    if (s == m_shaders.end())
    {
        LogFatal("Tried to set shader (%s) float (%s), but shader doesn't exist.", shader.c_str(), name.c_str());
        return false;
    }

    glUniform1f(glGetUniformLocation(s->second, name.c_str()), value);
    return true;
}

bool View::ShaderSetVec2f(std::string shader, std::string name, float32 x, float32 y) const
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

    auto s = m_shaders.find(name);
    if (s == m_shaders.end())
    {
        LogFatal("Tried to set shader (%s) Vec2f (%s), but shader doesn't exist.", shader.c_str(), name.c_str());
        return false;
    }

    glUniform2f(glGetUniformLocation(s->second, name.c_str()), x, y);
    return true;
}

bool View::ShaderSetVec3f(std::string shader, std::string name, float32 x, float32 y, float32 z) const
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

    auto s = m_shaders.find(name);
    if (s == m_shaders.end())
    {
        LogFatal("Tried to set shader (%s) Vec3f (%s), but shader doesn't exist.", shader.c_str(), name.c_str());
        return false;
    }

    glUniform3f(glGetUniformLocation(s->second, name.c_str()), x, y, z);
    return true;
}

bool View::ShaderSetVec4f(std::string shader, std::string name, float32 x, float32 y, float32 z, float32 w) const
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

    auto s = m_shaders.find(name);
    if (s == m_shaders.end())
    {
        LogFatal("Tried to set shader (%s) Vec4f (%s), but shader doesn't exist.", shader.c_str(), name.c_str());
        return false;
    }

    glUniform4f(glGetUniformLocation(s->second, name.c_str()), x, y, z, w);
    return true;
}

bool View::LoadShader_(std::string name, bool vertex, uint32& shader)
{
    LogInfo("Loading %s shader: %s.", (vertex ? "vertex" : "fragment"), name.c_str());

    if (name.empty())
    {
        LogFatal("No name given.");
        return false;
    }

    std::string file = m_shaderPath + name + (vertex ? ".vert" : ".frag");
    std::string shaderStr;
    if (!App::Get().LoadFile(file, shaderStr))
    {
        LogFatal("Failed to load file.");
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
