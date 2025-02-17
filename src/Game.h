#ifndef GAME_H
#define GAME_H

#include "entity/Entity.h"
#include "entity/World.h"
#include "List.h"

struct GameState {
    EntityID selectedEntity;
    
    bool canDrawPath;
    List<Vec2, MAX_PATH_SIZE> drawnPath;

    Vec2 lastMousePos;
    
    int lastTileX;
    int lastTileY;

    int tileSize = 16;

    World world;
};

inline GameState m_gameState;

void UpdateInputState();
void PlacePathPoint();

void DebugDrawPath(EntityID entityID);

#endif