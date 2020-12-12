/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef LOGIC_HPP
#define LOGIC_HPP

#include "global.hpp"
#include "events_old.hpp"

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

    // @todo Replace with an ECS camera entity when possible.
    void UpdateCameraVectors();

    void OnQuit(EventPtr e);

    void OnMoveCamera  (EventPtr e);
    void OnRotateCamera(EventPtr e);
    void OnZoomCamera  (EventPtr e);
};

#endif // LOGIC_HPP
