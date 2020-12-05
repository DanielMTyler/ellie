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
#include "process_manager.hpp"
#include "view.hpp"

class App {
public:
    // These are used to name the saves folder among other things, so ASCII without spaces is probably best.
    const char* ORGANIZATION_NAME = "DanielMTyler";
    const char* APPLICATION_NAME  = "Ellie";

    const std::string PATH_SEP = PATH_SEPARATOR;

    View m_view;

    // @todo Event Manager.
    // @todo Resource Manager / Memory Manager.
    // @todo Game Logic (with Time Dilation [simply multiply dt by it]).

    std::string SavePath() const { return m_savePath; }
    std::string DataPath() const { return m_dataPath; }
    std::string ExecutablePath() const { return m_executablePath; }
    std::string CWD() const { return m_cwd; }

    static App& Get();

    // Returns the last error (may be "") and clears it.
    std::string LastError();
    void SetError(std::string e);
    bool HasError();
    void ClearError();

    bool FolderExists(std::string folder);

    bool Init();
    void Cleanup();
    int Loop();

private:
    std::string m_lastError;

    std::string m_savePath;
    std::string m_dataPath;
    std::string m_executablePath;
    std::string m_cwd;

    ProcessManager m_processManager;

    App() {};

    // Returns true if we're the only running instance or false if we're not; returns false after SetError() on failure.
    bool ForceSingleInstanceInit() const;
    void ForceSingleInstanceCleanup() const;

    bool InitLog();
    bool InitSavePath();
    bool InitCWD();
    bool InitExecutablePath();
    bool InitDataPath();
};

#endif // APP_HPP
