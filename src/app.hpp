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

class IView;
class EventManager;
class Logic;

class App {
public:
    // @todo Resource Manager.
    // @todo Memory Manager.
    // @todo CVar/Console System.

    // @todo Move to options.hpp?
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

    EventManager* Commands() { return m_commands; }
    EventManager* Events()   { return m_events; }
    class Logic*  Logic()    { return m_logic; }

    // Current value from the high-res counter.
    static TimeStamp Time() { return SDL_GetPerformanceCounter(); }
    // Time() / TimePerSecond() == elapsed time in seconds.
    static TimeStamp TimePerSecond() { return SDL_GetPerformanceFrequency(); }
    static DeltaTime SecondsBetween(TimeStamp start, TimeStamp end) { return (DeltaTime)(end - start) / (DeltaTime)TimePerSecond(); }
    static DeltaTime SecondsElapsed(TimeStamp start) { return SecondsBetween(start, Time()); }
    static DeltaTime MillisecondsBetween(TimeStamp start, TimeStamp end) { return (DeltaTime)(end - start) * 1000.0f / (DeltaTime)TimePerSecond(); }
    static DeltaTime MillisecondsElapsed(TimeStamp start) { return MillisecondsBetween(start, Time()); }

    static bool FolderExists(std::string folder);
    static bool LoadFile(std::string file, std::string& contents);

    bool Init();
    void Cleanup();
    int  Loop(); // Returns main() return code.

private:
    EventManager* m_commands = nullptr; // Critical; WILL be processed ASAP.
    EventManager* m_events   = nullptr; // Non-critical; can be dropped.
    class Logic*  m_logic    = nullptr;
    IView*        m_view     = nullptr;

    // Creation by App::Get() only.
    App() {};

    // Returns true if we're the only running instance or false if we're not; returns false after SetError() on failure.
    static bool ForceSingleInstanceInit_();
    static void ForceSingleInstanceCleanup_();

    static void InitLogSystemInfo_();
    bool InitSavePath_();
    bool InitCWD_();
    bool InitExecutablePath_();
    bool InitDataPath_();
};

#endif // APP_HPP
