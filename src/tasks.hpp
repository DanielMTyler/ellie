/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef TASKS_HPP_INCLUDED
#define TASKS_HPP_INCLUDED

#include "global.hpp"
#include <cstdint> // size_t
#include <list>
#include <memory>  // make_shared, shared_ptr, weak_ptr
#include <vector>



struct Task;
typedef std::shared_ptr<Task> StrongTaskPtr;
typedef std::weak_ptr<Task>   WeakTaskPtr;

struct Task
{
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
    
    State state = State::Uninitialized; // Set after OnInit by the task manager.
    StrongTaskPtr child;
    
    typedef bool (OnInit)   (StrongTaskPtr t); /// It's ok to TaskSucceed() and never reach OnUpdate().
    typedef void (OnCleanup)(StrongTaskPtr t); /// Always called, even if OnInit failed.
    typedef void (OnUpdate) (StrongTaskPtr t,
                             DeltaTime dt);    /// If not implemented, TaskSucceed() instead.
    typedef void (OnSuccess)(StrongTaskPtr t);
    typedef void (OnFail)   (StrongTaskPtr t);
    typedef void (OnAbort)  (StrongTaskPtr t);
    
    OnInit*    onInit    = nullptr; /// Must be implemented.
    OnCleanup* onCleanup = nullptr;
    OnUpdate*  onUpdate  = nullptr;
    OnSuccess* onSuccess = nullptr;
    OnFail*    onFail    = nullptr;
    OnAbort*   onAbort   = nullptr;
    
    void* userData = nullptr;
};


/// Helper method for creating a Task with assigned callbacks and userData.
StrongTaskPtr TaskCreate(Task::OnInit*    onInit,
                         Task::OnCleanup* onCleanup = nullptr,
                         Task::OnUpdate*  onUpdate  = nullptr,
                         Task::OnSuccess* onSuccess = nullptr,
                         Task::OnFail*    onFail    = nullptr,
                         Task::OnAbort*   onAbort   = nullptr,
                         void*            userData  = nullptr)
{
    StrongTaskPtr t = std::make_shared<Task>();
    t->onInit    = onInit;
    t->onCleanup = onCleanup;
    t->onUpdate  = onUpdate;
    t->onSuccess = onSuccess;
    t->onFail    = onFail;
    t->onAbort   = onAbort;
    t->userData  = userData;
    return t;
}

void TaskSucceed(StrongTaskPtr t)
{
    SDL_assert(t);
    SDL_assert(t->state == Task::State::Uninitialized || t->state == Task::State::Running || t->state == Task::State::Paused);
    t->state = Task::State::Succeeded;
}

void TaskFail(StrongTaskPtr t)
{
    SDL_assert(t);
    SDL_assert(t->state == Task::State::Running || t->state == Task::State::Paused);
    t->state = Task::State::Failed;
}

void TaskAbort(StrongTaskPtr t)
{
    SDL_assert(t);
    SDL_assert(t->state == Task::State::Running || t->state == Task::State::Paused);
    t->state = Task::State::Aborted;
}

void TaskPause(StrongTaskPtr t)
{
    SDL_assert(t);
    SDL_assert(t->state == Task::State::Running);
    t->state = Task::State::Paused;
}

void TaskUnpause(StrongTaskPtr t)
{
    SDL_assert(t);
    SDL_assert(t->state == Task::State::Paused);
    t->state = Task::State::Running;
}

bool IsTaskAlive(StrongTaskPtr t)
{
    SDL_assert(t);
    return (t->state == Task::State::Running || t->state == Task::State::Paused);
}

bool IsTaskDead (StrongTaskPtr t)
{
    SDL_assert(t);
    return (t->state == Task::State::Succeeded || t->state == Task::State::Failed || t->state == Task::State::Aborted);
}

StrongTaskPtr TaskAttachChild(StrongTaskPtr t, StrongTaskPtr child)
{
    SDL_assert(t);
    t->child = child;
    return t->child;
}

StrongTaskPtr TaskRemoveChild(StrongTaskPtr t)
{
    SDL_assert(t);
    StrongTaskPtr c = t->child;
    t->child.reset();
    return c;
}

WeakTaskPtr TaskGetChild(StrongTaskPtr t)
{
    SDL_assert(t);
    return t->child;
}



