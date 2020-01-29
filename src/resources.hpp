/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef RESOURCES_HPP_INCLUDED
#define RESOURCES_HPP_INCLUDED

#include "global.hpp"



bool        g_ResManInited = false;
std::string g_ResManDataPath;
std::string g_ResManSavePath;



/// dataPath and savePath must end with a path separator.
bool ResManInit(std::string dataPath, std::string savePath)
{
    SDL_assert(!g_ResManInited);
    SDL_assert(!dataPath.empty());
    SDL_assert(!savePath.empty());
    
    g_ResManDataPath = dataPath;
    g_ResManSavePath = savePath;
    g_ResManInited = true;
    
    return true;
}

void ResManCleanup()
{
    if (!g_ResManInited)
        return;
}

/// The returned SDL_RWops MUST be closed with ResManClose() instead of SDL_RWclose().
SDL_RWops* ResManOpenFile(std::string file, std::string mode)
{
    return SDL_RWFromFile(std::string(g_ResManDataPath + file).c_str(), mode.c_str());
}

/// The returned SDL_RWops MUST be closed with ResManClose() instead of SDL_RWclose().
SDL_RWops* ResManOpenShader(std::string file, std::string mode)
{
    return SDL_RWFromFile(std::string(g_ResManDataPath + "shaders" + PATH_SEPARATOR + file).c_str(), mode.c_str());
}

void ResManCloseFile(SDL_RWops* file)
{
    SDL_RWclose(file);
}

#endif // RESOURCES_HPP_INCLUDED.
