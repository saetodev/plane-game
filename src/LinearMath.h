#ifndef LINEAR_MATH_H
#define LINEAR_MATH_H

#include "Basic.h"

#include <math.h>

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    float Length() {
        return sqrtf((x * x) + (y * y));
    }

    Vec2 Normalized() {
        float len = Length();

        if (len == 0) {
            return {};
        }

        return { x / len, y / len };
    }
};

inline bool operator==(const Vec2& a, const Vec2& b) {
    return (a.x == b.x) && (a.y == b.y);
}

inline Vec2& operator+=(Vec2& a, const Vec2& b) {
    a.x += b.x;
    a.y += b.y;
    return a;
}

inline Vec2 operator-(const Vec2& a, const Vec2& b) {
    return { a.x - b.x, a.y - b.y };
}

inline Vec2 operator*(const Vec2& v, float s) {
    return { v.x * s, v.y * s };
}

#endif
