/*
    ==================================
    Copyright (C) 2021 Daniel Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef EVENT_BUS_HPP
#define EVENT_BUS_HPP

// @todo This event bus works, but it has a code smell.
//       These don't do everything I want, but they're cleaner:
//       1) https://github.com/DeveloperPaul123/eventbus
//       2) https://github.com/eXpl0it3r/PubBus

#include "global.hpp"
#include "app.hpp"

#include <functional> // bind/function/placeholders
#include <list>
#include <map>
#include <memory> // make_shared/shared_ptr/weak_ptr

//-- These are passed to EventBus::Subscribe(HERE):
#define EVENTBUS_SUB_FUNCTION(f, t)           std::bind(f, std::placeholders::_1),     t::TYPE
#define EVENTBUS_SUB_INSTANCE_MEMBER(f, i, t) std::bind(&f, i, std::placeholders::_1), t::TYPE
#define EVENTBUS_SUB_THIS_MEMBER(f, t)        EVENTBUS_SUB_INSTANCE_MEMBER(f, this, t)
//--

//-- Define a new event type:
#define EVENT_BEGIN(name, type)                    \
    class name : public IEvent                     \
    {                                              \
    public:                                        \
        static const UUID TYPE = type;             \
        UUID   Type() const { return type; }       \
        const char* Name() const { return #name; }
#define EVENT_END };
//--

class IEvent
{
public:
    virtual ~IEvent() {}

    virtual UUID        Type() const = 0;
    virtual const char* Name() const = 0;
};

typedef std::shared_ptr<IEvent> EventStrongPtr;
typedef std::weak_ptr<IEvent>   EventWeakPtr;

// @note SubscriberIDStrongPtr/WeakPtr are used to catch dead subscribers at
//       attempted usage and unsubscribe them rather than trying to use
//       invalid memory.
class EventBus
{
public:
    typedef uint32 SubscriberID;
    typedef std::shared_ptr<SubscriberID> SubscriberIDStrongPtr;
    typedef std::weak_ptr<SubscriberID> SubscriberIDWeakPtr;

    typedef void SubscriberSignature(EventStrongPtr e);
    typedef std::function<SubscriberSignature> Subscriber;

    SubscriberIDStrongPtr Subscribe(const Subscriber& subscriber, UUID type)
    {
        SubscriberIDStrongPtr sid = std::make_shared<SubscriberID>(NewSubscriberID());
        SubscriberWithIDWeakPtr_ s;
        s.s = subscriber;
        s.id = sid;
        m_subscribers[type].push_back(s);
        return sid;
    }

    void PublishNow(const EventStrongPtr& event)
    {
        auto findIt = m_subscribers.find(event->Type());
        if (findIt == m_subscribers.end())
            return;

        SubscriberList& l = findIt->second;
        auto it = l.begin();
        while (it != l.end())
        {
            auto thisIt = it++;
            SubscriberWithIDWeakPtr_& s = *thisIt;
            SubscriberIDStrongPtr sid = s.id.lock();
            if (sid)
                s.s(event);
            else
                l.erase(thisIt);
        }
    }

    void Publish(const EventStrongPtr& event)
    {
        if (m_subscribers.find(event->Type()) != m_subscribers.end())
            m_queues[m_activeQueue].push_back(event);
    }

    void Update(bool limitTime = false, DeltaTime maxMilliseconds = 0.0f)
    {
        TimeStamp startTime = App::Time();
        DeltaTime elapsedMilliseconds = 0.0f;

        Queue& q = m_queues[m_activeQueue];
        m_activeQueue++;
        if (m_activeQueue >= NUM_QUEUES)
            m_activeQueue = 0;
        m_queues[m_activeQueue].clear();

        while (!q.empty())
        {
            EventStrongPtr e = q.front();
            q.pop_front();

            auto findIt = m_subscribers.find(e->Type());
            if (findIt != m_subscribers.end())
            {
                SubscriberList& l = findIt->second;
                auto it = l.begin();
                while (it != l.end())
                {
                    auto thisIt = it++;
                    SubscriberWithIDWeakPtr_& s = *thisIt;
                    SubscriberIDStrongPtr sid = s.id.lock();
                    if (sid)
                        s.s(e);
                    else
                        l.erase(thisIt);
                }
            }

            if (limitTime && !q.empty())
            {
                elapsedMilliseconds += App::MillisecondsElapsed(startTime);
                if (elapsedMilliseconds >= maxMilliseconds)
                {
                    LogWarning("Aborting event processing; ran out of time.");
                    Queue& nextQ = m_queues[m_activeQueue];
                    while (!q.empty())
                    {
                        // reuse e
                        e = q.back();
                        q.pop_back();
                        nextQ.push_front(e);
                    }

                    break;
                }
            }
        }
    }

private:
    struct SubscriberWithIDWeakPtr_
    {
        SubscriberIDWeakPtr id;
        Subscriber s;
    };

    typedef std::list<SubscriberWithIDWeakPtr_> SubscriberList;
    typedef std::map<UUID, SubscriberList>      SubscriberMap;

    typedef std::list<EventStrongPtr> Queue;

    static const uint32 NUM_QUEUES = 2; // Must be 2+.

    SubscriberID  m_nextSubscriberID = 0;
    SubscriberMap m_subscribers;

    Queue  m_queues[NUM_QUEUES];
    uint32 m_activeQueue = 0;

    SubscriberID NewSubscriberID()
    {
        // @todo Reuse ids and actually FAIL when we're out.

        SubscriberID r = m_nextSubscriberID++;
        if (m_nextSubscriberID == 0)
            LogFatal("Ran out of subscriber ids; weirdness and crashes are likely.");
        return r;
    }
};

#endif // EVENT_BUS_HPP
