#ifndef CORE_APPLICATION_H
#define CORE_APPLICATION_H

#include "Basic.h"

#include <SDL2/SDL.h>

class TimeStep {
public:
    void Update();

    float DeltaTime() const {
        return m_deltaTime;
    }

    float DeltaTimeMS() const {
        return m_deltaTime * 1000;
    }

private:
    uint64_t m_startTime = 0;
    uint64_t m_endTime = 0;
    float m_deltaTime = 0;
};

class Application {
public:
    Application() {
        ASSERT(!m_instance);
        m_instance = this;
    }

    void Run();

    virtual void OnInit() = 0;
    virtual void OnUpdate(TimeStep timeStep) = 0;

    SDL_Renderer* Renderer() const {
        return m_renderer;
    }

    const TimeStep& FrameTime() const {
        return m_timeStep;
    }

    static Application* Instance() {
        return m_instance;
    }
private:
    void Init();
    void Shutdown();
protected:
    SDL_Window* m_window = NULL;
    SDL_Renderer* m_renderer = NULL;

    bool m_running = false;

    TimeStep m_timeStep;
private:
    static Application* m_instance;
};

inline Application* Application::m_instance = NULL;

#endif
