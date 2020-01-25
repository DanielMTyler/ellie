/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef GLOBAL_HPP_INCLUDED
#define GLOBAL_HPP_INCLUDED

#ifndef ORGANIZATION
    #error ORGANIZATION isn't defined.
#endif
#ifndef PROJECT_NAME
    #error PROJECT_NAME isn't defined.
#endif



#if defined(_WIN32) || defined(_WIN64)
    #define OS_WINDOWS 1
#elif defined(__linux__)
    #define OS_LINUX 1
#else
    #error Unknown OS.
#endif

#if defined(__clang__)
    #define COMPILER_CLANG 1
#elif defined(__GNUC__)
    #define COMPILER_GCC 1
#elif defined(__MINGW32__) || defined(__MINGW64__)
    #define COMPILER_MINGW 1
#elif defined(_MSC_VER)
    #define COMPILER_VISUAL_C
#else
    #error Unknown Compiler.
#endif

#if defined(COMPILER_CLANG) || defined(COMPILER_GCC) || defined(COMPILER_MINGW)
    #if defined(__i386__)
        #define ARCH_X86
    #elif defined(__x86_64__)
        #define ARCH_X64
    #else
        #error Unknown CPU Architecture.
    #endif
#elif defined(COMPILER_VISUAL_C)
    #if defined(_M_IX86)
        #define ARCH_X86
    #elif defined(_M_X64)
        #define ARCH_X64
    #else
        #error Unknown CPU Architecture.
    #endif
#else
    #error Unknown Compiler.
#endif



#include <cstddef> // (u)int(8/16/32/64)_t
#include <cstdint> // size_t
#include <cstring> // std::memset
#include <memory> // make_shared, shared_ptr
#include <string>  // std::string

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
    
    WARNING: glad and spdlog both include windows.h and cause the problem above.
 */
#include <SDL.h>

#ifdef NDEBUG
    #define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#else
    #define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#endif
#include <spdlog/spdlog.h>



#define KIBIBYTES(v) ((v) * 1024LL)
#define MEBIBYTES(v) (KIBIBYTES(v) * 1024LL)
#define GIBIBYTES(v) (MEBIBYTES(v) * 1024LL)

#define ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))

#define ZERO_STRUCT(s) std::memset(&(s), 0, sizeof(s))



typedef std::int8_t int8;
typedef std::int16_t int16;
typedef std::int32_t int32;
typedef std::int64_t int64;

typedef std::uint8_t uint8;
typedef std::uint16_t uint16;
typedef std::uint32_t uint32;
typedef std::uint64_t uint64;

typedef float real32;
typedef double real64;

typedef std::size_t MemoryIndex;
typedef std::size_t MemorySize;

typedef float DeltaTime;

struct ResultBool
{
    bool result;
    std::string error;
    
    explicit operator bool() const { return result; }
};

uint32 CheckedUInt64ToUInt32(uint64 v)
{
    SDL_assert_release(v <= 0xFFFFFFFF);
    uint32 r = (uint32)v;
    return r;
}

const char* TrueFalseToStr(bool b)
{
    return (b ? "True" : "False");
}

const char* OnOffToStr(bool b)
{
    return (b ? "On" : "Off");
}



class IApp {
public:
    virtual ~IApp() {}
    
    virtual char        PathSeparatorChar() = 0;
    virtual std::string PathSeparator()     = 0;
    virtual std::string SharedLibPrefix()   = 0;
    virtual std::string SharedLibExt()      = 0;
    
    virtual ResultBool CopyFile(std::string src, std::string dst, bool failIfExists) = 0;
    virtual ResultBool CreateTempFile(std::string& file) = 0;
    virtual ResultBool DeleteFile(std::string file)      = 0;
    virtual ResultBool FileExists(std::string file)      = 0;
    virtual ResultBool FolderExists(std::string folder)  = 0;
    /// cwd will end with a path separator.
    virtual ResultBool GetCWD(std::string& cwd)          = 0;
    
    /// Call Cleanup() even after failure.
    virtual bool Init()    = 0;
    virtual void Cleanup() = 0;
    /// Returns main() return code.
    virtual int  Run()     = 0;
    
    virtual std::shared_ptr<spdlog::logger> Logger() = 0;
    virtual std::string DataPath() = 0;
    virtual std::string PrefPath() = 0;
    
    /// Time Dilation is how fast time moves, e.g., 1.0f = 100% = real-time.
    virtual DeltaTime TimeDilation()             = 0;
    virtual void      TimeDilation(DeltaTime td) = 0;
};

#endif
