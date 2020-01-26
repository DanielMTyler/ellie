/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

// @cleanup

#ifndef RENDERER_OPENGL_HPP_INCLUDED
#define RENDERER_OPENGL_HPP_INCLUDED

struct Shader
{
    enum class Type
    {
        VERTEX,
        FRAGMENT
    };
    
    std::string file;
    Type type;
    GLuint id;
};

#include <fstream> // std::ifstream

namespace CoreShaderImpl
{
    // @todo This shouldn't throw exceptions.
    // @todo Use ResultBool return with a useful error message.
    // @todo Use SDL_RWops?
    bool ReadFileToStr(std::string file, std::string& contents)
    {
        std::ifstream in(file, std::ios::in | std::ios::binary);
        if (in)
            contents = std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        else
            return false;
        
        return true;
    }
    
    bool LoadAndCompileShader(Shader& shader, std::string baseFilename, Shader::Type type)
    {
        DEBUG_ASSERT(!baseFilename.empty());
        
        ResultBool r;
        r.result = true;
        
        shader.file = gDataPath + "shaders" + PLATFORM_PATH_SEPARATOR + baseFilename;
        if (type == Shader::Type::VERTEX)
        {
            shader.file += ".vert";
            shader.type = Shader::Type::VERTEX;
            shader.id = glCreateShader(GL_VERTEX_SHADER);
        }
        else
        {
            shader.file += ".frag";
            shader.type = Shader::Type::FRAGMENT;
            shader.id = glCreateShader(GL_FRAGMENT_SHADER);
        }
        
        std::string text;
        if (!ReadFileToStr(shader.file.c_str(), text))
        {
            SPDLOG_LOGGER_CRITICAL(gLogger, "Failed to read shader: {}.", shader.file);
            return false;
        }
        const char* textBuf = text.c_str();
        GLCHECK(glShaderSource(shader.id, 1, &textBuf, nullptr));
        GLCHECK(glCompileShader(shader.id));
        
        const uint32 INFOLOGSIZE = KIBIBYTES(1);
        char infoLog[INFOLOGSIZE];
        int success = GL_FALSE; // If glGetShaderiv were to fail for some reason, success will == this value.
        GLCHECK(glGetShaderiv(shader.id, GL_COMPILE_STATUS, &success));
        if (!success)
        {
            GLCHECK(glGetShaderInfoLog(shader.id, INFOLOGSIZE, nullptr, infoLog));
            SPDLOG_LOGGER_CRITICAL(gLogger, "Failed to compile shader ({}): {}.", shader.file, infoLog);
            return false;
        }
        
        return true;
    }
}; // namespace CoreShaderImpl.

bool LoadVertexShader(Shader& shader, std::string name)
{
    DEBUG_ASSERT(!name.empty());
    if (!CoreShaderImpl::LoadAndCompileShader(shader, name, Shader::Type::VERTEX))
        return false;
    return true;
}

bool LoadFragmentShader(Shader& shader, std::string name)
{
    DEBUG_ASSERT(!name.empty());
    if (!CoreShaderImpl::LoadAndCompileShader(shader, name, Shader::Type::FRAGMENT))
        return false;
    return true;
}

#endif


#if 0

#include <glad/glad.h>
#include <SDL.h>

#define OPENGL_DEBUG 1

#ifdef OPENGL_DEBUG
    #define GLCHECK(Call) \
        Call; \
        GLCheckImpl(__FILE__, __LINE__)
#else
    #define GLCHECK(Call)
#endif

void GLCheckImpl(const char* /*file*/, uint32 /*line*/)
{
    GLenum e;
    while ((e = glGetError()) != GL_NO_ERROR)
    {
        switch (e)
        {
            case GL_INVALID_ENUM:                  // @todo gLog.debug("OpenGL", "GL_INVALID_ENUM"); break;
            case GL_INVALID_VALUE:                 // @todo gLog.debug("OpenGL", "GL_INVALID_VALUE"); break;
            case GL_INVALID_OPERATION:             // @todo gLog.debug("OpenGL", "GL_INVALID_OPERATION"); break;
            case GL_OUT_OF_MEMORY:                 // @todo gLog.debug("OpenGL", "GL_OUT_OF_MEMORY"); break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: // @todo gLog.debug("OpenGL", "GL_INVALID_FRAMEBUFFER_OPERATION"); break;
            default:
                const uint32 BUFSIZE = 11; // Should be 7, but could be 11 max due to uint32.
                char buf[BUFSIZE];
                if (!std::snprintf(buf, BUFSIZE, "0x%.4X", e))
                    // @todo gLog.debug("OpenGL", "%i", e);
                    ;
                else
                    // @todo gLog.debug("OpenGL", "%s", buf);
                    ;
                break;
        }
    }
}

#include "renderer_opengl.cpp"

