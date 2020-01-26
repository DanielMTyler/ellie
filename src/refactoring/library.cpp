/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef SERVICES_HPP_INCLUDED
#define SERVICES_HPP_INCLUDED

#include "global.hpp"
#include <memory> // make_shared, shared_ptr

struct CoreServices
{
    std::shared_ptr<spdlog::logger> logger;
};



#define GAME_INIT bool GameInit(CoreServices* services)
typedef bool GameInitCB(CoreServices* services);

#define GAME_PRERELOAD bool GamePreReload()
typedef bool GamePreReloadCB();

#define GAME_POSTRELOAD bool GamePostReload(CoreServices* services)
typedef bool GamePostReloadCB(CoreServices* services);

#define GAME_CLEANUP void GameCleanup()
typedef void GameCleanupCB();

struct GameServices
{
    GameInitCB*       init;
    GamePreReloadCB*  preReload;
    GamePostReloadCB* postReload;
    GameCleanupCB*    cleanup;
};

#endif



/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#include "global.hpp"
#include "services.hpp"

static std::shared_ptr<spdlog::logger> gLogger;
static AppServices* gAppServices = nullptr;

extern "C" GAME_INIT
{
    DEBUG_ASSERT(services);
    DEBUG_ASSERT(services->logger);
    gServices = services;
    gLogger = services->logger;
    spdlog::register_logger(gLogger);
    spdlog::set_default_logger(gLogger);
    SPDLOG_LOGGER_INFO(gLogger, "Game initializing.");
    SPDLOG_LOGGER_INFO(gLogger, "Game initialized.");
    return true;
}

extern "C" GAME_PRERELOAD
{
    SPDLOG_LOGGER_INFO(gLogger, "Game reloading.");
    return true;
}

extern "C" GAME_POSTRELOAD
{
    DEBUG_ASSERT(services);
    DEBUG_ASSERT(services->logger);
    gServices = services;
    gLogger = services->logger;
    spdlog::register_logger(gLogger);
    spdlog::set_default_logger(gLogger);
    SPDLOG_LOGGER_INFO(gLogger, "Game reloaded.");
    return true;
}

extern "C" GAME_CLEANUP
{
    SPDLOG_LOGGER_INFO(gLogger, "Game cleaning up.");
    SPDLOG_LOGGER_INFO(gLogger, "Game cleaned up.");
    gLogger.reset();
}

#if 0

bool GameRetrieveFunctions(void* game, GameServices& gameServices)
{
    DEBUG_ASSERT(game);
    
    gameServices.init = (GameInitCB*)SDL_LoadFunction(game, "GameInit");
    if (!gameServices.init)
    {
        // @todo gLog.fatal("Core", "Failed to retrieve GameInit function from game: %s", SDL_GetError());
        return false;
    }
    
    gameServices.preReload = (GamePreReloadCB*)SDL_LoadFunction(game, "GamePreReload");
    if (!gameServices.preReload)
    {
        // @todo gLog.fatal("Core", "Failed to retrieve GamePreReload function from game: %s", SDL_GetError());
        ZERO_STRUCT(gameServices);
        return false;
    }
    
    gameServices.postReload = (GamePostReloadCB*)SDL_LoadFunction(game, "GamePostReload");
    if (!gameServices.postReload)
    {
        // @todo gLog.fatal("Core", "Failed to retrieve GamePostReload function from game: %s", SDL_GetError());
        ZERO_STRUCT(gameServices);
        return false;
    }
    
    gameServices.cleanup = (GameCleanupCB*)SDL_LoadFunction(game, "GameCleanup");
    if (!gameServices.cleanup)
    {
        // @todo gLog.fatal("Core", "Failed to retrieve GameCleanup function from game: %s", SDL_GetError());
        ZERO_STRUCT(gameServices);
        return false;
    }
    
    return true;
}


// Copy the game to a temp file for opening; this allows the original to be replaced for live reloading.
bool GameCopyToTemp()
{
    ResultBool r = PlatformCopyFile(gGamePath, gGameLivePath, false);
    if (!r.result)
    {
        // @todo gLog.fatal("Core", "Failed to copy game to live file: %s", r.error.c_str());
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

    void* g = SDL_LoadObject(gGameLivePath.c_str());
    if (!g)
    {
        // @todo gLog.fatal("Core", "Failed to load game: %s", SDL_GetError());
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
    DEBUG_ASSERT(oldGame);

    gameServices.preReload();
    ZERO_STRUCT(gameServices);

    SDL_UnloadObject(oldGame);
    oldGame = nullptr;

    if (!GameCopyToTemp())
    {
        return nullptr;
    }

    void* g = SDL_LoadObject(gGameLivePath.c_str());
    if (!g)
    {
        // @todo gLog.fatal("Core", "Failed to load game: %s", SDL_GetError());
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
    DEBUG_ASSERT(game);

    gameServices.cleanup();

    ZERO_STRUCT(gameServices);
    SDL_UnloadObject(game);
    game = nullptr;

    // Check if file exists before trying to delete it to avoid a useless error message being logged.
    ResultBool r = PlatformDeleteFile(gGameLivePath);
    if (!r.result)
        // @todo gLog.warn("Core", "Failed to delete live game: %s", r.error.c_str());
        ;
}




#endif

