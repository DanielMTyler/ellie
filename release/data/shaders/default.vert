/*
    ==================================
    Copyright (C) 2021 Daniel Tyler.
      This file is part of Ellie.
    ==================================
*/

#version 330 core
out vec3 fragWorldPosition;
out vec3 normal;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal_;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
    fragWorldPosition = vec3(model * vec4(position, 1.0f));
    normal = mat3(transpose(inverse(model))) * normal_;
}
