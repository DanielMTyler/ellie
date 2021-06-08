/*
    ==================================
    Copyright (C) 2021 Daniel Tyler.
      This file is part of Ellie.
    ==================================
*/

#version 330 core
out vec4 outColor;

in vec3 fragWorldPosition;
in vec3 normal;

uniform bool isLightSource;
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    if (isLightSource)
    {
        outColor = vec4(1.0f);
    }
    else
    {
        float ambientStrength = 0.1f;
        vec3 ambient = ambientStrength * lightColor;

        // @todo Diffuse looks wrong; it comes through opposite faces of the cube.
        vec3 n = normalize(normal);
        vec3 lightDirection = normalize(lightPos - fragWorldPosition);
        float d = max(dot(n, lightDirection), 0.0f);
        vec3 diffuse = d * lightColor;

        float specularStrength = 0.5f;
        vec3 viewDir = normalize(viewPos - fragWorldPosition);
        vec3 reflectDir = reflect(-lightDirection, n);
        int shininess = 32;
        float spec = pow(max(dot(viewDir, reflectDir), 0.0f), shininess);
        vec3 specular = specularStrength * spec * lightColor;

        vec3 result = (ambient + diffuse + specular) * objectColor;
        outColor = vec4(result, 1.0f);
    }
}
