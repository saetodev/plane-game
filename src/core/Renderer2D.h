#ifndef CORE_RENDERER_H
#define CORE_RENDERER_H

#include "LinearMath.h"

struct Texture2D {
    u32 id;
    u32 width;
    u32 height;
};

namespace Renderer2D {
    void Init();
    void Shutdown();

    Texture2D LoadTexture(const char* filename);
    Texture2D CreateTexture(u32 width, u32 height, void* pixels);

    void Clear(const Vec4& color);

    void DrawRect(const Vec2& position, const Vec2& size, const Vec4& color);
    void DrawTexture(const Texture2D& texture, const Vec2& position, const Vec2& size, const Vec4& color);
}

#endif