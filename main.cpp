#include "ECS.hpp"

struct MyComponent : Component {
    int Value;
};

class MySystem : public System {
public:
    ComponentGroup group = ComponentGroup::createComponentGroup<MyComponent>(*this);
    
    MySystem(World& w) : System(w) {
        
    }
    
    virtual void OnUpdate(){
        std::cout << group.size() << std::endl;
    }
    
};

int main(){
    
    World* world = World::createWorld<MySystem>();
    Entity entity = world->createEntity();
    world->setComponent(entity, new MyComponent());
    
    while(true){
        World::updateActive(World::allWorlds);
    }
    
    return 0;
}
