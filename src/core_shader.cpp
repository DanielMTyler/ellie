/*
    ==================================
    Copyright (C) 2020 Daniel M Tyler.
      This file is part of Ellie.
    ==================================
*/

// I should only be included by core.cpp.

#ifndef CORE_SHADER_HPP
#define CORE_SHADER_HPP

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

namespace CoreShaderImpl
{
    bool LoadAndCompileShader(Shader& shader, std::string baseFilename, Shader::Type type)
    {
        ASSERT(!baseFilename.empty());
        
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
            gLog.fatal("OpenGL", "Failed to read shader: %s.", shader.file.c_str());
            return false;
        }
        const char* textBuf = text.c_str();
        glShaderSource(shader.id, 1, &textBuf, nullptr);
        glCompileShader(shader.id);
        
        const uint32 infoLogSize = KIBIBYTES(1);
        char infoLog[infoLogSize];
        int success = GL_FALSE; // If glGetShaderiv were to fail for some reason, success will == this value.
        glGetShaderiv(shader.id, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader.id, infoLogSize, nullptr, infoLog);
            gLog.fatal("OpenGL", "Failed to compile shader (%s): %s.", shader.file.c_str(), GetGLErrors().c_str());
            return false;
        }
        
        return true;
    }
}; // namespace CoreShaderImpl.

bool LoadVertexShader(Shader& shader, std::string name)
{
    ASSERT(!name.empty());
    if (!CoreShaderImpl::LoadAndCompileShader(shader, name, Shader::Type::VERTEX))
        return false;
    return true;
}

bool LoadFragmentShader(Shader& shader, std::string name)
{
    ASSERT(!name.empty());
    if (!CoreShaderImpl::LoadAndCompileShader(shader, name, Shader::Type::FRAGMENT))
        return false;
    return true;
}

#endif
