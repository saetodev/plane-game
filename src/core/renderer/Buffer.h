#ifndef CORE_RENDERER_BUFFER_H
#define CORE_RENDERER_BUFFER_H

#include "List.h"

#include <string>

namespace Renderer2D {

inline constexpr int MAX_ATTRIBUTE_COUNT = 8;

//NOTE: attributes in opengl es2 are always floats (float, vec2, vec3, vec4)
struct Attribute {
    int type;
    int count;
    std::string name;
};

struct Buffer {
    u32 renderID;
    List<Attribute, MAX_ATTRIBUTE_COUNT> attributes;
};

Buffer CreateBuffer(usize size);
void DestroyBuffer(Buffer& buffer);
void SetBufferData(const Buffer& buffer, usize size, const void* data);
void EnableAttributes(const Buffer& buffer);

}

#endif