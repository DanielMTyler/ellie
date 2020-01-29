/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef TASKS_HPP_INCLUDED
#define TASKS_HPP_INCLUDED



// @todo Replace Task with a struct of callbacks where nullptr indicates non-use.
//       Probably need a userdata ptr for saved data.
// @todo Replace TaskManager with a group of functions that work on a list of Task structs.



#include "global.hpp"
#include <cstdint> // size_t
#include <list>    // list
#include <memory>  // make_shared, shared_ptr, weak_ptr



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
    
    State GetState() const { return m_state; }
    bool IsAlive() const { return (m_state == State::Running || m_state == State::Paused); }
    bool IsDead() const { return (m_state == State::Succeeded || m_state == State::Failed || m_state == State::Aborted); }
    bool IsPaused() const { return m_state == State::Paused; }
    
    
    // Returns StrongPtr to allow easy chaining, e.g., AttachChild(1)->AttachChild(2).
    StrongPtr AttachChild(StrongPtr child) { m_child = child; return m_child; }
    StrongPtr RemoveChild() { StrongPtr c = m_child; m_child.reset(); return c; }
    WeakPtr   GetChild()    { return m_child; }
    
    
protected:
    /// Override if desired, but don't call Task::OnInit.
    /// It's ok to Succeed() and never reach OnUpdate().
    virtual bool OnInit() { return true; }
    
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
    
    State m_state = State::Uninitialized; // Set after OnInit by TaskManager.
    StrongPtr m_child;
};


class TaskManager
{
public:
    typedef std::size_t TaskCount;
    
    
    void Init()
    {
        SDL_assert(!m_init);
        
        m_init = true;
        m_numSucceeded = 0;
        m_numFailed    = 0;
    }
    
    void Cleanup()
    {
        for (auto& t : m_tasks)
        {
            t->OnAbort();
            t->m_state = Task::State::Aborted;
            t->OnCleanup();
        }
        m_tasks.clear();
        m_numSucceeded = 0;
        m_numFailed    = 0;
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
                    m_numFailed++;
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
                        m_numSucceeded++;
                        t->OnSuccess();
                        Task::StrongPtr c = t->RemoveChild();
                        if (c)
                            AttachTask(c);
                        break;
                    }
                    case Task::State::Failed:
                    {
                        m_numFailed++;
                        t->OnFail();
                        break;
                    }
                    case Task::State::Aborted:
                    {
                        m_numFailed++;
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
    
    
    // Returns a StrongPtr to allow easy chaining, e.g., AttachTask(t)->AttachChild(1)->AttachChild(2).
    Task::StrongPtr AttachTask(Task::StrongPtr task)
    {
        SDL_assert(m_init);
        SDL_assert(task);
        SDL_assert(task->m_state == Task::State::Uninitialized);
        m_tasks.push_back(task);
        return task;
    }
    
    void AbortAndRemoveAll()
    {
        SDL_assert(m_init);
        for (auto& t : m_tasks)
        {
            t->OnAbort();
            t->m_state = Task::State::Aborted;
            t->OnCleanup();
        }
        m_numFailed += m_tasks.size();
        m_tasks.clear();
    }
    
    TaskCount GetCount() const
    {
        SDL_assert(m_init);
        return m_tasks.size();
    }
    
    TaskCount GetSucceeded() const
    {
        SDL_assert(m_init);
        return m_numSucceeded;
    }
    
    /// Returns the number of tasks that didn't Succeed.
    TaskCount GetFailed() const
    {
        SDL_assert(m_init);
        return m_numFailed;
    }
    
    
private:
    typedef std::list<Task::StrongPtr> TaskList;
    bool m_init = false;
    TaskList m_tasks;
    TaskCount m_numSucceeded;
    TaskCount m_numFailed;
};

#endif // TASKS_HPP_INCLUDED.
