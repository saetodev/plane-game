#ifndef ENTITY_H
#define ENTITY_H

#include "LinearMath.h"
#include "List.h"

#define MAX_ENTITY_COUNT 256
#define MAX_PATH_SIZE 512

using EntityID = u64;

enum EntityFlags {
    TRANSFORM  = 1 << 0,
    RENDERABLE = 1 << 1,
    MOTION     = 1 << 2,
    PATH       = 1 << 3,
};

struct Transform {
    Vec2 position;
    Vec2 size;
};

struct Motion {
    Vec2 velocity;
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
    Color color;
    List<Vec2, MAX_PATH_SIZE> path;
};

#endif