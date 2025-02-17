#include "Buffer.h"
#include "LinearMath.h"
#include "Renderer2D.h"
#include "Shader.h"
#include "core/Util.h"

#include <iostream>

#include <glad/glad.h>

using namespace Renderer2D;

static constexpr int MAX_BATCH_SIZE    = 256;
static constexpr int MAX_ATTRIBUTES    = 8;
static constexpr int MAX_TEXTURE_COUNT = 8;

static constexpr int VERTICES_PER_QUAD = 6;
static constexpr int VERTICES_PER_LINE = 2;

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

/* QUAD PIPELINE */
static Buffer m_quadVBO;
static u32 m_quadShader;
static List<QuadVertex, VERTICES_PER_QUAD * MAX_BATCH_SIZE> m_quadVertexBuffer;

static Texture2D m_whiteTexture;
static List<Texture2D, MAX_TEXTURE_COUNT> m_textureSlots;

/* LINE PIPELINE */
static Buffer m_lineVBO;
static u32 m_lineShader;
static List<LineVertex, VERTICES_PER_LINE * MAX_BATCH_SIZE> m_lineVertexBuffer;

/* MISC */
static Mat4 m_projection;

static void DrawBuffer(const Buffer& buffer, u32 type, usize count) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer.renderID);
    glDrawArrays(type, 0, count);
}

static int GetTextureSlot(const Texture2D& texture) {
    int index = -1;

    for (int i = 0; i < m_textureSlots.Size(); i++) {
        if (m_textureSlots[i].renderID == texture.renderID) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        index = m_textureSlots.Size();
        m_textureSlots.Push(texture);
    }

    return index;
}

static void Flush() {
    if (!m_quadVertexBuffer.Empty()) {
        UseShader(m_quadShader);
        EnableAttributes(m_quadVBO);

        for (int i = 0; i < m_textureSlots.Size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, m_textureSlots[i].renderID);

            SetUniformIndex(m_quadShader, i, i, "u_textures");
        }

        SetBufferData(m_quadVBO, sizeof(QuadVertex) * m_quadVertexBuffer.Size(), m_quadVertexBuffer.Data());
        DrawBuffer(m_quadVBO, GL_TRIANGLES, m_quadVertexBuffer.Size());

        m_textureSlots.Clear();
        m_quadVertexBuffer.Clear();
    }

    if (!m_lineVertexBuffer.Empty()) {
        UseShader(m_lineShader);
        EnableAttributes(m_lineVBO);

        SetBufferData(m_lineVBO, sizeof(LineVertex) * m_lineVertexBuffer.Size(), m_lineVertexBuffer.Data());
        DrawBuffer(m_lineVBO, GL_LINES, m_lineVertexBuffer.Size());

        m_lineVertexBuffer.Clear();
    }
}

void Renderer2D::Init() {
    auto version  = (const char*)glGetString(GL_VERSION);
    auto renderer = (const char*)glGetString(GL_RENDERER);
    auto vendor   = (const char*)glGetString(GL_VENDOR);
    auto shading  = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    std::cout << "GL Version:   " << version << std::endl;
    std::cout << "Renderer:     " << renderer << std::endl;
    std::cout << "Vendor:       " << vendor << std::endl;
    std::cout << "GLSL Version: " << shading << std::endl;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_quadVBO = CreateBuffer(sizeof(QuadVertex) * m_quadVertexBuffer.Capacity());

    m_quadVBO.attributes.Push({ GL_FLOAT, 2, "a_position" });
    m_quadVBO.attributes.Push({ GL_FLOAT, 2, "a_textureCoord" });
    m_quadVBO.attributes.Push({ GL_FLOAT, 4, "a_color" });
    m_quadVBO.attributes.Push({ GL_FLOAT, 1, "a_textureID" });

    {
        std::string vertexSource   = Util::ReadEntireFile("data/vertex.glsl");
        std::string fragmentSource = Util::ReadEntireFile("data/frag.glsl");

        m_quadShader = CreateShader(vertexSource, fragmentSource, m_quadVBO.attributes);
    }

    m_lineVBO = CreateBuffer(sizeof(LineVertex) * m_lineVertexBuffer.Capacity());

    m_lineVBO.attributes.Push({ GL_FLOAT, 2, "a_position" });
    m_lineVBO.attributes.Push({ GL_FLOAT, 4, "a_color" });

    {
        std::string vertexSource   = Util::ReadEntireFile("data/line_vertex.glsl");
        std::string fragmentSource = Util::ReadEntireFile("data/line_frag.glsl");

        m_lineShader = CreateShader(vertexSource, fragmentSource, m_lineVBO.attributes);
    }

    u32 whitePixels[] = { 0xFFFFFFFF };
    m_whiteTexture = CreateTexture(1, 1, whitePixels);

    //TODO: hard coded
    m_projection = OrthoProjection(0, 1280, 720, 0, 0, 1);
}

void Renderer2D::Shutdown() {
    DestroyShader(m_lineShader);
    DestroyShader(m_quadShader);

    DestroyBuffer(m_lineVBO);
    DestroyBuffer(m_quadVBO);
}

void Renderer2D::Begin() {
    SetUniform(m_quadShader, m_projection, "u_projection");
    SetUniform(m_lineShader, m_projection, "u_projection");
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

    int textureSlot = GetTextureSlot(texture);

    for (int i = 0; i < VERTICES_PER_QUAD; i++) {
        Vec4 transformVertex = transform.Matrix() * quadVertices[i];

        QuadVertex vertex = {
            .position     = { transformVertex.x, transformVertex.y },
            .textureCoord = quadTextureCoords[i],
            .color        = color,
            .textureID    = (f32)textureSlot,
        };

        m_quadVertexBuffer.Push(vertex);
    }
}