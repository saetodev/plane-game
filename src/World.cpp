#include "World.h"

#include <random>

#include <SDL2/SDL.h>

static std::random_device s_randomDevice;
static std::mt19937_64 s_randomEngine(s_randomDevice());
static std::uniform_int_distribution<uint64_t> s_uniformDistribution;

static EntityID GenerateEntityID() {
    EntityID id;

    do {
        id = s_uniformDistribution(s_randomEngine);
    } while (id == 0);

    return id;
}

Entity* EntityStorage::CreateEntity() {
    int index = m_entities.size();
    EntityID id = GenerateEntityID();

    m_entities.push_back({ .id = id });
    m_entityMap.emplace(id, index);

    return &m_entities.back();
}

void EntityStorage::DestroyEntity(EntityID id) {
    if (id == 0) {
        return;
    }

    if (!m_entityMap.contains(id)) {
        return;
    }

    int index = m_entityMap[id];

    m_entities.erase(m_entities.begin() + index);
    m_entityMap.erase(id);
}

Entity* EntityStorage::GetEntity(EntityID id) {
    if (id == 0) {
        return NULL;
    }

    if (m_entityMap.contains(id)) {
        return &m_entities[m_entityMap[id]];
    }

    return NULL;
}

EntityID World::EntityAtPosition(const Vec2& position) {
    for (const Entity& entity : entityStorage) {
        SDL_FRect rect = {
            entity.position.x - (entity.size.x / 2.0f),
            entity.position.y - (entity.size.y / 2.0f),
            entity.size.x,
            entity.size.y,
        };

        if ((position.x <= rect.x + rect.w) && (position.x >= rect.x) && (position.y <= rect.y + rect.h) && (position.y >= rect.y)) {
            return entity.id;
        }
    }

    return 0;
}

static void MoveEntityAlongPath(Entity& entity) {
    if (entity.path.size() < 2) {
        return;
    }

    Vec2 targetPoint = entity.path[0];
    Vec2 moveVector = targetPoint - entity.position;

    if (moveVector.Length() < 1) {
        entity.path.erase(entity.path.begin());

        targetPoint = entity.path[0];
        moveVector = targetPoint - entity.position;
    }

    entity.velocity = moveVector.Normalized() * 75;
}

void World::DoPhysics(float deltaTime) {
    for (Entity& entity : entityStorage) {
        MoveEntityAlongPath(entity);

        entity.position += entity.velocity * deltaTime;

        float x0 = entity.position.x - (entity.size.x / 2.0f);
        float x1 = x0 + entity.size.x;

        float y0 = entity.position.y - (entity.size.y / 2.0f);
        float y1 = y0 + entity.size.y;

        if (x0 <= 0 || x1 >= 1280) {
            entity.velocity.x = -entity.velocity.x;
        }

        if (y0 <= 0 || y1 >= 720) {
            entity.velocity.y = -entity.velocity.y;
        }
    }
}

void World::Draw(SDL_Renderer* renderer) {
    for (const Entity& entity : entityStorage) {
        SDL_FRect rect = {
            entity.position.x - (entity.size.x / 2.0f),
            entity.position.y - (entity.size.y / 2.0f),
            entity.size.x,
            entity.size.y,
        };

        if (entity.path.size() > 0) {
            for (int i = 0; i < entity.path.size() - 1; i++) {
                Vec2 start = entity.path[i];
                Vec2 end = entity.path[i + 1];

                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_RenderDrawLine(renderer, start.x, start.y, end.x, end.y);
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderFillRectF(renderer, &rect);
    }
}
