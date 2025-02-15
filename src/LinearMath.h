#ifndef LINEAR_MATH_H
#define LINEAR_MATH_H

#include "Basic.h"

#include <math.h>

#define PI 3.14159265359f

#define RAD_TO_DEG (180.0f / PI)
#define DEG_TO_RAD (PI / 180.0f)

struct Vec2 {
    f32 x = 0.0f;
    f32 y = 0.0f;

    f32 Length() const {
        return sqrtf((x * x) + (y * y));
    }

    f32 Dot(const Vec2& v) const {
        return (x * v.x) + (y * v.y);
    }

    Vec2 Normalized() const {
        f32 len = Length();

        if (len == 0) {
            return {};
        }

        return { x / len, y / len };
    }

    f32 Angle() const {
        if (Length() == 0) {
            return 0;
        }

        return atan2f(y, x);
    }

    f32 AngleInDeg() const {
        if (Length() == 0) {
            return 0;
        }
        
        return atan2f(y, x) * RAD_TO_DEG;
    }

    f32 AngleBetween(const Vec2& v) const {
        f32 aLen = Length();
        f32 vLen = v.Length();

        if (aLen == 0 || vLen == 0) {
            return 0;
        }

        return Dot(v) / (Length() * v.Length());
    }
};

inline bool operator==(const Vec2& a, const Vec2& b) {
    return (a.x == b.x) && (a.y == b.y);
}

inline Vec2 operator+(const Vec2& a, const Vec2& b) {
    return { a.x + b.x, a.y + b.y };
}

inline Vec2& operator+=(Vec2& a, const Vec2& b) {
    a.x += b.x;
    a.y += b.y;
    return a;
}

inline Vec2 operator-(const Vec2& a, const Vec2& b) {
    return { a.x - b.x, a.y - b.y };
}

inline Vec2 operator*(const Vec2& v, f32 s) {
    return { v.x * s, v.y * s };
}

struct Transform {
    Vec2 position;
    Vec2 size;
    f32 rotation;
};

struct Vec4 {
    f32 x = 0.0f;
    f32 y = 0.0f;
    f32 z = 0.0f;
    f32 w = 0.0f;

    f32 Length() const {
        return sqrtf((x * x) + (y * y) + (z * z) + (w * w));
    }

    f32 Dot(const Vec4& v) const {
        return (x * v.x) + (y * v.y) + (z * v.z) + (w * v.w);
    }

    Vec4 Normalized() const {
        f32 len = Length();

        if (len == 0) {
            return {};
        }

        return { x / len, y / len, z / len, w / len };
    }
};

inline bool operator==(const Vec4& a, const Vec4& b) {
    return (a.x == b.x) && (a.y == b.y) && (a.z == b.z) && (a.w == b.w);
}

inline Vec4& operator+=(Vec4& a, const Vec4& b) {
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    a.w += b.w;
    return a;
}

inline Vec4 operator-(const Vec4& a, const Vec4& b) {
    return { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
}

inline Vec4 operator*(const Vec4& v, float s) {
    return { v.x * s, v.y * s, v.z * s, v.w * s };
}

struct Mat4 {
    Vec4 r0;
    Vec4 r1;
    Vec4 r2;
    Vec4 r3;

    Mat4 Transposed() const {
        return {
            .r0 = { r0.x, r1.x, r2.x, r3.x },
            .r1 = { r0.y, r1.y, r2.y, r3.y },
            .r2 = { r0.z, r1.z, r2.z, r3.z },
            .r3 = { r0.w, r1.w, r2.w, r3.w },
        };
    }
};

inline Vec4 operator*(const Mat4& m, const Vec4& v) {
    return {
        .x = m.r0.Dot(v),
        .y = m.r1.Dot(v),
        .z = m.r2.Dot(v),
        .w = m.r3.Dot(v),
    };
}

inline Mat4 operator*(const Mat4& a, const Mat4& b) {
    Mat4 bT = b.Transposed();

    return {
        .r0 = { a.r0.Dot(bT.r0), a.r0.Dot(bT.r1), a.r0.Dot(bT.r2), a.r0.Dot(bT.r3) },
        .r1 = { a.r1.Dot(bT.r0), a.r1.Dot(bT.r1), a.r1.Dot(bT.r2), a.r1.Dot(bT.r3) },
        .r2 = { a.r2.Dot(bT.r0), a.r2.Dot(bT.r1), a.r2.Dot(bT.r2), a.r2.Dot(bT.r3) },
        .r3 = { a.r3.Dot(bT.r0), a.r3.Dot(bT.r1), a.r3.Dot(bT.r2), a.r3.Dot(bT.r3) },
    };
}

inline Mat4 OrthoProjection(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far) {
    f32 sx = 2.0f / (right - left);
    f32 sy = 2.0f / (top - bottom);
    f32 sz = -2.0f / (far - near);

    f32 tx = -(right + left) / (right - left);
    f32 ty = -(top + bottom) / (top - bottom);
    f32 tz = -(far + near) / (far - near);
    
    return {
        .r0 = { sx,   0.0f, 0.0f, tx },
        .r1 = { 0.0f, sy,   0.0f, ty },
        .r2 = { 0.0f, 0.0f, sz,   tz },
        .r3 = { 0.0f, 0.0f, 0.0f, 1.0f },
    };
}

#endif
