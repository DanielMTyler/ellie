/*
    ==================================
    Copyright (C) 2021 Daniel Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef PROCESS_MANAGER_HPP
#define PROCESS_MANAGER_HPP

// Cooperative Multitasking from Game Coding Complete, Fourth Edition.

#include "global.hpp"

#include <list>
#include <memory> // shared_ptr/weak_ptr.

class Process
{
protected:
    virtual void OnInit()    { m_state = State::Running; }
    // Called by destructor.
    virtual void OnCleanup() {}
    virtual void OnUpdate(DeltaTime dt) = 0;
    virtual void OnSuccess() {}
    virtual void OnFail()    {}
    virtual void OnAbort()   {}

public:
    typedef std::shared_ptr<Process> StrongPtr;
    typedef std::weak_ptr<Process>   WeakPtr;

    enum class State
    {
        Uninitialized, // created, but not running.
        Removed, // removed, but not destroyed; this can happen when a
                 // running process is parented to another process.
        // Alive
        Running,
        Paused,
        // Dead
        Succeeded,
        Failed, // may not have initialized successfully.
        Aborted // may not have initialized at all (parent process may have failed).
    };

    Process() { m_state = State::Uninitialized; }
    virtual ~Process()
    {
        if (m_child)
        {
            if (m_child->IsAlive())
                m_child->OnAbort();
            m_child.reset();
        }

        OnCleanup();
    }

    State State()    const { return m_state; }
    bool IsAlive()   const { return (m_state == State::Running || m_state == State::Paused); }
    bool IsDead()    const { return (m_state == State::Succeeded || m_state == State::Failed || m_state == State::Aborted); }
    bool IsRemoved() const { return m_state == State::Removed; }
    bool IsPaused()  const { return m_state == State::Paused; }

    void Succeed() { m_state = State::Succeeded; }
    void Fail()    { m_state = State::Failed; }
    void Pause()   { if (m_state == State::Running) m_state = State::Paused; }
    void Unpause() { if (m_state == State::Paused)  m_state = State::Running; }

    void AttachChild(StrongPtr c)
    {
        if (m_child)
            m_child->AttachChild(c);
        else
            m_child = c;
    }

    StrongPtr RemoveChild()
    {
        if (m_child)
        {
            StrongPtr c = m_child;
            m_child.reset();
            return c;
        }
        else
        {
            return StrongPtr();
        }
    }

    bool HasChild() const { return m_child != nullptr; }
    StrongPtr PeekChild() { return m_child; }

private:
    friend class ProcessManager;

    enum State m_state;
    StrongPtr  m_child;
};

class ProcessManager
{
public:
    ~ProcessManager() { AbortAll(true); }

    void Update(DeltaTime dt)
    {
        m_lastSuccessCount = 0;
        m_lastFailCount    = 0;

        auto it = m_processList.begin();
        while (it != m_processList.end())
        {
            Process::StrongPtr p = (*it);
            auto thisIt = it++;

            if (p->State() == Process::State::Uninitialized)
                p->OnInit();

            // Ignore Process::State::Removed.

            if (p->State() == Process::State::Running)
                p->OnUpdate(dt);

            // Don't update if paused.

            if (p->IsDead())
            {
                enum Process::State s = p->State();

                if (s == Process::State::Succeeded)
                {
                    p->OnSuccess();
                    Process::StrongPtr c = p->RemoveChild();
                    if (c)
                        Attach(c);
                    else // The last child succeeded, so we can count a success.
                        m_lastSuccessCount++;
                }
                else if (s == Process::State::Failed)
                {
                    p->OnFail();
                    m_lastFailCount++;
                }
                else if (s == Process::State::Aborted)
                {
                    p->OnAbort();
                    m_lastFailCount++;
                }

                m_processList.erase(thisIt);
            }
        }
    }

    Process::WeakPtr Attach(Process::StrongPtr p)
    {
        m_processList.push_front(p);
        return Process::WeakPtr(p);
    }

    // If immediate == true, immediately call OnAbort() and destory.
    void AbortAll(bool immediate)
    {
        auto it = m_processList.begin();
        while (it != m_processList.end())
        {
            auto tempIt = it;
            it++;

            Process::StrongPtr p = *tempIt;
            if (p->IsAlive())
            {
                p->m_state = Process::State::Aborted;
                if (immediate)
                {
                    p->OnAbort();
                    m_processList.erase(tempIt);
                }
            }
        }
    }

    uint32 Count()            const { return m_processList.size(); }
    uint32 LastSuccessCount() const { return m_lastSuccessCount;   }
    uint32 LastFailCount()    const { return m_lastFailCount;      }

private:
    typedef std::list<Process::StrongPtr> ProcessList;

    ProcessList m_processList;
    uint32 m_lastSuccessCount;
    uint32 m_lastFailCount;
};

#endif // PROCESS_MANAGER_HPP
