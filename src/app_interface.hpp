/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef IAPP_HPP
#define IAPP_HPP

#include "global.hpp"
#include <string>

class IApp {
public:
    // @todo Game Logic.
    // @todo Game Logic: Time Dilation; modify dt to speed/slow time.
    // @todo Game Logic: Entity Component System (requires Event Manager).
    // @todo Event Manager.
    // @todo Memory Manager.

    virtual ~IApp() {}

    static IApp* Get();

    // @todo IView* View() = 0;

    virtual std::string SavePath() const = 0;
    virtual std::string DataPath() const = 0;
    virtual std::string ExecutablePath() const = 0;
    virtual std::string CWD() const = 0;

    virtual bool FolderExists(std::string folder) const = 0;
    virtual bool LoadFile(std::string file, std::string& contents) const = 0;

    virtual bool Init()    = 0;
    virtual void Cleanup() = 0;
    virtual int  Loop()    = 0;
};

#endif // IAPP_HPP
