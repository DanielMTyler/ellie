/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#include "app_interface.hpp"
#include "app.hpp"

IApp* IApp::Get()
{
    static App a;
    return &a;
}
