#ifndef WORLD_H
#define WORLD_H

#include "LinearMath.h"

#include <vector>

struct SDL_Renderer;

typedef uint64_t EntityID;

struct Entity {
    EntityID id;

    Vec2 position;
    Vec2 velocity;
    Vec2 size;

    std::vector<Vec2> path;
};

class World {
public:
    Entity* CreateEntity();
    Entity* GetEntity(EntityID id);

    EntityID EntityAtPosition(const Vec2& position);

    void DoPhysics(float deltaTime);
    void Draw(SDL_Renderer* renderer);

private:
    std::vector<Entity> m_entities;
};

#endif
