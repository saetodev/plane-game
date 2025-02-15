#ifndef ENTITY_H
#define ENTITY_H

#include "core/Renderer2D.h"
#include "LinearMath.h"
#include "List.h"

#define MAX_ENTITY_COUNT 256
#define MAX_PATH_SIZE 512

using EntityID = u64;

enum EntityFlags {
    TRANSFORM  = 1 << 0,
    SPRITE     = 1 << 1,
    MOTION     = 1 << 2,
    PATH       = 1 << 3,
};

struct Motion {
    Vec2 velocity;
    Vec2 acceleration;

    f32 angularVelocity;
    f32 angularAcceleration;
};

struct Color {
    u8 r;
    u8 g;
    u8 b;
    u8 a;
};

struct EntityData {
    EntityID id;
    u64 flags;

    Transform transform;
    Motion motion;
    Texture2D texture;
    List<Vec2, MAX_PATH_SIZE> path;
};

#endif