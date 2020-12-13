/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef EVENTS_HPP
#define EVENTS_HPP

#include "global.hpp"
#include "event_bus.hpp"

EVENT_BEGIN(EventMoveCamera, 0x1D9AAC2E)
    DeltaTime dt;
    bool forward;
    bool backward;
    bool left;
    bool right;

    EventMoveCamera(DeltaTime dt_, bool forward_, bool backward_, bool left_, bool right_) : dt(dt_), forward(forward_), backward(backward_), left(left_), right(right_) {}
EVENT_END

EVENT_BEGIN(EventRotateCamera, 0x28AD68FB)
    int32 xrel;
    int32 yrel;

    EventRotateCamera(int32 xrel_, int32 yrel_) : xrel(xrel_), yrel(yrel_) {}
EVENT_END

EVENT_BEGIN(EventZoomCamera, 0x474C31FD)
    bool in; // in or out?

    EventZoomCamera(bool in_) : in(in_) {}
EVENT_END

#endif // EVENTS_HPP
