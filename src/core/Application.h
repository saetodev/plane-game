#ifndef CORE_APPLICATION_H
#define CORE_APPLICATION_H

#include <SDL2/SDL.h>

class Application {
public:
    void Run();

    virtual void OnInit() = 0;
    virtual void OnUpdate() = 0;
private:
    void Init();
    void Shutdown();
protected:
    SDL_Window* m_window = NULL;
    SDL_Renderer* m_renderer = NULL;

    bool m_running = false;
};

#endif
