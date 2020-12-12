/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef LOGIC_HPP
#define LOGIC_HPP

#include "global.hpp"
#include "events.hpp"

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

    void OnQuit(IEventDataPtr e);

    void OnMoveCamera(IEventDataPtr e);
    void OnRotateCamera(IEventDataPtr e);
    void OnZoomCamera(IEventDataPtr e);
};

#endif // LOGIC_HPP
