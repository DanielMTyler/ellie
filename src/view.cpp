/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#include "view.hpp"
#include "SDL.h"
#include "SDL_syswm.h"
#include <cstdio>
#include <sstream>
#include "../thirdparty/glad/include/glad/glad.h"
#include "../thirdparty/glad/include/KHR/khrplatform.h"
#include "app.hpp"

bool View::Init()
{
    LogSystemInfo();

    if (!InitOpenGLLibrary_())
        return false;
    if (!InitWindowAndGLContext_())
        return false;
    if (!InitOpenGL_())
        return false;

    m_shaderPath = App::Get().DataPath() + "shaders" + PATH_SEPARATOR;

    glViewport(0, 0, DESIRED_WINDOW_WIDTH, DESIRED_WINDOW_HEIGHT);

    if (!RequireShader("default", Shader::Type::Vertex))
        return false;
    if (!RequireShader("default", Shader::Type::Fragment))
        return false;

    ShaderList vertexShaders;
    ShaderList fragmentShaders;
    vertexShaders.push_back("default");
    fragmentShaders.push_back("default");
    if (!CreateShaderProgram("default", vertexShaders, fragmentShaders))
        return false;

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float32), (void*)0);
    glEnableVertexAttribArray(0);

    float32 vertices[] = {
         0.5f,  0.5f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left
    };

    if (!CreateVBO("default", vertices, GL_STATIC_DRAW))
        return false;
    if (!UseVBO("default"))
        return false;

    m_fpsTimer = SDL_GetPerformanceCounter();

    return true;
}

void View::Cleanup()
{
    if (!m_vbos.empty())
    {
        for (auto it = m_vbos.begin(); it != m_vbos.end(); it++)
            glDeleteBuffers(1, &(it->vbo));

        m_vbos.clear();
    }

    if (!m_shaderPrograms.empty())
    {
        for (auto it = m_shaderPrograms.begin(); it != m_shaderPrograms.end(); it++)
            glDeleteProgram(it->program);

        m_shaderPrograms.clear();
    }

    if (!m_shaders.empty())
    {
        for (auto it = m_shaders.begin(); it != m_shaders.end(); it++)
            glDeleteShader(it->shader);

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
    static bool once = false;
    static uint32 vao;
    static uint32 ebo;
    static uint indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };

    if (!once)
    {
        once = true;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    }

    // @todo Deal with being minimized, toggling fullscreen, exiting, etc.
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
            m_windowWidth  = e.window.data1;
            m_windowHeight = e.window.data2;
            glViewport(0, 0, m_windowWidth, m_windowHeight);
            LogInfo("Window resized to %ux%u; viewport set.", m_windowWidth, m_windowHeight);
        }
    }

    if (((DeltaTime)(SDL_GetPerformanceCounter() - m_fpsTimer) / (DeltaTime)SDL_GetPerformanceFrequency()) >= 1.0f)
    {
        LogDebug("FPS: %u, DT: %f.", m_fpsCounter, dt);
        m_fpsCounter = 0;
        m_fpsTimer = SDL_GetPerformanceCounter();
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    if (!UseShaderProgram("default"))
        return false;
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    SDL_GL_SwapWindow(m_window);
    SDL_Delay(1);

    m_fpsCounter++;

    return !quit;
}

