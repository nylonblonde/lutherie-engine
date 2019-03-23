#ifndef ECS_hpp
#define ECS_hpp
#include <iostream>
#include <vector>
#include <variant>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <iterator>
#include <cstring>

#include "ecs_export.h"

namespace ECS {

    struct TypedObject {
    public:
        virtual size_t getType() const {
            return typeid(*this).hash_code();
        }

    };

    struct Component : TypedObject  {

    public:
        
        Component();
        ~Component();
        
        friend class World;
    };

    struct Entity {

    private:
        uint32_t _id;
//        Entity();

    public:
        uint32_t id() const { return _id; };
        
        //TODO: felt it was necessary to use Entity as a key rather than just the id because eventually either GUID or a combination of id and version will have to be used to distinguish entities. incremental id is fine for now, but obviously has scaling pitfalls
        bool operator ==(const Entity &other) const { return id() == other.id(); }
        bool operator !=(const Entity &other) const { return id() == other.id(); }
        bool operator >(const Entity &other) const { return id() > other.id(); }
        bool operator <(const Entity &other) const { return id() < other.id(); }

        friend class World;
    };
}


template<>
struct std::hash<ECS::Entity>{
    std::size_t operator()(const ECS::Entity & entity) const { return(std::hash<uint32_t>()(entity.id())); }
};

namespace ECS {

    class System;

    class World {
    private:
        static uint32_t _staticId;
        uint32_t _entityId;
        
        uint32_t _id;

        
        World();

        bool removeComponentHelper(const Entity entity, const size_t typeCode);
        
    protected:
        std::vector<System*> activeSystems;
        std::vector<System*> inactiveSystems;
        std::unordered_set<Entity> entities;
        std::unordered_multimap<Entity, Component*> components;
        template <typename... T>
        void createSystem(){
            [](...){ }((inactiveSystems.emplace(inactiveSystems.end(), new T(*this)), 0)...);
        }
        
        void addSystem(){}
        
        template<typename... Args>
        void addSystem(System* s, Args... args){
            this->inactiveSystems.emplace(inactiveSystems.end(), *std::move(&s));
            std::cout << "There are now " << inactiveSystems.size() << std::endl;
            addSystem(args...);
        }
                
    public:
        uint32_t id() const { return _id; }
        static ECS_EXPORT std::vector<World*> allWorlds;
        
        template<typename... Args>
        static World& createWorld(System* s, Args... args){
            allWorlds.emplace(allWorlds.end(), new World());
            World* world = allWorlds.back();
            world->addSystem(s, args...);
            return *world;
        }
        
        template<typename...T>
        static World& createWorld(){
            allWorlds.emplace(allWorlds.end(), new World());
            allWorlds.back()->createSystem<T...>();
            return *allWorlds.back();
        }

        static void destroyWorld(World* worldPtr);
        std::unordered_multimap<Entity, Component*> getComponents() const { return components; }
        const Entity& getEntity(Entity entity) const { 
            auto it = entities.find(entity);
            if(it == entities.end()){
                std::cout << "shit" << std::endl;
            }
            return *it; 
        }
        
        void queryActiveSystems();
        void queryInactiveSystems();
        
        const Entity& createEntity();
        void removeEntity(const Entity entity);

        template<typename T>
        bool getComponent(Entity entity, Component* component) {
            auto range = components.equal_range(entity);
            auto it = range.first;
            while(it != range.second){
                if(std::is_same_v<decltype(it->second), T>){
                    component = it->second;
                    return true;
                }
                it++;
            }
            return false;
        }
        
        Component* setComponent(const Entity entity, Component* component);
        
        bool removeComponent(const Entity entity, Component* component);
        
        template<typename T>
        bool removeComponent(const Entity entity){
            return removeComponentHelper(entity, typeid(T).hash_code());
        }
        
        static void updateActive(std::vector<World*>worlds);

    };


    class System : TypedObject {
    private:
        void AfterUpdate();
        void BeforeUpdate();

    protected:
        const World& world;
        std::unordered_set<size_t> dependencies;

        //ComponentGroup will be notified when there is a change
        virtual void notifyComponentChange();
        
        struct ComponentGroup {
        protected:
            System& parent;
            std::multimap<size_t, Component*> components;
            std::unordered_set<size_t> localDependencies;
            std::set<Entity> entities;
            void updateHelper(std::unordered_multimap<Entity, Component*> allComponents, std::unordered_set<size_t> tempDependencies);
        private:
            
            void componentGroupHelper(size_t typeCode){

                parent.dependencies.emplace(typeCode);
                localDependencies.emplace(typeCode);
            }
            ComponentGroup(System& s) : parent(s){
                entities = std::set<Entity>();
                parent.views.push_back(this);
            }
        public:

            System& getParent () const {
                return parent;
            }
            
            template<typename... Ts>
            static ComponentGroup createComponentGroup(System& s){
                ComponentGroup retVal = ComponentGroup(s);
                [](...){}((retVal.componentGroupHelper(typeid(Ts).hash_code()),0)...);
                return retVal;
            }
            
            template<typename... Ts>
            static ComponentGroup createComponentGroup(System& s, Ts... args){
                ComponentGroup retVal = ComponentGroup(s);
                [](...){}((retVal.componentGroupHelper(args),0)...);
                return retVal;
            }
            
            size_t size() const{
                return entities.size();
            }
            
            void updateComponents();
            
            Component* getComponent (size_t typeCode, size_t index) const {
                auto range = components.equal_range(typeCode);
                auto it = range.first;
                std::advance(it, index);

                return (it->second);
            }
            
            template<typename T>
            T& getComponent (size_t index) const {
                auto range = components.equal_range(typeid(T).hash_code());
                auto it = range.first;
                std::advance(it, index);

                return *dynamic_cast<T*>(it->second);
            }
            
            const Entity* getEntity (size_t index) const {
                auto it = entities.begin();
                std::advance(it, index);
                
                std::cout << &parent.world.getEntity(*it) << std::endl;
                
                return &parent.world.getEntity(*it);
            }
            
        }; //ComponentGroup

        void Update();
        //Component Groups are the observers in a Observer/Listener pattern
        std::vector<ComponentGroup*> views;

    public:
        std::unordered_set<size_t> getDependencies();
        
        System(World& w);

        virtual void OnUpdate();
        virtual void OnActive();

        friend class World;
    };

}


#endif /* ECS_hpp */
