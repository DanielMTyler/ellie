/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

#include "logic.hpp"
#include "app.hpp"
#include <cmath> // std::sin/std::cos

bool Logic::Init()
{
    m_app = &App::Get();
    m_cameraFOV = m_cameraZoomMax;

    UpdateCamera();

    if (!m_app->Events().AddListener(EVENT_BIND_MEMBER_FUNCTION(Logic::OnQuit), EventData_Quit::TYPE))
        return false;
    if (!m_app->Events().AddListener(EVENT_BIND_MEMBER_FUNCTION(Logic::OnMoveCamera), EventData_MoveCamera::TYPE))
        return false;
    if (!m_app->Events().AddListener(EVENT_BIND_MEMBER_FUNCTION(Logic::OnRotateCamera), EventData_RotateCamera::TYPE))
        return false;
    if (!m_app->Events().AddListener(EVENT_BIND_MEMBER_FUNCTION(Logic::OnZoomCamera), EventData_ZoomCamera::TYPE))
        return false;

    return true;
}

void Logic::Cleanup()
{
    // @todo
    //m_app->Events().RemoveListener(OnEventQuit, EventData_Quit::TYPE);
}

bool Logic::Update(DeltaTime /*dt*/)
{
    if (m_quit)
        LogDebug("Logic wants to quit.");

    return m_quit;
}

void Logic::UpdateCamera()
{
    m_cameraFront.x = std::cos(glm::radians(m_cameraYaw)) * std::cos(glm::radians(m_cameraPitch));
    m_cameraFront.y = std::sin(glm::radians(m_cameraPitch));
    m_cameraFront.z = std::sin(glm::radians(m_cameraYaw)) * std::cos(glm::radians(m_cameraPitch));
    m_cameraFront = glm::normalize(m_cameraFront);

    m_cameraRight = glm::normalize(glm::cross(m_cameraFront, m_cameraWorldUp));
    m_cameraUp    = glm::normalize(glm::cross(m_cameraRight, m_cameraFront));
}

void Logic::OnQuit(IEventDataPtr)
{
    m_quit = true;
}

void Logic::OnMoveCamera(IEventDataPtr e)
{
    EventData_MoveCamera* d = dynamic_cast<EventData_MoveCamera*>(e.get());
    // @todo This should move to logic.
    DeltaTime dt = 16.0f;
    if (d->f)
        m_cameraPosition += m_cameraFront * m_cameraSpeed * dt;
    else if (d->b)
        m_cameraPosition -= m_cameraFront * m_cameraSpeed * dt;
    if (d->l)
        m_cameraPosition -= m_cameraRight * m_cameraSpeed * dt;
    else if (d->r)
        m_cameraPosition += m_cameraRight * m_cameraSpeed * dt;
    // @todo This keeps the camera grounded FPS style, but it also makes
    //       forward/backward movement slow when at an extreme pitch. Why?
    //m_cameraPosition.y = 0.0f;
}

void Logic::OnRotateCamera(IEventDataPtr e)
{
    EventData_RotateCamera* d = dynamic_cast<EventData_RotateCamera*>(e.get());
    m_cameraYaw   += (m_cameraInvertedYaw   ? -d->xrel : d->xrel) * m_cameraSensitivityYaw;
    m_cameraPitch -= (m_cameraInvertedPitch ? -d->yrel : d->yrel) * m_cameraSensitivityPitch;

    if (m_cameraYaw > 360.0f)
        m_cameraYaw -= 360.0f;
    else if (m_cameraYaw < 0.0f)
        m_cameraYaw += 360.0f;

    if (m_cameraPitch > 89.0f)
        m_cameraPitch = 89.0f;
    else if (m_cameraPitch < -89.0f)
        m_cameraPitch = -89.0f;

    UpdateCamera();
}

void Logic::OnZoomCamera(IEventDataPtr e)
{
    EventData_ZoomCamera* d = dynamic_cast<EventData_ZoomCamera*>(e.get());
    if (d->in)
        m_cameraFOV -= m_cameraZoomStep;
    else
        m_cameraFOV += m_cameraZoomStep;

    if (m_cameraFOV < m_cameraZoomMin)
        m_cameraFOV = m_cameraZoomMin;
    else if (m_cameraFOV > m_cameraZoomMax)
        m_cameraFOV = m_cameraZoomMax;
}
