/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

// @cleanup

#include "global.hpp"
#include <glad/glad.h>
#include <glad.c>
#include <SDL.h>
#include <fstream>
#include <iostream>
#include <string>
#include "log.cpp"
#include "services.hpp"

static Log gLog;
static std::string gGameFullPath;
static std::string gGameLiveFullPath;
static std::string gDataPath;
static std::string gPrefPath;

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
        gLog.fatal("Core", "Failed to set OpenGL attributes: %s", SDL_GetError());
        return false;
    }

    gLog.info("Core", "Requested OpenGL support: HW accel=%s, v%i.%i %s profile, Double Buffered=%s.", BoolToStr(hwAccel), glMajor, glMinor, SDLGLProfileToStr(glProfile), BoolToStr(doubleBuffer));
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
        gLog.fatal("Core", "Failed to get OpenGL attributes: %s", SDL_GetError());
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
        gLog.fatal("Core", "OpenGL support doesn't match what was requested: HW accel=%s, v%i.%i %s profile, Double Buffered=%s.", BoolToStr(hwAccel), glMajor, glMinor, SDLGLProfileToStr(glProfile), BoolToStr(doubleBuffer));
        return false;
    }
    else
    {
        gLog.info("Core", "OpenGL support matches what was requested.");
        return true;
    }
}

void SDLUpdateViewport(int width, int height)
{
    // TODO: Error checking.
    glViewport(0, 0, width, height);
    gLog.info("Core", "Set OpenGL viewport to %ix%i.", width, height);
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
        ASSERT(!window);

        // TODO: Revisit this once OpenGL is up and running.
        if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
        {
            gLog.fatal("Core", "Failed to initialize SDL: %s", SDL_GetError());
            return false;
        }

        if (SDL_GL_LoadLibrary(nullptr) < 0)
        {
            gLog.fatal("Core", "Failed to load OpenGL library: %s", SDL_GetError());
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
            gLog.fatal("Core", "Failed to create window: %s", SDL_GetError());
            return false;
        }

        glContext = SDL_GL_CreateContext(window);
        if (!glContext)
        {
            gLog.fatal("Core", "Failed to create OpenGL Context: %s", SDL_GetError());
            return false;
        }

        gLog.info("Core", "OpenGL loaded.");

        if (!SDLCheckAndReportGLAttribs())
        {
            return false;
        }

        gLog.info("Core", "Loading OpenGL extensions via GLAD.");
        if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
        {
            gLog.fatal("Core", "Failed to load OpenGL extensions via GLAD: %s", SDL_GetError());
            return false;
        }
        gLog.info("Core", "Loaded OpenGL v%u.%u extensions.", GLVersion.major, GLVersion.minor);
        
        // Vsync
        // TODO: Check for errors.
        SDL_GL_SetSwapInterval(1);
        SDLUpdateViewport(SCREEN_WIDTH, SCREEN_HEIGHT);
        
        // TODO: Check for errors.
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        return true;
    }
};

