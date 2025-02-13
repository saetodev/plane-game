#include "core/Application.h"
#include "World.h"

static void MoveEntityAlongPath(PhysicsComponent& physics, PathComponent& path) {
    if (path.points.size() < 2) {
        return;
    }

    Vec2 targetPoint = path.points[0];
    Vec2 moveVector = targetPoint - physics.position;

    if (moveVector.Length() < 1) {
        path.points.erase(path.points.begin());

        targetPoint = path.points[0];
        moveVector = targetPoint - physics.position;
    }

    physics.velocity = moveVector.Normalized() * 75;
}

class App : public Application {
public:
    void OnInit() override {
        m_entityStorage.Register<PhysicsComponent>();
        m_entityStorage.Register<ColorComponent>();
        m_entityStorage.Register<PathComponent>();

        {
            EntityID entity = m_entityStorage.CreateEntity();

            PhysicsComponent& physics = m_entityStorage.CreateComponent<PhysicsComponent>(entity);
            ColorComponent& color = m_entityStorage.CreateComponent<ColorComponent>(entity);
            m_entityStorage.CreateComponent<PathComponent>(entity);

            physics.position.x = 1280 / 2.0f;
            physics.position.y = 720 / 2.0f;

            physics.size.x = 32;
            physics.size.y = 32;

            color.r = 0;
            color.g = 0;
            color.b = 255;
            color.a = 255;
        }

        {
            EntityID entity = m_entityStorage.CreateEntity();

            PhysicsComponent& physics = m_entityStorage.CreateComponent<PhysicsComponent>(entity);
            ColorComponent& color = m_entityStorage.CreateComponent<ColorComponent>(entity);
            m_entityStorage.CreateComponent<PathComponent>(entity);

            physics.position.x = 300;
            physics.position.y = 200;

            physics.size.x = 32;
            physics.size.y = 32;

            color.r = 0;
            color.g = 0;
            color.b = 255;
            color.a = 255;
        }
    }

    void OnUpdate(TimeStep timeStep) override {
        int mouseX, mouseY;
        int mouseState = SDL_GetMouseState(&mouseX, &mouseY);

        Vec2 mousePos = { (float)mouseX, (float)mouseY };

        if ((mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) && !m_canDrawPath) {
            //m_selectedEntity = m_world.EntityAtPosition(mousePos);
            //Entity* entity = m_world.entityStorage.GetEntity(m_selectedEntity);

            m_selectedEntity = EntityAtPosition(mousePos);

            if (m_selectedEntity != 0) {
                auto& path = m_entityStorage.GetComponent<PathComponent>(m_selectedEntity);
                
                m_canDrawPath = true;
                path.points.clear();
            }
        }
        else if (!(mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) && m_canDrawPath) {
            m_canDrawPath = false;
            m_selectedEntity = 0;
        }

        if (m_canDrawPath) {
            if (mousePos != m_lastMousePos) {
                auto& path = m_entityStorage.GetComponent<PathComponent>(m_selectedEntity);
                path.points.push_back(mousePos);
            }

            m_lastMousePos = mousePos;
        }

        DoPhysics(timeStep);
        DoRender(m_renderer);
    }
private:
    EntityID EntityAtPosition(const Vec2& position) {
        const auto& physicsData = m_entityStorage.GetComponents<PhysicsComponent>();

        for (const ComponentPair<PhysicsComponent>& data : physicsData) {
            SDL_FRect rect = {
                data.component.position.x - (data.component.size.x / 2.0f),
                data.component.position.y - (data.component.size.y / 2.0f),
                data.component.size.x,
                data.component.size.y,
            };

            if ((position.x <= rect.x + rect.w) && (position.x >= rect.x) && (position.y <= rect.y + rect.h) && (position.y >= rect.y)) {
                return data.id;
            }
        }
        
        return 0;
    }

    void DoPhysics(TimeStep timeStep) {
        std::vector<EntityID> query = m_entityStorage.GetQuery({ typeid(PhysicsComponent), typeid(PathComponent) });
    
        for (EntityID entity : query) {
            PhysicsComponent& physics = m_entityStorage.GetComponent<PhysicsComponent>(entity);
            PathComponent& path = m_entityStorage.GetComponent<PathComponent>(entity);
    
            MoveEntityAlongPath(physics, path);
    
            physics.position += physics.velocity * timeStep.DeltaTime();
    
            float x0 = physics.position.x - (physics.size.x / 2.0f);
            float x1 = x0 + physics.size.x;
    
            float y0 = physics.position.y - (physics.size.y / 2.0f);
            float y1 = y0 + physics.size.y;
    
            if (x0 <= 0 || x1 >= 1280) {
                physics.velocity.x = -physics.velocity.x;
            }
    
            if (y0 <= 0 || y1 >= 720) {
                physics.velocity.y = -physics.velocity.y;
            }
        }
    }

    void DoRender(SDL_Renderer* renderer) {
        std::vector<EntityID> query = m_entityStorage.GetQuery({ typeid(PhysicsComponent), typeid(PathComponent), typeid(ColorComponent)});
    
        for (EntityID entity : query) {
            auto& physics = m_entityStorage.GetComponent<PhysicsComponent>(entity);
            auto& path = m_entityStorage.GetComponent<PathComponent>(entity);
            auto& color = m_entityStorage.GetComponent<ColorComponent>(entity);
    
            SDL_FRect rect = {
                physics.position.x - (physics.size.x / 2.0f),
                physics.position.y - (physics.size.y / 2.0f),
                physics.size.x,
                physics.size.y,
            };
    
            if (path.points.size() > 0) {
                for (int i = 0; i < path.points.size() - 1; i++) {
                    Vec2 start = path.points[i];
                    Vec2 end = path.points[i + 1];
    
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                    SDL_RenderDrawLine(renderer, start.x, start.y, end.x, end.y);
                }
            }
    
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            SDL_RenderFillRectF(renderer, &rect);
        }
    }

private:
    Uint64 m_lastTime = 0;

    bool m_canDrawPath = false;
    EntityID m_selectedEntity = 0;
    Vec2 m_lastMousePos;

    EntityStorage m_entityStorage;
};

int main(int argc, char** argv) {
    App().Run();

    return 0;
}
