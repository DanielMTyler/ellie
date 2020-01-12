/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef GLOBAL_HPP_INCLUDED
#define GLOBAL_HPP_INCLUDED

// These are used for the user preferences folder, log filename, and game shared library filename.
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



// Ensure that assert() works in all builds.
#ifdef NDEBUG
    #undef NDEBUG
    #include <cassert> // assert()
    #define NDEBUG
#else
    #include <cassert> // assert()
#endif

#include <cstddef> // (u)int(8/16/32/64)_t
#include <cstdint> // size_t
#include <cstring> // std::memset
#include <fstream> // std::ifstream
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
 */
// spdlog and glad both include windows.h and caused the problem above, so just include SDL here now.
#include <SDL.h>

#ifndef NDEBUG
    #define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#else
    #define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#endif
#include <spdlog/spdlog.h>



#define ALWAYS_ASSERT(X) assert((X))

#ifndef NDEBUG
    #define DEBUG_ASSERT(X) ALWAYS_ASSERT(X)
#else
    #define DEBUG_ASSERT(X)
#endif

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

struct ResultBool
{
    bool result;
    std::string error;
};



uint32 CheckedUInt64ToUInt32(uint64 v)
{
    DEBUG_ASSERT(v <= 0xFFFFFFFF);
    uint32 r = (uint32)v;
    return r;
}

const char* BoolToStr(bool b)
{
    return (b ? "True" : "False");
}

const char* OnOffToStr(bool b)
{
    return (b ? "On" : "Off");
}

// @todo This shouldn't throw exceptions.
// @todo Use ResultBool return with a useful error message.
// @todo Use SDL_RWops?
bool ReadFileToStr(std::string file, std::string& contents)
{
    std::ifstream in(file, std::ios::in | std::ios::binary);
    if (in)
        contents = std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    else
        return false;
    
    return true;
}

#endif
