/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

// @todo This whole file needs cleaning up and refactoring.

#ifndef EVENTS_HPP
#define EVENTS_HPP

#include "global.hpp"
#include "app.hpp"

#include <SDL.h>

#include <functional> // bind/function/placeholders
#include <list>
#include <map>
#include <memory> // shared_ptr

typedef uint32 EventType;

class IEventData
{
public:
    DeltaTime dt; // Set by EventManager when calling listeners.

    virtual ~IEventData() {}

    virtual EventType Type()      const = 0;
    virtual TimeStamp Timestamp() const = 0;
    virtual const char* Name()    const = 0;
};

typedef std::shared_ptr<IEventData> IEventDataPtr;
typedef void EventListenerDelegateSignature(IEventDataPtr event);
//typedef std::function<void(IEventDataPtr)> EventListenerDelegate;
typedef std::function<EventListenerDelegateSignature> EventListenerDelegate;

#define EVENT_BIND_MEMBER_FUNCTION(x) std::bind(&x, this, std::placeholders::_1)

class BaseEventData : public IEventData
{
public:
    BaseEventData()
    {
        m_time = App::Time();
    }

    TimeStamp Timestamp() const override { return m_time; }

private:
    TimeStamp m_time;
};

class EventData_Quit : public BaseEventData
{
public:
    //d23d065b-4425-4b6c-b9a5-bfb158119177
    static const EventType TYPE = 0xd23d065b;

    EventType Type() const override { return TYPE; }
    const char* Name() const override { return "EventData_Quit"; }
};

class EventData_MoveCamera : public BaseEventData
{
public:
    //1d9aac2e-4ca7-4ea8-819e-53808e56523c
    static const EventType TYPE = 0x1d9aac2e;

    bool f;
    bool b;
    bool l;
    bool r;

    EventData_MoveCamera(bool forward, bool backward, bool left, bool right) : f(forward), b(backward), l(left), r(right) {}
    EventType Type() const override { return TYPE; }
    const char* Name() const override { return "EventData_MoveCamera"; }
};

class EventData_RotateCamera : public BaseEventData
{
public:
    //28ad68fb-b171-470b-89a9-5964aa2ed9d1
    static const EventType TYPE = 0x28ad68fb;

    int32 xrel;
    int32 yrel;

    EventData_RotateCamera(int32 xrel, int32 yrel) : xrel(xrel), yrel(yrel) {}
    EventType Type() const override { return TYPE; }
    const char* Name() const override { return "EventData_RotateCamera"; }
};

class EventData_ZoomCamera : public BaseEventData
{
public:
    //474c31fd-54bf-44c9-a3e9-a837fa324b51
    static const EventType TYPE = 0x474c31fd;

    bool in; // in or out?

    EventData_ZoomCamera(bool in_) : in(in_) {}
    EventType Type() const override { return TYPE; }
    const char* Name() const override { return "EventData_ZoomCamera"; }
};

class EventManager
{
public:
    bool AddListener(const EventListenerDelegate& delegate, EventType type)
    {
        void (*const* t)(IEventDataPtr) = delegate.target<void(*)(IEventDataPtr)>();
        ListenerList& l = m_listeners[type];
        for (auto it = l.begin(); it != l.end(); it++)
        {
            void (*const* tThis)(IEventDataPtr) = it->target<void(*)(IEventDataPtr)>();
            if (t == tThis)
            {
                LogWarning("Tried to register an existing event delegate.");
                return false;
            }
        }

        LogDebug("Registered event delegate %p for %u.", (void*)t, type);
        l.push_back(delegate);
        return true;
    }

    void RemoveListener(const EventListenerDelegate& delegate, EventType type)
    {
        void (*const* t)(IEventDataPtr) = delegate.target<void(*)(IEventDataPtr)>();

        auto findIt = m_listeners.find(type);
        if (findIt == m_listeners.end())
            return;

        ListenerList& l = findIt->second;
        for (auto it = l.begin(); it != l.end(); it++)
        {
            void (*const* tThis)(IEventDataPtr) = it->target<void(*)(IEventDataPtr)>();
            if (t == tThis)
            {
                l.erase(it);
                LogDebug("Removed event delegate: %p.", (void*)t);
                return;
            }
        }

        LogDebug("Tried to remove non-existant event delegate: %p.", (void*)t);
    }

    // Bypass the queue and call all delegates immediately.
    void TriggerEvent(DeltaTime dt, IEventDataPtr& event) const
    {
        auto findIt = m_listeners.find(event->Type());
        if (findIt == m_listeners.end())
            return;

        const ListenerList& l = findIt->second;
        for (auto it = l.begin(); it != l.end(); it++)
        {
            EventListenerDelegate listener = (*it);
            event->dt = dt;
            listener(event);
        }
    }

    void QueueEvent(const IEventDataPtr& event)
    {
        auto findIt = m_listeners.find(event->Type());
        if (findIt != m_listeners.end())
            m_queues[m_activeQueue].push_back(event);
    }

