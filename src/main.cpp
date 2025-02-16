#include "core/Application.h"
#include "core/renderer/Renderer2D.h"
#include "core/renderer/Texture.h"
#include "entity/Entity.h"
#include "entity/World.h"
#include "game.h"

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
    Renderer2D::Begin();
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

        DebugDrawPath(entities[i]);

        SDL_FRect rect = {
            transform.position.x - (transform.size.x / 2.0f),
            transform.position.y - (transform.size.y / 2.0f),
            transform.size.x,
            transform.size.y,
        };

        Renderer2D::DrawTexture(texture, { transform.position, transform.size, transform.rotation + (PI / 2) }, WHITE);
        
        // debug
        {
            Renderer2D::DrawLine(transform.position, transform.position + (motion.velocity.Normalized() * 100), BLUE);
        }
    }

    Renderer2D::End();
}

void OnInit() {
    //m_world.AddSystem(TRANSFORM | MOTION | PATH, PhysicsSystem);
    m_gameState.world.AddSystem(TRANSFORM, CollisionSystem);
    m_gameState.world.AddSystem(TRANSFORM | MOTION | SPRITE | PATH, RenderSystem);

    {
        EntityData* entity = m_gameState.world.CreateEntity(TRANSFORM | MOTION | SPRITE | PATH);

        entity->transform.position.x = 640;
        entity->transform.position.y = 360;

        entity->transform.size.x = 64;
        entity->transform.size.y = 64;

        entity->motion.velocity = { 75, 0 };

        entity->texture = Renderer2D::LoadTexture("data/kenney_pixel-shmup/Ships/ship_0000.png");
    }
}

void OnUpdate(const TimeStep& timeStep) {
    Application::SetWindowTitle(std::format("FrameTime: {} ms", timeStep.DeltaTimeMS()));
    
    UpdateInputState();
    PlacePathPoint();

    m_gameState.world.RunSystems();
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