#include "core/Application.h"
#include "core/Renderer2D.h"
#include "entity/Entity.h"
#include "entity/World.h"

#include <format>
#include <iostream>
#include <random>

#include <SDL2/SDL.h>

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
        motion.acceleration = { 0, 0 };
        return;
    }

    Vec2 targetPoint = path[0];
    Vec2 moveVector  = targetPoint - transform.position;

    if (moveVector.Length() < 1) {
        path.Remove(0);

        targetPoint = path[0];
        moveVector = targetPoint - transform.position;
    }

    f32 speed = motion.velocity.Length();

    Vec2 currentDirection = motion.velocity.Normalized();
    Vec2 nextDirection    = moveVector.Normalized();

    motion.acceleration = (nextDirection - currentDirection) * speed;
}

void IntegrateMotion(Transform& transform, Motion& motion, const TimeStep& timeStep) {
    motion.velocity += motion.acceleration;
    transform.position += motion.velocity * timeStep.DeltaTime();

    transform.rotation = motion.velocity.Angle();
}

void ForceBoundaryConditions(Transform& transform, Motion& motion) {
    float x0 = transform.position.x - (transform.size.x / 2.0f);
    float y0 = transform.position.y - (transform.size.y / 2.0f);

    float x1 = x0 + transform.size.x;
    float y1 = y0 + transform.size.y;

    Vec2 windowSize = Application::WindowSize();

    if (x0 <= 0 || x1 >= windowSize.x) {
        motion.velocity.x  = -motion.velocity.x;
        transform.rotation = motion.velocity.Angle();
    }

    if (y0 <= 0 || y1 >= windowSize.y) {
        motion.velocity.y  = -motion.velocity.y;
        transform.rotation = motion.velocity.Angle();
    }
}

void RenderPath(const List<Vec2, MAX_PATH_SIZE>& path) {
    if (path.Size() < 2) {
        return;
    }

    for (int i = 0; i < path.Size() - 1; i++) {
        Vec2 start = path[i];
        Vec2 end = path[i + 1];

        Renderer2D::DrawLine(start, end, RED);
    }
}

void PhysicsSystem(World* world, List<EntityID, MAX_ENTITY_COUNT>& entities) {
    const TimeStep& timeStep = Application::FrameTime();

    for (int i = 0; i < entities.Size(); i++) {
        EntityData* entity = world->GetEntityData(entities[i]);

        if (!entity) {
            continue;
        }

        Transform& transform = entity->transform;
        Motion& motion = entity->motion;
        List<Vec2, MAX_PATH_SIZE>& path = entity->path;

        std::cout << "ROTATION: " << transform.rotation << std::endl;

        MoveEntityAlongPath(transform, motion, path);
        IntegrateMotion(transform, motion, timeStep);
        ForceBoundaryConditions(transform, motion);
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
    Renderer2D::Clear({ 0.25f, 0.25f, 0.25f, 1.0f });
    
    for (int i = 0; i < entities.Size(); i++) {
        EntityData* entity = world->GetEntityData(entities[i]);

        if (!entity) {
            continue;
        }

        const Transform& transform = entity->transform;
        const Motion& motion = entity->motion;
        const List<Vec2, MAX_PATH_SIZE>& path = entity->path;
        const Texture2D& texture = entity->texture;

        RenderPath(path);

        SDL_FRect rect = {
            transform.position.x - (transform.size.x / 2.0f),
            transform.position.y - (transform.size.y / 2.0f),
            transform.size.x,
            transform.size.y,
        };

        Renderer2D::DrawTexture(texture, { transform.position, transform.size, transform.rotation * RAD_TO_DEG + 90 }, WHITE);
        
        // debug
        {
            Renderer2D::DrawLine(transform.position, transform.position + (motion.velocity.Normalized() * 100), BLUE);
        }
    }
}

bool m_canDrawPath = false;
EntityID m_selectedEntity = 0;
Vec2 m_lastMousePos;

Texture2D m_texture;

World m_world;

int m_tileSize = 8;
List<Vec2, MAX_PATH_SIZE> m_path;

int m_lastTileX = 0;
int m_lastTileY = 0;

void OnInit() {
    m_texture = Renderer2D::LoadTexture("data/kenney_pixel-shmup/Ships/ship_0000.png");

    //m_world.AddSystem(TRANSFORM | MOTION | PATH, PhysicsSystem);
    m_world.AddSystem(TRANSFORM, CollisionSystem);
    m_world.AddSystem(TRANSFORM | MOTION | SPRITE | PATH, RenderSystem);

    {
        EntityData* entity = m_world.CreateEntity(TRANSFORM | MOTION | SPRITE | PATH);

        entity->transform.position.x = 640;
        entity->transform.position.y = 360;

        entity->transform.size.x = 64;
        entity->transform.size.y = 64;

        entity->motion.velocity = { 75, 0 };

        entity->texture = m_texture;
    }
}

void OnUpdate(const TimeStep& timeStep) {
    Application::SetWindowTitle(std::format("FrameTime: {} ms", timeStep.DeltaTimeMS()));

#if 0
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
#endif
    Renderer2D::Begin();

    Vec2 mousePos = Application::MousePos();

    if (Application::MousePressed(SDL_BUTTON_LEFT)) {
        m_selectedEntity = EntityAtPosition(m_world, mousePos);
        
        if (m_selectedEntity != 0) {
            m_canDrawPath = true;
            m_path.Clear();
        }
    }
    else if (Application::MouseReleased(SDL_BUTTON_LEFT)) {
        m_canDrawPath = false;
    }

    if (m_canDrawPath && !m_path.Full()) {
        int tileX = mousePos.x / m_tileSize;
        int tileY = mousePos.y / m_tileSize;

        if (tileX != m_lastTileX || tileY != m_lastTileY) {
            f32 x = (tileX * m_tileSize) + (m_tileSize / 2);
            f32 y = (tileY * m_tileSize) + (m_tileSize / 2);

            m_path.Push({ x, y });
        }

        m_lastTileX = tileX;
        m_lastTileY = tileY;
    }

    m_world.RunSystems();

    Vec2 windowSize = Application::WindowSize();

    // draw path
    if (m_path.Size() > 2) {
        for (int i = 0; i < m_path.Size() - 1; i++) {
            Renderer2D::DrawLine(m_path[i], m_path[i + 1], RED);
        }
    }

    // draw path points
    for (int i = 0; i < m_path.Size(); i++) {
        Renderer2D::DrawRect(m_path[i], { (f32) m_tileSize / 4, (f32)m_tileSize / 4 }, WHITE);
    }

    Renderer2D::End();
}

int main(int argc, char** argv) {
    AppDesc desc = {
        .windowWidth  = 1280,
        .windowHeight = 720,
        .windowTitle  = "PLANE",
    };

    Application::Init(desc);
    Application::Run(OnInit, OnUpdate);
    Application::Shutdown();

    return 0;
}