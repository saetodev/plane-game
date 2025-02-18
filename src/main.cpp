#include "core/Application.h"
#include "core/renderer/Renderer2D.h"
#include "core/renderer/Texture.h"
#include "core/renderer/Shader.h"
#include "entity/Entity.h"
#include "entity/World.h"
#include "game.h"

#include <format>
#include <iostream>
#include <random>

#include <imgui.h>
#include <SDL.h>

void MotionSystem(World* world, List<EntityID, MAX_ENTITY_COUNT>& entities) {
    Vec2 windowSize = Application::WindowSize();
    const TimeStep& timeStep = Application::FrameTime();
    
    for (int i = 0; i < entities.Size(); i++) {
        EntityData* entity = world->GetEntityData(entities[i]);

        if (!entity) {
            continue;
        }

        Transform& transform = entity->transform;
        Motion& motion       = entity->motion;

        motion.velocity    += motion.acceleration * timeStep.DeltaTime();
        transform.position += motion.velocity * timeStep.DeltaTime();

        if (transform.position.x < 0 || transform.position.x > windowSize.x) {
            motion.velocity.x = -motion.velocity.x;
        }

        if (transform.position.y < 0 || transform.position.y > windowSize.y) {
            motion.velocity.y = -motion.velocity.y;
        }

        transform.rotation = motion.velocity.Angle();
    }
}

void PathSystem(World* world, List<EntityID, MAX_ENTITY_COUNT>& entities) {
    for (int i = 0; i < entities.Size(); i++) {
        EntityData* entity = world->GetEntityData(entities[i]);

        if (!entity) {
            continue;
        }

        Transform& transform = entity->transform;
        Motion& motion = entity->motion;
        List<Vec2, MAX_PATH_SIZE>& path = entity->path;

        if (path.Size() < 2) {
            motion.acceleration = {};
            motion.velocity = motion.velocity.Normalized() * 75;
            continue;
        }

        Vec2 targetPoint = path[0];
        Vec2 moveVector  = targetPoint - transform.position;

        //TODO: the position of this entity, is actually at the 
        //      center of any texture that we may draw. we should 
        //      move on to the next point if the target point is within 
        //      the rect that contains the texture (i.e. use tranform.size)
        if (moveVector.Length() <= m_gameState.tileSize * 2) {
            path.Remove(0);

            targetPoint = path[0];
            moveVector = targetPoint - transform.position;
        }

        Vec2 targetVelocity = moveVector.Normalized() * 75;
        motion.acceleration = targetVelocity - motion.velocity;
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
            Renderer2D::DrawRectLines(transform, GREEN);
            Renderer2D::DrawLine(transform.position, transform.position + motion.acceleration, {1, 0, 1, 1 });
            Renderer2D::DrawLine(transform.position, transform.position + motion.velocity, BLUE);
        }
    }

    Renderer2D::End();
}

void OnInit() {
    m_gameState.world.AddSystem(TRANSFORM | MOTION | PATH, PathSystem);
    m_gameState.world.AddSystem(TRANSFORM | MOTION, MotionSystem);
    m_gameState.world.AddSystem(TRANSFORM | MOTION | SPRITE | PATH, RenderSystem);

    Vec2 windowSize = Application::WindowSize();

    srand(time(NULL));

    for (int i = 0; i < 1; i++) {
        EntityData* entity = m_gameState.world.CreateEntity(TRANSFORM | MOTION | SPRITE | PATH);

        entity->transform.position.x = (rand() % (int)windowSize.x - 32) + 32;
        entity->transform.position.y = (rand() % (int)windowSize.y - 32) + 32;

        entity->transform.size.x = 80;
        entity->transform.size.y = 80;

        f32 angle = (rand() % 360) * DEG_TO_RAD;

        entity->motion.velocity.x = 75 * cosf(angle);
        entity->motion.velocity.y = 75 * sinf(angle);

        entity->texture = Renderer2D::LoadTexture("data/kenney_pixel-shmup/Ships/ship_0000.png");
    }
}

void OnUpdate(const TimeStep& timeStep) {
    Application::SetWindowTitle(std::format("FrameTime: {} ms", timeStep.DeltaTimeMS()));
    
    ImGuiIO& io = ImGui::GetIO();

    if (m_gameState.canDrawPath) {
        io.WantCaptureMouse = false;
    }

    if (!io.WantCaptureMouse) {
        UpdateInputState();
        PlacePathPoint();
    }

    m_gameState.world.RunSystems();

    Application::ImGuiNewFrame();

    ImGui::Begin("Entity Info");

    EntityData* entity = m_gameState.world.GetEntityData(m_gameState.selectedEntity);
    
    if (entity) {
        ImGui::Text("Entity ID: %llu", m_gameState.selectedEntity);
        ImGui::Text("Position: %f, %f", entity->transform.position.x, entity->transform.position.y);
        ImGui::Text("Size: %f, %f", entity->transform.size.x, entity->transform.size.y);
        ImGui::Text("Rotation: %f", entity->transform.rotation);
        ImGui::Text("Velocity: %f, %f", entity->motion.velocity.x, entity->motion.velocity.y);
        ImGui::Text("Speed: %f", entity->motion.velocity.Length());
        ImGui::Text("Acceleration: %f, %f", entity->motion.acceleration.x, entity->motion.acceleration.y);
    }
    
    ImGui::End();

    Application::ImGuiRender();
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