#ifndef ENTITY_WORLD_H
#define ENTITY_WORLD_H

#include "Entity.h"
#include "Map.h"

class World {
public:
    EntityData* CreateEntity(u64 flags);
    void DestroyEntity(EntityID entityID);

    EntityData* GetEntityData(EntityID entityID);

    List<EntityID, MAX_ENTITY_COUNT> EntitiesWithFlags(u64 flags);

private:
    List<EntityData, MAX_ENTITY_COUNT> m_entityData;
    Map<EntityID, usize, MAX_ENTITY_COUNT> m_entityMap;
};

#endif