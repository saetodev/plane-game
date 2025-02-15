#include "List.h"
#include "Renderer2D.h"
#include "Util.h"

#include <stdlib.h>
#include <iostream>

#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

struct QuadVertex {
    Vec2 position;
    Vec2 textureCoord;
    Vec4 color;
    f32 textureID;
};

struct LineVertex {
    Vec2 position;
    Vec4 color;
};

static constexpr int MAX_BATCH_SIZE    = 256;
static constexpr int MAX_TEXTURE_COUNT = 8;

static constexpr int VERTICES_PER_QUAD = 6;
static constexpr int VERTICES_PER_LINE = 2;

/* QUAD PIPELINE */
static u32 m_quadVBO;
static u32 m_quadShader;
static List<QuadVertex, VERTICES_PER_QUAD * MAX_BATCH_SIZE> m_quadVertexBuffer;

static Texture2D m_whiteTexture;
static List<Texture2D, MAX_TEXTURE_COUNT> m_textures;

/* LINE PIPELINE */
static u32 m_lineVBO;
static u32 m_lineShader;
static List<LineVertex, VERTICES_PER_LINE * MAX_BATCH_SIZE> m_lineVertexBuffer;

/* MISC */
static Mat4 m_projection;

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
        std::cout << "VERTEX SHADER ERROR: " << message << std::endl;
    }

    u32 fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fsSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        char message[512] = {};
        int logLength = 0;

        glGetShaderInfoLog(fragmentShader, 512, &logLength, message);
        std::cout << "FRAG SHADER ERROR: " << message << std::endl;
    }

    glAttachShader(shader, vertexShader);
    glAttachShader(shader, fragmentShader);

    if (quadShader) {
        glBindAttribLocation(shader, 0, "a_position");
        glBindAttribLocation(shader, 1, "a_textureCoord");
        glBindAttribLocation(shader, 2, "a_color");
        glBindAttribLocation(shader, 3, "a_textureID");
    } 
    else {
        glBindAttribLocation(shader, 0, "a_position");
        glBindAttribLocation(shader, 1, "a_color");
    }

    glLinkProgram(shader);

    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    return shader;
}

static void Flush() {
    if (!m_quadVertexBuffer.Empty()) {
        glUseProgram(m_quadShader);
        glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(QuadVertex), (const void*)offsetof(QuadVertex, position));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(QuadVertex), (const void*)offsetof(QuadVertex, textureCoord));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_FLOAT, false, sizeof(QuadVertex), (const void*)offsetof(QuadVertex, color));

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 1, GL_FLOAT, false, sizeof(QuadVertex), (const void*)offsetof(QuadVertex, textureID));

        int loc = glGetUniformLocation(m_quadShader, "u_textures");

        for (int i = 0; i < m_textures.Size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, m_textures[i].id);
            glUniform1i(loc + i, i);
        }

        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(QuadVertex) * m_quadVertexBuffer.Size(), m_quadVertexBuffer.Data());
        glDrawArrays(GL_TRIANGLES, 0, m_quadVertexBuffer.Size());

        m_quadVertexBuffer.Clear();
        m_textures.Clear();
    }

    if (!m_lineVertexBuffer.Empty()) {
        glUseProgram(m_lineShader);
        glBindBuffer(GL_ARRAY_BUFFER, m_lineVBO);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(LineVertex), (const void*)offsetof(LineVertex, position));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, false, sizeof(LineVertex), (const void*)offsetof(LineVertex, color));

        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(LineVertex) * m_lineVertexBuffer.Size(), m_lineVertexBuffer.Data());
        glDrawArrays(GL_LINES, 0, m_lineVertexBuffer.Size());

        m_lineVertexBuffer.Clear();
    }
}

