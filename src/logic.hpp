/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#ifndef LOGIC_HPP
#define LOGIC_HPP

#include "global.hpp"
#include "events.hpp"
#include <glm/glm.hpp>

class App;

class Logic
{
public:
    // @todo Time Dilation:
    //       modify dt to speed/slow time; may require adjusting App/View dt.
    // @todo Entity Component System.
    // @todo Command Interpreter instead of commands via Event Manager?

    bool m_cameraInvertedYaw   = false;
    bool m_cameraInvertedPitch = false;

    float32 m_cameraFOV;
    float32 m_nearPlane = 0.1f;
    float32 m_farPlane  = 100.0f;

    glm::vec3 m_cameraPosition = glm::vec3(0.0f, 0.0f,  3.0f);
    glm::vec3 m_cameraFront;
    glm::vec3 m_cameraUp;
    glm::vec3 m_cameraRight;
    glm::vec3 m_cameraWorldUp  = glm::vec3(0.0f, 1.0f,  0.0f);
    float32 m_cameraYaw   = -90.0f;
    float32 m_cameraPitch =   0.0f;

    bool Init();
    void Cleanup();
    bool Update(DeltaTime dt); // Returns true to quit.

private:
    const float32 m_cameraSpeed = 0.01f;
    const float32 m_cameraSensitivityYaw   = 0.1f;
    const float32 m_cameraSensitivityPitch = 0.1f;
    const float32 m_cameraZoomMin = 1.0f;
    const float32 m_cameraZoomMax = 45.0f;
    const float32 m_cameraZoomStep = 3.0f;

    App* m_app  = nullptr;
    bool m_quit = false;

    void UpdateCamera();

    void OnQuit(IEventDataPtr e);
    void OnMoveCamera(IEventDataPtr e);
    void OnRotateCamera(IEventDataPtr e);
    void OnZoomCamera(IEventDataPtr e);
};

#endif // LOGIC_HPP