bool GameRetrieveFunctions(void* game, GameServices& gameServices)
{
    ASSERT(game);
    
    gameServices.init = (GameInitCB*)SDL_LoadFunction(game, "GameInit");
    if (!gameServices.init)
    {
        gLog.fatal("Core", "Failed to retrieve GameInit function from game: %s", SDL_GetError());
        return false;
    }
    
    gameServices.preReload = (GamePreReloadCB*)SDL_LoadFunction(game, "GamePreReload");
    if (!gameServices.preReload)
    {
        gLog.fatal("Core", "Failed to retrieve GamePreReload function from game: %s", SDL_GetError());
        ZERO_STRUCT(gameServices);
        return false;
    }
    
    gameServices.postReload = (GamePostReloadCB*)SDL_LoadFunction(game, "GamePostReload");
    if (!gameServices.postReload)
    {
        gLog.fatal("Core", "Failed to retrieve GamePostReload function from game: %s", SDL_GetError());
        ZERO_STRUCT(gameServices);
        return false;
    }
    
    gameServices.cleanup = (GameCleanupCB*)SDL_LoadFunction(game, "GameCleanup");
    if (!gameServices.cleanup)
    {
        gLog.fatal("Core", "Failed to retrieve GameCleanup function from game: %s", SDL_GetError());
        ZERO_STRUCT(gameServices);
        return false;
    }
    
    gameServices.input = (GameInputCB*)SDL_LoadFunction(game, "GameInput");
    if (!gameServices.input)
    {
        gLog.fatal("Core", "Failed to retrieve GameInput function from game: %s", SDL_GetError());
        ZERO_STRUCT(gameServices);
        return false;
    }
    
    gameServices.logic = (GameLogicCB*)SDL_LoadFunction(game, "GameLogic");
    if (!gameServices.logic)
    {
        gLog.fatal("Core", "Failed to retrieve GameLogic function from game: %s", SDL_GetError());
        ZERO_STRUCT(gameServices);
        return false;
    }
    
    gameServices.render = (GameRenderCB*)SDL_LoadFunction(game, "GameRender");
    if (!gameServices.render)
    {
        gLog.fatal("Core", "Failed to retrieve GameRender function from game: %s", SDL_GetError());
        ZERO_STRUCT(gameServices);
        return false;
    }
    
    return true;
}


// Copy the game to a temp file for opening; this allows the original to be replaced for live reloading.
bool GameCopyToTemp()
{
    ResultBool r = PlatformCopyFile(gGameFullPath, gGameLiveFullPath, false);
    if (!r.result)
    {
        gLog.fatal("Core", "Failed to copy game to live file: %s", r.error.c_str());
        return false;
    }

    return true;
}

// Returns game via SDL_LoadObject or nullptr on error.
void* GameInit(CoreServices& coreServices, GameServices& gameServices)
{
    if (!GameCopyToTemp())
    {
        return nullptr;
    }

    void* g = SDL_LoadObject(gGameLiveFullPath.c_str());
    if (!g)
    {
        gLog.fatal("Core", "Failed to load game: %s", SDL_GetError());
        return nullptr;
    }

    if (!GameRetrieveFunctions(g, gameServices))
    {
        SDL_UnloadObject(g);
        g = nullptr;
        return nullptr;
    }

    if (!gameServices.init(&coreServices))
    {
        ZERO_STRUCT(gameServices);
        SDL_UnloadObject(g);
        g = nullptr;
        return nullptr;
    }

    return g;
}

// Returns game via SDL_LoadObject or nullptr on error.
void* GameReload(void* oldGame, CoreServices& coreServices, GameServices& gameServices)
{
    ASSERT(oldGame);

    gameServices.preReload();
    ZERO_STRUCT(gameServices);

    SDL_UnloadObject(oldGame);
    oldGame = nullptr;

    if (!GameCopyToTemp())
    {
        return nullptr;
    }

    void* g = SDL_LoadObject(gGameLiveFullPath.c_str());
    if (!g)
    {
        gLog.fatal("Core", "Failed to load game: %s", SDL_GetError());
        return nullptr;
    }

    if (!GameRetrieveFunctions(g, gameServices))
    {
        SDL_UnloadObject(g);
        g = nullptr;
        return nullptr;
    }

    gameServices.postReload(&coreServices);
    return g;
}

void GameCleanup(void* game, GameServices& gameServices)
{
    ASSERT(game);

    gameServices.cleanup();

    ZERO_STRUCT(gameServices);
    SDL_UnloadObject(game);
    game = nullptr;

    // Check if file exists before trying to delete it to avoid a useless error message being logged.
    ResultBool r = PlatformDeleteFile(gGameLiveFullPath);
    if (!r.result)
        gLog.warn("Core", "Failed to delete live game: %s", r.error.c_str());
}

