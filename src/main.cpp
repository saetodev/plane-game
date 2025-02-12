#include "core/Application.h"
#include "World.h"

class App : public Application {
public:
    void OnInit() override {
        {
            Entity* entity = m_world.entityStorage.CreateEntity();

            entity->position.x = 1280 / 2.0f;
            entity->position.y = 720 / 2.0f;

            entity->size.x = 32;
            entity->size.y = 32;
        }

        {
            Entity* entity = m_world.entityStorage.CreateEntity();

            entity->position.x = 300;
            entity->position.y = 200;

            entity->size.x = 32;
            entity->size.y = 32;
        }
    }

    void OnUpdate(TimeStep timeStep) override {
        int mouseX, mouseY;
        int mouseState = SDL_GetMouseState(&mouseX, &mouseY);

        Vec2 mousePos = { (float)mouseX, (float)mouseY };

        if ((mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) && !m_canDrawPath) {
            m_selectedEntity = m_world.EntityAtPosition(mousePos);
            Entity* entity = m_world.entityStorage.GetEntity(m_selectedEntity);

            if (entity) {
                m_canDrawPath = true;
                entity->path.clear();
            }
        }
        else if (!(mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) && m_canDrawPath) {
            m_canDrawPath = false;
            m_selectedEntity = 0;
        }

        if (m_canDrawPath) {
            if (mousePos != m_lastMousePos) {
                Entity* entity = m_world.entityStorage.GetEntity(m_selectedEntity);
                entity->path.push_back(mousePos);
            }

            m_lastMousePos = mousePos;
        }

        m_world.DoPhysics(timeStep.DeltaTime());
        m_world.Draw(m_renderer);
    }
private:
    Uint64 m_lastTime = 0;

    bool m_canDrawPath = false;
    EntityID m_selectedEntity = 0;
    Vec2 m_lastMousePos;

    World m_world;
};

int main(int argc, char** argv) {
    App().Run();

    return 0;
}
