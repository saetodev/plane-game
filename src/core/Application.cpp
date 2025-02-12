#include "Application.h"

#include <stdio.h>
#include <stdlib.h>

void TimeStep::Update() {
    m_startTime = SDL_GetTicks64();
    m_deltaTime = (float)(m_startTime - m_endTime) / 1000;
    m_endTime = m_startTime;
}

void Application::Run() {
    Init();
    OnInit();

    while (m_running) {
        m_timeStep.Update();

        printf("delta: %f\n", m_timeStep.DeltaTime());

        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: {
                    m_running = false;
                    break;
                }
            }
        }

        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
        SDL_RenderClear(m_renderer);

        OnUpdate(m_timeStep);

        SDL_RenderPresent(m_renderer);
    }

    Shutdown();
}

void Application::Init() {
    if (SDL_Init(SDL_INIT_VIDEO)) {
        printf("ERROR: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    m_window = SDL_CreateWindow("WINDOW", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN);

    if (!m_window) {
        printf("ERROR: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);

    if (!m_renderer) {
        printf("ERROR: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    m_running = true;
}

void Application::Shutdown() {
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);

    SDL_Quit();
}
