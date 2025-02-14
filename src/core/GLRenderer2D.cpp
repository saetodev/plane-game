#include "Renderer2D.h"
#include "Util.h"

#include <stdlib.h>
#include <iostream>

#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

struct Vertex {
    Vec2 position;
    Vec2 textureCoord;
};

struct StringRef {
    usize size;
    const char* str;
};

static u32 m_quadVBO;
static u32 m_quadShader;

static u32 m_lineVBO;
static u32 m_lineShader;

static Texture2D m_whiteTexture;

static Mat4 m_projection;

static const Vertex m_quadVertices[] = {
    { { -0.5f, -0.5f }, { 0, 0 } },
    { {  0.5f, -0.5f }, { 1, 0 } },
    { {  0.5f,  0.5f }, { 1, 1 } },

    { {  0.5f,  0.5f }, { 1, 1 } },
    { { -0.5f,  0.5f }, { 0, 1 } },
    { { -0.5f, -0.5f }, { 0, 0 } },
};

static u32 CreateShader(const char* vsSource, const char* fsSource, bool quadShader) {
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

    if (quadShader) {
        glBindAttribLocation(shader, 0, "a_position");
        glBindAttribLocation(shader, 1, "a_textureCoord");
    } 
    else {
        glBindAttribLocation(shader, 0, "a_position");
    }

    glLinkProgram(shader);

    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    return shader;
}

void Renderer2D::Init() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glGenBuffers(1, &m_quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_quadVertices), m_quadVertices, GL_STATIC_DRAW);

    {
        char* vsSource = Util::ReadEntireFile("data/vertex.glsl");
        char* fsSource = Util::ReadEntireFile("data/frag.glsl");

        m_quadShader = CreateShader(vsSource, fsSource, true);

        free(fsSource);
        free(vsSource);
    }

    glGenBuffers(1, &m_lineVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_lineVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2) * 2, NULL, GL_DYNAMIC_DRAW);

    {
        char* vsSource = Util::ReadEntireFile("data/line_vertex.glsl");
        char* fsSource = Util::ReadEntireFile("data/line_frag.glsl");

        m_lineShader = CreateShader(vsSource, fsSource, false);

        free(fsSource);
        free(vsSource);
    }

    u32 whitePixels[] = { 0xFFFFFFFF };
    m_whiteTexture = CreateTexture(1, 1, whitePixels);

    m_projection = OrthoProjection(0, 1280, 720, 0, 0, 1);
}

void Renderer2D::Shutdown() {
    glDeleteShader(m_lineShader);
    glDeleteShader(m_quadShader);

    glDeleteBuffers(1, &m_lineVBO);
    glDeleteBuffers(1, &m_quadVBO);
}

Texture2D Renderer2D::LoadTexture(const char* filename) {
    //stbi_set_flip_vertically_on_load(true);

    int w, h, comp;
    u8* data = stbi_load(filename, &w, &h, &comp, 4);

    Texture2D texture = CreateTexture(w, h, data);
    stbi_image_free(data);

    return texture;
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
    glBindTexture(GL_TEXTURE_2D, 0);

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

void Renderer2D::DrawLine(const Vec2& start, const Vec2& end, const Vec4& color) {
    glBindBuffer(GL_ARRAY_BUFFER, m_lineVBO);
    glUseProgram(m_lineShader);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, NULL);

    int projectionLoc = glGetUniformLocation(m_lineShader, "u_projection");
    int colorLoc = glGetUniformLocation(m_lineShader, "u_color");

    glUniformMatrix4fv(projectionLoc, 1, true, (float*)&m_projection);
    glUniform4fv(colorLoc, 1, (float*)&color);

    Vec2 vertices[] = { start, end };
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glDrawArrays(GL_LINES, 0, 2);
}

void Renderer2D::DrawRect(const Vec2& position, const Vec2& size, const Vec4& color) {
    DrawRect({ position, size }, color);
}

