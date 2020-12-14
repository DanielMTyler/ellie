#version 330 core
out vec4 outColor;

uniform bool isLightSource;
uniform vec3 objectColor;
uniform vec3 lightColor;

void main()
{
    if (isLightSource)
        outColor = vec4(1.0f);
    else
        outColor = vec4(lightColor * objectColor, 1.0f);
}
