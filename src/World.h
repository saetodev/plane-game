#ifndef WORLD_H
#define WORLD_H

#include "LinearMath.h"

#include <any>
#include <map>
#include <vector>
#include <typeindex>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

#ifdef WIN32
#define DEBUG_BREAK() __debugbreak()
#elif __linux__
#define DEBUG_BREAK() __builtin_debugtrap()
#elif __APPLE__
#define DEBUG_BREAK() __builtin_trap()
#else
#define DEBUG_BREAK()
#endif

#define ASSERT(condition) do { if (!(condition)) { DEBUG_BREAK(); } } while(0)

struct SDL_Renderer;

typedef uint64_t EntityID;

struct Entity {
    EntityID id;

    Vec2 position;
    Vec2 velocity;
    Vec2 size;

    std::vector<Vec2> path;
};

struct PhysicsComponent {
    Vec2 position;
    Vec2 velocity;
    Vec2 size;
};

struct ColorComponent {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

struct PathComponent {
    std::vector<Vec2> points;
};

struct EntityInfo {
    EntityID id;
    std::unordered_map<std::type_index, int> components;
};

template<typename T>
struct ComponentPair {
    EntityID id;
    T component;
};

class EntityStorage {
public:
    template<typename T>
    void Register() {
        std::type_index typeID = typeid(T);

        if (m_componentData.contains(typeID)) {
            return;
        }

        m_componentData.emplace(typeID, std::vector<ComponentPair<T>>());
    }

    template<typename T>
    T& CreateComponent(EntityID entity) {
        ASSERT(entity != 0);
        std::type_index typeID = typeid(T);

        ASSERT(m_entityInfoMap.contains(entity));
        EntityInfo& info = m_entityInfoMap[entity];

        if (info.components.contains(typeID)) {
            int index = info.components[typeID];

            ASSERT(m_componentData.contains(typeID));
            std::vector<ComponentPair<T>>& data = std::any_cast<std::vector<ComponentPair<T>>&>(m_componentData[typeID]);

            return data[index].component;
        }

        ASSERT(m_componentData.contains(typeID));
        std::vector<ComponentPair<T>>& data = std::any_cast<std::vector<ComponentPair<T>>&>(m_componentData[typeID]);

        int index = data.size();

        data.push_back({ entity, T() });
        info.components.emplace(typeID, index);

        return data.back().component;
    }

    template<typename T>
    T& GetComponent(EntityID entity) {
        ASSERT(entity != 0);
        std::type_index typeID = typeid(T);

        ASSERT(m_entityInfoMap.contains(entity));
        EntityInfo info = m_entityInfoMap[entity];

        ASSERT(info.components.contains(typeID));
        int index = info.components[typeID];

        std::vector<ComponentPair<T>>& data = GetComponents<T>();

        return data[index].component;
    }

    template<typename T>
    std::vector<ComponentPair<T>>& GetComponents() {
        std::type_index typeID = typeid(T);

        ASSERT(m_componentData.contains(typeID));
        return std::any_cast<std::vector<ComponentPair<T>>&>(m_componentData[typeID]);
    }

    std::vector<EntityID> GetQuery(const std::vector<std::type_index>& query) {
        std::vector<EntityID> result;

        for (const auto& [ id, info ] : m_entityInfoMap) {
            int validCount = 0;

            for (const auto& [ typeID, index ] : info.components) {
                bool valid = false;
                
                for (std::type_index queryTypeID : query) {
                    if (queryTypeID == typeID) {
                        valid = true;
                        break;
                    }
                }

                if (valid) {
                    validCount++;
                }
            }

            if (validCount == query.size()) {
                result.push_back(id);
            }
        }

        return result;
    }

    EntityID CreateEntity();

private:
    std::unordered_map<EntityID, EntityInfo> m_entityInfoMap;
    std::unordered_map<std::type_index, std::any> m_componentData;
};

#endif
