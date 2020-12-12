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
    return m_quit;
}

void Logic::UpdateCamera()
{
    m_app->m_options.camera.front.x = std::cos(glm::radians(m_app->m_options.camera.yaw)) * std::cos(glm::radians(m_app->m_options.camera.pitch));
    m_app->m_options.camera.front.y = std::sin(glm::radians(m_app->m_options.camera.pitch));
    m_app->m_options.camera.front.z = std::sin(glm::radians(m_app->m_options.camera.yaw)) * std::cos(glm::radians(m_app->m_options.camera.pitch));
    m_app->m_options.camera.front = glm::normalize(m_app->m_options.camera.front);

    m_app->m_options.camera.right = glm::normalize(glm::cross(m_app->m_options.camera.front, m_app->m_options.camera.worldUp));
    m_app->m_options.camera.up    = glm::normalize(glm::cross(m_app->m_options.camera.right, m_app->m_options.camera.front));
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
        m_app->m_options.camera.position += m_app->m_options.camera.front * m_app->m_options.camera.speed * dt;
    else if (d->b)
        m_app->m_options.camera.position -= m_app->m_options.camera.front * m_app->m_options.camera.speed * dt;
    if (d->l)
        m_app->m_options.camera.position -= m_app->m_options.camera.right * m_app->m_options.camera.speed * dt;
    else if (d->r)
        m_app->m_options.camera.position += m_app->m_options.camera.right * m_app->m_options.camera.speed * dt;
    // @todo This keeps the camera grounded FPS style, but it also makes
    //       forward/backward movement slow when at an extreme pitch. Why?
    //m_cameraPosition.y = 0.0f;
}

void Logic::OnRotateCamera(IEventDataPtr e)
{
    EventData_RotateCamera* d = dynamic_cast<EventData_RotateCamera*>(e.get());
    m_app->m_options.camera.yaw   += (m_app->m_options.camera.yawInverted   ? -d->xrel : d->xrel) * m_app->m_options.camera.yawSensitivity;
    m_app->m_options.camera.pitch -= (m_app->m_options.camera.pitchInverted ? -d->yrel : d->yrel) * m_app->m_options.camera.pitchSensitivity;

    if (m_app->m_options.camera.yaw > 360.0f)
        m_app->m_options.camera.yaw -= 360.0f;
    else if (m_app->m_options.camera.yaw < 0.0f)
        m_app->m_options.camera.yaw += 360.0f;

    if (m_app->m_options.camera.pitch > m_app->m_options.camera.pitchMax)
        m_app->m_options.camera.pitch = m_app->m_options.camera.pitchMax;
    else if (m_app->m_options.camera.pitch < m_app->m_options.camera.pitchMin)
        m_app->m_options.camera.pitch = m_app->m_options.camera.pitchMin;

    UpdateCamera();
}

void Logic::OnZoomCamera(IEventDataPtr e)
{
    EventData_ZoomCamera* d = dynamic_cast<EventData_ZoomCamera*>(e.get());
    if (d->in)
        m_app->m_options.camera.fov -= m_app->m_options.camera.fovStep;
    else
        m_app->m_options.camera.fov += m_app->m_options.camera.fovStep;

    if (m_app->m_options.camera.fov < m_app->m_options.camera.fovMin)
        m_app->m_options.camera.fov = m_app->m_options.camera.fovMin;
    else if (m_app->m_options.camera.fov > m_app->m_options.camera.fovMax)
        m_app->m_options.camera.fov = m_app->m_options.camera.fovMax;
}
