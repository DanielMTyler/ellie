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
#include <cstdlib> // malloc, calloc, free
#include <cstring> // memset
#include <list>    // list
#include <memory>  // make_shared, shared_ptr, weak_ptr
#include <new>     // nothrow
#include <string>  // string

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

// Memory macros to allow easier tracking in the future.
#define ELLIE_NEW new
#define ELLIE_DELETE delete
#define ELLIE_MALLOC std::malloc
#define ELLIE_CALLOC std::calloc
#define ELLIE_FREE std::free



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

typedef std::shared_ptr<spdlog::logger> StrongLoggerPtr;
typedef std::weak_ptr<spdlog::logger>   WeakLoggerPtr;

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



// Cooperative multitasking (inspired by Game Coding Complete 4e):



class IApp;


class Task
{
public:
    typedef std::shared_ptr<Task> StrongPtr;
    typedef std::weak_ptr<Task> WeakPtr;
    
    
    enum class State
    {
    // Not alive or dead:
        Uninitialized,
    // Alive:
        Running,
        Paused,
    // Dead:
        Succeeded,
        Failed,
        Aborted
    };
    
    
    virtual ~Task() {}
    
    
    void Succeed()
    {
        SDL_assert(m_state == State::Uninitialized || m_state == State::Running || m_state == State::Paused);
        m_state = State::Succeeded;
    }
    
    void Fail()
    {
        SDL_assert(m_state == State::Running || m_state == State::Paused);
        m_state = State::Failed;
    }
    
    void Abort()
    {
        SDL_assert(m_state == State::Running || m_state == State::Paused);
        m_state = State::Aborted;
    }
    
    void Pause()
    {
        SDL_assert(m_state == State::Running);
        m_state = State::Paused;
    }
    
    void Unpause()
    {
        SDL_assert(m_state == State::Paused);
        m_state = State::Running;
    }
    
    IApp& GetApp() { return *m_app; }
    State GetState() const { return m_state; }
    bool IsAlive() const { return (m_state == State::Running || m_state == State::Paused); }
    bool IsDead() const { return (m_state == State::Succeeded || m_state == State::Failed || m_state == State::Aborted); }
    bool IsPaused() const { return m_state == State::Paused; }
    
    
    WeakPtr AttachChild(StrongPtr child) { m_child = child; return m_child; }
    StrongPtr RemoveChild() { StrongPtr c = m_child; m_child.reset(); return c; }
    WeakPtr GetChild() { return m_child; }
    
    
protected:
    /// Override if desired, but don't call Task::OnInit.
    /// It's ok to Succeed() and never reach OnUpdate().
    virtual ResultBool OnInit() { ResultBool r; r.result = true; return r; }
    
    /// Override if desired. Always called, even if OnInit failed.
    virtual void OnCleanup() {}
    
    /// Override if desired. Default implementation will Succeed() immediately.
    virtual void OnUpdate(DeltaTime dt) { Succeed(); }
    
    /// Override if desired.
    virtual void OnSuccess() {}
    
    /// Override if desired.
    virtual void OnFail() {}
    
    /// Override if desired.
    virtual void OnAbort() {}
    
    
private:
    friend class TaskManager;
    
    IApp* m_app   = nullptr; // Set by TaskManager before OnInit.
    State m_state = State::Uninitialized; // Set after OnInit by TaskManager.
    StrongPtr m_child;
};


class TaskManager
{
public:
    typedef std::size_t TaskCount;
    
    
    ResultBool Init(IApp* app)
    {
        SDL_assert(!m_init);
        SDL_assert(app);
        
        m_app = app;
        m_init = true;
        
        ResultBool r;
        r.result = true;
        return r;
    }
    
