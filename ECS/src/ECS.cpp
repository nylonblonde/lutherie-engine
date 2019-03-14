#include "ECS.hpp"
#include <iostream>

using namespace ECS;

System::System(World& w) : world(w) {}

void System::notifyComponentChange() {
    
    std::cout << views.size() << std::endl;
    for(auto view : views){
        std::cout << typeid(decltype(view)).name() << std::endl;
        view->updateComponents();
    }
}

void System::ComponentGroup::updateHelper(std::unordered_multimap<Entity, Component*> allComponents, std::unordered_set<size_t> tempDependencies){
    entities.clear();
    components.clear();
    
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

void System::ComponentGroup::updateComponents(){

    std::cout << "updateComponents" << std::endl;
    std::cout << &parent << std::endl;
    auto allComponents = parent.world.getComponents();
    auto tempDependencies = localDependencies;

    updateHelper(allComponents, tempDependencies);
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

//Entity::Entity() {}

std::vector<World*> World::allWorlds = {};
uint32_t World::_staticId = 0;

World::World() {
    
    _entityId = 0;
    _id = _staticId;
    _staticId+=1;
    entities = std::unordered_set<Entity>();
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



const Entity& World::createEntity() {
    Entity entity = Entity();
    entity._id = _entityId;
    entities.insert(entity);
    _entityId+=1;
    
    std::cout << "Created entity " << entity.id() << " in world " << this->id() << std::endl; 
    
    return *entities.find(entity);
}



Component* World::setComponent(const Entity entity, Component* component) {

    if(entities.find(entity) == entities.end()){
        return nullptr;
    }
    
    auto its = components.equal_range(entity);
    if(its.first != its.second){
        for(auto it = its.first; it != its.second; ++it){
            if(it->second->getType() == component->getType()){
                delete it->second;
                (*it).second = component;
                std::cout << "Replaced component " << component->getType() << " for entity " << entity.id() << " to world " << this->id() <<  std::endl;
                return component;
            }
        }
    }
    
    components.emplace(entity, component);
    std::cout<< "Inserted component " << component->getType() << " for entity " << entity.id() << " in world " << this->id() << std::endl;
    queryInactiveSystems();
    return component;
}

bool World::removeComponentHelper(const Entity entity, const size_t typeCode) {
    auto its = components.equal_range(entity);
    if(its.first != its.second){
        auto it = its.first;
        while(it != its.second){
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

bool World::removeComponent(const Entity entity, Component* component) {
    return removeComponentHelper(entity, component->getType());
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
//            return;
//            std::cout << *it << " " << activeSystems.back() << std::endl;

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

void World::removeEntity(const Entity entity){
    auto its = components.equal_range(entity);
    if(its.first != its.second){
        auto it = its.first;
        while(it != its.second){
            delete it->second;
            it = components.erase(it);
        }
        queryActiveSystems();
    }
    auto it = entities.find(entity);
    if(it != entities.end()) {
        entities.erase(it);
        std::cout << "Removed entity " << entity.id() << " from world " << this->id() << std::endl;
    }
}