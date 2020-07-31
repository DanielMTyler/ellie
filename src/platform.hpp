/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef PLATFORM_HPP_INCLUDED
#define PLATFORM_HPP_INCLUDED

// This is defined per OS.
//#define PATH_SEPARATOR "/"

// Calls AppSetError() on failure.
internal bool RetrieveCWD(std::string& cwd);

// Calls AppSetError() on failure.
internal bool FolderExists(std::string folder);

// Returns true if we're the only running instance or false if we're not; also returns false and calls AppSetError() on failure.
internal bool CheckSingleInstanceInit();

internal void CheckSingleInstanceCleanup();



#if defined(OS_WINDOWS)
    
    #define PATH_SEPARATOR "\\"
    
    
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    
    
    global_variable HANDLE g_windowsSingleInstanceMutex_ = nullptr;
    
    internal bool CheckSingleInstanceInit()
    {
        SDL_assert(!g_windowsSingleInstanceMutex_);
        
        const char* name = ORGANIZATION_NAME "/" APPLICATION_NAME "/ForceSingleInstance";
        
        g_windowsSingleInstanceMutex_ = CreateMutex(nullptr, true, name);
        if (g_windowsSingleInstanceMutex_ && GetLastError() != ERROR_SUCCESS)
        {
            // GetLastError() should be ERROR_ALREADY_EXISTS || ERROR_ACCESS_DENIED.
            g_windowsSingleInstanceMutex_ = nullptr;
            return false;
        }
        
        return true;
    }
    
    internal void CheckSingleInstanceCleanup()
    {
        if (g_windowsSingleInstanceMutex_)
        {
            ReleaseMutex(g_windowsSingleInstanceMutex_);
            g_windowsSingleInstanceMutex_ = nullptr;
        }
    }

#elif defined(OS_LINUX)
    
    #define PATH_SEPARATOR "/"
    
    
    internal bool CheckSingleInstanceInit()
    {
        // @todo
        std::cerr << "TODO: CheckSingleInstanceInit." << std::endl;
        return true;
    }
    
    internal void CheckSingleInstanceCleanup()
    {
        // @todo
        std::cerr << "TODO: CheckSingleInstanceCleanup." << std::endl;
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
        AppSetError(ec.message());
        return false;
    }

    // std::filesystem::path::preferred_separator can be a wchar, so I prefer PATH_SEPARATOR.
    cwd = p.string() + PATH_SEPARATOR;
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
            AppSetError(ec.message());
        
        return false;
    }
}

#endif // PLATFORM_HPP_INCLUDED.
