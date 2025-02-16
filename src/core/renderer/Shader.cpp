#include "Shader.h"
#include "core/Util.h"

#include <iostream>

#include <glad/glad.h>

static const char* GetGLName(u32 value) {
    switch (value) {
        case GL_VERTEX_SHADER: return "VERTEX SHADER";
        case GL_FRAGMENT_SHADER: return "FRAGMENT SHADER";
    }

    ASSERT(false);
}

static u32 CreateShaderStage(const std::string& source, u32 shaderType) {
    ASSERT(shaderType == GL_VERTEX_SHADER || shaderType == GL_FRAGMENT_SHADER);

    const char* sourceRaw = source.c_str();

    u32 shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &sourceRaw, NULL);
    glCompileShader(shader);

    int compileStatus = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

    if (compileStatus == GL_FALSE) {
        int length;
        char message[512];
        
        glGetShaderInfoLog(shader, 512, &length, message);

        std::cout << "ERROR: Failed to compile " << GetGLName(shaderType) << "\n" << message << std::endl;
    }

    return shader;
}

u32 Renderer2D::CreateShader(const std::string& vertexSource, const std::string& fragmentSource, const List<Attribute, MAX_ATTRIBUTE_COUNT>& attributes) {
    u32 program = glCreateProgram();

    for (int i = 0; i < attributes.Size(); i++) {
        glBindAttribLocation(program, i, attributes[i].name.c_str());
    }

    u32 vertexShader   = CreateShaderStage(vertexSource, GL_VERTEX_SHADER);
    u32 fragmentShader = CreateShaderStage(fragmentSource, GL_FRAGMENT_SHADER);

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int linkStatus = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

    if (linkStatus == GL_FALSE) {
        int length;
        char message[512];
        
        glGetProgramInfoLog(program, 512, &length, message);

        std::cout << "ERROR: Failed to link shader program" << "\n" << message << std::endl;
    }

    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    return program;
}

void Renderer2D::DestroyShader(u32 shader) {
    glDeleteProgram(shader);
}

void Renderer2D::UseShader(u32 shader) {
    glUseProgram(shader);
}

void Renderer2D::SetUniform(u32 shader, int value, const char* name) {
    int loc = glGetUniformLocation(shader, name);
    
    glUseProgram(shader);
    glUniform1i(loc, value);
}

void Renderer2D::SetUniform(u32 shader, const Mat4& mat, const char* name) {
    int loc = glGetUniformLocation(shader, name);
    
    glUseProgram(shader);
    glUniformMatrix4fv(loc, 1, true, (float*)&mat);
}

void Renderer2D::SetUniformIndex(u32 shader, int index, int value, const char* name) {
    int loc = glGetUniformLocation(shader, name);
    
    glUseProgram(shader);
    glUniform1i(loc + index, value);
}