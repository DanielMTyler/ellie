#version 330 core
layout (location = 0) in vec3 p;
layout (location = 1) in vec2 texCoord_;

out vec2 texCoord;

void main()
{
    gl_Position = vec4(p, 1.0);
    texCoord = texCoord_;
}