bool LoadShaderFromFile(std::string file, std::string& shader)
{
    std::string filePath = gDataPath + "shaders" + PLATFORM_PATH_SEPARATOR + file;
    std::ifstream in(filePath, std::ios::in | std::ios::binary);
    if (in)
    {
        shader = std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        gLog.info("Core", "Loaded shader: %s.", filePath.c_str());
    }
    else
    {
        gLog.fatal("Core", "Failed to load shader: %s.", filePath.c_str());
        return false;
    }
    
    return true;
}

bool CompileShaderFromStr(GLuint shaderID, std::string shader)
{
    const char* shaderBuf = shader.c_str();
    glShaderSource(shaderID, 1, &shaderBuf, nullptr);
    glCompileShader(shaderID);
    const uint32 infoLogSize = KIBIBYTES(1);
    char infoLog[infoLogSize];
    int success;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shaderID, infoLogSize, nullptr, infoLog);
        gLog.fatal("OpenGL", "Failed to compile shader: %s.", infoLog);
        return false;
    }
    
    gLog.info("OpenGL", "Compiled shader: %u.", shaderID);
    return true;
}

bool LoadAndCompileVertexShader(GLuint& shaderID, std::string baseFileName)
{
    shaderID = glCreateShader(GL_VERTEX_SHADER);
    std::string fileName = baseFileName + ".vert";
    std::string shader;
    if (!LoadShaderFromFile(fileName, shader))
        return false;
    if (!CompileShaderFromStr(shaderID, shader))
        return false;
    return true;
}

bool LoadAndCompileFragmentShader(GLuint& shaderID, std::string baseFileName)
{
    shaderID = glCreateShader(GL_FRAGMENT_SHADER);
    std::string fileName = baseFileName + ".frag";
    std::string shader;
    if (!LoadShaderFromFile(fileName, shader))
        return false;
    if (!CompileShaderFromStr(shaderID, shader))
        return false;
    return true;
}

struct TestData
{
    GLuint shaderProgramID;
    GLuint vao;
};

static TestData testData;

bool TestInit()
{
    GLuint   vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    LoadAndCompileVertexShader(vertexShaderID, "default");
    LoadAndCompileFragmentShader(fragmentShaderID, "default");
    
    testData.shaderProgramID = glCreateProgram();
    glAttachShader(testData.shaderProgramID, vertexShaderID);
    glAttachShader(testData.shaderProgramID, fragmentShaderID);
    glLinkProgram(testData.shaderProgramID);
    const uint32 infoLogSize = KIBIBYTES(1);
    char infoLog[infoLogSize];
    int success;
    glGetProgramiv(testData.shaderProgramID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(testData.shaderProgramID, infoLogSize, nullptr, infoLog);
        gLog.fatal("Test", "Shader Program linking failed: %s", infoLog);
        return false;
    }
    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);
    
    glGenVertexArrays(1, &testData.vao);
    glBindVertexArray(testData.vao);

    float vertices[] = {
         0.5f,  0.5f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left
    };
    GLuint vbo;
    // TODO: Error checking.
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint indices[] = { // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };
    GLuint ebo;
    // TODO: Error checking.
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    
    return true;
}

