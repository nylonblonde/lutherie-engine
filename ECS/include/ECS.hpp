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
        Entity();

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

        std::vector<System*> activeSystems;
        std::vector<System*> inactiveSystems;
        std::unordered_multimap<Entity, Component*> components;
        World();

    protected:

        
        template <typename... T>
        void createSystem(){
            [](...){ }((inactiveSystems.emplace(inactiveSystems.end(), new T(*this)), 0)...);
        }
        
    public:
        uint32_t id() const { return _id; }
        static std::vector<World*> allWorlds;
        
        template<typename...T>
        static World& createWorld(){
            allWorlds.emplace(allWorlds.end(), new World);
            allWorlds.back()->createSystem<T...>();
            return *allWorlds.back();
        }

        static void destroyWorld(World* worldPtr);
        std::vector<Entity> entities;
        std::unordered_multimap<Entity, Component*> getComponents() const { return components; }
        
        void queryActiveSystems();
        void queryInactiveSystems();
        
        Entity createEntity();
        bool removeEntity(Entity entity);

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
        
        void setComponent(Entity entity, Component* component);
        
        template<typename T>
        bool removeComponent(Entity entity){
            auto its = components.equal_range(entity);
            if(its.first != its.second){
                auto it = its.first;
                while(it != its.second){
                    size_t typeCode = typeid(T).hash_code();
                    if(it->second->getType() == typeCode){
                        delete it->second;
                        it = components.erase(it);
                        std::cout << "Removed component " << typeCode << " for entity " << entity.id() << " from world " << this->id() <<  std::endl;
                        queryActiveSystems();
                        return true;
                    }else{
                        it++;
                    }
                }
            }
            
            return false;
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
        void notifyComponentChange();
        
        struct ComponentGroup {
        private:
            System& parent;
            
            std::multimap<size_t, Component*> components;
            std::unordered_set<size_t> localDependencies;
            std::set<Entity> entities;
            
            template<typename T>
            void componentGroupHelper(){
                parent.dependencies.emplace(typeid(T).hash_code());
                localDependencies.emplace(typeid(T).hash_code());
            }

            ComponentGroup(System& s) : parent(s){
                entities = std::set<Entity>();
                parent.views.push_back(this);
            }
            
        public:
            
            template<typename... Ts>
            static ComponentGroup createComponentGroup(System& s){
                ComponentGroup retVal = ComponentGroup(s);
                [](...){}((retVal.componentGroupHelper<Ts>(),0)...);
                return retVal;
                
            }
            
            size_t size() const{
                return entities.size();
            }
            
            virtual void updateComponents(){
               
                entities.clear();
                components.clear();
                auto allComponents = parent.world.getComponents();
                auto tempDependencies = localDependencies;
                
                std::set<Component*> tempComponents = std::set<Component*>();
                
                auto it = allComponents.begin();

                auto range = allComponents.equal_range(it->first);

                //the reason we don't do a nested loop here is because my tests showed it was slightly faster to do it as a single loop, and performance is critical for this function as it will run whenever components are added or removed
                while(it != allComponents.end()){

                    auto dIt = tempDependencies.find(it->second->getType());
                    if(dIt != tempDependencies.end()){
                        tempComponents.insert(it->second);
                        tempDependencies.erase(dIt);
                    }
                    
                    if(tempDependencies.size() == 0){

                        for(auto cIt = tempComponents.begin(); cIt != tempComponents.end(); ++cIt){
                            components.emplace((*cIt)->getType(), *cIt);
                        }
                        entities.insert(it->first);
                        tempComponents.clear();
                        tempDependencies = localDependencies;
                        it = range.second;
                        if(it == allComponents.end()){
                            break;
                        }
                        range = allComponents.equal_range(it->first);
                        continue;
                    }
                
                    it++;
                    
                    if(range.second != allComponents.end() && it == range.second){
                        tempDependencies = localDependencies;
                        tempComponents.clear();
                        range = allComponents.equal_range(it->first);
                    }
                }
            }
            
            template<typename T>
            T& getComponent (size_t index) const {
                auto range = components.equal_range(typeid(T).hash_code());
                auto it = range.first;
                std::advance(it, index);

                return *dynamic_cast<T*>(it->second);

            }
            
            Entity getEntity (size_t index) const {
                auto it = entities.begin();
                std::advance(it, index);
                
                return *it;
            }
            
        };
        
        
        
        void Update();

    private:
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

extern "C" void* createWorld();

#endif /* ECS_hpp */
