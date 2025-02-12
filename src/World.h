#ifndef WORLD_H
#define WORLD_H

#include "LinearMath.h"

#include <vector>
#include <unordered_map>

struct SDL_Renderer;

typedef uint64_t EntityID;

struct Entity {
    EntityID id;

    Vec2 position;
    Vec2 velocity;
    Vec2 size;

    std::vector<Vec2> path;
};

class EntityStorage {
public:
    Entity* CreateEntity();
    void DestroyEntity(EntityID id);

    Entity* GetEntity(EntityID id);

    std::vector<Entity>::iterator begin() {
        return m_entities.begin();
    }

    std::vector<Entity>::iterator end() {
        return m_entities.end();
    }

private:
    std::vector<Entity> m_entities;
    std::unordered_map<EntityID, int> m_entityMap;
};

struct World {
    EntityStorage entityStorage;

    EntityID EntityAtPosition(const Vec2& position);

    void DoPhysics(float deltaTime);
    void Draw(SDL_Renderer* renderer);
};

#endif
