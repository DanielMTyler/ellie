/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef SERVICES_HPP
#define SERVICES_HPP

#include "global.hpp"

//
// Core services.
//

#define CORE_SETWIREFRAME void CoreSetWireframe(bool on)
typedef void CoreSetWireframeCB(bool on);

#define CORE_CLEAR void CoreClear(uint32 mask)
typedef void CoreClearCB(uint32 mask);

struct CoreServices
{
    ILog* log;
    IMemory* memory;
    CoreSetWireframeCB* SetWireframe;
    CoreClearCB* Clear;
};

//
// Game services.
//

#define GAME_INIT bool GameInit(CoreServices* coreServices)
typedef bool GameInitCB(CoreServices* coreServices);

#define GAME_PRERELOAD bool GamePreReload()
typedef bool GamePreReloadCB();

#define GAME_POSTRELOAD bool GamePostReload(CoreServices* coreServices)
typedef bool GamePostReloadCB(CoreServices* coreServices);

#define GAME_CLEANUP void GameCleanup()
typedef void GameCleanupCB();

// @todo This probably shouldn't be accessible to Game.
#include <SDL.h>
#define GAME_EVENT bool GameEvent(SDL_Event& e, float dt)
typedef bool GameEventCB(SDL_Event& e, float dt);

#define GAME_INPUT bool GameInput(float dt)
typedef bool GameInputCB(float dt);

#define GAME_LOGIC bool GameLogic(float dt)
typedef bool GameLogicCB(float dt);

#define GAME_RENDER bool GameRender(float dt)
typedef bool GameRenderCB(float dt);

struct GameServices
{
    GameInitCB*       init;
    GamePreReloadCB*  preReload;
    GamePostReloadCB* postReload;
    GameCleanupCB*    cleanup;
    GameEventCB*      event;
    GameInputCB*      input;
    GameLogicCB*      logic;
    GameRenderCB*     render;
};

#endif
