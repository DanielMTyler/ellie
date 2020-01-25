/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

// @cleanup

#ifndef RENDERER_OPENGL_HPP_INCLUDED
#define RENDERER_OPENGL_HPP_INCLUDED

struct Shader
{
    enum class Type
    {
        VERTEX,
        FRAGMENT
    };
    
    std::string file;
    Type type;
    GLuint id;
};

#include <fstream> // std::ifstream

namespace CoreShaderImpl
{
    // @todo This shouldn't throw exceptions.
    // @todo Use ResultBool return with a useful error message.
    // @todo Use SDL_RWops?
    bool ReadFileToStr(std::string file, std::string& contents)
    {
        std::ifstream in(file, std::ios::in | std::ios::binary);
        if (in)
            contents = std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        else
            return false;
        
        return true;
    }
    
    bool LoadAndCompileShader(Shader& shader, std::string baseFilename, Shader::Type type)
    {
        DEBUG_ASSERT(!baseFilename.empty());
        
        ResultBool r;
        r.result = true;
        
        shader.file = gDataPath + "shaders" + PLATFORM_PATH_SEPARATOR + baseFilename;
        if (type == Shader::Type::VERTEX)
        {
            shader.file += ".vert";
            shader.type = Shader::Type::VERTEX;
            shader.id = glCreateShader(GL_VERTEX_SHADER);
        }
        else
        {
            shader.file += ".frag";
            shader.type = Shader::Type::FRAGMENT;
            shader.id = glCreateShader(GL_FRAGMENT_SHADER);
        }
        
        std::string text;
        if (!ReadFileToStr(shader.file.c_str(), text))
        {
            SPDLOG_LOGGER_CRITICAL(gLogger, "Failed to read shader: {}.", shader.file);
            return false;
        }
        const char* textBuf = text.c_str();
        GLCHECK(glShaderSource(shader.id, 1, &textBuf, nullptr));
        GLCHECK(glCompileShader(shader.id));
        
        const uint32 INFOLOGSIZE = KIBIBYTES(1);
        char infoLog[INFOLOGSIZE];
        int success = GL_FALSE; // If glGetShaderiv were to fail for some reason, success will == this value.
        GLCHECK(glGetShaderiv(shader.id, GL_COMPILE_STATUS, &success));
        if (!success)
        {
            GLCHECK(glGetShaderInfoLog(shader.id, INFOLOGSIZE, nullptr, infoLog));
            SPDLOG_LOGGER_CRITICAL(gLogger, "Failed to compile shader ({}): {}.", shader.file, infoLog);
            return false;
        }
        
        return true;
    }
}; // namespace CoreShaderImpl.

bool LoadVertexShader(Shader& shader, std::string name)
{
    DEBUG_ASSERT(!name.empty());
    if (!CoreShaderImpl::LoadAndCompileShader(shader, name, Shader::Type::VERTEX))
        return false;
    return true;
}

bool LoadFragmentShader(Shader& shader, std::string name)
{
    DEBUG_ASSERT(!name.empty());
    if (!CoreShaderImpl::LoadAndCompileShader(shader, name, Shader::Type::FRAGMENT))
        return false;
    return true;
}

#endif