void Renderer2D::DrawRect(const Transform& transform, const Vec4& color) {
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glUseProgram(m_quadShader);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(Vertex), (const void*)offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(Vertex), (const void*)offsetof(Vertex, textureCoord));
    
    int transformLoc  = glGetUniformLocation(m_quadShader, "u_transform");
    int projectionLoc = glGetUniformLocation(m_quadShader, "u_projection");
    int colorLoc = glGetUniformLocation(m_quadShader, "u_color");
    int textureLoc = glGetUniformLocation(m_quadShader, "u_texture");

    f32 tx = transform.position.x;
    f32 ty = transform.position.y;

    f32 sx = transform.size.x;
    f32 sy = transform.size.y;

    f32 rotation = transform.rotation * DEG_TO_RAD;

    Mat4 rotMatrix = {
        .r0 = { cosf(rotation), -sinf(rotation), 0.0f, 0.0f },
        .r1 = { sinf(rotation),  cosf(rotation), 0.0f, 0.0f },
        .r2 = { 0.0f,            0.0f,           1.0f, 0.0f },
        .r3 = { 0.0f,            0.0f,           0.0f, 1.0f },
    };

    Mat4 transformMatrix = {
        .r0 = { sx,   0.0f, 0.0f, tx },
        .r1 = { 0.0f, sy,   0.0f, ty },
        .r2 = { 0.0f, 0.0f, 1.0f, 0.0f },
        .r3 = { 0.0f, 0.0f, 0.0f, 1.0f },
    };

    transformMatrix = transformMatrix * rotMatrix;

    glUniformMatrix4fv(transformLoc, 1, true, (float*)&transformMatrix);
    glUniformMatrix4fv(projectionLoc, 1, true, (float*)&m_projection);
    glUniform4fv(colorLoc, 1, (float*)&color);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_whiteTexture.id);
    glUniform1i(textureLoc, 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer2D::DrawTexture(const Texture2D& texture, const Vec2& position, const Vec2& size, const Vec4& color) {
    DrawTexture(texture, { position, size }, color);
}

void Renderer2D::DrawTexture(const Texture2D& texture, const Transform& transform, const Vec4& color) {
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glUseProgram(m_quadShader);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(Vertex), (const void*)offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(Vertex), (const void*)offsetof(Vertex, textureCoord));
    
    int transformLoc  = glGetUniformLocation(m_quadShader, "u_transform");
    int projectionLoc = glGetUniformLocation(m_quadShader, "u_projection");
    int colorLoc = glGetUniformLocation(m_quadShader, "u_color");
    int textureLoc = glGetUniformLocation(m_quadShader, "u_texture");

    f32 tx = transform.position.x;
    f32 ty = transform.position.y;

    f32 sx = transform.size.x;
    f32 sy = transform.size.y;

    f32 rotation = transform.rotation * DEG_TO_RAD;

    Mat4 rotMatrix = {
        .r0 = { cosf(rotation), -sinf(rotation), 0.0f, 0.0f },
        .r1 = { sinf(rotation),  cosf(rotation), 0.0f, 0.0f },
        .r2 = { 0.0f,            0.0f,           1.0f, 0.0f },
        .r3 = { 0.0f,            0.0f,           0.0f, 1.0f },
    };

    Mat4 transformMatrix = {
        .r0 = { sx,   0.0f, 0.0f, tx },
        .r1 = { 0.0f, sy,   0.0f, ty },
        .r2 = { 0.0f, 0.0f, 1.0f, 0.0f },
        .r3 = { 0.0f, 0.0f, 0.0f, 1.0f },
    };

    transformMatrix = transformMatrix * rotMatrix;

    glUniformMatrix4fv(transformLoc, 1, true, (float*)&transformMatrix);
    glUniformMatrix4fv(projectionLoc, 1, true, (float*)&m_projection);
    glUniform4fv(colorLoc, 1, (float*)&color);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glUniform1i(textureLoc, 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}