#ifndef ENTITY_WORLD_H
#define ENTITY_WORLD_H

#include "Entity.h"
#include "Map.h"

#include <functional>

#define MAX_SYSTEM_COUNT 256

class World;

struct System {
    u64 flags;
    std::function<void(World*, List<EntityID, MAX_ENTITY_COUNT>&)> func;
};

class World {
public:
    EntityData* CreateEntity(u64 flags);
    void DestroyEntity(EntityID entityID);

    void AddSystem(u64 flags, std::function<void(World*, List<EntityID, MAX_ENTITY_COUNT>&)> func);

    EntityData* GetEntityData(EntityID entityID);
    List<EntityID, MAX_ENTITY_COUNT> EntitiesWithFlags(u64 flags);

    void RunSystems();

private:
    Map<EntityID, usize, MAX_ENTITY_COUNT> m_entityMap;

    List<EntityData, MAX_ENTITY_COUNT> m_entityData;
    List<System, MAX_SYSTEM_COUNT> m_systems;
};

#endif