#ifndef WORLD_H
#define WORLD_H

#include "Basic.h"
#include "LinearMath.h"
#include "List.h"

#include <any>
#include <bitset>
#include <map>
#include <set>
#include <vector>
#include <typeindex>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <string>

struct SDL_Renderer;

typedef uint64_t EntityID;

inline constexpr int MAX_ENTITY_COUNT = 256;
inline constexpr int MAX_COMPONENT_COUNT = 32;

using Signature = std::bitset<MAX_COMPONENT_COUNT>;

EntityID GenerateEntityID();

class EntityManager {
public:
    EntityID CreateEntity() {
        if (m_signatures.Full()) {
            return 0;
        }

        EntityID entity = GenerateEntityID();
        size_t index = m_signatures.Size();

        ASSERT(!m_entityIndexMap.contains(entity));
        ASSERT(!m_indexEntityMap.contains(index));

        m_signatures.Push({});
        m_entityIndexMap.emplace(entity, index);
        m_indexEntityMap.emplace(index, entity);

        return entity;
    }

    void DestroyEntity(EntityID entity) {
        if (entity == 0) {
            return;
        }

        ASSERT(m_entityIndexMap.contains(entity));
        size_t currentIndex = m_entityIndexMap[entity];

        size_t lastIndex = m_signatures.Size() - 1;
        ASSERT(m_indexEntityMap.contains(lastIndex));
        EntityID lastEntity = m_indexEntityMap[lastIndex];

        ASSERT(m_entityIndexMap.contains(lastEntity));
        m_entityIndexMap[lastEntity] = currentIndex;
        m_indexEntityMap[currentIndex] = lastEntity;

        m_indexEntityMap.erase(lastIndex);
        m_entityIndexMap.erase(entity);
        m_signatures.QuickRemove(currentIndex);
    }

    void SetSignature(EntityID entity, Signature signature) {
        if (entity == 0) {
            return;
        }

        ASSERT(m_entityIndexMap.contains(entity));
        size_t index = m_entityIndexMap[entity];

        m_signatures[index] = signature;
    }

    Signature GetSignature(EntityID entity) {
        ASSERT(m_entityIndexMap.contains(entity));
        size_t index = m_entityIndexMap[entity];

        return m_signatures[index];
    }

private:
    std::unordered_map<EntityID, size_t> m_entityIndexMap;
    std::unordered_map<size_t, EntityID> m_indexEntityMap;

    List<Signature, MAX_ENTITY_COUNT> m_signatures;
};

struct IComponentArray {
    virtual void EntityDestroyed(EntityID entity) = 0;
};

template<typename T>
class ComponentArray : public IComponentArray {
public:
    void Insert(EntityID entity, const T& component) {
        size_t index = m_componentData.Size();

        ASSERT(!m_entityIndexMap.contains(entity));
        ASSERT(!m_indexEntityMap.contains(index));

        m_entityIndexMap.emplace(entity, index);
        m_indexEntityMap.emplace(index, entity);

        m_componentData.Push(component);
    }

    void Remove(EntityID entity) {
        ASSERT(m_entityIndexMap.contains(entity));
        size_t currentIndex = m_entityIndexMap[entity];

        size_t lastIndex = m_componentData.Size() - 1;
        ASSERT(m_indexEntityMap.contains(lastIndex));
        EntityID lastEntity = m_indexEntityMap[lastIndex];

        ASSERT(m_entityIndexMap.contains(lastEntity));
        m_entityIndexMap[lastEntity] = currentIndex;
        m_indexEntityMap[currentIndex] = lastEntity;

        m_entityIndexMap.erase(entity);
        m_indexEntityMap.erase(lastIndex);

        m_componentData.QuickRemove(currentIndex);
    }

    T& GetData(EntityID entity) {
        ASSERT(m_entityIndexMap.contains(entity));
        size_t index = m_entityIndexMap[entity];

        return m_componentData[index];
    }

    void EntityDestroyed(EntityID entity) override {
        Remove(entity);
    }

    const List<T, MAX_ENTITY_COUNT>& GetComponentData() {
        return m_componentData;
    }

private:
    std::unordered_map<EntityID, size_t> m_entityIndexMap;
    std::unordered_map<size_t, EntityID> m_indexEntityMap;

    List<T, MAX_ENTITY_COUNT> m_componentData;
};

class ComponentManager {
public:
    template<typename T>
    void Register() {
        std::string name = typeid(T).name();

        ASSERT(m_nextComponentType < MAX_COMPONENT_COUNT);
        ASSERT(!m_componentTypes.contains(name));

        m_componentTypes.emplace(name, m_nextComponentType);
        m_componentMap.emplace(name, new ComponentArray<T>());

        m_nextComponentType += 1;
    }

