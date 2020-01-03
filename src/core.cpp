/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#include "global.hpp"
#include <glad/glad.h>
#include <glad.c>
#include <SDL.h>
#include <iostream>
#include <string>
#include "log.cpp"

static Log gLog;
static std::string gGameFullPath;
static std::string gGameLiveFullPath;

const char *BoolToStr(bool b)
{
    return (b ? "True" : "False");
}

const char *SDLGLProfileToStr(int p)
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
    
    failed = SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, hwAccel) < 0 ? true : false;
    failed = SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, glMajor) < 0 ? true : false;
    failed = SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, glMinor) < 0 ? true : false;
    failed = SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, glProfile) < 0 ? true : false;
    failed = SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, doubleBuffer) < 0 ? true : false;

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

    failed = SDL_GL_GetAttribute(SDL_GL_ACCELERATED_VISUAL, &hwAccel) < 0 ? true : false;
    failed = SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &glMajor) < 0 ? true : false;
    failed = SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &glMinor) < 0 ? true : false;
    failed = SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &glProfile) < 0 ? true : false;
    failed = SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &doubleBuffer) < 0 ? true : false;

    if (failed)
    {
        gLog.fatal("Core", "Failed to get OpenGL attributes: %s", SDL_GetError());
        return false;
    }

    failed = !hwAccel ? true : false;
    failed = glMajor != 3 ? true : false;
    failed = glMinor != 3 ? true : false;
    failed = glProfile != SDL_GL_CONTEXT_PROFILE_CORE ? true : false;
    failed = !doubleBuffer ? true : false;

    if (failed)
    {
        gLog.fatal("Core", "OpenGL support doesn't match what was requested: HW accel=%s, v%i.%i %s profile, Double Buffered=%s.", BoolToStr(hwAccel), glMajor, glMinor, SDLGLProfileToStr(glProfile), BoolToStr(doubleBuffer));
        return false;
    } else
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
    SDL_Window *window = nullptr;
    // I'm just a void *.
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

        const bool SCREEN_FULLSCREEN = false;
        const uint32 SCREEN_WIDTH = 800;
        const uint32 SCREEN_HEIGHT = 600;
        const char *SCREEN_TITLE = "Ellie";

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

