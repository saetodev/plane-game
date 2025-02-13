#include "core/Application.h"
#include "LinearMath.h"
#include "List.h"
#include "Map.h"

#include <random>

struct Transform {
    Vec2 position;
    Vec2 size;
};

struct Motion {
    Vec2 velocity;
};

struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

using Path = List<Vec2, 512>;
using EntityID = uint64_t;

static constexpr int MAX_ENTITY_COUNT = 256;

enum EntityFlags {
    TRANSFORM = 1 << 0,
    MOTION    = 1 << 1,
    PATH      = 1 << 2,
    COLOR     = 1 << 3,
};

struct Entity {
    EntityID id;

    uint32_t flags;

    Transform transform;
    Motion motion;
    Path path;
    Color color;
};

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

struct World {
    List<Entity, MAX_ENTITY_COUNT> entityList;
    Map<EntityID, size_t, MAX_ENTITY_COUNT> entityMap;

    EntityID CreateEntity(uint32_t flags) {
        EntityID id = GenerateEntityID();
        size_t index = entityList.Size();

        ASSERT(!entityMap.Contains(id));

        entityMap.Add(id, index);
        entityList.Push({ .id = id, .flags = flags });

        return id;
    }

    void DestroyEntity(EntityID id) {
        if (!entityMap.Contains(id)) {
            return;
        }

        size_t lastIndex    = entityList.Size() - 1;
        size_t currentIndex = entityMap.Get(id);
        EntityID lastEntity = entityList[lastIndex].id;

        entityMap.Set(lastEntity, currentIndex);
        entityMap.Remove(id);

        entityList.QuickRemove(currentIndex);
    }

    Entity* GetEntity(EntityID id) {
        if (!entityMap.Contains(id)) {
            return NULL;
        }

        return &entityList[entityMap.Get(id)];
    }

    List<EntityID, MAX_ENTITY_COUNT> EntitiesWithFlags(uint32_t flags) const {
        List<EntityID, MAX_ENTITY_COUNT> result;

        for (int i = 0; i < entityList.Size(); i++) {
            const Entity& entity = entityList[i];

            if ((entity.flags & flags) == flags) {
                result.Push(entity.id);
            }
        }

        return result;
    }
};

EntityID EntityAtPosition(World& world, const Vec2& position) {
    auto entities = world.EntitiesWithFlags(EntityFlags::TRANSFORM);

    for (int i = 0; i < entities.Size(); i++) {
        const Transform& transform = world.GetEntity(entities[i])->transform;

        float x0 = transform.position.x - (transform.size.x / 2.0f);
        float y0 = transform.position.y - (transform.size.y / 2.0f);

        float x1 = x0 + transform.size.x;
        float y1 = y0 + transform.size.y;

        if (position.x >= x0 && position.x <= x1 && position.y >= y0 && position.y <= y1) {
            return entities[i];
        }
    }

    return 0;
}

void MoveEntityAlongPath(Transform& transform, Motion& motion, Path& path) {
    if (path.Size() < 2) {
        return;
    }

    Vec2 targetPoint = path[0];
    Vec2 moveVector = targetPoint - transform.position;

    if (moveVector.Length() < 1) {
        path.Remove(0);

        targetPoint = path[0];
        moveVector = targetPoint - transform.position;
    }

    motion.velocity = moveVector.Normalized() * 75;
}

void UpdateEntityPhysics(World& world, TimeStep timeStep) {
    auto entities = world.EntitiesWithFlags(EntityFlags::TRANSFORM | EntityFlags::MOTION | EntityFlags::PATH);

    for (int i = 0; i < entities.Size(); i++) {
        Entity* entity = world.GetEntity(entities[i]);

        Transform& transform = entity->transform;
        Motion& motion = entity->motion;
        Path& path = entity->path;

        MoveEntityAlongPath(transform, motion, path);

        transform.position += motion.velocity * timeStep.DeltaTime();

        float x0 = transform.position.x - (transform.size.x / 2.0f);
        float y0 = transform.position.y - (transform.size.y / 2.0f);

        float x1 = x0 + transform.size.x;
        float y1 = y0 + transform.size.y;

        if (x0 <= 0 || x1 >= 1280) {
            motion.velocity.x = -motion.velocity.x;
        }

        if (y0 <= 0 || y1 >= 720) {
            motion.velocity.y = -motion.velocity.y;
        }
    }
}

void RenderPath(SDL_Renderer* renderer, const Path& path) {
    if (path.Size() < 2) {
        return;
    }

    for (int i = 0; i < path.Size() - 1; i++) {
        Vec2 start = path[i];
        Vec2 end = path[i + 1];

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawLine(renderer, start.x, start.y, end.x, end.y);
    }
}

void RenderWorld(SDL_Renderer* renderer, World& world) {
    auto entities = world.EntitiesWithFlags(EntityFlags::TRANSFORM | EntityFlags::PATH | EntityFlags::COLOR);

    for (int i = 0; i < entities.Size(); i++) {
        Entity* entity = world.GetEntity(entities[i]);

        const Transform& transform = entity->transform;
        const Path& path = entity->path;
        const Color& color = entity->color;

        RenderPath(renderer, path);

        SDL_FRect rect = {
            transform.position.x - (transform.size.x / 2.0f),
            transform.position.y - (transform.size.y / 2.0f),
            transform.size.x,
            transform.size.y,
        };

        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRectF(renderer, &rect);
    }
}

class App : public Application {
public:
    void OnInit() override {
        {
            EntityID id = m_world.CreateEntity(TRANSFORM | MOTION | PATH | COLOR);

            Entity* entity = m_world.GetEntity(id);

            entity->transform.position = { 1280 / 2, 720 / 2 };
            entity->transform.size = { 32, 32 };

            entity->color = { 0, 0, 255, 255 };
        }

        {
            EntityID id = m_world.CreateEntity(TRANSFORM | MOTION | PATH | COLOR);

            Entity* entity = m_world.GetEntity(id);

            entity->transform.position = { 300, 200 };
            entity->transform.size = { 32, 32 };

            entity->color = { 128, 0, 255, 255 };
        }
    }

    void OnUpdate(TimeStep timeStep) override {
        int mouseX, mouseY;
        int mouseState = SDL_GetMouseState(&mouseX, &mouseY);

        Vec2 mousePos = { (float)mouseX, (float)mouseY };

        if ((mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) && !m_canDrawPath) {
            m_selectedEntity = EntityAtPosition(m_world, mousePos);

            if (m_selectedEntity != 0) {
                Entity* entity = m_world.GetEntity(m_selectedEntity);

                m_canDrawPath = true;
                entity->path.Clear();
            }
        }
        else if (!(mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) && m_canDrawPath) {
            m_canDrawPath = false;
            m_selectedEntity = 0;
        }

        if (m_canDrawPath) {
            if (mousePos != m_lastMousePos) {
                Entity* entity = m_world.GetEntity(m_selectedEntity);
                if (!entity->path.Full()) {
                    entity->path.Push(mousePos);
                }
            }

            m_lastMousePos = mousePos;
        }

        UpdateEntityPhysics(m_world, timeStep);
        RenderWorld(m_renderer, m_world);
    }
private:
    Uint64 m_lastTime = 0;

    bool m_canDrawPath = false;
    EntityID m_selectedEntity = 0;
    Vec2 m_lastMousePos;

    World m_world;
};

int main(int argc, char** argv) {
    (new App())->Run();

    return 0;
}
