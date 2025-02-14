#include "core/Application.h"
#include "entity/Entity.h"
#include "entity/World.h"

#include <random>

struct Rect {
    Vec2 position;
    Vec2 size;
};

bool PointInRect(const Vec2& point, const Rect& rect) {
    return (point.x >= rect.position.x) && 
           (point.x <= rect.position.x + rect.size.x) && 
           (point.y >= rect.position.y) && 
           (point.y <= rect.position.y + rect.size.y);
}

bool RectInRect(const Rect& a, const Rect& b) {
    return (a.position.x + a.size.x >= b.position.x) &&
           (a.position.x <= b.position.x + b.size.x) && 
           (a.position.y + a.size.y >= b.position.y) && 
           (a.position.y <= b.position.y + b.size.y);
}

Rect GetEntityRect(World& world, EntityID id) {
    EntityData* entity = world.GetEntityData(id);

    if (!entity) {
        return {};
    }

    return {
        .position = {
            entity->transform.position.x - (entity->transform.size.x / 2.0f),
            entity->transform.position.y - (entity->transform.size.y / 2.0f),
        },

        .size = {
            entity->transform.size.x,
            entity->transform.size.y,
        },
    };
}

EntityID EntityAtPosition(World& world, const Vec2& position) {
    auto entities = world.EntitiesWithFlags(EntityFlags::TRANSFORM);

    for (int i = 0; i < entities.Size(); i++) {
        EntityID id = entities[i];
        const Transform& transform = world.GetEntityData(id)->transform;

        if (PointInRect(position, GetEntityRect(world, id))) {
            return id;
        }
    }

    return 0;
}

void MoveEntityAlongPath(Transform& transform, Motion& motion, List<Vec2, MAX_PATH_SIZE>& path) {
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

void RenderPath(SDL_Renderer* renderer, const List<Vec2, MAX_PATH_SIZE>& path) {
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

void PhysicsSystem(World* world, List<EntityID, MAX_ENTITY_COUNT>& entities) {
    const TimeStep& timeStep = Application::Instance()->FrameTime();

    for (int i = 0; i < entities.Size(); i++) {
        EntityData* entity = world->GetEntityData(entities[i]);

        if (!entity) {
            continue;
        }

        Transform& transform = entity->transform;
        Motion& motion = entity->motion;
        List<Vec2, MAX_PATH_SIZE>& path = entity->path;

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

void CollisionSystem(World* world, List<EntityID, MAX_ENTITY_COUNT>& entities) {
    for (int i = 0; i < entities.Size() - 1; i++) {
        for (int j = i + 1; j < entities.Size(); j++) {
            EntityData* entityA = world->GetEntityData(entities[i]);
            EntityData* entityB = world->GetEntityData(entities[j]);

            if (!entityA) {
                break;
            }
            else if (!entityB) {
                continue;
            }

            Rect rectA = GetEntityRect(*world, entityA->id);
            Rect rectB = GetEntityRect(*world, entityB->id);

            if (RectInRect(rectA, rectB)) {
                world->DestroyEntity(entityA->id);
                world->DestroyEntity(entityB->id);
            }
        }
    }
}

void RenderSystem(World* world, List<EntityID, MAX_ENTITY_COUNT>& entities) {
    SDL_Renderer* renderer = Application::Instance()->Renderer();

    for (int i = 0; i < entities.Size(); i++) {
        EntityData* entity = world->GetEntityData(entities[i]);

        if (!entity) {
            continue;
        }

        const Transform& transform = entity->transform;
        const List<Vec2, MAX_PATH_SIZE>& path = entity->path;
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
        m_world.AddSystem(TRANSFORM | MOTION | PATH, PhysicsSystem);
        m_world.AddSystem(TRANSFORM, CollisionSystem);
        m_world.AddSystem(RENDERABLE | TRANSFORM | PATH, RenderSystem);

        for (int i = 0; i < 100; i++) {
            EntityData* entity = m_world.CreateEntity(TRANSFORM | MOTION | PATH | RENDERABLE);

            entity->transform.position.x = (rand() % 1200) + 40;
            entity->transform.position.y = (rand() % 640) + 40;

            entity->transform.size.x = 32;
            entity->transform.size.y = 32;

            entity->motion.velocity.x = (rand() % 2 * 75) - 75;
            entity->motion.velocity.y = (rand() % 2 * 75) - 75;

            entity->color = { 0, 0, 255, 255 };
        }
    }

    void OnUpdate(TimeStep timeStep) override {
        int mouseX, mouseY;
        int mouseState = SDL_GetMouseState(&mouseX, &mouseY);

        Vec2 mousePos = { (float)mouseX, (float)mouseY };

        if ((mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) && !m_canDrawPath) {
            m_selectedEntity = EntityAtPosition(m_world, mousePos);

            if (m_selectedEntity != 0) {
                EntityData* entity = m_world.GetEntityData(m_selectedEntity);

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
                EntityData* entity = m_world.GetEntityData(m_selectedEntity);
                if (!entity->path.Full()) {
                    entity->path.Push(mousePos);
                }
            }

            m_lastMousePos = mousePos;
        }

        m_world.RunSystems();
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