bool GameRetrieveFunctions(void *game, GameServices &gameServices)
{
    ASSERT(game);

    gameServices.onInit = (Game_OnInit *)SDL_LoadFunction(game, "OnInit");
    if (!gameServices.onInit)
    {
        gLog.fatal("Core", "Failed to retrieve OnInit function from game: %s", SDL_GetError());
        return false;
    }

    gameServices.onPreReload = (Game_OnPreReload *)SDL_LoadFunction(game, "OnPreReload");
    if (!gameServices.onPreReload)
    {
        gLog.fatal("Core", "Failed to retrieve OnPreReload function from game: %s", SDL_GetError());
        ZERO_STRUCT(gameServices);
        return false;
    }

    gameServices.onPostReload = (Game_OnPostReload *)SDL_LoadFunction(game, "OnPostReload");
    if (!gameServices.onPostReload)
    {
        gLog.fatal("Core", "Failed to retrieve OnPostReload function from game: %s", SDL_GetError());
        ZERO_STRUCT(gameServices);
        return false;
    }

    gameServices.onCleanup = (Game_OnCleanup *)SDL_LoadFunction(game, "OnCleanup");
    if (!gameServices.onCleanup)
    {
        gLog.fatal("Core", "Failed to retrieve OnCleanup function from game: %s", SDL_GetError());
        ZERO_STRUCT(gameServices);
        return false;
    }

    gameServices.onInput = (Game_OnInput *)SDL_LoadFunction(game, "OnInput");
    if (!gameServices.onInput)
    {
        gLog.fatal("Core", "Failed to retrieve OnInput function from game: %s", SDL_GetError());
        ZERO_STRUCT(gameServices);
        return false;
    }

    gameServices.onLogic = (Game_OnLogic *)SDL_LoadFunction(game, "OnLogic");
    if (!gameServices.onLogic)
    {
        gLog.fatal("Core", "Failed to retrieve OnLogic function from game: %s", SDL_GetError());
        ZERO_STRUCT(gameServices);
        return false;
    }

    gameServices.onRender = (Game_OnRender *)SDL_LoadFunction(game, "OnRender");
    if (!gameServices.onRender)
    {
        gLog.fatal("Core", "Failed to retrieve OnRender function from game: %s", SDL_GetError());
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
void *GameInit(PlatformServices &platformServices, GameServices &gameServices)
{
    if (!GameCopyToTemp())
    {
        return nullptr;
    }

    void *g = SDL_LoadObject(gGameLiveFullPath.c_str());
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

    if (!gameServices.onInit(&platformServices))
    {
        ZERO_STRUCT(gameServices);
        SDL_UnloadObject(g);
        g = nullptr;
        return nullptr;
    }

    return g;
}

// Returns game via SDL_LoadObject or nullptr on error.
void *GameReload(void *oldGame, PlatformServices &platformServices, GameServices &gameServices)
{
    ASSERT(oldGame);

    gameServices.onPreReload();
    ZERO_STRUCT(gameServices);

    SDL_UnloadObject(oldGame);
    oldGame = nullptr;

    if (!GameCopyToTemp())
    {
        return nullptr;
    }

    void *g = SDL_LoadObject(gGameLiveFullPath.c_str());
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

    gameServices.onPostReload(&platformServices);
    return g;
}

void GameCleanup(void *game, GameServices &gameServices)
{
    ASSERT(game);

    gameServices.onCleanup();

    ZERO_STRUCT(gameServices);
    SDL_UnloadObject(game);
    game = nullptr;

    // Check if file exists before trying to delete it to avoid a useless error message being logged.
    ResultBool r = PlatformDeleteFile(gGameLiveFullPath);
    if (!r.result)
        gLog.fatal("Core", "Failed to delete live game: %s", r.error.c_str());
}

// WARNING: SDL 2 requires this exact function signature, changing it will give "undefined reference to SDL_main" linker errors.
int main(int argc, char *argv[])
{
    char *exePathBuf = SDL_GetBasePath();
    if (!exePathBuf)
    {
        std::cerr << "Failed to get executable path: " << SDL_GetError() << std::endl;
        return 1;
    }
    std::string exePath = exePathBuf;
    SDL_free(exePathBuf);
    exePathBuf = nullptr;

    // I prefer the log sticking with the exe, but should it be in the CWD instead?
    if (!gLog.init((exePath + "\\" + PLATFORM_LOG_FILENAME).c_str()))
        return 1;

#ifdef BUILD_DEBUG
    gLog.warn("Core", "Debug build.");
#endif

    PlatformServices platformServices;
    platformServices.log = &gLog;
    platformServices.programPath = exePath;
    gLog.info("Core", "Program path: %s", platformServices.programPath.c_str());
    gGameFullPath = platformServices.programPath + GAME_FILENAME;
    gLog.info("Core", "Game fullpath: %s", gGameFullPath.c_str());
    gGameLiveFullPath = platformServices.programPath + GAME_FILENAME + ".live";
    gLog.info("Core", "Game Live fullpath: %s", gGameLiveFullPath.c_str());

    std::string cwd;
    ResultBool r = PlatformGetCWD(cwd);
    if (!r.result)
    {
        gLog.fatal("Core", "Failed to get the current working directory: %s", r.error.c_str());
        return 1;
    }
    // Prefer the CWD over programPath for releasePath, but verify the data folder exists in releasePath.
    platformServices.releasePath = cwd;
    // GetCurrentDirectoryA doesn't append a "\".
    platformServices.releasePath += "\\";
    platformServices.dataPath = platformServices.releasePath + "data\\";
    r = PlatformFolderExists(platformServices.dataPath);
    if (!r.result)
    {
        platformServices.releasePath = platformServices.programPath;
        platformServices.dataPath = platformServices.releasePath + "data\\";
        r = PlatformFolderExists(platformServices.dataPath);
        if (!r.result)
        {
            gLog.fatal("Core", "The data folder wasn't found in the current working directory (%s) or the executable directory (%s).", cwd.c_str(), platformServices.programPath.c_str());
            return 1;
        }
    }
    gLog.info("Core", "Release path: %s", platformServices.releasePath.c_str());
    gLog.info("Core", "Data path: %s", platformServices.dataPath.c_str());

    SDLWrapper sdl;
    if (!sdl.init())
        return 1;

    GameServices gameServices;
    void *game = GameInit(platformServices, gameServices);
    if (!game)
        return 1;

    game = GameReload(game, platformServices, gameServices);
    if (!game)
        return 1;



    const char *vertexShaderStr = R"string(
#version 330 core
layout (location = 0) in vec3 aPos;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
})string";
    uint32 vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderID, 1, &vertexShaderStr, nullptr);
    glCompileShader(vertexShaderID);
    int success;
    const uint32 infoLogSize = KIBIBYTES(1);
    char infoLog[infoLogSize];
    glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderID, infoLogSize, nullptr, infoLog);
        gLog.fatal("OpenGL", "Vertex Shader compiliation failed: %s", infoLog);
        return 1;
    }

    const char *fragmentShaderStr = R"string(
#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
})string";
    uint32 fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderID, 1, &fragmentShaderStr, nullptr);
    glCompileShader(fragmentShaderID);
    glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderID, infoLogSize, nullptr, infoLog);
        gLog.fatal("OpenGL", "Fragment Shader compiliation failed: %s", infoLog);
        return 1;
    }

    uint32 shaderProgramID = glCreateProgram();
    glAttachShader(shaderProgramID, vertexShaderID);
    glAttachShader(shaderProgramID, fragmentShaderID);
    glLinkProgram(shaderProgramID);
    glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgramID, infoLogSize, nullptr, infoLog);
        gLog.fatal("OpenGL", "Shader Program linking failed: %s", infoLog);
        return 1;
    }
    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);


    uint32 vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    float vertices[] = {
         0.5f,  0.5f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left
    };
    uint32 vbo;
    // TODO: Error checking.
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    uint32 indices[] = { // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };
    uint32 ebo;
    // TODO: Error checking.
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);



    SDL_Event event;
    bool quit = false;
    while (!quit)
    {
        //
        // Input
        //

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
                    {
                        SDLUpdateViewport(event.window.data1, event.window.data2);
                    }
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                    {
                        quit = true;
                        break;
                    }
                    else if (event.key.keysym.sym == SDLK_w)
                    {
                        static bool wireframe = false;
                        wireframe = !wireframe;
                        if (wireframe)
                            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                        else
                            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                        gLog.info("Game", "Set wireframe to %i", wireframe);
                    }
                    break;
                default:
                    break;
            }
        }

        if (!gameServices.onInput())
            break;

        //
        // Logic
        //

        if (!gameServices.onLogic())
            break;

        //
        // Render
        //

        if (!gameServices.onRender())
            break;

        // TODO: Check for errors.
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgramID);
        glBindVertexArray(vao);
#if 0
        glDrawArrays(GL_TRIANGLES, 0, 3);
#endif
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
        SDL_GL_SwapWindow(sdl.window);
        SDL_Delay(1);
    }

    GameCleanup(game, gameServices);
    return 0;
}
