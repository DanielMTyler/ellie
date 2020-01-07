/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#include "global.hpp"
#include <SDL.h>
#include <glad/glad.h>
#include "services.hpp"

static ILog* gLog = nullptr;
static CoreServices* gCoreServices = nullptr;

struct Vars
{
    bool wireframe;
};

static Vars* gVars = nullptr;

void InitVars()
{
    gVars->wireframe = false;
}

extern "C" GAME_INIT
{
    gLog = coreServices->log;
    gCoreServices = coreServices;
    gLog->info("Game", "Game initializing.");
    
    gVars = (Vars*)gCoreServices->memory->allocate("vars", sizeof(Vars));
    if (!gVars)
    {
        gLog->fatal("Game", "Failed to allocate memory for vars.");
        return false;
    }
    InitVars();

    /* @todo
    testData.sdl = &sdl;
    
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
        return 1;
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
    */
    
    gLog->info("Game", "Game initialized.");
    return true;
}

extern "C" GAME_PRERELOAD
{
    gLog->info("Game", "Game reloading.");
    return true;
}

extern "C" GAME_POSTRELOAD
{
    gLog = coreServices->log;
    gCoreServices = coreServices;
    gVars = (Vars*)coreServices->memory->get("vars");
    gLog->info("Game", "Game reloaded.");
    return true;
}

extern "C" GAME_CLEANUP
{
    gLog->info("Game", "Game cleaning up.");
    gCoreServices->memory->release("vars");
    gVars = nullptr;
    gLog->info("Game", "Game cleaned up.");
}

extern "C" GAME_EVENT
{
    switch (e.type)
    {
        case SDL_KEYDOWN:
            if (e.key.keysym.sym == SDLK_ESCAPE)
            {
                return false;
            }
            else if (e.key.keysym.sym == SDLK_w)
            {
                gVars->wireframe = !gVars->wireframe;
                gCoreServices->SetWireframe(gVars->wireframe);
                gLog->info("Test", "Set wireframe to %s.", OnOffToStr(gVars->wireframe));
            }
            
            break;
        default:
            break;
    }
    
    return true;
}

extern "C" GAME_INPUT
{
    return true;
}

extern "C" GAME_LOGIC
{
    return true;
}

extern "C" GAME_RENDER
{
    // @todo Check for errors.
    gCoreServices->Clear(GL_COLOR_BUFFER_BIT);
    /* @todo
    glUseProgram(testData.shaderProgramID);
    glBindVertexArray(testData.vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
    */
    
    return true;
}
