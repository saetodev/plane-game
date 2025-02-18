#ifndef CORE_APPLICATION_H
#define CORE_APPLICATION_H

#include "LinearMath.h"

#include <functional>
#include <string>

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

typedef std::function<void()> AppInitCB;
typedef std::function<void(const TimeStep&)> AppUpdateCB;

struct AppDesc {
    int windowWidth;
    int windowHeight;
    std::string windowTitle;
};

namespace Application {
    void Init(const AppDesc& desc);
    void Shutdown();

    void Run(AppInitCB onInit, AppUpdateCB onUpdate);

    void ImGuiNewFrame();
    void ImGuiRender();

    const TimeStep& FrameTime();

    bool KeyDown(int key);
    bool KeyPressed(int key);
    bool KeyReleased(int key);

    bool MouseDown(int button);
    bool MousePressed(int button);
    bool MouseReleased(int button);

    Vec2 MousePos();

    Vec2 WindowSize();
    void SetWindowTitle(const std::string& title);
}

#endif