bool View::InitOpenGLLibrary_()
{
    if (SDL_GL_LoadLibrary(nullptr))
    {
        LogFatal("SDL failed to load OpenGL: %s.", SDL_GetError());
        return false;
    }
    LogInfo("Loaded OpenGL library.");

    return true;
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

    // @note Cleanup is handled by Cleanup().
    m_window = SDL_CreateWindow(App::Get().APPLICATION_NAME,
                                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                DESIRED_WINDOW_WIDTH, DESIRED_WINDOW_HEIGHT, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE

);
    if (!m_window)
    {
        LogFatal("Failed to create window: %s.", SDL_GetError());
        return false;
    }
    LogInfo("Created window.");

    // @note Cleanup is handled by Cleanup().
    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext)
    {
        LogFatal("Failed to create OpenGL context: %s.", SDL_GetError());
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

bool View::InitOpenGL_()
{
    // This requires an OpenGL context.
    if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
    {
        LogFatal("Failed to load OpenGL library functions.");
        return false;
    }
    LogInfo("Loaded OpenGL library functions.");

    if (GLVersion.major < (int)MINIMUM_OPENGL_MAJOR ||
        (GLVersion.major == (int)MINIMUM_OPENGL_MAJOR && GLVersion.minor < (int)MINIMUM_OPENGL_MINOR))
    {
        LogFatal("Loaded OpenGL v%i.%i, but need v%i.%i+.", GLVersion.major, GLVersion.minor,
                 MINIMUM_OPENGL_MAJOR, MINIMUM_OPENGL_MINOR);
        return false;
    }

    return true;
}

void View::LogSystemInfoPower_() const
{
    int secondsLeft;
    int batteryPercentage;
    SDL_PowerState s = SDL_GetPowerInfo(&secondsLeft, &batteryPercentage);

    if (s == SDL_POWERSTATE_ON_BATTERY)
    {
        std::string time = "unknown time";
        std::string charge = "unknown charge";

        if (batteryPercentage != -1)
            charge = std::to_string(batteryPercentage) + "%";

        if (secondsLeft != -1)
        {
            int s = secondsLeft;
            int m = 60;
            int h = 60*m;
            int hours = s/h;
            s -= hours*h;
            int minutes = s/m;
            s -= minutes*m;
            int seconds = s;

            if (hours > 0)
                time = std::to_string(hours) + "h" + std::to_string(minutes) + "m" + std::to_string(seconds) + "s";
            else if (minutes > 0)
                time = std::to_string(minutes) + "m" + std::to_string(seconds) + "s";
            else
                time = std::to_string(seconds) + "s";
        }

        LogInfo("- Power Source: Battery with %s and %s remaining.", charge.c_str(), time.c_str());
    }
    else if (s == SDL_POWERSTATE_NO_BATTERY)
    {
        LogInfo("- Power Source: AC with no battery.");
    }
    else if (s == SDL_POWERSTATE_CHARGING)
    {
        LogInfo("- Power Source: AC with a charging battery.");
    }
    else if (s == SDL_POWERSTATE_CHARGED)
    {
        LogInfo("- Power Source: AC with a fully charged battery.");
    }
    else
    {
        LogInfo("- Power Source: Unknown.");
    }
}

void View::LogSystemInfoSDLVersion_() const
{
    SDL_version c;
    SDL_version l;
    SDL_VERSION(&c);
    SDL_GetVersion(&l);
    LogInfo("- SDL version: %u.%u.%u compiled & %u.%u.%u linked.",
            c.major, c.minor, c.patch, l.major, l.minor, l.patch);
}

void View::LogSystemInfoWindowManager_() const
{
    SDL_Window* w = nullptr;
    SDL_SysWMinfo i;
    SDL_VERSION(&i.version);

    w = SDL_CreateWindow("View::LogSystemInfoWindowManager",
                         SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                         1, 1, SDL_WINDOW_HIDDEN);
    if (!w)
    {
        LogInfo("- Window Manager: Unknown (failed to create window: %s).", SDL_GetError());
        return;
    }
    else
    {
        if (!SDL_GetWindowWMInfo(w, &i))
        {
            LogInfo("- Window Manager: Unknown (%s).", SDL_GetError());
            SDL_DestroyWindow(w);
            w = nullptr;
            return;
        }
        else
        {
            SDL_DestroyWindow(w);
            w = nullptr;
        }
    }

    const char* wm = "Unknown";
    switch (i.subsystem)
    {
    case SDL_SYSWM_WINDOWS:  wm = "Microsoft Windows"; break;
    case SDL_SYSWM_X11:      wm = "X Window System"; break;
    case SDL_SYSWM_WINRT:    wm = "WinRT"; break;
    case SDL_SYSWM_DIRECTFB: wm = "DirectFB"; break;
    case SDL_SYSWM_COCOA:    wm = "Apple OS X"; break;
    case SDL_SYSWM_UIKIT:    wm = "UIKit"; break;
    case SDL_SYSWM_WAYLAND:  wm = "Wayland"; break;
    case SDL_SYSWM_MIR:      wm = "Mir"; break;
    case SDL_SYSWM_ANDROID:  wm = "Android"; break;
    case SDL_SYSWM_VIVANTE:  wm = "Vivante"; break;
    default: break;
    }

    LogInfo("- Window Manager: %s.", wm);
}

void View::LogSystemInfoRAM_() const
{
    LogInfo("- RAM: %i MiB.", SDL_GetSystemRAM());
}

void View::LogSystemInfoCPU_() const
{
    LogInfo("- CPU: %i logical cores, L1 cache: %i bytes, 3DNow!: %s, AVX: %s, AVX2: %s, AltiVec: %s, MMX: %s, RDTSC: %s, SSE: %s, SSE2: %s, SSE3: %s, SSE4.1: %s, SSE4.2: %s.",
            SDL_GetCPUCount(), SDL_GetCPUCacheLineSize(), YesNoBoolToStr(SDL_Has3DNow()), YesNoBoolToStr(SDL_HasAVX()), YesNoBoolToStr(SDL_HasAVX2()),
            YesNoBoolToStr(SDL_HasAltiVec()), YesNoBoolToStr(SDL_HasMMX()), YesNoBoolToStr(SDL_HasRDTSC()),
            YesNoBoolToStr(SDL_HasSSE()), YesNoBoolToStr(SDL_HasSSE2()), YesNoBoolToStr(SDL_HasSSE3()), YesNoBoolToStr(SDL_HasSSE41()), YesNoBoolToStr(SDL_HasSSE42()));
}

void View::LogSystemInfoGraphics_() const
{
    // @todo
    LogInfo("- Graphics: TODO.");
}

void View::LogSystemInfo() const
{
    LogInfo("System Information:");
    LogSystemInfoPower_();
    LogInfo("- Platform: %s.", SDL_GetPlatform());
    LogSystemInfoSDLVersion_();
    LogSystemInfoWindowManager_();
    LogSystemInfoRAM_();
    LogSystemInfoCPU_();
    LogSystemInfoGraphics_();
}

bool View::IsShaderLoaded(std::string name, enum Shader::Type type)
{
    for (auto it = m_shaders.begin(); it != m_shaders.end(); it++)
    {
        if (it->type == type && it->name == name)
            return true;
    }

    return false;
}

bool View::RetrieveShader(std::string name, enum Shader::Type type, uint& shader)
{
    for (auto it = m_shaders.begin(); it != m_shaders.end(); it++)
    {
        if (it->type == type && it->name == name)
        {
            shader = it->shader;
            return true;
        }
    }

    return false;
}

void View::DeleteShader(std::string name, enum Shader::Type type)
{
    for (auto it = m_shaders.begin(); it != m_shaders.end(); it++)
    {
        if (it->type == type && it->name == name)
        {
            glDeleteShader(it->shader);
            m_shaders.erase(it);
            return;
        }
    }
}

bool View::LoadShaderFile_(std::string name, enum Shader::Type type, std::string& contents)
{
    if (type == Shader::Type::Vertex)
        name += ".vert";
    else
        name += ".frag";

    LogInfo("Loading shader from file: %s.", name.c_str());

    std::FILE* f = std::fopen((m_shaderPath + name).c_str(), "rb");
    if (!f)
    {
        LogWarning("Failed to open file.");
        return false;
    }

    const uint32 bufSize = KIBIBYTES(4);
    char buf[bufSize];
    std::stringstream ss;
    bool hasRead = false;

    while (!std::feof(f))
    {
        std::fread(buf, 1, bufSize, f);
        if (std::ferror(f))
        {
            LogWarning("Failed to read from file.");
            std::fclose(f);
            f = nullptr;
            return false;
        }

        hasRead = true;
        ss << buf;
    }

    std::fclose(f);
    f = nullptr;

    if (!hasRead)
    {
        LogWarning("File was empty.");
        return false;
    }
    else
    {
        contents = ss.str();
        return true;
    }
}

bool View::LoadShader_(std::string name, enum Shader::Type type, bool required)
{
    bool vertex = type == Shader::Type::Vertex;
    LogInfo("Loading %s %s shader: %s.", (required ? "required" : "optional"), (vertex ? "vertex" : "fragment"), name.c_str());

    if (IsShaderLoaded(name, type))
    {
        LogInfo("Shader already loaded.");
        return true;
    }

    Shader s;
    s.name = name;
    s.type = type;

    std::string shaderStr;
    if (!LoadShaderFile_(name, type, shaderStr))
    {
        if (required)
            LogFatal("Failed to load shader.");
        else
            LogWarning("Failed to load shader.");

        return false;
    }

    if (vertex)
        s.shader = glCreateShader(GL_VERTEX_SHADER);
    else
        s.shader = glCreateShader(GL_FRAGMENT_SHADER);

    const char* shaderCStr = shaderStr.c_str();
    glShaderSource(s.shader, 1, &shaderCStr, nullptr);
    glCompileShader(s.shader);

    int success;
    const uint32 infoLogSize = KIBIBYTES(1);
    char infoLog[infoLogSize];
    glGetShaderiv(s.shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(s.shader, infoLogSize, nullptr, infoLog);

        if (required)
            LogFatal("Failed to compile shader: %s.", infoLog);
        else
            LogWarning("Failed to compile shader: %s.", infoLog);

        glDeleteShader(s.shader);
        return false;
    }

    m_shaders.push_back(s);
    return true;
}

bool View::IsShaderProgramCreated(std::string name)
{
    for (auto it = m_shaderPrograms.begin(); it != m_shaderPrograms.end(); it++)
    {
        if (it->name == name)
            return true;
    }

    return false;
}

bool View::RetrieveShaderProgram(std::string name, uint& program)
{
    for (auto it = m_shaderPrograms.begin(); it != m_shaderPrograms.end(); it++)
    {
        if (it->name == name)
        {
            program = it->program;
            return true;
        }
    }

    return false;
}

void View::DeleteShaderProgram(std::string name)
{
    for (auto it = m_shaderPrograms.begin(); it != m_shaderPrograms.end(); it++)
    {
        if (it->name == name)
        {
            glDeleteProgram(it->program);
            m_shaderPrograms.erase(it);
            return;
        }
    }
}

bool View::CreateShaderProgram(std::string name, ShaderList vertexShaders, ShaderList fragmentShaders, bool deleteShadersOnSuccess)
{
    LogInfo("Creating shader program: %s.", name.c_str());

    if (IsShaderProgramCreated(name))
    {
        LogFatal("Shader program already exists.");
        return false;
    }

    if (vertexShaders.empty() && fragmentShaders.empty())
    {
        LogFatal("Shader lists are empty.");
        return false;
    }

    ShaderProgram p;
    p.name = name;
    p.program = glCreateProgram();

    for (auto it = vertexShaders.begin(); it != vertexShaders.end(); it++)
    {
        uint32 s;
        if (!RetrieveShader(*it, Shader::Type::Vertex, s))
        {
            LogFatal("Vertex shader not loaded: %s.", it->c_str());
            glDeleteProgram(p.program);
            return false;
        }

        glAttachShader(p.program, s);
    }

    for (auto it = fragmentShaders.begin(); it != fragmentShaders.end(); it++)
    {
        uint32 s;
        if (!RetrieveShader(*it, Shader::Type::Fragment, s))
        {
            LogFatal("Fragment shader not loaded: %s.", it->c_str());
            glDeleteProgram(p.program);
            return false;
        }

        glAttachShader(p.program, s);
    }

    glLinkProgram(p.program);

    int success;
    const uint32 infoLogSize = KIBIBYTES(1);
    char infoLog[infoLogSize];
    glGetProgramiv(p.program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(p.program, infoLogSize, nullptr, infoLog);
        LogFatal("Failed to link shader program: %s.", infoLog);
        glDeleteProgram(p.program);
        return false;
    }

    if (deleteShadersOnSuccess)
    {
        for (auto it = vertexShaders.begin(); it != vertexShaders.end(); it++)
            DeleteShader(*it, Shader::Type::Vertex);

        for (auto it = fragmentShaders.begin(); it != fragmentShaders.end(); it++)
            DeleteShader(*it, Shader::Type::Fragment);
    }

    m_shaderPrograms.push_back(p);
    return true;
}

bool View::UseShaderProgram(std::string name)
{
    uint32 p;
    if (!RetrieveShaderProgram(name, p))
    {
        LogFatal("Tried to use non-existant shader program: %s.", name.c_str());
        return false;
    }

    glUseProgram(p);
    return true;
}

bool View::IsVBOCreated(std::string name)
{
    for (auto it = m_vbos.begin(); it != m_vbos.end(); it++)
    {
        if (it->name == name)
            return true;
    }

    return false;
}

bool View::RetrieveVBO(std::string name, uint& vbo)
{
    for (auto it = m_vbos.begin(); it != m_vbos.end(); it++)
    {
        if (it->name == name)
        {
            vbo = it->vbo;
            return true;
        }
    }

    return false;
}

void View::DeleteVBO(std::string name)
{
    for (auto it = m_vbos.begin(); it != m_vbos.end(); it++)
    {
        if (it->name == name)
        {
            glDeleteBuffers(1, &(it->vbo));
            m_vbos.erase(it);
            return;
        }
    }
}

bool View::CreateVBO(std::string name, float32* vertices, uint32 usage)
{
    LogInfo("Creating VBO: %s.", name.c_str());

    if (!vertices)
    {
        LogFatal("Vertices is nullptr.");
        return false;
    }

    VBO v;
    v.name = name;
    v.usage = usage;
    glGenBuffers(1, &v.vbo);

    glBindBuffer(GL_ARRAY_BUFFER, v.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, usage);

    m_vbos.push_back(v);
    return true;
}

bool View::UseVBO(std::string name)
{
    uint32 v;
    if (!RetrieveVBO(name, v))
    {
        LogFatal("Tried to use non-existant VBO: %s.", name.c_str());
        return false;
    }

    glBindBuffer(GL_ARRAY_BUFFER, v);
    return true;
}
