#version 330 core
layout (location = 0) in vec3 p;
layout (location = 1) in vec2 texCoord_;

uniform mat4 transform;

out vec2 texCoord;

void main()
{
    gl_Position = transform * vec4(p, 1.0f);
    texCoord = texCoord_;
}
