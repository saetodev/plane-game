#include "World.h"

#include <random>

#include <SDL2/SDL.h>

static std::random_device s_randomDevice;
static std::mt19937_64 s_randomEngine(s_randomDevice());
static std::uniform_int_distribution<uint64_t> s_uniformDistribution;

static EntityID GenerateEntityID() {
    EntityID id;

    do {
        id = s_uniformDistribution(s_randomEngine);
    } while (id == 0);

    return id;
}

EntityID EntityStorage::CreateEntity() {
    EntityInfo info = { .id = GenerateEntityID() };

    ASSERT(!m_entityInfoMap.contains(info.id));
    m_entityInfoMap.emplace(info.id, info);

    return info.id;
}