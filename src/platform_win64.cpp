/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#include "global.hpp"
#include <string>
/*
    WARNING: SDL.h must be included before windows.h or you'll get compile errors like this:
        In file included from ..\..\src\platform_win64.cpp:143:
        In file included from ..\..\src/core.cpp:10:
        In file included from ..\..\deps\include\SDL2\SDL.h:38:
        In file included from ..\..\deps\include\SDL2/SDL_cpuinfo.h:59:
        In file included from C:\Program Files\LLVM\lib\clang\10.0.0\include\intrin.h:12:
        In file included from C:\Program Files\mingw-w64\x86_64-8.1.0-posix-seh-rt_v6-rev0\mingw64\x86_64-w64-mingw32\include\intrin.h:41:
        C:\Program Files\mingw-w64\x86_64-8.1.0-posix-seh-rt_v6-rev0\mingw64\x86_64-w64-mingw32\include\psdk_inc/intrin-impl.h:1781:18: error: redefinition of '__builtin_ia32_xgetbv' as different kind of symbol
        unsigned __int64 _xgetbv(unsigned int);
 */
#include <SDL.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// @todo Support Unicode, at least for paths > MAX_PATH.

namespace PlatformImpl {

// Format GetLastError() and put it into msg.
// Return 0 if successful and the error if not.
DWORD FormattedLastError(std::string& msg)
{
    DWORD e = GetLastError();
    if (e == 0)
        return e;

    char *rawMsg = nullptr;
    DWORD s = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                            nullptr, e, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&rawMsg, 0, nullptr);
    if (!s)
        return e;
    
    msg = rawMsg;
    LocalFree(rawMsg);
    return e;
}

}; // namespace PlatformImpl.

bool PlatformCopyFile(std::string src, std::string dst, bool failIfExists, std::string& error)
{
    if (!CopyFile(src.c_str(),dst.c_str(), failIfExists))
    {
        std::string msg;
        DWORD e = PlatformImpl::FormattedLastError(msg);
        if (!msg.empty())
            error = "CopyFile failed: " + msg;
        else
            error = "CopyFile failed: GetLastError()=" + std::to_string(e);
        
        return false;
    }

    return true;
}

bool PlatformDeleteFile(std::string file, std::string& error)
{
    if (!DeleteFile(file.c_str()))
    {
        std::string msg;
        DWORD e = PlatformImpl::FormattedLastError(msg);
        if (!msg.empty())
            error = "DeleteFile failed: " + msg;
        else
            error = "DeleteFile failed: GetLastError()=" + std::to_string(e);
        
        return false;
    }
    
    return true;
}

// Return false if file doesn't exist or if an error was occured.
bool PlatformFileExists(std::string file, std::string& error)
{
    if (GetFileAttributes(file.c_str()) == INVALID_FILE_ATTRIBUTES)
    {
        if (GetLastError() != ERROR_FILE_NOT_FOUND)
        {
            std::string msg;
            DWORD e = PlatformImpl::FormattedLastError(msg);
            if (!msg.empty())
                error = "GetFileAttributes failed: " + msg;
            else
                error = "GetFileAttributes failed: GetLastError()=" + std::to_string(e);
        }
        
        return false;
    }
    
    return true;
}

// Return false if folder doesn't exist or if an error occured.
bool PlatformFolderExists(std::string folder, std::string& error)
{
    DWORD a = GetFileAttributes(folder.c_str());
    if (a == INVALID_FILE_ATTRIBUTES)
    {
        std::string msg;
        DWORD e = PlatformImpl::FormattedLastError(msg);
        if (!msg.empty())
            error = "GetFileAttributes failed: " + msg;
        else
            error = "GetFileAttributes failed: GetLastError()=" + std::to_string(e);
        
        return false;
    }
    else if (a & FILE_ATTRIBUTE_DIRECTORY)
    {
        return true;
    }
    
    return false;
}

// Return false if an error occured.
bool PlatformGetCWD(std::string& cwd, std::string& error)
{
    // @todo Once Unicode is supported, paths can be 32,767 WCHARs long.
    const uint32 cwdBufSize = KIBIBYTES(1);
    char cwdBuf[cwdBufSize];
    if (!GetCurrentDirectory(cwdBufSize, cwdBuf))
    {
        std::string msg;
        DWORD e = PlatformImpl::FormattedLastError(msg);
        if (!msg.empty())
            error = "GetCurrentDirectory failed: " + msg;
        else
            error = "GetCurrentDirectory failed: GetLastError()=" + std::to_string(e);
        
        return false;
    }
    
    cwd = cwdBuf;
    return true;
}

#include "core.cpp"
