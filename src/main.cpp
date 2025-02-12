#include "core/Application.h"

#include <vector>

struct Vec2 {
    float x;
    float y;
};

Vec2& operator+=(Vec2& a, const Vec2& b) {
    a.x += b.x;
    a.y += b.y;
    return a;
}

Vec2 operator*(const Vec2& v, float s) {
    return { v.x * s, v.y * s };
}

struct Entity {
    Vec2 position;
    Vec2 velocity;
    Vec2 size;

    std::vector<Vec2> path;
};

class World {
public:
    Entity& CreateEntity();

    void DoPhysics(float deltaTime);
    void Draw(SDL_Renderer* renderer);

private:
    std::vector<Entity> m_entities;
};

Entity& World::CreateEntity() {
    m_entities.push_back({});
    return m_entities.back();
}

void World::DoPhysics(float deltaTime) {
    for (Entity& entity : m_entities) {
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
    for (const Entity& entity : m_entities) {
        SDL_FRect rect = {
            entity.position.x - (entity.size.x / 2.0f),
            entity.position.y - (entity.size.y / 2.0f),
            entity.size.x,
            entity.size.y,
        };

        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderFillRectF(renderer, &rect);
    }
}

class App : public Application {
public:
    void OnInit() override {
        Entity& entity = m_world.CreateEntity();

        entity.position.x = 1280 / 2.0f;
        entity.position.y = 720 / 2.0f;

        entity.velocity.x = 150;

        entity.size.x = 32;
        entity.size.y = 32;
    }

    void OnUpdate() override {
        Uint64 nowTime = SDL_GetTicks64();
        float deltaTime = (float)(nowTime - m_lastTime) / 1000.0f;
        m_lastTime = nowTime;

        m_world.DoPhysics(deltaTime);
        m_world.Draw(m_renderer);
    }
private:
    Uint64 m_lastTime = 0;

    World m_world;
};

int main(int argc, char** argv) {
    App().Run();

    return 0;
}
