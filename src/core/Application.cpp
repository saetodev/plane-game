#include "Application.h"
#include "renderer/Renderer2D.h"

#include <iostream>

#include <glad/glad.h>
#include <SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>

#define IMGUI_IMPL_OPENGL_ES2
#include <imgui_impl_opengl3.h>

enum ErrorType {
    ERROR_INIT,
    ERROR_GL_CONTEXT,
    ERROR_GL_LOAD,
};

struct InputButton {
    bool down;
    bool pressed;
    bool released;
};

// platform stuff
static SDL_Window* m_window;
static SDL_GLContext m_context;
static bool m_running;

static TimeStep m_timeStep;

#define MAX_KEY_COUNT 256
#define MAX_BUTTON_COUNT 6

static InputButton m_keys[MAX_KEY_COUNT];
static InputButton m_buttons[MAX_BUTTON_COUNT];

static void InitImGui() {
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplSDL2_InitForOpenGL(m_window, m_context);
    ImGui_ImplOpenGL3_Init("#version 100");
}

static void ShutdownImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void TimeStep::Update() {
    m_startTime = SDL_GetTicks64();
    m_deltaTime = (float)(m_startTime - m_endTime) / 1000;
    m_endTime = m_startTime;
}

void Application::Init(const AppDesc& desc) {
    /* INIT SUBSYSTEMS */

    if (SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "ERROR: " << SDL_GetError() << std::endl;
        std::exit(ERROR_INIT);
    }

    /* INIT OPENGL CONTEXT */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    m_window = SDL_CreateWindow(desc.windowTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, desc.windowWidth, desc.windowHeight, SDL_WINDOW_OPENGL);

    if (!m_window) {
        std::cout << "ERROR: " << SDL_GetError() << std::endl;
        std::exit(ERROR_GL_CONTEXT);
    }

    m_context = SDL_GL_CreateContext(m_window);

    if (!m_context) {
        std::cout << "ERROR: " << SDL_GetError() << std::endl;
        std::exit(ERROR_GL_CONTEXT);
    }

    if (SDL_GL_MakeCurrent(m_window, m_context)) {
        std::cout << "ERROR: " << SDL_GetError() << std::endl;
        std::exit(ERROR_GL_CONTEXT);
    }

    if (!gladLoadGLES2Loader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cout << "ERROR: GLAD failed to load opengl functions" << std::endl;
        std::exit(ERROR_GL_LOAD);
    }

    SDL_GL_SetSwapInterval(1);

    /* MISC */
    Renderer2D::Init();
    InitImGui();

    m_running = true;
}

void Application::Shutdown() {
    ShutdownImGui();
    Renderer2D::Shutdown();

    SDL_GL_DeleteContext(m_context);
    SDL_DestroyWindow(m_window);

    SDL_Quit();
}

void Application::Run(AppInitCB onInit, AppUpdateCB onUpdate) {
    if (onInit) {
        onInit();
    }

    while (m_running) {
        m_timeStep.Update();
        
        for (int i = 0; i < MAX_KEY_COUNT; i++) {
            m_keys[i].pressed  = false;
            m_keys[i].released = false;
        }

        for (int i = 0; i < MAX_BUTTON_COUNT; i++) {
            m_buttons[i].pressed  = false;
            m_buttons[i].released = false;
        }

        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);

            switch (event.type) {
                case SDL_QUIT: {
                    m_running = false;
                    break;
                }

                case SDL_KEYUP:
                case SDL_KEYDOWN: {
                    int key = event.key.keysym.sym;

                    if (key < 0 || key >= MAX_KEY_COUNT) {
                        break;
                    }

                    bool isDown  = (event.key.state == SDL_PRESSED);
                    bool wasDown = (event.key.state == SDL_RELEASED) || (event.key.repeat != 0);

                    m_keys[key].down     = isDown;
                    m_keys[key].pressed  = isDown && !wasDown;
                    m_keys[key].released = !isDown && wasDown;
                    
                    break;
                }

                case SDL_MOUSEBUTTONUP:
                case SDL_MOUSEBUTTONDOWN: {
                    int button = event.button.button;

                    if (button < 0 || button >= MAX_BUTTON_COUNT) {
                        break;
                    }

                    bool isDown  = (event.button.state == SDL_PRESSED);
                    bool wasDown = (event.button.state == SDL_RELEASED);

                    m_buttons[button].down     = isDown;
                    m_buttons[button].pressed  = isDown && !wasDown;
                    m_buttons[button].released = !isDown && wasDown;

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

void Application::ImGuiNewFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void Application::ImGuiRender() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

const TimeStep& Application::FrameTime() {
    return m_timeStep;
}

bool Application::KeyDown(int key) {
    if (key < 0 || key >= MAX_KEY_COUNT) {
        return false;
    }

    return m_keys[key].down;
}

bool Application::KeyPressed(int key) {
    if (key < 0 || key >= MAX_KEY_COUNT) {
        return false;
    }

    return m_keys[key].pressed;
}

bool Application::KeyReleased(int key) {
    if (key < 0 || key >= MAX_KEY_COUNT) {
        return false;
    }

    return m_keys[key].released;
}

bool Application::MouseDown(int button) {
    if (button < 0 || button >= MAX_BUTTON_COUNT) {
        return false;
    }

    return m_buttons[button].down;
}

bool Application::MousePressed(int button) {
    if (button < 0 || button >= MAX_BUTTON_COUNT) {
        return false;
    }

    return m_buttons[button].pressed;
}

bool Application::MouseReleased(int button) {
    if (button < 0 || button >= MAX_BUTTON_COUNT) {
        return false;
    }

    return m_buttons[button].released;
}

Vec2 Application::MousePos() {
    int x, y;
    SDL_GetMouseState(&x, &y);

    return { (f32)x, (f32)y };
}

Vec2 Application::WindowSize() {
    int width, height;
    SDL_GetWindowSize(m_window, &width, &height);

    return { (f32)width, (f32)height };
}

void Application::SetWindowTitle(const std::string& title) {
    SDL_SetWindowTitle(m_window, title.c_str());
}