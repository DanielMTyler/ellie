/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#include "global.hpp"
#include <string>
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

}; // namespace Platform.

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
