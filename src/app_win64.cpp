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



// @todo WinApi unicode support to support paths > MAX_PATH.
class BaseApp : public IApp
{
public:
    virtual ~BaseApp() {}
    
    virtual char PathSeparatorChar() override { return '\\'; }
    virtual std::string PathSeparator() override { return "\\"; }
    virtual std::string SharedLibPrefix() override { return ""; }
    virtual std::string SharedLibExt() override { return ".dll"; }
    
    
    
    virtual ResultBool CopyFile(std::string src, std::string dst, bool failIfExists) override
    {
        ResultBool r;
        
        if (!::CopyFile(src.c_str(), dst.c_str(), failIfExists))
        {
            std::string msg;
            DWORD e = FormattedLastError(msg);
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
    
    virtual ResultBool CreateTempFile(std::string& file) override
    {
        ResultBool r;
        
        char tempPathBuf[MAX_PATH];
        DWORD retDW = GetTempPath(MAX_PATH, tempPathBuf);
        if (retDW > MAX_PATH || !retDW)
        {
            std::string msg;
            DWORD e = FormattedLastError(msg);
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
            DWORD e = FormattedLastError(msg);
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
    
    virtual ResultBool DeleteFile(std::string file) override
    {
        ResultBool r;
        
        if (!::DeleteFile(file.c_str()))
        {
            std::string msg;
            DWORD e = FormattedLastError(msg);
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
    
    virtual ResultBool FileExists(std::string file) override
    {
        ResultBool r;
        
        if (GetFileAttributes(file.c_str()) == INVALID_FILE_ATTRIBUTES)
        {
            if (GetLastError() != ERROR_FILE_NOT_FOUND)
            {
                std::string msg;
                DWORD e = FormattedLastError(msg);
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
    
    virtual ResultBool FolderExists(std::string folder) override
    {
        ResultBool r;
        
        DWORD a = GetFileAttributes(folder.c_str());
        if (a == INVALID_FILE_ATTRIBUTES)
        {
            std::string msg;
            DWORD e = FormattedLastError(msg);
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
    
    virtual ResultBool GetCWD(std::string& cwd) override
    {
        // This could easily be cached on Init or first run, but it's probably not needed.
        
        ResultBool r;
        
        char cwdBuf[MAX_PATH];
        if (!GetCurrentDirectory(MAX_PATH, cwdBuf))
        {
            std::string msg;
            DWORD e = FormattedLastError(msg);
            if (!msg.empty())
                r.error = "GetCurrentDirectory failed: " + msg;
            else
                r.error = "GetCurrentDirectory failed: GetLastError()=" + std::to_string(e);
            
            r.result = false;
            return r;
        }
        
        cwd = cwdBuf;
        // GetCurrentDirectory doesn't add a path separator at the end.
        cwd += PathSeparator();
        r.result = true;
        return r;
    }
    
    virtual bool Init() override { return true; }
    virtual void Cleanup() override {}



private:
    // Format GetLastError() and put it into msg.
    // Return 0 if successful and the error if not.
    DWORD FormattedLastError(std::string& msg)
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
};



#include "app.cpp"
