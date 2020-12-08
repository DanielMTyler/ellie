/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef VIEW_INTERFACE_HPP
#define VIEW_INTERFACE_HPP

#include "global.hpp"

class IView
{
public:
    virtual ~IView() {}

    virtual bool Init()               = 0;
    virtual void Cleanup()            = 0;
    virtual bool Update(DeltaTime dt) = 0; // Returns false when time to exit.
};

#endif // VIEW_INTERFACE_HPP
