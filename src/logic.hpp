/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef LOGIC_HPP
#define LOGIC_HPP

#include "global.hpp"

class Logic
{
public:
    // @todo Time Dilation; modify dt to speed/slow time.
    // @todo Entity Component System (requires Event Manager).
    // @todo Command Interpreter (or just Event Manager).

    bool Init();
    void Cleanup();
    bool Update(DeltaTime dt);
};

#endif // LOGIC_HPP
