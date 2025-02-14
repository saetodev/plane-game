#include "Application.h"
#include "Renderer2D.h"

#include <iostream>

#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// platform stuff
static SDL_Window* m_window;
static SDL_GLContext m_context;
static bool m_running;

static TimeStep m_timeStep;

void TimeStep::Update() {
    m_startTime = SDL_GetTicks64();
    m_deltaTime = (float)(m_startTime - m_endTime) / 1000;
    m_endTime = m_startTime;
}

void Application::Init() {
    if (SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "ERROR: " << SDL_GetError() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (!IMG_Init(IMG_INIT_PNG)) {
        std::cout << "ERROR: " << SDL_GetError() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    m_window = SDL_CreateWindow("WINDOW", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);

    if (!m_window) {
        std::cout << "ERROR: " << SDL_GetError() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    m_context = SDL_GL_CreateContext(m_window);

    if (!m_context) {
        std::cout << "ERROR: " << SDL_GetError() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    SDL_GL_MakeCurrent(m_window, m_context);

    gladLoadGLES2Loader((GLADloadproc)SDL_GL_GetProcAddress);

    Renderer2D::Init();

    m_running = true;
}

void Application::Shutdown() {
    Renderer2D::Shutdown();

    SDL_GL_DeleteContext(m_context);
    SDL_DestroyWindow(m_window);

    IMG_Quit();
    SDL_Quit();
}

void Application::Run(std::function<void()> onInit, std::function<void(const TimeStep&)> onUpdate) {
    if (onInit) {
        onInit();
    }

    while (m_running) {
        m_timeStep.Update();
        
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: {
                    m_running = false;
                    break;
                }
            }
        }

        if (onUpdate) {
            onUpdate(m_timeStep);
        }

        SDL_GL_SwapWindow(m_window);
    }
}

const TimeStep& Application::FrameTime() {
    return m_timeStep;
}