typedef std::vector<StrongTaskPtr> TaskList;
typedef std::size_t TaskCount;

struct TaskManager
{
    bool inited = false;
    std::list<StrongTaskPtr> tasks;
    TaskCount numSucceeded;
    TaskCount numFailed;
};


void TaskManInit(TaskManager& tm)
{
    SDL_assert(!tm.inited);
    
    tm.numSucceeded = 0;
    tm.numFailed    = 0;
    tm.inited = true;
}

void TaskManCleanup(TaskManager& tm)
{
    for (auto& t : tm.tasks)
    {
        if (t->onAbort)
            t->onAbort(t);
        t->state = Task::State::Aborted;
        if (t->onCleanup)
            t->onCleanup(t);
    }
    tm.tasks.clear();
    tm.inited = false;
}

StrongTaskPtr TaskManAttachTask(TaskManager& tm, StrongTaskPtr task)
{
    SDL_assert(tm.inited);
    SDL_assert(task);
    SDL_assert(task->state == Task::State::Uninitialized);
    SDL_assert(task->onInit);
    tm.tasks.push_back(task);
    return task;
}

void TaskManAttachTasks(TaskManager& tm, TaskList tasks)
{
    SDL_assert(tm.inited);
    for (auto& t : tasks)
    {
        SDL_assert(t);
        SDL_assert(t->state == Task::State::Uninitialized);
        SDL_assert(t->onInit);
        tm.tasks.push_back(t);
    }
}

void TaskManUpdate(TaskManager& tm, DeltaTime dt)
{
    SDL_assert(tm.inited);
    for (auto it = tm.tasks.begin(); it != tm.tasks.end(); it++)
    {
        StrongTaskPtr t = *it;
        // Save the iterator and increment the loop's in case we remove this task (and iterator).
        auto thisIt = it++;
        
        if (t->state == Task::State::Uninitialized)
        {
            if (t->onInit(t))
            {
                SDL_assert(t->state == Task::State::Uninitialized || t->state == Task::State::Succeeded);
                if (t->state == Task::State::Uninitialized)
                    t->state = Task::State::Running;
            }
            else
            {
                t->state = Task::State::Failed;
                tm.numFailed++;
                if (t->onCleanup)
                    t->onCleanup(t);
                tm.tasks.erase(thisIt);
                continue;
            }
        }
        
        if (t->state == Task::State::Running)
        {
            if (t->onUpdate)
                t->onUpdate(t, dt);
            else
                t->state = Task::State::Succeeded;
        }
        
        if (IsTaskDead(t))
        {
            // State can only be succeeded, failed, or aborted.
            switch (t->state)
            {
                case Task::State::Succeeded:
                {
                    tm.numSucceeded++;
                    if (t->onSuccess)
                        t->onSuccess(t);
                    StrongTaskPtr c = TaskRemoveChild(t);
                    if (c)
                        TaskManAttachTask(tm, c);
                    break;
                }
                case Task::State::Failed:
                {
                    tm.numFailed++;
                    if (t->onFail)
                        t->onFail(t);
                    break;
                }
                case Task::State::Aborted:
                {
                    tm.numFailed++;
                    if (t->onAbort)
                        t->onAbort(t);
                    break;
                }
                default:
                {
                    SDL_assert(false);
                    break;
                }
            }
            
            if (t->onCleanup)
                t->onCleanup(t);
            tm.tasks.erase(thisIt);
        }
    }
}

void TaskManAbortAndRemoveAll(TaskManager& tm)
{
    SDL_assert(tm.inited);
    for (auto& t : tm.tasks)
    {
        if (t->onAbort)
            t->onAbort(t);
        t->state = Task::State::Aborted;
        if (t->onCleanup)
            t->onCleanup(t);
    }
    tm.numFailed += tm.tasks.size();
    tm.tasks.clear();
}

TaskCount TaskManNumTasks(TaskManager& tm)
{
    SDL_assert(tm.inited);
    return tm.tasks.size();
}

TaskCount TaskManNumSucceeded(TaskManager& tm)
{
    SDL_assert(tm.inited);
    return tm.numSucceeded;
}

/// Returns the number of tasks that didn't Succeed.
TaskCount TaskManNumFailed(TaskManager& tm)
{
    SDL_assert(tm.inited);
    return tm.numFailed;
}

#endif // TASKS_HPP_INCLUDED.
