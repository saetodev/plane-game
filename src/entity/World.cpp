#include "core/Util.h"
#include "World.h"

EntityData* World::CreateEntity(u64 flags) {
    EntityID id = Util::RandomID();
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

void World::AddSystem(u64 flags, std::function<void(World*, List<EntityID, MAX_ENTITY_COUNT>&)> func) {
    m_systems.Push({ flags, func });
}

EntityData* World::GetEntityData(EntityID entityID) {
    if (!m_entityMap.Contains(entityID)) {
        return NULL;
    }
    usize index = m_entityMap.Get(entityID);
    return &m_entityData[index];
}

List<EntityID, MAX_ENTITY_COUNT> World::EntitiesWithFlags(u64 flags) {
    List<EntityID, MAX_ENTITY_COUNT> result;

    for (int i = 0; i < m_entityData.Size(); i++) {
        const EntityData& entity = m_entityData[i];

        if ((entity.flags & flags) == flags) {
            result.Push(entity.id);
        }
    }

    return result;
}

void World::RunSystems() {
    for (int i = 0; i < m_systems.Size(); i++) {
        const System& system = m_systems[i];

        List<EntityID, MAX_ENTITY_COUNT> entities;
        
        for (int i = 0; i < m_entityData.Size(); i++) {
            const EntityData& entity = m_entityData[i];

            if ((entity.flags & system.flags) == system.flags) {
                entities.Push(entity.id);
            }
        }

        if (entities.Empty()) {
            continue;
        }

        if (system.func) {
            system.func(this, entities);
        }
    }
}