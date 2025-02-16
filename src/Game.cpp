#include "core/Application.h"
#include "core/renderer/Renderer2D.h"
#include "Game.h"

struct Rect {
    Vec2 position;
    Vec2 size;
};

static bool PointInRect(const Vec2& point, const Rect& rect) {
    return (point.x >= rect.position.x) && 
           (point.x <= rect.position.x + rect.size.x) && 
           (point.y >= rect.position.y) && 
           (point.y <= rect.position.y + rect.size.y);
}

static bool RectInRect(const Rect& a, const Rect& b) {
    return (a.position.x + a.size.x >= b.position.x) &&
           (a.position.x <= b.position.x + b.size.x) && 
           (a.position.y + a.size.y >= b.position.y) && 
           (a.position.y <= b.position.y + b.size.y);
}

static Rect GetEntityRect(World& world, EntityID id) {
    EntityData* entity = world.GetEntityData(id);

    if (!entity) {
        return {};
    }

    return {
        .position = {
            entity->transform.position.x - (entity->transform.size.x / 2.0f),
            entity->transform.position.y - (entity->transform.size.y / 2.0f),
        },

        .size = {
            entity->transform.size.x,
            entity->transform.size.y,
        },
    };
}

EntityID EntityAtPosition(World& world, const Vec2& position) {
    auto entities = world.EntitiesWithFlags(EntityFlags::TRANSFORM);

    for (int i = 0; i < entities.Size(); i++) {
        EntityID id = entities[i];
        const Transform& transform = world.GetEntityData(id)->transform;

        if (PointInRect(position, GetEntityRect(world, id))) {
            return id;
        }
    }

    return 0;
}

void UpdateInputState() {
    Vec2 mousePos = Application::MousePos();

    if (Application::MousePressed(1)) {
        m_gameState.selectedEntity = EntityAtPosition(m_gameState.world, mousePos);
        
        EntityData* entity = m_gameState.world.GetEntityData(m_gameState.selectedEntity);

        if (!entity) {
            return;
        }

        entity->path.Clear();
        m_gameState.canDrawPath = true;
    }
    else if (Application::MouseReleased(1)) {
        m_gameState.canDrawPath = false;
    }
}

void PlacePathPoint() {
    if (!m_gameState.canDrawPath || m_gameState.drawnPath.Full()) {
        return;
    }

    Vec2 mousePos = Application::MousePos();

    int tileX = mousePos.x / m_gameState.tileSize;
    int tileY = mousePos.y / m_gameState.tileSize;

    if (tileX == m_gameState.lastTileX && tileY == m_gameState.lastTileY) {
        return;
    }

    EntityData* entity = m_gameState.world.GetEntityData(m_gameState.selectedEntity);

    if (!entity) {
        return;
    }

    f32 x = (tileX * m_gameState.tileSize) + (m_gameState.tileSize / 2);
    f32 y = (tileY * m_gameState.tileSize) + (m_gameState.tileSize / 2);

    entity->path.Push({ x, y });

    m_gameState.lastTileX = tileX;
    m_gameState.lastTileY = tileY;
}

void DebugDrawPath(EntityID entityID) {
    EntityData* entity = m_gameState.world.GetEntityData(entityID);

    if (!entity) {
        return;
    }

    if (entity->path.Size() < 2) {
        return;
    }

    // draw path lines
    for (int i = 0; i < entity->path.Size() - 1; i++) {
        Renderer2D::DrawLine(entity->path[i], entity->path[i + 1], RED);
    }

    // draw path points
    for (int i = 0; i < entity->path.Size(); i++) {
        Renderer2D::DrawRect(entity->path[i], { (f32) m_gameState.tileSize / 4, (f32)m_gameState.tileSize / 4 }, WHITE);
    }
}