#include "ECS.hpp"
#include <iostream>

using namespace ECS;

System::System(World& w) : world(w) {}

void System::notifyComponentChange() {
    for(auto view : views){
        view->updateComponents();
    }
}

void System::Update(){
    //do updates before OnUpdate in case any entities or components have been added or removed
    BeforeUpdate();
    //virtual call for the derived class, which can be user-defined
    OnUpdate();
    //so we can run update stuff without having to have the user define it, this might be useful for something like a command buffer, idk
    AfterUpdate();
}

void System::OnUpdate(){}

void System::BeforeUpdate() {
    //gonna add some logic to actually ensure that only relevant componentGroups are acted on later
    notifyComponentChange();
}

void System::AfterUpdate() {}

void System::OnActive(){}

std::unordered_set<size_t> System::getDependencies(){
    return dependencies;
}

Component::Component()  {}

Component::~Component() {}

Entity::Entity() {}

std::vector<World*> World::allWorlds = {};
uint32_t World::_staticId = 0;

World::World() {
    
    _entityId = 0;
    _id = _staticId;
    _staticId+=1;
    entities = std::vector<Entity>();
    components = std::unordered_multimap<Entity, Component*>();
    std::cout << "Components initialized " << components.size() << std::endl;
    //systems are allocated in dynamic memory and will be deleted on destroyWorld
    activeSystems = std::vector<System*>();
    inactiveSystems = std::vector<System*>();

}

//call destroyWorld to clean up components and systems
void World::destroyWorld(World *world) {
    auto it = allWorlds.begin();
    while(it != allWorlds.end()){
        if((*it)->id() == world->id()){
            
            for(auto system : (*it)->inactiveSystems){
                delete system;
            }
            
            for(auto system : (*it)->activeSystems){
                delete system;
            }
            
            for(auto cIt = (*it)->components.begin(); cIt != (*it)->components.end(); cIt++){
                delete cIt->second;
            }
            
            delete world;
            it = allWorlds.erase(it);
            break;
        }else{
            it++;
        }
    }
    
}



Entity World::createEntity(){
    Entity entity = Entity();
    entity._id = _entityId;
    entities.push_back(entity);
    _entityId+=1;
    return entities.back();
}



void World::setComponent(Entity entity, Component* component) {

    auto its = components.equal_range(entity);
    if(its.first != its.second){
        for(auto it = its.first; it != its.second; ++it){
            if(it->second->getType() == component->getType()){
                delete it->second;
                (*it).second = component;
                std::cout << "Replaced component " << component->getType() << " for entity " << entity.id() << " to world " << this->id() <<  std::endl;
                return;
            }
        }
    }
    
    components.emplace(entity, component);
    std::cout<< "Inserted component " << component->getType() << " for entity " << entity.id() << " in world " << this->id() << std::endl;
    queryInactiveSystems();
    
}

void World::updateActive(std::vector<World*>worlds){
    for(World* world : worlds){
        for(auto system : world->activeSystems){
            system->Update();
        }
    }
}

void World::queryInactiveSystems(){

    auto it = inactiveSystems.begin();
    while(it != inactiveSystems.end()){

        std::unordered_set<size_t> dependencies = (*it)->getDependencies();
        
        if(dependencies.size() == 0){
            it++;
            continue;
        }

        std::unordered_set<size_t> tempDependencies = dependencies;
        Entity currEntity = components.begin()->first;

        bool active = false;
        
        auto cIt = components.begin();
        while(cIt != components.end()){

            //check to see if the component meets a dependency, erase dependency because we already know it is met, and won't need to check the next component for it
            auto dIt = tempDependencies.find(cIt->second->getType());
            if(dIt != tempDependencies.end()){
                tempDependencies.erase(dIt);
            }
            
            if(tempDependencies.size() == 0){
                active = true;
                break;
            }
            
            cIt++;
            
            if(cIt != components.end() && currEntity != cIt->first){
                
                tempDependencies = dependencies;
                currEntity = cIt->first;
            }
        
        }

        if(active){
            
            std::cout << "Adding system of type " << (*it)->getType() << " to active systems in world " << (*it)->world.id() << std::endl;
            
            (*it)->OnActive();
            //remove from inactiveSystems, add to activeSystems
            activeSystems.push_back(std::move(*it));
            it = inactiveSystems.erase(it);
        }else{
            it++;
        }
    }
    
}

void World::queryActiveSystems(){
    
    if(components.size() == 0){
        return;
    }
    
    auto it = activeSystems.begin();
    while(it != activeSystems.end()){
        
        std::unordered_set<size_t> dependencies = (*it)->getDependencies();
        
        std::unordered_set<size_t> tempDependencies = dependencies;
        
        bool active = true;
        
        Entity currEntity = components.begin()->first;
        
        auto cIt = components.begin();
        while(cIt != components.end()){

            auto dIt = tempDependencies.find(cIt->second->getType());
            if(dIt != tempDependencies.end()){
                tempDependencies.erase(dIt);
            }
            
            if(tempDependencies.size() > 0){
                active = false;
                break;
            }
            
            cIt++;
            
            if(cIt != components.end() && currEntity != cIt->first){
                
                tempDependencies = dependencies;
                currEntity = cIt->first;
            }
            
        }
        
        if(!active){
            std::cout << "Adding system of type " << (*it)->getType() << " to inactive systems in world " << (*it)->world.id() << std::endl;
            
            //remove from activeSystems, add to inactiveSystems
            inactiveSystems.push_back(std::move(*it));
            it = activeSystems.erase(it);
        }else{
            it++;
        }
    }
}

bool World::removeEntity(Entity entity){
    auto its = components.equal_range(entity);
    if(its.first != its.second){
        auto it = its.first;
        while(it != its.second){
            delete it->second;
            it = components.erase(it);
        }
        std::cout << "Removed entity " << entity.id() << " from world " << this->id() << std::endl;
        queryActiveSystems();
        return true;
    }
    return false;
}

extern "C" void* createWorld() {
    World& world = World::createWorld<>();
    
    return &world;
}