const char* SDLGLProfileToStr(int p)
{
    if (p == SDL_GL_CONTEXT_PROFILE_CORE)
        return "Core";
    else if (p == SDL_GL_CONTEXT_PROFILE_COMPATIBILITY)
        return "Compatibility";
    else if (p == SDL_GL_CONTEXT_PROFILE_ES)
        return "ES";
    else
        return "Unknown";
}

// Returns false on failure.
bool SDLSetGLAttribs()
{
    int hwAccel = 1;
    int glMajor = 3;
    int glMinor = 3;
    int glProfile = SDL_GL_CONTEXT_PROFILE_CORE;
    int doubleBuffer = 1;
    bool failed = false;
    
    if (SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, hwAccel) < 0)
        failed = true;
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, glMajor) < 0)
        failed = true;
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, glMinor) < 0)
        failed = true;
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, glProfile) < 0)
        failed = true;
    if (SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, doubleBuffer) < 0)
        failed = true;
    
    if (failed)
    {
        // @todo gLog.fatal("Core", "Failed to set OpenGL attributes: %s", SDL_GetError());
        return false;
    }

    // @todo gLog.info("Core", "Requested OpenGL support: HW accel=%s, v%i.%i %s profile, Double Buffered=%s.", BoolToStr(hwAccel), glMajor, glMinor, SDLGLProfileToStr(glProfile), BoolToStr(doubleBuffer));
    return true;
}

// Returns false if we didn't get what was requested or on failure.
bool SDLCheckAndReportGLAttribs()
{
    int hwAccel;
    int glMajor;
    int glMinor;
    int glProfile;
    int doubleBuffer;
    bool failed = false;
    
    if (SDL_GL_GetAttribute(SDL_GL_ACCELERATED_VISUAL, &hwAccel) < 0)
        failed = true;
    if (SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &glMajor) < 0)
        failed = true;
    if (SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &glMinor) < 0)
        failed = true;
    if (SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &glProfile) < 0)
        failed = true;
    if (SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &doubleBuffer) < 0)
        failed = true;

    if (failed)
    {
        // @todo gLog.fatal("Core", "Failed to get OpenGL attributes: %s", SDL_GetError());
        return false;
    }
    
    if (!hwAccel)
        failed = true;
    if (glMajor != 3)
        failed = true;
    if (glMinor != 3)
        failed = true;
    if (glProfile != SDL_GL_CONTEXT_PROFILE_CORE)
        failed = true;
    if (!doubleBuffer)
        failed = true;

    if (failed)
    {
        // @todo gLog.fatal("Core", "OpenGL support doesn't match what was requested: HW accel=%s, v%i.%i %s profile, Double Buffered=%s.", BoolToStr(hwAccel), glMajor, glMinor, SDLGLProfileToStr(glProfile), BoolToStr(doubleBuffer));
        return false;
    }
    else
    {
        // @todo gLog.info("Core", "OpenGL support matches what was requested.");
        return true;
    }
}

void SDLUpdateViewport(int width, int height)
{
    // TODO: Error checking.
    glViewport(0, 0, width, height);
    // @todo gLog.info("Core", "Set OpenGL viewport to %ix%i.", width, height);
}

class SDLWrapper
{
public:
    SDL_Window* window = nullptr;
    // I'm just a void*.
    SDL_GLContext glContext = nullptr;

    ~SDLWrapper()
    {
        if (glContext)
        {
            SDL_GL_DeleteContext(glContext);
            glContext = nullptr;
        }
        if (window)
        {
            SDL_DestroyWindow(window);
            window = nullptr;
        }
        SDL_Quit();
    }

