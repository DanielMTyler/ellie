/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef LOGIC_HPP
#define LOGIC_HPP

#include "global.hpp"
#include "event_bus.hpp"
#include "process_manager.hpp"

class App;

class Logic
{
public:
    // @todo Time Dilation:
    //       modify dt to speed/slow time; may require adjusting App/View dt.
    // @todo Entity Component System.

    bool Init();
    void Cleanup();
    bool Update(DeltaTime dt);

private:
    App* m_app  = nullptr;
    bool m_quit = false;
    ProcessManager m_processes;

    EventBus::SubscriberIDStrongPtr m_subscriberMoveCamera;
    EventBus::SubscriberIDStrongPtr m_subscriberRotateCamera;
    EventBus::SubscriberIDStrongPtr m_subscriberZoomCamera;

    // @todo Replace with an ECS camera entity when possible.
    void UpdateCameraVectors();

    void OnMoveCamera  (EventStrongPtr e);
    void OnRotateCamera(EventStrongPtr e);
    void OnZoomCamera  (EventStrongPtr e);
};

#endif // LOGIC_HPP