void Renderer2D::Init() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glGenBuffers(1, &m_quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QuadVertex) * m_quadVertexBuffer.Capacity(), NULL, GL_DYNAMIC_DRAW);

    {
        char* vsSource = Util::ReadEntireFile("data/vertex.glsl");
        char* fsSource = Util::ReadEntireFile("data/frag.glsl");

        m_quadShader = CreateShader(vsSource, fsSource, true);

        free(fsSource);
        free(vsSource);
    }

    glGenBuffers(1, &m_lineVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_lineVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(LineVertex) * m_lineVertexBuffer.Capacity(), NULL, GL_DYNAMIC_DRAW);

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

void Renderer2D::Begin() {
    {
        int projectionLoc = glGetUniformLocation(m_quadShader, "u_projection");

        glUseProgram(m_quadShader);
        glUniformMatrix4fv(projectionLoc, 1, true, (float*)&m_projection);
    }

    {
        int projectionLoc = glGetUniformLocation(m_lineShader, "u_projection");

        glUseProgram(m_lineShader);
        glUniformMatrix4fv(projectionLoc, 1, true, (float*)&m_projection);
    }
}

void Renderer2D::End() {
    Flush();
}

void Renderer2D::Clear(const Vec4& color) {
    glClearColor(color.x, color.y, color.z, color.w);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer2D::DrawLine(const Vec2& start, const Vec2& end, const Vec4& color) {
    if (m_lineVertexBuffer.Full()) {
        Flush();
    }

    m_lineVertexBuffer.Push({ start, color });
    m_lineVertexBuffer.Push({ end, color });
}

void Renderer2D::DrawRect(const Vec2& position, const Vec2& size, const Vec4& color) {
    DrawRect({ position, size }, color);
}

void Renderer2D::DrawRect(const Transform& transform, const Vec4& color) {
    DrawTexture(m_whiteTexture, transform, color);
}

void Renderer2D::DrawRectLines(const Vec2& position, const Vec2& size, const Vec4& color) {
    f32 x0 = position.x - (size.x / 2.0f);
    f32 y0 = position.y - (size.y / 2.0f);

    f32 x1 = x0 + size.x;
    f32 y1 = y0 + size.y;

    DrawLine({ x0, y0 }, { x1, y0 }, color); // top
    DrawLine({ x1, y0 }, { x1, y1 }, color); // right
    DrawLine({ x0, y1 }, { x1, y1 }, color); // bottom
    DrawLine({ x0, y0 }, { x0, y1 }, color); // left
}

void Renderer2D::DrawTexture(const Texture2D& texture, const Vec2& position, const Vec2& size, const Vec4& color) {
    DrawTexture(texture, { position, size }, color);
}

void Renderer2D::DrawTexture(const Texture2D& texture, const Transform& transform, const Vec4& color) {
    if (m_quadVertexBuffer.Full()) {
        Flush();
    }

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

    static constexpr Vec4 quadVertices[] = {
        { -0.5f, -0.5f, 0.0f, 1.0f },
        {  0.5f, -0.5f, 0.0f, 1.0f },
        {  0.5f,  0.5f, 0.0f, 1.0f },

        {  0.5f,  0.5f, 0.0f, 1.0f },
        { -0.5f,  0.5f, 0.0f, 1.0f },
        { -0.5f, -0.5f, 0.0f, 1.0f },
    };

    static constexpr Vec2 quadTextureCoords[] = {
        { 0, 0 },
        { 1, 0 },
        { 1, 1 },

        { 1, 1 },
        { 0, 1 },
        { 0, 0 },
    };

    int index = -1;

    for (int i = 0; i < m_textures.Size(); i++) {
        if (m_textures[i].id == texture.id) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        index = m_textures.Size();
        m_textures.Push(texture);
    }

    for (int i = 0; i < VERTICES_PER_QUAD; i++) {
        Vec4 transformVertex = transformMatrix * quadVertices[i];

        QuadVertex vertex = {
            .position     = { transformVertex.x, transformVertex.y },
            .textureCoord = quadTextureCoords[i],
            .color        = color,
            .textureID    = (f32)texture.id,
        };

        m_quadVertexBuffer.Push(vertex);
    }
}