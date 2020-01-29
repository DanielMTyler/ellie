/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef PLATFORM_HPP_INCLUDED
#define PLATFORM_HPP_INCLUDED

#include "global.hpp"
#include <SDL.h>
#include <string>



/// These defines are per OS below.
// #define PATH_SEPARATOR "/"
// #define SHARED_LIBRARY_PREFIX "lib"
// #define SHARED_LIBRARY_EXTENSION "so"



// FileCopy/FileDelete avoids Windows' CopyFile/DeleteFile.
ResultBool  FileCopy      (std::string  src, std::string dst, bool failIfExists);
ResultBool  FileDelete    (std::string  file);
ResultBool  CreateTempFile(std::string& file);
ResultBool  FileExists    (std::string  file);
ResultBool  FolderExists  (std::string  folder);
ResultBool  RetrieveCWD   (std::string& cwd);

/// Returns true if we're alone OR if a failure occurs.
/// Displays an error message to the user if another instance is running.
bool VerifySingleInstanceInit();
void VerifySingleInstanceCleanup();



#ifdef OS_WINDOWS


// @todo Support paths > MAX_PATH via Unicode/path prefixes.


#define PATH_SEPARATOR "\\"
#define SHARED_LIBRARY_PREFIX ""
#define SHARED_LIBRARY_EXTENSION "dll"


#define WIN32_LEAN_AND_MEAN
#include <windows.h>


// Format GetLastError() and put it into msg.
// Returns 0 on success or the error code.
DWORD WindowsFormatLastError(std::string& msg)
{
    DWORD e = GetLastError();
    if (!e)
        return e;
    
    char* rawMsg = nullptr;
    DWORD s = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                            nullptr, e, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&rawMsg, 0, nullptr);
    if (!s)
        return e;
    
    msg = rawMsg;
    LocalFree(rawMsg);
    return e;
}


ResultBool FileCopy(std::string src, std::string dst, bool failIfExists) 
{
    ResultBool r;
    
    if (!CopyFile(src.c_str(), dst.c_str(), failIfExists))
    {
        std::string msg;
        DWORD e = WindowsFormatLastError(msg);
        if (!msg.empty())
            r.error = "CopyFile failed: " + msg;
        else
            r.error = "CopyFile failed: GetLastError()=" + std::to_string(e);
        
        r.result = false;
        return r;
    }
    
    r.result = true;
    return r;
}

ResultBool FileDelete(std::string file)
{
    ResultBool r;
    
    if (!DeleteFile(file.c_str()))
    {
        std::string msg;
        DWORD e = WindowsFormatLastError(msg);
        if (!msg.empty())
            r.error = "DeleteFile failed: " + msg;
        else
            r.error = "DeleteFile failed: GetLastError()=" + std::to_string(e);
        
        r.result = false;
        return r;
    }
    
    r.result = true;
    return r;
}

ResultBool CreateTempFile(std::string& file)
{
    ResultBool r;
    
    char tempPathBuf[MAX_PATH];
    DWORD retDW = GetTempPath(MAX_PATH, tempPathBuf);
    if (retDW > MAX_PATH || !retDW)
    {
        std::string msg;
        DWORD e = WindowsFormatLastError(msg);
        if (!msg.empty())
            r.error = "GetTempPath failed: " + msg;
        else
            r.error = "GetTempPath failed: GetLastError()=" + std::to_string(e);
        
        r.result = false;
        return r;
    }
    
    char tempFileNameBuf[MAX_PATH];
    if (!GetTempFileName(tempPathBuf, PROJECT_NAME, 0, tempFileNameBuf))
    {
        std::string msg;
        DWORD e = WindowsFormatLastError(msg);
        if (!msg.empty())
            r.error = "GetTempFileName failed: " + msg;
        else
            r.error = "GetTempFileName failed: GetLastError()=" + std::to_string(e);
        
        r.result = false;
        return r;
    }
    
    file = tempFileNameBuf;
    r.result = true;
    return r;
}


ResultBool FileExists(std::string file)
{
    ResultBool r;
    
    if (GetFileAttributes(file.c_str()) == INVALID_FILE_ATTRIBUTES)
    {
        if (GetLastError() != ERROR_FILE_NOT_FOUND)
        {
            std::string msg;
            DWORD e = WindowsFormatLastError(msg);
            if (!msg.empty())
                r.error = "GetFileAttributes failed: " + msg;
            else
                r.error = "GetFileAttributes failed: GetLastError()=" + std::to_string(e);
        }
        
        r.result = false;
        return r;
    }
    
    r.result = true;
    return r;
}

ResultBool FolderExists(std::string folder)
{
    ResultBool r;
    
    DWORD a = GetFileAttributes(folder.c_str());
    if (a == INVALID_FILE_ATTRIBUTES)
    {
        std::string msg;
        DWORD e = WindowsFormatLastError(msg);
        if (!msg.empty())
            r.error = "GetFileAttributes failed: " + msg;
        else
            r.error = "GetFileAttributes failed: GetLastError()=" + std::to_string(e);
        
        r.result = false;
        return r;
    }
    else if (a & FILE_ATTRIBUTE_DIRECTORY)
    {
        r.result = true;
        return r;
    }
    
    r.result = false;
    return r;
}

ResultBool RetrieveCWD(std::string& cwd)
{
    ResultBool r;
    
    char cwdBuf[MAX_PATH];
    if (!GetCurrentDirectory(MAX_PATH, cwdBuf))
    {
        std::string msg;
        DWORD e = WindowsFormatLastError(msg);
        if (!msg.empty())
            r.error = "GetCurrentDirectory failed: " + msg;
        else
            r.error = "GetCurrentDirectory failed: GetLastError()=" + std::to_string(e);
        
        r.result = false;
        return r;
    }
    
    cwd = cwdBuf;
    // GetCurrentDirectory doesn't add a path separator at the end.
    cwd += PATH_SEPARATOR;
    r.result = true;
    return r;
}


HANDLE g_windowsSingleInstanceMutex = nullptr;

bool VerifySingleInstanceInit()
{
    SDL_assert(!g_windowsSingleInstanceMutex);
    
    const char* name = ORGANIZATION "/" PROJECT_NAME "/VerifySingleInstance";
    
    g_windowsSingleInstanceMutex = CreateMutex(nullptr, true, name);
    if (g_windowsSingleInstanceMutex && GetLastError() != ERROR_SUCCESS)
    {
        // GetLastError() should be ERROR_ALREADY_EXISTS || ERROR_ACCESS_DENIED.
        g_windowsSingleInstanceMutex = nullptr;
        const char* msg = "Another instance of " PROJECT_NAME " is already running.";
        const char* title = PROJECT_NAME " Is Already Running";
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, msg, nullptr);
        std::cerr << msg << std::endl;
        return false;
    }
    
    return true;
}

void VerifySingleInstanceCleanup()
{
    if (g_windowsSingleInstanceMutex)
    {
        ReleaseMutex(g_windowsSingleInstanceMutex);
        g_windowsSingleInstanceMutex = nullptr;
    }
}



#else
    #error Unknown platform.
#endif // OS_WINDOWS.

#endif // PLATFORM_HPP_INCLUDED.
