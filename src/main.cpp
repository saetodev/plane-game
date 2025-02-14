#include "core/Application.h"
#include "entity/Entity.h"
#include "entity/World.h"

#include <random>

struct Rect {
    Vec2 position;
    Vec2 size;
};

bool PointInRect(const Vec2& point, const Rect& rect) {
    return (point.x >= rect.position.x) && (point.x <= rect.position.x + rect.size.x) && (point.y >= rect.position.y) && (point.y <= rect.position.y + rect.size.y);
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

void UpdateEntityPhysics(World& world, TimeStep timeStep) {
    auto entities = world.EntitiesWithFlags(TRANSFORM | MOTION | PATH);

    for (int i = 0; i < entities.Size(); i++) {
        EntityData* entity = world.GetEntityData(entities[i]);

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

void RenderWorld(SDL_Renderer* renderer, World& world) {
    auto entities = world.EntitiesWithFlags(EntityFlags::TRANSFORM | EntityFlags::PATH | EntityFlags::RENDERABLE);

    for (int i = 0; i < entities.Size(); i++) {
        EntityData* entity = world.GetEntityData(entities[i]);

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
        {
            EntityData* entity = m_world.CreateEntity(TRANSFORM | MOTION | PATH | RENDERABLE);

            entity->transform.position = { 1280 / 2, 720 / 2 };
            entity->transform.size = { 32, 32 };

            entity->color = { 0, 0, 255, 255 };
        }

        {
            EntityData* entity = m_world.CreateEntity(TRANSFORM | MOTION | PATH | RENDERABLE);

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
