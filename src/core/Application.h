#ifndef CORE_APPLICATION_H
#define CORE_APPLICATION_H

#include "Basic.h"

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

namespace Application {
    void Init();
    void Shutdown();

    void Run(std::function<void()> onInit, std::function<void(const TimeStep&)> onUpdate);

    const TimeStep& FrameTime();
}

#endif
