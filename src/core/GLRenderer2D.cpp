#include "Renderer2D.h"
#include "Util.h"

#include <stdlib.h>
#include <iostream>

#include <glad/glad.h>

static u32 m_vao;
static u32 m_vbo;
static u32 m_shader;

static Mat4 m_projection;

static const Vec2 m_vertices[] = {
    { -0.5f, -0.5f },
    {  0.5f, -0.5f },
    {  0.5f,  0.5f },

    {  0.5f,  0.5f },
    { -0.5f,  0.5f },
    { -0.5f, -0.5f },
};

static u32 CreateShader(const char* vsSource, const char* fsSource) {
    int result = 0;
    u32 shader = glCreateProgram();

    u32 vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vsSource, NULL);
    glCompileShader(vertexShader);

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        char message[512] = {};
        int logLength = 0;

        glGetShaderInfoLog(vertexShader, 512, &logLength, message);
        std::cout << "ERROR: " << message << std::endl;
    }

    u32 fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fsSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        char message[512] = {};
        int logLength = 0;

        glGetShaderInfoLog(fragmentShader, 512, &logLength, message);
        std::cout << "ERROR: " << message << std::endl;
    }

    glAttachShader(shader, vertexShader);
    glAttachShader(shader, fragmentShader);
    glLinkProgram(shader);

    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    return shader;
}

void Renderer2D::Init() {
    //glGenVertexArraysOES(1, &m_vao);
    //glBindVertexArrayOES(m_vao);

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), m_vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, 0);

    char* vsSource = Util::ReadEntireFile("data/vertex.glsl");
    char* fsSource = Util::ReadEntireFile("data/frag.glsl");

    m_shader = CreateShader(vsSource, fsSource);
    glUseProgram(m_shader);

    free(fsSource);
    free(vsSource);

    m_projection = OrthoProjection(0, 1280, 720, 0, 0, 1);
}

void Renderer2D::Shutdown() {
    glDeleteBuffers(1, &m_vbo);
    //glDeleteVertexArraysOES(1, &m_vao);
}

Texture2D Renderer2D::LoadTexture(const char* filename) {
    return {};
}

Texture2D Renderer2D::CreateTexture(u32 width, u32 height, void* pixels) {
    u32 id = 0;
    
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    return {
        .id     = id,
        .width  = width,
        .height = height,
    };
}

void Renderer2D::Clear(const Vec4& color) {
    glClearColor(color.x, color.y, color.z, color.w);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer2D::DrawRect(const Vec2& position, const Vec2& size, const Vec4& color) {
    int transformLoc  = glGetUniformLocation(m_shader, "u_transform");
    int projectionLoc = glGetUniformLocation(m_shader, "u_projection");
    int colorLoc = glGetUniformLocation(m_shader, "u_color");

    Mat4 transform = {
        .r0 = { size.x, 0.0f,   0.0f, position.x },
        .r1 = { 0.0f,   size.y, 0.0f, position.y },
        .r2 = { 0.0f,   0.0f,   1.0f, 0.0f },
        .r3 = { 0.0f,   0.0f,   0.0f, 1.0f },
    };

    glUniformMatrix4fv(transformLoc, 1, true, (float*)&transform);
    glUniformMatrix4fv(projectionLoc, 1, true, (float*)&m_projection);
    glUniform4fv(colorLoc, 1, (float*)&color);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer2D::DrawTexture(const Texture2D& texture, const Vec2& position, const Vec2& size, const Vec4& color) {

}