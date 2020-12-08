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
#include "app_interface.hpp"
#include "view_interface.hpp"

class App : public IApp {
public:
    std::string SavePath() const override { return m_savePath; }
    std::string DataPath() const override { return m_dataPath; }
    std::string ExecutablePath() const override { return m_executablePath; }
    std::string CWD() const override { return m_cwd; }

    bool FolderExists(std::string folder) const override;
    bool LoadFile(std::string file, std::string& contents) const override;

    bool Init()    override;
    void Cleanup() override;
    int  Loop()    override;

protected:
    friend IApp;

    // Creation by IApp::Get() only.
    App() {};

private:
    std::string m_savePath;
    std::string m_dataPath;
    std::string m_executablePath;
    std::string m_cwd;

    IView* m_view = nullptr;

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
