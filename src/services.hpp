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
