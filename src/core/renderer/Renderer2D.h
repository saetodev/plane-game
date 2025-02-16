#ifndef CORE_RENDERER_H
#define CORE_RENDERER_H

#include "LinearMath.h"
#include "Texture.h"

inline constexpr Vec4 WHITE = { 1, 1, 1, 1};
inline constexpr Vec4 BLACK = { 0, 0, 0, 1};
inline constexpr Vec4 RED   = { 1, 0, 0, 1};
inline constexpr Vec4 GREEN = { 0, 1, 0, 1};
inline constexpr Vec4 BLUE  = { 0, 0, 1, 1};

namespace Renderer2D {
    void Init();
    void Shutdown();

    void Begin();
    void End();

    void Clear(const Vec4& color);

    void DrawLine(const Vec2& start, const Vec2& end, const Vec4& color);
    
    void DrawRect(const Vec2& position, const Vec2& size, const Vec4& color);
    void DrawRect(const Transform& transform, const Vec4& color);

    void DrawRectLines(const Vec2& position, const Vec2& size, const Vec4& color);

    void DrawTexture(const Texture2D& texture, const Vec2& position, const Vec2& size, const Vec4& color);
    void DrawTexture(const Texture2D& texture, const Transform& transform, const Vec4& color);
}

#endif