bool TestRender(float dt)
{
    // @todo Check for errors.
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(testData.shaderProgramID);
    glBindVertexArray(testData.vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
    return true;
}

bool TestLogic(float dt)
{
    return true;
}

bool TestInput(float dt)
{
    return true;
}

bool TestEvent(SDL_Event& e, float dt)
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
                // @todo Error checking.
                if (wireframe)
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                else
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                gLog.info("Test", "Set wireframe to %s.", OnOffToStr(wireframe));
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

bool CoreInit(CoreServices& coreServices)
{
    char* exePathBuf = SDL_GetBasePath();
    if (!exePathBuf)
    {
        std::cerr << "Failed to get executable path: " << SDL_GetError() << std::endl;
        return false;
    }
    std::string exePath = exePathBuf;
    SDL_free(exePathBuf);
    exePathBuf = nullptr;
    
    char* sdlPrefPath = SDL_GetPrefPath("DanielMTyler", PROJECT_NAME);
    if (sdlPrefPath)
    {
        gPrefPath = sdlPrefPath;
        SDL_free(sdlPrefPath);
        sdlPrefPath = nullptr;
    }
    else
    {
        std::cerr << "Failed to get user preferences path: " << SDL_GetError() << std::endl;
        return false;
    }
    
    if (!gLog.init((gPrefPath + PLATFORM_LOG_FILENAME).c_str()))
        return false;

    #if BUILD_RELEASE
        gLog.info("Core", "Build: Release.");
    #elif BUILD_DEBUG
        gLog.info("Core", "Build: Debug.");
    #else
        #error Build type unknown.
    #endif
    
    coreServices.log = &gLog;
    gLog.info("Core", "EXE path: %s", exePath.c_str());
    gGameFullPath = exePath + GAME_FILENAME;
    gLog.info("Core", "Game fullpath: %s", gGameFullPath.c_str());
    ResultBool r = PlatformCreateTempFile(gGameLiveFullPath);
    if (!r.result)
    {
        gLog.fatal("Core", "Failed to create temp live game file: %s", r.error.c_str());
        return false;
    }
    gLog.info("Core", "Game Live fullpath: %s", gGameLiveFullPath.c_str());
    
    std::string cwdPath;
    r = PlatformGetCWD(cwdPath);
    if (!r.result)
    {
        gLog.fatal("Core", "Failed to get the current working directory: %s", r.error.c_str());
        return false;
    }
    // Prefer the CWD over programPath for releasePath, but verify the data folder exists in releasePath.
    std::string releasePath = cwdPath;
    gDataPath = releasePath + "data" + PLATFORM_PATH_SEPARATOR;
    r = PlatformFolderExists(gDataPath);
    if (!r.result)
    {
        releasePath = exePath;
        gDataPath = releasePath + "data" + PLATFORM_PATH_SEPARATOR;
        r = PlatformFolderExists(gDataPath);
        if (!r.result)
        {
            gLog.fatal("Core", "The data folder wasn't found in the current working directory (%s) or the executable directory (%s).", cwdPath.c_str(), exePath.c_str());
            return false;
        }
    }
    gLog.info("Core", "Release path: %s", releasePath.c_str());
    gLog.info("Core", "Data path: %s", gDataPath.c_str());
    gLog.info("Core", "User preferences path: %s", gPrefPath.c_str());
    gLog.info("Core", "CWD path: %s", cwdPath.c_str());
    
    if (!TestInit())
        return false;
    
    return true;
}

// WARNING: SDL 2 requires this exact function signature, changing it will give "undefined reference to SDL_main" linker errors.
int main(int argc, char* argv[])
{
    CoreServices coreServices;
    if (!CoreInit(coreServices))
        return 1;
    
    SDLWrapper sdl;
    if (!sdl.init())
        return false;
    
    GameServices gameServices;
    void* game = GameInit(coreServices, gameServices);
    if (!game)
        return 1;

    game = GameReload(game, coreServices, gameServices);
    if (!game)
        return 1;
    
    SDL_Event event;
    bool quit = false;
    // @todo Implement.
    float dt = 0.0f;
    while (!quit)
    {
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
        if (!gameServices.input(dt) || !TestInput(dt))
            break;
        if (!gameServices.logic(dt) || !TestLogic(dt))
            break;
        if (!gameServices.render(dt) || !TestRender(dt))
            break;
        
        SDL_GL_SwapWindow(sdl.window);
        SDL_Delay(1);
    }
    
    GameCleanup(game, gameServices);
    TestCleanup();
    // @todo Should there be OGL/SDL cleanup here?
    return 0;
}
