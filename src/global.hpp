/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef GLOBAL_HPP
#define GLOBAL_HPP

//
// Includes.
//

#include <cassert> // assert()
#include <cstddef> // (u)int(8/16/32/64)_t
#include <cstdint> // size_t
#include <cstring> // std::memset
#include <string>  // std::string

//
// Defines from compiler flags.
//

#if !defined(BUILD_RELEASE) && !defined(BUILD_DEBUG)
    #error Build Type not defined or not supported.
#endif

#ifndef GAME_FILENAME
    #error GAME_FILENAME not defined.
#endif

#ifndef PLATFORM_LOG_FILENAME
    #error PLATFORM_LOG_FILENAME not defined.
#endif

//
// OS, Compiler, and CPU Architecture detection.
//

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

//
// Macros and defines.
//

// Clang doesn't like "*(int*)0=0" in my ASSERT macro, so enable assert() in all build types and use it instead.
#undef NDEBUG
// ASSERT() is always enabled and doesn't get logged.
#define ASSERT(X) assert((X))

#define INVALID_CODE_PATH ASSERT(!"Invalid Code Path")

#define KIBIBYTES(v) ((v) * 1024LL)
#define MEBIBYTES(v) (KIBIBYTES(v) * 1024LL)
#define GIBIBYTES(v) (MEBIBYTES(v) * 1024LL)

#define ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))

#define ZERO_STRUCT(s) std::memset(&(s), 0, sizeof((s)))

//
// Types
//

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

uint32 CheckedUInt64ToUInt32(uint64 v)
{
    ASSERT(v <= 0xFFFFFFFF);
    uint32 r = (uint32)v;
    return r;
}

struct ResultBool
{
    bool result;
    std::string error;
};

const char* BoolToStr(bool b)
{
    return (b ? "True" : "False");
}

const char* OnOffToStr(bool b)
{
    return (b ? "On" : "Off");
}

class ILog
{
public:
    virtual ~ILog() {}
    
    virtual void fatal(const char* system, const char* format, ...) = 0;
    virtual void warn (const char* system, const char* format, ...) = 0;
    virtual void info (const char* system, const char* format, ...) = 0;
    virtual void debug(const char* system, const char* format, ...) = 0;
    virtual void trace(const char* system, const char* format, ...) = 0;
};

#endif