    bool init()
    {
        // TODO: This whole function needs to be reworked and logging added.
        DEBUG_ASSERT(!window);

        // TODO: Revisit this once OpenGL is up and running.
        if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
        {
            // @todo gLog.fatal("Core", "Failed to initialize SDL: %s", SDL_GetError());
            return false;
        }

        if (SDL_GL_LoadLibrary(nullptr) < 0)
        {
            // @todo gLog.fatal("Core", "Failed to load OpenGL library: %s", SDL_GetError());
            return false;
        }

        if (!SDLSetGLAttribs())
            return false;

        const bool   SCREEN_FULLSCREEN = false;
        const uint32 SCREEN_WIDTH  = 800;
        const uint32 SCREEN_HEIGHT = 600;
        const char*  SCREEN_TITLE  = "Ellie";

        if (SCREEN_FULLSCREEN)
        {
            window = SDL_CreateWindow(SCREEN_TITLE,
                                      SDL_WINDOWPOS_UNDEFINED,
                                      SDL_WINDOWPOS_UNDEFINED,
                                      0,
                                      0,
                                      SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_OPENGL);
        }
        else
        {
            window = SDL_CreateWindow(SCREEN_TITLE,
                                      SDL_WINDOWPOS_UNDEFINED,
                                      SDL_WINDOWPOS_UNDEFINED,
                                      SCREEN_WIDTH,
                                      SCREEN_HEIGHT,
                                      SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
        }
        if (!window)
        {
            // @todo gLog.fatal("Core", "Failed to create window: %s", SDL_GetError());
            return false;
        }

        glContext = SDL_GL_CreateContext(window);
        if (!glContext)
        {
            // @todo gLog.fatal("Core", "Failed to create OpenGL Context: %s", SDL_GetError());
            return false;
        }

        // @todo gLog.info("Core", "OpenGL loaded.");

        if (!SDLCheckAndReportGLAttribs())
        {
            return false;
        }

        // @todo gLog.info("Core", "Loading OpenGL extensions via GLAD.");
        if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
        {
            // @todo gLog.fatal("Core", "Failed to load OpenGL extensions via GLAD: %s", SDL_GetError());
            return false;
        }
        // @todo gLog.info("Core", "Loaded OpenGL v%u.%u extensions.", GLVersion.major, GLVersion.minor);
        
        // Vsync
        // TODO: Check for errors.
        SDL_GL_SetSwapInterval(1);
        SDLUpdateViewport(SCREEN_WIDTH, SCREEN_HEIGHT);
        return true;
    }
};



struct TestData
{
    GLuint shaderProgramID;
    GLuint vao;
};

static TestData testData;

bool TestInit()
{
    std::string e; // GL Errors.
    
    Shader vertexShader;
    Shader fragmentShader;
    if (!LoadVertexShader(vertexShader, "default"))
        return false;
    if (!LoadFragmentShader(fragmentShader, "default"))
        return false;
    
    GLCHECK(testData.shaderProgramID = glCreateProgram());
    if (!testData.shaderProgramID)
        return false;
    GLCHECK(glAttachShader(testData.shaderProgramID, vertexShader.id));
    GLCHECK(glAttachShader(testData.shaderProgramID, fragmentShader.id));
    GLCHECK(glLinkProgram(testData.shaderProgramID));
    const uint32 INFOLOGSIZE = KIBIBYTES(1);
    char infoLog[INFOLOGSIZE];
    int success = GL_FALSE; // If glGetShaderiv were to fail for some reason, success will == this value.
    GLCHECK(glGetProgramiv(testData.shaderProgramID, GL_LINK_STATUS, &success));
    if (!success)
    {
        glGetProgramInfoLog(testData.shaderProgramID, INFOLOGSIZE, nullptr, infoLog);
        // @todo gLog.fatal("OpenGL", "Failed to link shader program: %s.", infoLog);
        return false;
    }
    GLCHECK(glDeleteShader(vertexShader.id));
    GLCHECK(glDeleteShader(fragmentShader.id));
    
    GLCHECK(glGenVertexArrays(1, &testData.vao));
    GLCHECK(glBindVertexArray(testData.vao));
    
    float vertices[] = {
         0.5f,  0.5f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left
    };
    GLuint vbo;
    // TODO: Error checking.
    GLCHECK(glGenBuffers(1, &vbo));
    GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo));
    GLCHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

    GLuint indices[] = { // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };
    GLuint ebo;
    GLCHECK(glGenBuffers(1, &ebo));
    GLCHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo));
    GLCHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));
    GLCHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr));
    GLCHECK(glEnableVertexAttribArray(0));
    
    return true;
}

bool TestRender(float /*dt*/)
{
    std::string e; // GL Errors.
    GLCHECK(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
    GLCHECK(glClear(GL_COLOR_BUFFER_BIT));
    GLCHECK(glUseProgram(testData.shaderProgramID));
    GLCHECK(glBindVertexArray(testData.vao));
    GLCHECK(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));
    GLCHECK(glBindVertexArray(0));
    return true;
}

bool TestLogic(float /*dt*/)
{
    return true;
}

bool TestInput(float /*dt*/)
{
    return true;
}

bool TestEvent(SDL_Event& e, float /*dt*/)
{
    static bool wireframe = false;
    switch (e.type)
    {
        case SDL_KEYDOWN:
            if (e.key.keysym.sym == SDLK_ESCAPE)
            {
                return false;
            }
            else if (e.key.keysym.sym == SDLK_w)
            {
                wireframe = !wireframe;
                if (wireframe)
                {
                    GLCHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
                }
                else
                {
                    GLCHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
                }
                
                // @todo gLog.info("Test", "Set wireframe to %s.", OnOffToStr(wireframe));
            }
            
            break;
        default:
            break;
    }
    
    return true;
}

void TestCleanup()
{
}


        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_WINDOWEVENT:
                    // SDL_WINDOWEVENT_SIZE_CHANGED = Any size change.
                    // SDL_WINDOWEVENT_RESIZED = Additional event to indicate the size change was due to an external event such as the user or window manager.
                    if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                        SDLUpdateViewport(event.window.data1, event.window.data2);
                    break;
                default:
                    break;
            }
            
            if (!quit && !TestEvent(event, dt))
            {
                quit = true;
                break;
            }
        }

        if (quit)
            break;
        
        SDL_GL_SwapWindow(sdl.window);
        SDL_Delay(1);
    }





#endif