    void AbortEvent(EventType type, bool allOfType = false)
    {
        auto findIt = m_listeners.find(type);
        if (findIt == m_listeners.end())
            return;

        Queue& q = m_queues[m_activeQueue];
        auto it = q.begin();
        while (it != q.end())
        {
            auto thisIt = it;
            it++;
            if ((*thisIt)->Type() == type)
            {
                q.erase(thisIt);
                if (!allOfType)
                    break;
            }
        }
    }

    void Update(DeltaTime dt, bool limitTime = false, DeltaTime maxMilliseconds = 0.0f)
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
            IEventDataPtr e = q.front();
            e->dt = dt;
            q.pop_front();

            EventType t = e->Type();

            auto findIt = m_listeners.find(t);
            if (findIt != m_listeners.end())
            {
                const ListenerList& listeners = findIt->second;
                for (auto it = listeners.begin(); it != listeners.end(); it++)
                {
                    EventListenerDelegate l = (*it);
                    l(e);
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

                    return;
                }
            }
        }
    }

private:
    static const uint32 NUM_QUEUES = 2;

    typedef std::list<EventListenerDelegate> ListenerList;
    typedef std::map<EventType, ListenerList> ListenerMap;
    typedef std::list<IEventDataPtr> Queue;

    ListenerMap m_listeners;
    Queue m_queues[NUM_QUEUES];
    uint32 m_activeQueue = 0;
};

