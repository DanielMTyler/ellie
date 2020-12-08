/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef APP_HPP
#define APP_HPP

#include "global.hpp"
#include "SDL.h"
#include <string>
#include "view.hpp"

class App {
public:
    // @todo Game Logic.
    // @todo Game Logic: Time Dilation; modify dt to speed/slow time.
    // @todo Game Logic: Entity Component System (requires Event Manager).
    // @todo Event Manager.
    // @todo Memory Manager.

    // These are used to name the saves folder among other things, so ASCII without spaces is probably best.
    const char* ORGANIZATION_NAME = "DanielMTyler";
    const char* APPLICATION_NAME  = "Ellie";

    View m_view;

    std::string SavePath() const { return m_savePath; }
    std::string DataPath() const { return m_dataPath; }
    std::string ExecutablePath() const { return m_executablePath; }
    std::string CWD() const { return m_cwd; }

    static App& Get();

    bool FolderExists(std::string folder) const;
    bool LoadFile(std::string file, std::string& contents) const;

    bool Init();
    void Cleanup();
    int Loop();

private:
    std::string m_savePath;
    std::string m_dataPath;
    std::string m_executablePath;
    std::string m_cwd;

    App() {};

    // Returns true if we're the only running instance or false if we're not; returns false after SetError() on failure.
    bool ForceSingleInstanceInit_() const;
    void ForceSingleInstanceCleanup_() const;

    void InitLogSystemInfo_() const;
    bool InitSavePath_();
    bool InitCWD_();
    bool InitExecutablePath_();
    bool InitDataPath_();
};

#endif // APP_HPP
