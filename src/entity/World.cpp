#include "World.h"

#include <random>

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


EntityData* World::CreateEntity(u64 flags) {
    EntityID id = GenerateEntityID();
    size_t index = m_entityData.Size();

    ASSERT(!m_entityMap.Contains(id));

    m_entityMap.Add(id, index);
    m_entityData.Push({ .id = id, .flags = flags });

    return &m_entityData[index];
}

void World::DestroyEntity(EntityID entityID) {
    if (!m_entityMap.Contains(entityID)) {
        return;
    }

    size_t lastIndex    = m_entityData.Size() - 1;
    size_t currentIndex = m_entityMap.Get(entityID);
    EntityID lastEntity = m_entityData[lastIndex].id;

    m_entityMap.Set(lastEntity, currentIndex);
    m_entityMap.Remove(entityID);

    m_entityData.QuickRemove(currentIndex);
}

EntityData* World::GetEntityData(EntityID entityID) {
    if (!m_entityMap.Contains(entityID)) {
        return NULL;
    }

    return &m_entityData[m_entityMap.Get(entityID)];
}

List<EntityID, MAX_ENTITY_COUNT> World::EntitiesWithFlags(u64 flags) {
    List<EntityID, 256> result;

    for (int i = 0; i < m_entityData.Size(); i++) {
        const EntityData& entity = m_entityData[i];

        if ((entity.flags & flags) == flags) {
            result.Push(entity.id);
        }
    }

    return result;
}