// 495 UUIDs from https://www.uuidgenerator.net/; use as needed.
#if 0
79574583-797b-45bf-93c6-ae74f5c792ce
1d461bce-c233-42ab-a6c0-9ec9ee586d77
da359fa7-65a2-4382-9726-adefe98aec60
3af85c10-f7c8-4066-817d-f8d4e5fcd1bd
7f18382e-89f7-49c1-bb9e-810a7b7c9d9e
707d173b-046d-4a2e-808c-50b740e4060f
be40bb62-5f96-4bda-b2df-9bfd9107a063
8f79f15d-968e-4134-8c2e-e6154446139c
d815eda3-1aa6-411d-b148-97a023fdf112
6d0b6ac6-a872-40ab-986a-eef4afd15613
459416c1-898a-4b65-9eb4-5e8a7060a9d8
f00f6203-e43e-4ec2-9c09-b9d1ac6c0b9d
fca22e56-4b7f-4e54-8a22-5ba0a8eb93f6
0ef2b7a4-b845-441d-81eb-4a3f92cdf19e
fc9ff503-64ae-4d09-b82e-d5fbfe64f269
77609ed6-5405-4fc9-a967-6e982b1958dc
7a500da2-5685-4f58-bff5-bb7f7ef0228b
3220d743-15cb-4a05-89a2-f8df0671a6f5
630438e1-16cf-4a4e-ad58-0c983fc374dd
2dbca692-0c60-4b17-a879-a2c5e77c4638
ae977cf8-0d92-4368-ad89-7b94547b5d14
b1a95849-1f7b-46ce-a715-c84b1cabd80f
7634c927-a206-46c6-b117-6bd73e510920
43c1a262-432e-4d4f-970c-8c7e39e78657
022913d2-dd80-4408-8e50-dac077d785fd
58fea5b9-10e8-4999-b2ab-0e864c56f522
e704a67e-27d5-4b25-861b-2d5287789b27
dbc237a1-57af-49f8-931d-85a285e6cc82
73fbb1a3-8c68-49ce-8e32-2ee71a9ecc36
d33b5e09-7817-4574-a36a-5cf97db2e5ed
a7115c89-7dd4-4224-a6c4-86cb0181a606
051b92e9-d0eb-420c-9a8a-80014c75e7a5
e1aeaa1b-29b9-499f-8453-a05b80a04e41
6caf65d3-58ff-4874-8b81-c7cc274aa032
94dfdfc3-0950-4d80-833f-b567c8668cdf
ac8ababb-43d8-4be4-a857-1719e4503711
42b5ac4a-7d6e-43f7-a0b0-bee2c7c77b1b
590cc535-eeda-4825-a0ec-ee8d66ca6755
ace1f5ed-4460-4f43-8ccc-0ed1b94daf9a
3174c11d-7dd0-45b8-94b9-2086fcaf63c1
506b6a54-e292-422b-980e-d3cd801d6d27
eb34a896-3d0e-4d66-933b-f3db8c894972
6ead47d0-57ee-4312-93aa-7a2850e6c09e
9a339c60-9a55-4ba7-b40b-dafd99ede2bc
db11f79f-3571-4ad3-b1fc-81bfcd6524cb
abd09d6e-bdd7-4433-bc48-a4571536e9a3
57c1e661-2ce5-4914-a7a8-3f6d4c72199a
6397d3d9-d6f2-41ce-9f59-b58c77876822
d8d59b0e-70b5-428e-8b85-b16873b6d82c
9171620d-8c04-4afa-97ff-8ce3f8750308
6bb53832-d69c-4883-9aa5-c8909d1b6649
6b973b72-97f9-409e-9fa1-71a4aed99601
11b69a07-b443-429c-be4e-04b831d1aa7c
31ac21cf-6892-4999-8fc8-49c019b68822
b24f424d-f46b-4c22-8c28-8e79afa2eedd
c1a185cf-cc64-44e5-99f2-0ca1e2b9a8c2
3c0453ab-0c30-4414-8c90-0106b64d0c66
7d388ed4-242f-4912-b816-d170756862c2
93382bea-1269-4877-b706-e19b55cc3d58
33d20a5b-535e-40ec-a1a8-edbfba47b992
39de546b-23e8-4754-8fc2-811fdd0413d4
ffd69be8-a979-4656-941e-c102de360dbd
e8a6cbfc-8f5f-4403-9691-704ebd24a66d
88c46e21-8568-4b5c-be04-12be79249f5b
ae241dd5-174d-4129-beff-66a83c459da9
430cd06c-f4cc-457f-a362-e3c94d7d37a9
8c7a5b28-d678-4094-bfcd-8a2b3df000aa
0233fdc8-2dc2-4d0c-ac26-66f4232b6964
a5164105-f1d6-4a28-8828-2c9fd9441160
d51361d7-2fb1-4d06-b7ff-7f2a2b522a42
b70aa955-36a1-4f09-a6da-b9a1c66cb336
03c2cf62-52d4-44d6-8a0e-87cd485f7e5a
6f48d0b8-9046-4191-aa34-22869b49dec0
cceb2048-f838-4298-a26a-c9051a367a73
6523fa83-914c-4fe1-bd12-84cb49f21b7b
2f3f79d2-5952-4be1-bc33-7a886097937e
56e415f6-0187-4386-80a5-797e85b16ed6
cbeb09cf-34cb-400e-b59e-baa6f63475c1
a941c4c6-18fa-4254-9702-aa40802cd9b0
1b9f4c85-71fc-41db-8f80-c50ce861b264
62e30b61-d1e2-441b-a3dc-cec39f4b534f
06efb8a9-d277-403d-8d42-87bd8fb4d427
100a97b1-9339-4432-80c2-4c5f501460ef
66fea4fe-c3af-44d2-a261-7bc5e73dcb93
7a93746e-326e-4095-8a2d-49d7112202b9
c955fe6e-391c-46b4-b976-159bbf50091e
423fe32a-cd28-460f-9368-49fd4e92aa24
a950de23-b51b-4bdb-adf8-6cb58d770b11
fe2b4853-241d-4d3d-aebc-923357d810e9
854859e2-b94d-4a88-8e21-bc69ae9fffec
adfb6aea-f009-447a-89e1-89df68561c84
cfd2a078-9378-4d4d-a63e-97c2f1a4433f
d1b315d8-55ed-411c-99c0-9185df88152a
6b41ef2b-5dae-4579-9fc7-5885656bd569
d587400e-b659-4e48-9185-d5cef0dc5e33
a3de16cf-f2d8-465b-b1fe-8a9efc6b7f1d
e80c52b7-adfc-452a-9002-a2db264379e4
e1f395c9-aa5a-4636-b991-03fa378b6012
9bb8bdb8-7de7-471c-8ccf-fddca7d284e9
03d5485f-0ecb-4650-a015-50613a1f5dbe
0d823bac-cb2e-408a-8ae4-d59f73a62678
c245e420-4fd1-422e-8ca2-c6b39b73700e
e14772f6-c25a-45b3-9d9f-776c0b1500a6
245f2232-534d-418b-a217-6755ce85db81
ea6cd71a-b02f-4ef2-ade4-62f124b9d791
7b571ae9-588f-47d3-a768-f8a74c99a0ea
2392d16c-1bff-4734-a019-6e13f75318c2
a727688c-b734-48ad-aa0e-20eae34fb177
57147522-3365-44f1-91f8-6bf4151118cf
2fae936a-9bb8-4ca7-b6e8-75a682f8e5cc
a78c0ab4-3003-4cfd-9535-db769155a99a
26d04a28-4181-4dd3-a4e7-aaddb41c33d7
827095c6-ba26-4666-a21c-161ea43808ea
68969346-ded0-48f5-b6f2-a920851b7076
d04c3ec9-462f-4298-976c-ef40e8a74578
9c38f88d-c945-48fd-9240-8e395f39deea
788ed9bd-ff1b-4cda-ade2-3501a23aadb1
81a3bf9b-5bb6-4e07-995f-3f7320e687e3
4150bb9e-bb46-4335-ab68-a40843ceba1d
b7b89324-68d6-4831-bacc-15f91e649189
8196926d-25a4-428a-b21b-04aa816cb376
59794801-894d-40d5-9904-6a07023219cd
bbcd9667-7925-4764-91ad-a31e69921818
9ce31d12-3584-4946-9a3d-e2e4744f885e
9021ce6b-576e-4648-9fc7-75791619ce17
de6177f2-a67d-45f9-9713-e329541e97ae
6064b22a-5bc1-4440-840c-a2e8d74d5807
c62a91b1-46db-4772-85b2-d10c6b1e9267
42354446-df17-49cc-9cea-3863e2329e0f
80f1794a-d39c-45d8-8286-2251b7c79e51
bcef603e-03c3-40ba-b563-57d87fcd4904
5cece6d6-8485-4b64-9fe0-85014ffcba58
9c6f43ef-5a8e-4121-b61c-de36d0501f11
3505d90e-ebe4-489a-98b1-80299f3e5c4f
8da96ce7-ee32-4d80-a3dc-9bbe4fdc28db
fc3bc182-7e80-43ea-9f62-1710e4208bb4
995154cb-14ae-457c-bbdb-07c91ed4875e
d20c912a-ac21-497f-bdf0-9c9f072442ee
ff9b9f79-13f8-46d3-86ec-7d092319cc02
0c4eb11f-8afa-4b93-ac99-6e4a9f30e4a3
5dede38a-069d-4538-aec9-f0ddd0ede0b4
be7bf4c5-5d14-417f-bf9a-5ca6e3948c79
3127fb33-be0c-4113-a929-b5e68d5545c1
f9199344-8a44-424b-b3c1-1e5b12e62984
b17eb387-b91f-4abe-9bf5-c3088f1266ad
da1230ed-caed-4a04-9909-f216f1fdfd36
2d0fec66-165d-4b92-84c3-402a65018699
049ea5d5-04cd-4678-9856-3d9cff7c6ca2
06c4d555-95db-42eb-ab5c-7f9f021460be
1525d5af-aa23-41c4-8481-677e774d2789
73f1986b-9bcd-482a-9c12-79fd0e6a7388
b282ffeb-aeaf-453f-afc3-cc8e4f1b7ec3
5dec244e-acc2-4d71-bd40-0b995f8dd5e4
4b3a6e4c-177d-485b-a147-cb5fba5c029e
22b6364f-888e-45e0-97c0-92e66a56e70a
71af2615-63a9-4c76-a84d-f50bdac032e3
319ebfea-9bb7-49b9-b4aa-d9dd8c9d45e9
604733fd-f68f-4583-9976-e79f2c0df072
03e576d0-1546-4442-b927-0b6e9ba6f06d
3710a348-6689-4a0f-9d5b-7d3f3ecf6895
df8c0bd4-daa3-44b2-a4fe-be597a31d1eb
792cbda1-8f61-49d5-8547-895fe40f7ea2
9d854896-c0c6-48d2-b800-c6a22352ed88
ef5281d7-e569-49d4-8d1b-3659ece0c5c7
a3b24420-6933-434d-8a9b-d8aada374c64
7678a7ef-8ef2-478b-9de4-b504d33c60d0
ef262e20-4ec1-4328-bb03-a61d04865ed1
97b6c2e0-bb61-44f5-ad78-82fa00f5b6aa
465e24a5-619a-44f9-8fb2-f0f88095d1e6
4dc571bd-76cc-4ecf-9b15-7c23e5cc3c20
798791cf-14e5-4eca-843b-2ca1024c711f
377d47bb-23cf-4ea4-9bf0-2e48d9f468f6
02f7ab7b-c7a8-4742-81e9-d30babf1e80a
e392ad10-ec42-426d-8e05-417525ccd223
6f3184c9-cd3c-48b1-bfca-1e320f54cd24
0d76e56b-6bb7-4ac6-ac29-720dc0789ffc
7cbcf4dd-f342-4088-8bc2-b8855da98e8e
9d5799c0-af99-4059-8ea8-85024469ac0e
298bd34a-5cc0-43ff-9adc-862e227bc251
a9899cb9-3e3a-49a6-a603-326cda6a5718
dd951b33-2100-445d-b1ac-bc679a7702e7
f0df38fb-40dd-4811-8894-c30656d311e2
0ce941c7-0092-4368-b27e-917c23b3ecff
fdbb2abb-85b1-49ee-97d4-08f53d1c9711
ec3cdf8e-f6a5-41e6-989f-e11164f73246
058ad088-c3ca-45d8-a803-8945c5733dfb
a816f81b-67e3-4ebc-8b43-aeddabb453b4
e5ea0690-f1ec-4664-9e45-2ef5b2de1ca0
9381fa0e-d16d-4165-a8d6-8456a6e2e546
31c15270-acdd-41d2-bf08-fe2b2c1736cc
d255aab5-5ca8-4b44-8ed6-b4047d84f335
6ef8ad0d-9cde-4653-a2a2-69c1b3454130
f79340b5-e6a9-43a2-aa79-626ba5d04a3b
164750ae-6957-4b2f-892d-47370bbc6557
b3657388-53a7-4d93-8790-a18f4b549371
2f48b7a0-37f4-493a-8a7f-873872bc7c7e
418eedf6-c32d-4df7-bbf7-ac16dfda2e9c
4f926e4c-d170-4624-9ef2-017f04488aec
0ad87ee7-88da-4ad1-a886-c7ee6f0acdea
df1ee6e1-b010-4902-9516-52d3baad4726
09b21f27-286a-46b9-817b-80bc9ea190e2
226fe78c-1e14-447e-a087-29121808678a
c96140f5-d8cc-4f09-8268-978ebfb79090
7d3f8ac2-e5fe-4080-93f4-ea1d7b2626c0
e6973158-1ecc-4bac-8160-b537d2f18baf
6874ab40-2dab-4a81-9324-a63737efe1db
f48d40ef-ba8c-46b2-aa10-632d79994a9c
ae4985f4-12cc-4611-97d8-901c2eec20bc
a3dc1ed1-1965-44a7-a214-edca4490e2a5
ae333263-1dc0-4c2c-976b-57a4c7eb62ea
aeac3f3b-220f-45db-875b-2e30cf4bb52c
31ab8b27-3b01-455b-a7fe-e28148f6430c
572a7e53-3c66-4716-b67e-2c0953c05c47
d012ae87-bea1-4817-a279-cd95c2bc0ed9
a4d4252a-2423-4dbd-8760-4a80d123690f
e1add593-c1fc-4fab-b144-bc6bd24b3142
9c00cfc8-0ca9-4222-9db1-9d229e39c383
d5707201-cc0b-4466-8478-0093107fad98
1fb6996b-5427-4704-b7bc-d5e1a00467ff
2c3b66d6-96c7-410c-8efa-699a7f003df1
c3730171-3ab8-438e-be63-167615f4de41
814cf8e5-4562-4c61-99c1-61061e7d15de
dd89900b-59d0-4069-999a-2d294c59915b
f95bfec6-16d4-4e3e-941b-f15fe403489f
6825ae69-b51d-4e37-b6f7-71f6b2df60ff
b0a54a9c-e50b-4971-95b3-c60bf1d27ee2
127759e0-b356-40f2-8ba7-c89e6653b25d
fdb4632b-f978-4627-b1e0-5c382b6c1fe4
915efa6f-0eb6-4271-bbab-65c83c192be5
3c7512a3-334b-4f69-b889-3d872f62025d
4d29ff4f-a48c-475f-8058-50deec37ddbc
c844330f-4555-4abf-bbdc-b3486b6567e7
965d4a74-be3e-4b41-ac90-9cdd3d2aa554
4f917972-674c-4382-b1f0-c818a1ff9c5c
9b066e83-b03f-4c2b-a2a3-0680cb48a832
2225801c-e7ec-48bd-9b70-2c04a03b8f2e
cebedb50-a63c-4f76-94ee-1faf2330cb11
55b76301-1dd5-4002-b431-c4b4336d9726
1f0f8065-64e5-4f0e-b8aa-3a7594c61c33
c90adce9-b327-4f00-8da6-63537a04f44a
ab9698d8-69f6-474a-9546-3dd92765cafd
4122cb95-f219-4e68-bff9-eac1d8970731
6ee81c3e-8b68-4bb6-87fa-617e722ddd23
c583adee-c971-49aa-86b8-ce1f1a55c3a9
bf434622-866f-46e3-aefe-dd595a734796
0739776d-ed74-4840-bf92-67c91fbe6900
2ac302e5-e914-4d7d-b64a-ff05890a2951
fa7f1184-b27b-40e7-ad47-9bd6e97b41bd
b16e6b0f-8814-4f93-b983-10a979487750
d937e4f4-d92e-4974-b300-4e6f2f02c87b
fd0dc203-682e-4bda-b198-fb2a968f2af0
71cb5a72-307e-4434-a4f1-39d71a8a86f8
36c1d80a-5ec7-4790-b68b-0f47eb4e00c1
443e7746-3f8b-4a62-ad74-234c7c4542b1
dca13b5f-add9-467e-a5c9-7c9a53206d65
b9d1aad3-ea84-4592-a290-2276f15f689b
024280fd-44e7-45e3-a6fa-2a0c857b5636
36961945-2754-4a78-9e06-898a7d3799f8
21a6ec7b-303a-486a-9d8f-e13dec84cdb2
adb471b4-d7b0-47c0-9049-87f515c51d64
49324b06-470a-42c4-a4c0-a74e330f6c20
ff0fa0f7-5328-44de-b352-1fbb09e8eccd
7b7de735-8da8-4f6f-9017-3dd93259e7d2
87448704-b0e1-4de8-91f7-b13c9c817306
2492ff7c-ff35-4ae7-b21a-54a86ffa9152
a64e24c1-cb22-488c-ad9b-98869d4a10ea
025c1d2f-e266-4a15-a586-6b1b47be041c
b8300460-6f23-4870-8ad6-c7263b96d242
0fa90269-3598-4437-85ab-000e371a17c6
0ce0c6cd-4f92-48b2-99af-624e26d58a17
41f9c946-d635-46ff-b448-1c40d1c7323c
e8d4537d-a251-4f34-b98e-0f533add5e8e
67a78d4b-4196-41ce-9cc4-6cf1787834e2
27660382-2122-4588-9188-2d7f4fe91a47
fd23fc37-4d5e-490a-b733-37d999483a57
eb977410-b4c4-46da-87bd-2491dab62936
4f1c4779-471b-4142-b98f-3f7863ec5e77
4e2fc5ee-37a7-41ed-a408-2dbc0ed59982
a5bac489-6955-4f30-97d4-aaa46f624402
3b748c7e-09c3-477f-8f7e-74a3c9937ada
cf90f3a3-0af4-4bc1-a54a-53d6fb2fe68c
1c1be18c-1f3d-4612-9393-27ee7cb53da1
c7fffea2-b243-4b52-8749-4523fc06e9fa
6163d90c-cc57-4277-a04b-d68e38d47bd7
5bfc4451-418f-4cda-b0e6-5ce0d7815c3d
af8cc781-6cf8-4d57-89c6-bdaa2d2f71a0
da0dfefb-395a-413c-913b-e07e6954f2d6
26612112-3d3b-4673-9cd8-488ab1218e9d
2effcd36-7c5a-400b-91dc-d2d5191167fe
e34cc4e5-5ca6-4004-833d-fa33d6c00b46
271a552a-e814-494a-a728-ce53fc1145ad
0257b3fc-f7f0-43ee-8bf8-b71421f090b1
f38ca1b9-fcc6-4123-923e-fdd8a5c9c5d5
88238eb7-9091-43bf-8068-f6419bde047b
3605eab6-3d94-4323-969d-3e9806510d8b
45d07726-0b44-448a-9ac3-58a488fc5095
0465fe7d-2fd9-48ec-957e-1f2c004c9516
562e33f0-6226-4bd8-8d1b-8c4f0e438baf
dd5857c7-6f4d-4731-99c0-b0baf7343e07
00452824-6cd1-4fee-a8b8-9dbac8da33e2
c10f108c-f4d0-4270-80ea-9aeb04763361
d86b59fe-74d8-4b8a-a6dc-90c5a1161815
3dbbc5af-d0db-43f9-9660-0f342bb9645a
b5698a94-6ae9-4478-8277-70ffec3ba687
369c87dd-0e12-4f8e-b7c3-dc9d21600346
1ae3cd59-73bc-463c-baf1-0de7a892c8a5
a3f24b57-138d-4b29-aa17-b81ea3300822
d19ed03f-86a7-4d33-8b80-50c4c3a30440
a8a77495-233d-493e-9c0a-46dc2413280a
215e6c59-4811-43ce-b512-7f024d405350
3c489aff-6474-418f-98a2-fc2931aeb1bb
2288b92b-81bb-4401-bee1-1d7a55af76a5
8cf300c0-9c90-487e-88f8-82d36dd66772
69d8a8ce-bebc-48f1-b71c-4b413e1de725
b79293ea-cf6f-4186-ad26-c83ad0112d1c
4c6ddb61-e289-4687-91e3-6a1f49301901
a13e2876-cf76-4f38-9b2c-ec187659c38a
ded6e90c-9fac-4cf1-86b1-3e3e18b4f8d6
c08d9d6e-2bfd-4d0b-a513-2eba3da18fe3
8c4478cd-0916-44d2-8b61-5fe232f73dfb
9e9bfb44-a1b9-408e-bd66-102630a1047e
4de1d3ee-caf1-4e59-ad83-160102c5320b
d8803707-73ef-4776-a6c3-1e2a16a957c3
99efd698-bc4a-4515-bda5-c77e998b4c6f
57461541-df9f-45ea-8c7c-48620b6a4999
226d4807-7f03-40d2-b9f3-aa179967f89b
b4a3e146-652f-4454-bfd3-9d1957142a78
b7346e10-5612-4ca1-b77f-58aca1d38e64
44262ee6-4624-4924-aee9-5e01dac0301e
fe242fff-a736-412d-a66e-af6b4b3c6733
490ff452-0134-4a5f-a392-437b3cec8bbd
b871f01a-82ca-4fa0-845c-362b0c1d444d
abf8bf15-80e7-4f1f-b2c9-ea7aaac5d1d6
16bded96-d92d-49de-ba2b-16b051303df3
21e15c42-1d7f-418b-bc4a-025617941f59
dbbbdece-e3e2-4d18-a90a-69da496f4882
4348689d-8aef-4981-b5b3-d1ff26108261
cf865d2b-d806-4a9b-b468-49fb496fe71e
f7b7d1db-aedf-4392-ac0e-51260b0ebb39
2c6591a1-7978-4271-af59-bcf260f8737d
fa65a7af-69fc-456e-bf46-2a1ea1195949
1f300e60-653e-40fc-83c4-af53205b8253
aee1d944-a4c5-461b-a887-9f5b2502484b
3a5e4b92-fc29-4b6c-87ed-31e84cc5e9a8
8d8840ac-58af-4d79-be2f-f710ab2493fd
e892a80f-ad8e-4673-88a5-4c6525ab4672
bce22ca8-b18e-4fc9-b11a-72db4712b392
f7eba4d8-baca-4f89-9a2b-ca2717ae7a62
8a47f19c-7381-4d70-a1de-47493bdc154a
2c353fbf-cbf2-480f-902b-9559aa2fc010
a4517255-37c6-49e0-90c9-f619661d5f23
72158b43-d887-44d7-b340-f728870205af
baad948e-d228-4002-95c1-6c87f4b69903
42afb8d4-3be0-47ab-be55-fc824c6f7ae6
1a02582e-7643-49b4-8063-da07ea54cbfd
b7c9a251-5cd0-4ee2-bf21-5df45fbfc1c1
762c51a6-1cf1-4e4d-bf76-c7638d0c0b40
ef780925-5641-47ca-a78d-7a36f3932d72
0f78fc32-bf66-4bd9-a2af-2c0c5653deb5
1e163731-b128-4642-89e8-7e41a9b4ac23
197479e4-093c-4240-aea7-b9d9b6712bd8
af80267a-bc3f-421a-8fa1-c9856c5c68b9
a2f5cbba-4130-4bbd-b7ef-ce07556ed28f
0f30b4bc-f91d-4f14-a754-90524002b4dc
5214ed4a-ec9c-44c8-af68-b77d7f1acc87
69b103ca-3d13-49a8-beeb-2f8a12194396
04f3e937-d238-467f-9ab2-7c0233e8bd96
f991d2db-7d66-4229-8bd4-92c2d42e57db
f73db487-68c8-4e66-a816-11dc8323bc58
5b4d555f-13c9-4e06-a017-44e3f0022026
9436a1ef-ffda-4160-a1a9-f3f16809fcba
02892ce5-3eb9-4201-aa67-20f14ecb8b73
faa7a87c-7588-4917-98c6-8bba9f7b04b7
37dc687a-6764-4a23-9dac-3a08562c76a2
af9b87ff-5296-4178-9327-f642ae0df620
f791e551-6105-4fec-9085-50e55c6b42ea
6dc52114-e818-4b92-89fe-faeb8be5eec6
21d69d37-4f81-413d-b099-10facf779b29
269144d6-4393-49ed-b5ba-b0ad48d976a0
31102402-65ed-4576-a30e-67142ddf8b68
976bc7c1-3f33-4f78-b946-bedc2946eed9
da74d89e-c4ce-4dd9-9be7-96bd48d2c60a
a56cd80e-03b7-42d7-95b1-fa07b0480038
da0745ef-a8c6-4755-b34a-a7251ac57aaf
fb8fb4b7-6caf-4fd3-85f8-099c84d44b7b
0ca406aa-1819-4d42-8bff-4cdab1f024ab
d20af3ba-65e5-4d88-aa32-cc3535036a2d
35b5d325-8a4e-4f2c-b14b-4760b1907d54
bdd1fa7b-7f3c-448c-8253-4cd5590e33a2
016b63e2-321e-4f62-adbf-3a09a60ded44
facedf7c-c2e1-4354-b98a-288d991e2738
f50c42e4-732b-4f87-8326-1d54e3047da1
b8027e4b-24ca-47bf-ba8e-716721a058b5
f7f5b929-5241-4893-a5b9-116c6594e50a
f1358ebd-d1f0-40ca-b0a2-6a5df1dac179
3b7a84a8-78c3-4123-8a8c-4be41c7d7613
3c38df3f-166d-4417-b7ae-e2cab5bddafb
f7a82d8f-2e2f-407d-bc64-c114bb918460
c1594b4e-f6a5-436c-a49c-c9b0e641e5b4
4a7879e7-2914-4a6b-9475-02cb910ab677
f3193bd2-9583-4e81-8783-30ad6ce2000b
4ea12d8f-bd0e-4c1e-9d26-6bfb42e02dca
1ba73221-799b-401a-be00-24b0061e2c4f
6947adbc-da3a-49c3-92df-e2557135da30
417b9877-b239-439a-b4d2-4b2c98babc9b
24b895b6-126a-45eb-9e9e-2ed994f5185c
176c74b1-1858-4701-91ed-00f999e1c482
c062f1b8-fa61-44e3-9eee-76d8bd72c80f
52c19ace-fe39-4d6a-9987-3120f83f0589
7b5d4513-9317-4f49-b4a9-8c31676e7bf4
1e923e98-07f7-445e-a494-d7ea6cca85a1
f376e5ee-1e19-4b54-9dce-9c1c1d62e6db
bf9f8b01-0855-47fc-8440-f35ec9d5a03c
87e567a4-50b2-4595-90df-5bed5f979bcc
3ce0ea13-686c-4a9b-967d-f6a974563336
94c9fdbb-5830-4b76-877f-611981264a94
1b6f670e-89e7-4df1-bf02-b0f52c7c77ec
cb6d150f-a80e-470d-8e25-15a6a43a679a
8f5aae8a-9c71-46f9-b3ba-ad9293e16b11
8d6e9a1e-2211-4abb-9007-7ff18504b646
c079fd18-7b79-4514-b28a-dfd05a3ff74a
e5216912-cf87-4b21-85e8-7e7524feba1f
b45166ba-e19d-4f8a-80de-8010e5558f0b
c23390b8-4cee-45e1-a280-f6e3198efd49
aa9f2949-332f-4633-b580-a7d45aebdd26
424c9cbf-c55e-43b5-89da-96a57a5c277c
45f82f17-59ec-42ff-b4b4-65b1e7566435
5b8db1a2-0a19-429e-857b-6dd8c4929306
944ccf34-7b58-416f-9d30-a4217225ee79
f77e98f3-72a6-48d5-9b14-99f6f6c577cd
fad1c9c8-d0d6-4081-8ae8-a3b0bcceb5d3
5eeccec2-e4f7-486f-bb86-85f47ecdaf26
1d8854b9-fc8b-4f4b-b4ae-1f08656420e8
513b9af7-bd6c-4270-8725-b82b56830882
593939a3-41d6-4870-89d1-eb2012591230
45ff1da4-3da2-48a0-989a-f298faae4411
eaf05ecd-7a6a-4c7e-9884-77f4f770ca0c
2d40cd30-b5b5-4e9c-b5a4-5bad6a7f313b
280e3093-8228-4557-a0aa-4c6a881c173a
29abe611-0957-4cc7-b39d-5a04f1dc70f9
12e64ff9-aca1-4edb-97bd-28848454a54e
92d082e6-de32-4382-bd7b-2bbc8d755e06
91a4d4fe-b510-437d-a188-992e19c5b515
18ca2185-2d55-46ce-baed-02d4ba56501d
16b00c35-7014-4c1a-a5dd-48c2e0616f85
f98efb38-21ae-45b2-a981-2a717a2fd14e
528f043d-181c-473c-95b3-1381a8661f20
11e33531-a95d-4210-a5eb-a65ba8aadeac
41074265-7d99-40d0-8207-e857be2ec41f
3b9a7f7f-70bc-4004-bc9b-3e047d1eb8cf
b4261146-0fe2-4323-b651-e25d0ff2920c
989d9e9b-0687-4d43-9c6e-c60c988276f0
6bf74973-27c9-471d-b000-14d12885fdad
827ce0fe-e362-43cb-ae79-4c597d304b52
245a0f6d-a299-4a76-999d-cfec32563aea
80945f7d-26f7-4290-a893-5f1a8e286389
212aa001-1e62-49e1-85ee-2a236c565ce5
f5a65152-ad22-48f3-83cb-58a2aac33781
303f8988-fe91-48cc-a773-35925a7db39f
64779384-9612-4642-a83e-d2f61925da88
6901a3e5-6e80-4f64-a721-f8f6001d35b0
c1cac14f-65bf-4a33-afee-0088b6a538b3
47f7eff9-fe99-4fc8-912f-763bc35e683f
39385135-782c-40c4-9413-d322e1f79cc3
46b40585-0f05-497c-b5cd-94e678d17f12
0fdce284-c4f7-4f60-aa70-5fdf7993baf8
f5228eab-b73c-4985-ac28-fbd1ee477803
643b1815-f606-4239-ae30-afaab32bec51
601350b8-a416-4dd9-b7ba-c9fa5ac668b7
a0545826-54a6-40f7-8b72-0245a36f116c
017b1051-78ef-42d1-a386-ebd09f404345
47a72996-be22-466b-9e1e-4387620a209a
444e826c-e656-4609-928e-0e8f131a1f7b
952a94c1-6249-4ee7-a230-2d9bce52ba65
3a0d3f9c-d428-4161-b72c-30967f5b1245
2f15a72d-9311-4fc2-afe7-5e028ccaa821
16ef81e4-347d-4f00-aa8e-db88a89ddc20
eb78751b-3f6e-412b-b8c5-fdeda5bfe559
8fb8cf46-4504-4409-9cae-8f3717c76894
c301b079-630c-4720-b421-70717a033bdb
4b51a657-438f-4150-993f-610193ba3855
c7d91dca-0c7c-4871-be6c-b5dc19ea8e5c
b3b3d822-066d-4d5e-9390-2f6528debab3
9e16b833-96f1-474f-abad-903a980fcc87
70d8fbef-f4e1-4366-8f86-fbf606da5d01
62f43125-28bc-4b16-8649-3e09c1ada6ef
9d0f574d-0256-4498-9b7a-624862cf12f6
97812653-1a17-467b-9e54-317efad5a8e2
63a1f7d4-e88e-4a09-b446-2ad34dfa6cfc
a99fe91c-60c7-4a10-9643-dcc3fabad1a0
216b3e48-757f-405d-9233-42fb49d6782c
35ee028f-20e8-4a33-a4a3-45ae3505cc85
fb5d5ce5-c52f-4fb6-9753-2209f43b95fd
87bd93a2-cd18-4899-831c-8a3ce955c1aa
975b42bd-da14-45d6-b6d2-332fd38ea79e
#endif // 0

#endif // EVENTS_HPP
