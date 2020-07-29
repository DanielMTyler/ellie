/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef PLATFORM_HPP_INCLUDED
#define PLATFORM_HPP_INCLUDED

#include "global.hpp"



// This is defined per OS.
//#define PATH_SEPARATOR "/"

// Calls AppSetError() on failure.
internal bool RetrieveCWD(std::string& cwd);

// Calls AppSetError() on failure.
internal bool FolderExists(std::string folder);

// Returns true if we're the only running instance.
// Returns false and displays a pop-up error message to the user if another instance is running.
// Returns false and calls AppSetError() on failure.
internal bool ForceSingleInstanceInit();

internal void ForceSingleInstanceCleanup();



#ifdef OS_WINDOWS
    #define PATH_SEPARATOR "\\"
    
    
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    
    
    global_variable HANDLE g_windowsSingleInstanceMutex_ = nullptr;
    
    internal bool ForceSingleInstanceInit()
    {
        SDL_assert(!g_windowsSingleInstanceMutex_);
        
        const char* name = ORGANIZATION_NAME "/" APPLICATION_NAME "/ForceSingleInstance";
        
        g_windowsSingleInstanceMutex_ = CreateMutex(nullptr, true, name);
        if (g_windowsSingleInstanceMutex_ && GetLastError() != ERROR_SUCCESS)
        {
            // GetLastError() should be ERROR_ALREADY_EXISTS || ERROR_ACCESS_DENIED.
            g_windowsSingleInstanceMutex_ = nullptr;
	    LogFatal("Another instance is already running.");
            const char* msg = "Another instance of " APPLICATION_NAME " is already running.";
            const char* title = APPLICATION_NAME " Is Already Running";
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, msg, nullptr);
            return false;
        }
        
        return true;
    }
    
    internal void ForceSingleInstanceCleanup()
    {
        if (g_windowsSingleInstanceMutex_)
        {
            ReleaseMutex(g_windowsSingleInstanceMutex_);
            g_windowsSingleInstanceMutex_ = nullptr;
        }
    }

#else
    
    #error Unknown platform.
    
#endif // OS_WINDOWS.



internal bool RetrieveCWD(std::string& cwd)
{
    std::error_code ec;
    std::filesystem::path p = std::filesystem::current_path(ec);
    if (ec)
    {
        AppLastError(ec.message());
        return false;
    }
    
    cwd = p.string() + std::filesystem::path::preferred_separator;
    return true;
}

internal bool FolderExists(std::string folder)
{
    std::error_code ec;
    if (std::filesystem::is_directory(folder, ec))
    {
        return true;
    }
    else
    {
        if (ec)
            AppSetLastError(ec.message());
        
        return false;
    }
}

#endif // PLATFORM_HPP_INCLUDED.
