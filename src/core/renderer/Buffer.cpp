#include "LinearMath.h"
#include "Buffer.h"

#include <glad/glad.h>

using namespace Renderer2D;

static usize GetBufferSize(const Buffer& buffer) {
    usize size = 0;

    for (int i = 0; i < buffer.attributes.Size(); i++) {
        size += sizeof(f32) * buffer.attributes[i].count;
    }

    return size;
}

Buffer Renderer2D::CreateBuffer(usize size) {
    Buffer buffer = {};

    glGenBuffers(1, &buffer.renderID);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.renderID);
    glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_DRAW);

    return buffer;
}

void Renderer2D::DestroyBuffer(Buffer& buffer) {
    glDeleteBuffers(1, &buffer.renderID);

    buffer.renderID = 0;
    buffer.attributes.Clear();
}

void Renderer2D::SetBufferData(const Buffer& buffer, usize size, const void* data) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer.renderID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
}

void Renderer2D::EnableAttributes(const Buffer& buffer) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer.renderID);

    usize size   = GetBufferSize(buffer);
    usize offset = 0;

    for (int i = 0; i < buffer.attributes.Size(); i++) {
        const Attribute& attribute = buffer.attributes[i];

        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, attribute.count, GL_FLOAT, false, size, (const void*)offset);

        offset += sizeof(f32) * attribute.count;
    }
}