    template<typename T>
    int GetComponentType() {
        std::string name = typeid(T).name();

        ASSERT(m_componentTypes.contains(name));
        return m_componentTypes[name];
    }

    template<typename T>
    void AddComponent(EntityID entity, const T& component) {
        GetComponentArray<T>()->Insert(entity, component);
    }

    template<typename T>
    void RemoveComponent(EntityID entity) {
        GetComponentArray<T>()->Remove(entity);
    }

    template<typename T>
    T& GetComponent(EntityID entity) {
        return GetComponentArray<T>()->GetData(entity);
    }

    void EntityDestroyed(EntityID entity) {
        for (const auto& [ name, componentArray ] : m_componentMap) {
            componentArray->EntityDestroyed(entity);
        }
    }

    template<typename T>
    ComponentArray<T>* GetComponentArray() {
        std::string name = typeid(T).name();

        ASSERT(m_componentMap.contains(name));
        return (ComponentArray<T>*)(m_componentMap[name]);
    }
private:
    std::unordered_map<EntityID, size_t> m_entityIndexMap;
    std::unordered_map<size_t, EntityID> m_indexEntityMap;

    std::unordered_map<std::string, int> m_componentTypes;
    std::unordered_map<std::string, IComponentArray*> m_componentMap;

    int m_nextComponentType;
};

struct System {
    std::set<EntityID> m_entities;
};

class SystemManager {
public:
    template<typename T>
    System* Register() {
        std::string name = typeid(T).name();

        ASSERT(!m_systems.contains(name));

        System* system = new System();
        m_systems.emplace(name, system);

        return system;
    }

    template<typename T>
    void SetSignature(Signature signature) {
        std::string name = typeid(T).name();

        ASSERT(m_systems.contains(name));
        ASSERT(!m_signatures.contains(name));
        m_signatures.emplace(name, signature);
    }

    void EntityDestroyed(EntityID entity) {
        for (const auto& [ name, system ] : m_systems) {
            system->m_entities.erase(entity);
        }
    }

    void EntitySignatureChanged(EntityID entity, Signature newSignature) {
        for (const auto& [ name, system ] : m_systems) {
            ASSERT(m_signatures.contains(name));
            Signature& signature = m_signatures[name];

            if ((newSignature & signature) == signature) {
                system->m_entities.emplace(entity);
            }
            else {
                system->m_entities.erase(entity);
            }
        }
    }
private:
    std::unordered_map<std::string, Signature> m_signatures;
    std::unordered_map<std::string, System*> m_systems;
};

class EntityStorage {
public:
    EntityStorage() {
        ASSERT(!s_instance);
        s_instance = this;
    }

    static EntityStorage* Instance() {
        return s_instance;
    }

    EntityID CreateEntity() {
        return m_entityManager.CreateEntity();
    }

    void DestroyEntity(EntityID entity) {
        m_entityManager.DestroyEntity(entity);
        m_componentManager.EntityDestroyed(entity);
        m_systemManager.EntityDestroyed(entity);
    }

    template<typename T>
    void RegisterComponent() {
        m_componentManager.Register<T>();
    }

    template<typename T>
    void AddComponent(EntityID entity, const T& component) {
        m_componentManager.AddComponent(entity, component);

        Signature signature = m_entityManager.GetSignature(entity);
        signature.set(m_componentManager.GetComponentType<T>(), true);

        m_entityManager.SetSignature(entity, signature);
        m_systemManager.EntitySignatureChanged(entity, signature);
    }

    template<typename T>
    void RemoveComponent(EntityID entity) {
        m_componentManager.RemoveComponent<T>(entity);

        Signature signature = m_entityManager.GetSignature(entity);
        signature.set(m_componentManager.GetComponentType<T>(), false);

        m_entityManager.SetSignature(entity, signature);
        m_systemManager.EntitySignatureChanged(entity, signature);
    }

    template<typename T>
    T& GetComponent(EntityID entity) {
        return m_componentManager.GetComponent<T>(entity);
    }

    template<typename T>
    int GetComponentType() {
        return m_componentManager.GetComponentType<T>();
    }

    template<typename T>
    System* RegisterSystem() {
        return m_systemManager.Register<T>();
    }

    template<typename T>
    void SetSystemSignature(Signature signature) {
        m_systemManager.SetSignature<T>(signature);
    }
private:
    static EntityStorage* s_instance;

    EntityManager m_entityManager;
    ComponentManager m_componentManager;
    SystemManager m_systemManager;
};

inline EntityStorage* EntityStorage::s_instance = NULL;

#endif
