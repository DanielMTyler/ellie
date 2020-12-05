/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef PROCESS_HPP
#define PROCESS_HPP

// Cooperative Multitasking from Game Coding Complete, Fourth Edition

#include "global.hpp"
#include <memory> // std::shared_ptr/weak_ptr.

class Process
{
protected:
    virtual void OnInit() { m_state = State::Running; }
    virtual void OnUpdate(DeltaTime dt) = 0;
    virtual void OnSuccess() {}
    virtual void OnFail() {}
    virtual void OnAbort() {}

public:
    typedef std::shared_ptr<Process> StrongPtr;
    typedef std::weak_ptr<Process> WeakPtr;

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
    StrongPtr m_child;

    void SetState(enum State state) { m_state = state; }
};

#endif // PROCESS_HPP
