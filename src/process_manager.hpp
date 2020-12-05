/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef PROCESS_MANAGER_HPP
#define PROCESS_MANAGER_HPP

// Cooperative Multitasking from Game Coding Complete, Fourth Edition

#include "global.hpp"
#include "process.hpp"
#include <list>

class ProcessManager
{
public:
    ~ProcessManager() { m_processList.clear(); }

    void Update(DeltaTime dt)
    {
        m_lastSuccessCount = 0;
        m_lastFailCount = 0;

        auto it = m_processList.begin();
        while (it != m_processList.end())
        {
            Process::StrongPtr p = (*it);
            auto thisIt = it;
            it++;

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
                p->SetState(Process::State::Aborted);
                if (immediate)
                {
                    p->OnAbort();
                    m_processList.erase(tempIt);
                }
            }
        }
    }

    uint Count() const { return m_processList.size(); }
    uint LastSuccessCount() const { return m_lastSuccessCount; }
    uint LastFailCount() const { return m_lastFailCount; }

private:
    typedef std::list<Process::StrongPtr> ProcessList;
    ProcessList m_processList;
    uint m_lastSuccessCount;
    uint m_lastFailCount;
};

#endif // PROCESS_MANAGER_HPP
