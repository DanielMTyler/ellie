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

struct CoreServices
{
    ILog* log;
};

//
// Game services.
//

#define GAME_INIT bool GameInit(CoreServices* services)
typedef bool GameInitCB(CoreServices* services);

#define GAME_PRERELOAD bool GamePreReload()
typedef bool GamePreReloadCB();

#define GAME_POSTRELOAD bool GamePostReload(CoreServices* services)
typedef bool GamePostReloadCB(CoreServices* services);

#define GAME_CLEANUP void GameCleanup()
typedef void GameCleanupCB();

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
    GameInputCB*      input;
    GameLogicCB*      logic;
    GameRenderCB*     render;
};

#endif
