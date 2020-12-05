/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#if defined(_WIN32) || defined(_WIN64)
    #define OS_WINDOWS 1
    #define PATH_SEPARATOR "\\"
#elif defined(__linux__)
    #define OS_LINUX 1
    #define PATH_SEPARATOR "/"
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


#define KIBIBYTES(v) ((v) * 1024LL)
#define MEBIBYTES(v) (KIBIBYTES(v) * 1024LL)
#define GIBIBYTES(v) (MEBIBYTES(v) * 1024LL)

#define ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))

#include <cstring> // memset
#define ZERO_STRUCT(s) std::memset(&(s), 0, sizeof(s))


#include <cstdint> // std::(u)int(8/16/32/64)_t
typedef std::int8_t int8;
typedef std::int16_t int16;
typedef std::int32_t int32;
typedef std::int64_t int64;
typedef std::uint8_t uint8;
typedef std::uint16_t uint16;
typedef std::uint32_t uint32;
typedef std::uint64_t uint64;
typedef float float32;
typedef double float64;

typedef float32 DeltaTime;

#define global_variable static


inline const char* TrueFalseBoolToStr(bool b)
{
    return (b ? "True" : "False");
}

inline const char* OnOffBoolToStr(bool b)
{
    return (b ? "On" : "Off");
}

inline const char* YesNoBoolToStr(bool b)
{
    return (b ? "Yes" : "No");
}


#include "SDL.h"
// @note SDL logging works without initialization, so it can be used anytime :).
#define LogInfo(...)    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)
#define LogWarning(...) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)
#define LogDebug(...)   SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)
#define LogFatal(...)   SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)

#endif // GLOBAL_HPP
