/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef RESOURCES_HPP_INCLUDED
#define RESOURCES_HPP_INCLUDED

#include "global.hpp"



global_variable bool        g_resManInited = false;
global_variable std::string g_resManDataPath;
global_variable std::string g_resManSavePath;



/// dataPath and savePath must end with a path separator.
internal bool ResManInit(std::string dataPath, std::string savePath)
{
    SDL_assert(!g_resManInited);
    SDL_assert(!dataPath.empty());
    SDL_assert(!savePath.empty());
    
    g_resManDataPath = dataPath;
    g_resManSavePath = savePath;
    g_resManInited = true;
    
    return true;
}

internal void ResManCleanup()
{
    if (!g_resManInited)
        return;
}

// @todo
#if 0
/// The returned SDL_RWops MUST be closed with ResManClose() instead of SDL_RWclose().
internal SDL_RWops* ResManOpenFile(std::string file, std::string mode)
{
    return SDL_RWFromFile(std::string(g_resManDataPath + file).c_str(), mode.c_str());
}

/// The returned SDL_RWops MUST be closed with ResManClose() instead of SDL_RWclose().
internal SDL_RWops* ResManOpenShader(std::string file, std::string mode)
{
    return SDL_RWFromFile(std::string(g_resManDataPath + "shaders" + PATH_SEPARATOR + file).c_str(), mode.c_str());
}

internal void ResManCloseFile(SDL_RWops* file)
{
    SDL_RWclose(file);
}
#endif // 0

#endif // RESOURCES_HPP_INCLUDED.
