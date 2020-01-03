/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef GLOBAL_HPP
#define GLOBAL_HPP

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
// OS & Compiler detection.
//

// @todo 32/64-bit detection?

#if defined(_WIN32) || defined(_WIN64)
    #define OS_WINDOWS 1
#elif defined(__linux__)
    #define OS_LINUX 1
#else
    #error OS is unknown.
#endif

#if defined(__clang__)
    #define COMPILER_CLANG 1
#elif defined(__GNUC__)
    #define COMPILER_GCC 1
#elif defined(__MINGW32__) || defined(__MINGW64__)
    #define COMPILER_MINGW 1
#elif defined(_MSC_VER)
    #define COMPILER_VISUAL_STUDIO
#else
    #error Compiler is unknown.
#endif

//
// Macros and defines.
//

// Clang doesn't like "*(int*)0=0" in my ASSERT macro, so enable assert() in all build types and use it instead.
#undef NDEBUG
#include <cassert> // assert
// ASSERT() is always enabled and doesn't get logged.
#define ASSERT(X) assert((X))

#define INVALID_CODE_PATH ASSERT(!"Invalid Code Path")

#define KILOBYTES(v) ((v) * 1000LL)
#define MEGABYTES(v) (KILOBYTES(v) * 1000LL)
#define GIGABYTES(v) (MEGABYTES(v) * 1000LL)

#define KIBIBYTES(v) ((v) * 1024LL)
#define MEBIBYTES(v) (KIBIBYTES(v) * 1024LL)
#define GIBIBYTES(v) (MEBIBYTES(v) * 1024LL)

#define ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))

#include <cstring> // std::memset
#define ZERO_STRUCT(s) std::memset(&(s), 0, sizeof((s)))

//
// Types
//

#include <cstddef> // (u)int(8/16/32/64)_t
#include <cstdint> // size_t

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

// Might need this later.
#if 0
    uint32 CheckedUInt64ToUInt32(uint64 value)
    {
        ASSERT(value <= 0xFFFFFFFF);
        uint32 result = (uint32)value;
        return result;
    }
#endif

//
// Platform provided services
//

class ILog
{
public:
    virtual ~ILog() {}

    /*
          Highest priority to lowest:
          - fatal
          - warn
          - info
          - debug
          - trace
    */

    virtual void fatal(const char *system, const char *format, ...) = 0;
    virtual void warn(const char *system, const char *format, ...) = 0;
    virtual void info(const char *system, const char *format, ...) = 0;
    virtual void debug(const char *system, const char *format, ...) = 0;
    virtual void trace(const char *system, const char *format, ...) = 0;
};

#include <string>

struct PlatformServices
{
    // @todo Add an event manager / message bus for messaging between systems.
    // @todo Memory allocator/pool for retained memory during game reloads.
    ILog *log;
    std::string programPath; // The folder that contains the executable.
    std::string releasePath; // The "release" folder that the executable is expected to run from outside of development.
    std::string dataPath;    // The data folder.
};

//
// Game provided services
//

// Returns false on failure.
#define GAME_ONINIT(Name) bool Name(PlatformServices *platformServices)
typedef GAME_ONINIT(Game_OnInit);

#define GAME_ONPRERELOAD(Name) void Name()
typedef GAME_ONPRERELOAD(Game_OnPreReload);

#define GAME_ONPOSTRELOAD(Name) void Name(PlatformServices *platformServices)
typedef GAME_ONPOSTRELOAD(Game_OnPostReload);

#define GAME_ONCLEANUP(Name) void Name()
typedef GAME_ONCLEANUP(Game_OnCleanup);

// Returns false on failure.
#define GAME_ONINPUT(Name) bool Name()
typedef GAME_ONINPUT(Game_OnInput);

// Returns false on failure.
#define GAME_ONLOGIC(Name) bool Name()
typedef GAME_ONLOGIC(Game_OnLogic);

// Returns false on failure.
#define GAME_ONRENDER(Name) bool Name()
typedef GAME_ONRENDER(Game_OnRender);

struct GameServices
{
    Game_OnInit *onInit;
    Game_OnPreReload *onPreReload;
    Game_OnPostReload *onPostReload;
    Game_OnCleanup *onCleanup;
    Game_OnInput *onInput;
    Game_OnLogic *onLogic;
    Game_OnRender *onRender;
};

#endif
