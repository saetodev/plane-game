#include "core/Application.h"
#include "World.h"

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

using Path = std::vector<Vec2>;

struct SpatialSystem : public System {
    EntityID EntityAtPosition(const Vec2& position) {
        EntityStorage* entityStorage = EntityStorage::Instance();

        for (EntityID entity : m_entities) {
            const Transform& transform = entityStorage->GetComponent<Transform>(entity);

            float x0 = transform.position.x - (transform.size.x / 2.0f);
            float y0 = transform.position.y - (transform.size.y / 2.0f);

            float x1 = x0 + transform.size.x;
            float y1 = y0 + transform.size.y;

            if (position.x >= x0 && position.x <= x1 && position.y >= y0 && position.y <= y1) {
                return entity;
            }
        }

        return 0;
    }
};

struct PhysicsSystem : public System {
    void Update(TimeStep timeStep) {
        EntityStorage* entityStorage = EntityStorage::Instance();

        for (EntityID entity : m_entities) {
            Transform& transform = entityStorage->GetComponent<Transform>(entity);
            Motion& motion = entityStorage->GetComponent<Motion>(entity);
            Path& path = entityStorage->GetComponent<Path>(entity);

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

     void MoveEntityAlongPath(Transform& transform, Motion& motion, Path& path) {
        if (path.size() < 2) {
            return;
        }

        Vec2 targetPoint = path[0];
        Vec2 moveVector = targetPoint - transform.position;

        if (moveVector.Length() < 1) {
            path.erase(path.begin());

            targetPoint = path[0];
            moveVector = targetPoint - transform.position;
        }

        motion.velocity = moveVector.Normalized() * 75;
    }
};

struct RenderSystem : public System {
    void Render(SDL_Renderer* renderer) {
        EntityStorage* entityStorage = EntityStorage::Instance();

        for (EntityID entity : m_entities) {
            const Transform& transform = entityStorage->GetComponent<Transform>(entity);
            const Path& path = entityStorage->GetComponent<Path>(entity);
            const Color& color = entityStorage->GetComponent<Color>(entity);

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

    void RenderPath(SDL_Renderer* renderer, const Path& path) {
        if (path.size() < 2) {
            return;
        }

        for (int i = 0; i < path.size() - 1; i++) {
            Vec2 start = path[i];
            Vec2 end = path[i + 1];

            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderDrawLine(renderer, start.x, start.y, end.x, end.y);
        }
    }
};

class App : public Application {
public:
    void OnInit() override {
        m_entityStorage.RegisterComponent<Transform>();
        m_entityStorage.RegisterComponent<Motion>();
        m_entityStorage.RegisterComponent<Path>();
        m_entityStorage.RegisterComponent<Color>();

        {
            m_spatialSystem = reinterpret_cast<SpatialSystem*>(m_entityStorage.RegisterSystem<SpatialSystem>());

            Signature signature;
            signature.set(m_entityStorage.GetComponentType<Transform>());

            m_entityStorage.SetSystemSignature<SpatialSystem>(signature);
        }

        {
            m_physicsSystem = reinterpret_cast<PhysicsSystem*>(m_entityStorage.RegisterSystem<PhysicsSystem>());

            Signature signature;
            signature.set(m_entityStorage.GetComponentType<Transform>());
            signature.set(m_entityStorage.GetComponentType<Motion>());
            signature.set(m_entityStorage.GetComponentType<Path>());

            m_entityStorage.SetSystemSignature<PhysicsSystem>(signature);
        }
        
        {
            m_renderSystem = reinterpret_cast<RenderSystem*>(m_entityStorage.RegisterSystem<RenderSystem>());

            Signature signature;
            signature.set(m_entityStorage.GetComponentType<Transform>());
            signature.set(m_entityStorage.GetComponentType<Path>());
            signature.set(m_entityStorage.GetComponentType<Color>());

            m_entityStorage.SetSystemSignature<RenderSystem>(signature);
        }

        {
            EntityID entity = m_entityStorage.CreateEntity();

            m_entityStorage.AddComponent<Transform>(entity, {
                .position = { 1280 / 2.0f, 720 / 2.0f },
                .size = { 32, 32 },
            });

            m_entityStorage.AddComponent<Motion>(entity, {});
            m_entityStorage.AddComponent<Path>(entity, {});
            m_entityStorage.AddComponent<Color>(entity, { 0, 0, 255, 255 });
        }
        
        {
            EntityID entity = m_entityStorage.CreateEntity();

            m_entityStorage.AddComponent<Transform>(entity, {
                .position = { 300, 200 },
                .size = { 32, 32 },
            });

            m_entityStorage.AddComponent<Motion>(entity, {});
            m_entityStorage.AddComponent<Path>(entity, {});
            m_entityStorage.AddComponent<Color>(entity, { 128, 0, 255, 255 });
        }
    }

    void OnUpdate(TimeStep timeStep) override {
        int mouseX, mouseY;
        int mouseState = SDL_GetMouseState(&mouseX, &mouseY);

        Vec2 mousePos = { (float)mouseX, (float)mouseY };

        if ((mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) && !m_canDrawPath) {
            m_selectedEntity = m_spatialSystem->EntityAtPosition(mousePos);

            if (m_selectedEntity != 0) {
                Path& path = m_entityStorage.GetComponent<Path>(m_selectedEntity);

                m_canDrawPath = true;
                path.clear();
            }
        }
        else if (!(mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) && m_canDrawPath) {
            m_canDrawPath = false;
            m_selectedEntity = 0;
        }

        if (m_canDrawPath) {
            if (mousePos != m_lastMousePos) {
                Path& path = m_entityStorage.GetComponent<Path>(m_selectedEntity);
                path.push_back(mousePos);
            }

            m_lastMousePos = mousePos;
        }

        m_physicsSystem->Update(timeStep);
        m_renderSystem->Render(m_renderer);
    }
private:
    Uint64 m_lastTime = 0;

    bool m_canDrawPath = false;
    EntityID m_selectedEntity = 0;
    Vec2 m_lastMousePos;

    EntityStorage m_entityStorage;

    SpatialSystem* m_spatialSystem;
    PhysicsSystem* m_physicsSystem;
    RenderSystem* m_renderSystem;
};

int main(int argc, char** argv) {
    App().Run();

    return 0;
}
