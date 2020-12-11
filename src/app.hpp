/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef APP_HPP
#define APP_HPP

#include "global.hpp"
#include <SDL.h>
#include <string>
#include "logic.hpp"
#include "view_interface.hpp"

class App {
public:
    // @todo Game Logic.
    // @todo Event Manager.
    // @todo Memory Manager.
    // @todo CVar/Config/Settings System/Manager.

    static App& Get();

    std::string SavePath() const { return m_savePath; }
    std::string DataPath() const { return m_dataPath; }
    std::string ExecutablePath() const { return m_executablePath; }
    std::string CWD() const { return m_cwd; }

    bool FolderExists(std::string folder) const;
    bool LoadFile(std::string file, std::string& contents) const;

    bool Init();
    void Cleanup();
    int  Loop();

private:
    std::string m_savePath;
    std::string m_dataPath;
    std::string m_executablePath;
    std::string m_cwd;

    IView* m_view = nullptr;
    Logic m_logic;

    // Creation by App::Get() only.
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
