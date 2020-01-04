/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#include "platform.hpp"
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
    if (!e)
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

ResultBool PlatformCopyFile(std::string src, std::string dst, bool failIfExists)
{
    ResultBool r;
    
    if (!CopyFile(src.c_str(),dst.c_str(), failIfExists))
    {
        std::string msg;
        DWORD e = PlatformImpl::FormattedLastError(msg);
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

ResultBool PlatformDeleteFile(std::string file)
{
    ResultBool r;
    
    if (!DeleteFile(file.c_str()))
    {
        std::string msg;
        DWORD e = PlatformImpl::FormattedLastError(msg);
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

ResultBool PlatformFileExists(std::string file)
{
    ResultBool r;
    
    if (GetFileAttributes(file.c_str()) == INVALID_FILE_ATTRIBUTES)
    {
        if (GetLastError() != ERROR_FILE_NOT_FOUND)
        {
            std::string msg;
            DWORD e = PlatformImpl::FormattedLastError(msg);
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

ResultBool PlatformFolderExists(std::string folder)
{
    ResultBool r;
    
    DWORD a = GetFileAttributes(folder.c_str());
    if (a == INVALID_FILE_ATTRIBUTES)
    {
        std::string msg;
        DWORD e = PlatformImpl::FormattedLastError(msg);
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

ResultBool PlatformGetCWD(std::string& cwd)
{
    ResultBool r;
    
    char cwdBuf[MAX_PATH];
    if (!GetCurrentDirectory(MAX_PATH, cwdBuf))
    {
        std::string msg;
        DWORD e = PlatformImpl::FormattedLastError(msg);
        if (!msg.empty())
            r.error = "GetCurrentDirectory failed: " + msg;
        else
            r.error = "GetCurrentDirectory failed: GetLastError()=" + std::to_string(e);
        
        r.result = false;
        return r;
    }
    
    // GetCurrentDirectory doesn't add a path separator at the end.
    cwd = cwdBuf + std::string("\\");
    r.result = true;
    return r;
}

ResultBool PlatformCreateTempFile(std::string& filePath)
{
    ResultBool r;
    
    char tempPathBuf[MAX_PATH];
    DWORD retDW = GetTempPath(MAX_PATH, tempPathBuf);
    if (retDW > MAX_PATH || !retDW)
    {
        std::string msg;
        DWORD e = PlatformImpl::FormattedLastError(msg);
        if (!msg.empty())
            r.error = "GetTempPath failed: " + msg;
        else
            r.error = "GetTempPath failed: GetLastError()=" + std::to_string(e);
        
        r.result = false;
        return r;
    }
    
    char tempFileNameBuf[MAX_PATH];
    UINT retUI = GetTempFileName(tempPathBuf, PROJECT_NAME, 0, tempFileNameBuf);
    if (!retUI)
    {
        std::string msg;
        DWORD e = PlatformImpl::FormattedLastError(msg);
        if (!msg.empty())
            r.error = "GetTempFileName failed: " + msg;
        else
            r.error = "GetTempFileName failed: GetLastError()=" + std::to_string(e);
        
        r.result = false;
        return r;
    }
    
    filePath = tempFileNameBuf;
    r.result = true;
    return r;
}

#include "core.cpp"
