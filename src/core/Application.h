#ifndef CORE_APPLICATION_H
#define CORE_APPLICATION_H

#include <SDL2/SDL.h>

class TimeStep {
public:
    void Update();

    float DeltaTime() {
        return m_deltaTime;
    }

    float DeltaTimeMS() {
        return m_deltaTime * 1000;
    }

private:
    uint64_t m_startTime = 0;
    uint64_t m_endTime = 0;
    float m_deltaTime = 0;
};

class Application {
public:
    void Run();

    virtual void OnInit() = 0;
    virtual void OnUpdate(TimeStep timeStep) = 0;
private:
    void Init();
    void Shutdown();
protected:
    SDL_Window* m_window = NULL;
    SDL_Renderer* m_renderer = NULL;

    bool m_running = false;

    TimeStep m_timeStep;
};

#endif
