/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#include "global.hpp"

static ILog *gLog = nullptr;
static CoreServices *gCoreServices = nullptr;

extern "C" GAME_ONINIT(OnInit)
{
    gLog = coreServices->log;
    gCoreServices = coreServices;
    gLog->info("Game", "Game initializing.");
    gLog->info("Game", "Game initialized.");
    return true;
}

extern "C" GAME_ONPRERELOAD(OnPreReload)
{
    gLog->info("Game", "Game reloading.");
}

extern "C" GAME_ONPOSTRELOAD(OnPostReload)
{
    gLog = coreServices->log;
    gCoreServices = coreServices;
    gLog->info("Game", "Game reloaded.");
}

extern "C" GAME_ONCLEANUP(OnCleanup)
{
    gLog->info("Game", "Game cleaning up.");
    gLog->info("Game", "Game cleaned up.");
}

extern "C" GAME_ONINPUT(OnInput)
{
    // @todo
    return true;
}

extern "C" GAME_ONLOGIC(OnLogic)
{
    // @todo
    return true;
}

extern "C" GAME_ONRENDER(OnRender)
{
    // @todo
    return true;
}
