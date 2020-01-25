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
