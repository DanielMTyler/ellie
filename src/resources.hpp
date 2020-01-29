/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef RESOURCES_HPP_INCLUDED
#define RESOURCES_HPP_INCLUDED

#include "global.hpp"



bool        g_resManInited = false;
std::string g_resManDataPath;
std::string g_resManSavePath;



/// dataPath and savePath must end with a path separator.
bool ResManInit(std::string dataPath, std::string savePath)
{
    SDL_assert(!g_resManInited);
    SDL_assert(!dataPath.empty());
    SDL_assert(!savePath.empty());
    
    g_resManDataPath = dataPath;
    g_resManSavePath = savePath;
    g_resManInited = true;
    
    return true;
}

void ResManCleanup()
{
    if (!g_resManInited)
        return;
}

/// The returned SDL_RWops MUST be closed with ResManClose() instead of SDL_RWclose().
SDL_RWops* ResManOpenFile(std::string file, std::string mode)
{
    return SDL_RWFromFile(std::string(g_resManDataPath + file).c_str(), mode.c_str());
}

/// The returned SDL_RWops MUST be closed with ResManClose() instead of SDL_RWclose().
SDL_RWops* ResManOpenShader(std::string file, std::string mode)
{
    return SDL_RWFromFile(std::string(g_resManDataPath + "shaders" + PATH_SEPARATOR + file).c_str(), mode.c_str());
}

void ResManCloseFile(SDL_RWops* file)
{
    SDL_RWclose(file);
}

#endif // RESOURCES_HPP_INCLUDED.
