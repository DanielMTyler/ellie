/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef APP_HPP
#define APP_HPP

#include "global.hpp"
#include <glm/glm.hpp>
#include <string>
#include "events.hpp"
#include "logic.hpp"
#include "view_interface.hpp"

class App {
public:
    // @todo Resource Manager.
    // @todo Memory Manager.
    // @todo CVar/Console System.

    struct Options {
        struct Camera {
            float32 fov     = 45.0f;
            float32 fovMin  =  1.0f;
            float32 fovMax  = 45.0f;
            float32 fovStep =  3.0f;

            float32 pitch            =   0.0f;
            float32 pitchMin         = -89.0f;
            float32 pitchMax         =  89.0f;
            bool    pitchInverted    =  false;
            float32 pitchSensitivity =   0.1f;

            glm::vec3 position = glm::vec3(0.0f, 0.0f,  3.0f);
            glm::vec3 front;
            glm::vec3 right;
            glm::vec3 up;
            glm::vec3 worldUp  = glm::vec3(0.0f, 1.0f,  0.0f);

            float32 speed = 0.01f;

            float32 yaw            = -90.0f;
            bool    yawInverted    =  false;
            float32 yawSensitivity =   0.1f;
        } camera;

        struct Core {
            std::string savePath;
            std::string dataPath;
            std::string executablePath;
            std::string cwdPath;
            std::string shaderPath;
            std::string texturePath;
        } core;

        struct Graphics {
            bool   multisampling = true;
            uint32 multisamplingNumSamples = 4; // 2 or 4.

            float32 planeNear = 0.1f;
            float32 planeFar  = 100.0f;

            bool vsync         = true;
            bool vsyncAdaptive = true; // Classic or Adaptive VSync?

            uint32 windowWidth  = 800;
            uint32 windowHeight = 600;
        } graphics;
    } m_options;

    static App& Get();

    EventManager& Events() { return m_events; }
    class Logic&  Logic()  { return m_logic; }

    bool FolderExists(std::string folder) const;
    bool LoadFile(std::string file, std::string& contents) const;

    bool Init();
    void Cleanup();
    int  Loop(); // Returns main() return code.

private:
    EventManager m_events;
    class Logic m_logic;
    IView* m_view = nullptr;

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