    void Cleanup()
    {
        for (auto it = m_tasks.begin(); it != m_tasks.end(); it++)
        {
            Task::StrongPtr t = *it;
            t->OnAbort();
            t->m_state = Task::State::Aborted;
            t->OnCleanup();
        }
        m_tasks.clear();
        m_app = nullptr;
        m_init = false;
    }
    
    
    void Update(DeltaTime dt)
    {
        SDL_assert(m_init);
        for (auto it = m_tasks.begin(); it != m_tasks.end(); it++)
        {
            Task::StrongPtr t = *it;
            // Save the iterator and increment the loop's in case we remove this task (and iterator).
            auto thisIt = it++;
            
            if (t->m_state == Task::State::Uninitialized)
            {
                if (t->OnInit())
                {
                    SDL_assert(t->m_state == Task::State::Uninitialized || t->m_state == Task::State::Succeeded);
                    if (t->m_state == Task::State::Uninitialized)
                        t->m_state = Task::State::Running;
                }
                else
                {
                    t->m_state = Task::State::Failed;
                    t->OnCleanup();
                    m_tasks.erase(thisIt);
                    continue;
                }
            }
            
            if (t->m_state == Task::State::Running)
                t->OnUpdate(dt);
            
            if (t->IsDead())
            {
                // State can only be succeeded, failed, or aborted.
                switch (t->m_state)
                {
                    case Task::State::Succeeded:
                    {
                        t->OnSuccess();
                        Task::StrongPtr c = t->RemoveChild();
                        if (c)
                            AttachTask(c);
                        break;
                    }
                    case Task::State::Failed:
                    {
                        t->OnFail();
                        break;
                    }
                    case Task::State::Aborted:
                    {
                        t->OnAbort();
                        break;
                    }
                    default:
                    {
                        SDL_assert(false);
                        break;
                    }
                }
                
                t->OnCleanup();
                m_tasks.erase(thisIt);
            }
        }
    }
    
    
    Task::WeakPtr AttachTask(Task::StrongPtr task)
    {
        SDL_assert(m_init);
        SDL_assert(task);
        SDL_assert(task->m_state == Task::State::Uninitialized);
        task->m_app = m_app;
        m_tasks.push_back(task);
        return task;
    }
    
    void AbortAndRemoveAll()
    {
        SDL_assert(m_init);
        for (auto it = m_tasks.begin(); it != m_tasks.end(); it++)
        {
            Task::StrongPtr t = *it;
            t->OnAbort();
            t->m_state = Task::State::Aborted;
            t->OnCleanup();
        }
        m_tasks.clear();
    }
    
    TaskCount GetSize() const
    {
        SDL_assert(m_init);
        return m_tasks.size();
    }
    
    
private:
    typedef std::list<Task::StrongPtr> TaskList;
    bool m_init = false;
    IApp* m_app;
    TaskList m_tasks;
};



// Interfaces:



class IApp {
public:
    virtual ~IApp() {}
    
    virtual char        GetPathSeparatorChar() = 0;
    virtual std::string GetPathSeparator()     = 0;
    virtual std::string GetSharedLibPrefix()   = 0;
    virtual std::string GetSharedLibExt()      = 0;
    
    virtual ResultBool CopyFile(std::string src, std::string dst, bool failIfExists) = 0;
    virtual ResultBool CreateTempFile(std::string& file) = 0;
    virtual ResultBool DeleteFile(std::string file)      = 0;
    virtual ResultBool FileExists(std::string file)      = 0;
    virtual ResultBool FolderExists(std::string folder)  = 0;
    /// cwd will end with a path separator.
    virtual ResultBool GetCWD(std::string& cwd)          = 0;
    
    virtual bool Init()    = 0;
    /// Always call, even if Init fails.
    virtual void Cleanup() = 0;
    /// Returns main() return code.
    virtual int  Run()     = 0;
    
    virtual StrongLoggerPtr GetLogger()   = 0;
    virtual std::string     GetDataPath() = 0;
    virtual std::string     GetPrefPath() = 0;
    virtual TaskManager& GetTaskManager() = 0;
    
    /// Time Dilation is how fast time moves, e.g., 1.0f = 100% = real-time.
    virtual DeltaTime GetTimeDilation()             = 0;
    virtual void      SetTimeDilation(DeltaTime td) = 0;
};

#endif
