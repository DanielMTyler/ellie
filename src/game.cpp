/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#include "global.hpp"
#include "services.hpp"

static ILog* gLog = nullptr;
static CoreServices* gServices = nullptr;

extern "C" GAME_INIT
{
    gLog = services->log;
    gServices = services;
    gLog->info("Game", "Game initializing.");
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
    gLog = services->log;
    gServices = services;
    gLog->info("Game", "Game reloaded.");
    return true;
}

extern "C" GAME_CLEANUP
{
    gLog->info("Game", "Game cleaning up.");
    gLog->info("Game", "Game cleaned up.");
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
    return true;
}
