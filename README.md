# Lutherie Engine

Lutherie Engine is a game development engine powered by an Entity Component System pattern written in C++. Currently, the engine only features the ECS architecture, to which features like rendering will be added to as development continues.

## Getting Started

**Components** are structs that carry data which will be used by our `System` classes to perform logic. 

```
struct MyComponent : Component {
    int Value;
}
```

**Systems** are user-defined classes that are derived from the base `System` class and they are where the logic takes place. When constructed, they require a reference to the World they are created in. Later, world creation handles the initialization of the Systems, so don't worry too much about what value to pass to the parameter, just know that the constructor will require a reference to a World in its definition. 

Systems use `ComponentGroup` definitions to determine on which entities the system will act. ComponentGroups are created with the public static `createComponentGroup<Ts...>(System& system)` method where `Ts` are Component types.

```
class MySystem : System {
public:
    ComponentGroup group = ComponentGroup::createComponentGroup<MyComponent>(*this);

    MySystem(World& w) : System (w) {
        std::cout << "Hello, World!";
    }
    
    virtual void OnUpdate(){
        //Update loop game logic would go here
    }
}

```

**Worlds** host our `System` classes and their accompanying components. `World` creation is handled by a static function which takes the types that we want to register. System classes do not need to be initialized manually as they are handled in World creation.

```
World::createWorld<MySystem>();
```

On cleanup, the public method `destroyWorld()` should be called on all created worlds, as registered `System`s and attached `Component`s are stored in dynamic memory and will be deleted by this function.

## Attaching Components to Entities

`Component`s require an `Entity` to be "attached" to. Entities are essentially just a glorified index that Components are organized by. Entities should be created by `World`'s public `createEntity()` method. This is so indexing can be handled properly.

```
// createWorld returns a reference to a World stored in the static vector World::allWorlds
World& world = World::createWorld<MySystem>();
Entity entity = world.createEntity();
```

Components are allocated with `new` and are passed to the `setComponent()` method.

```
MyComponent* myComponent = new MyComponent();
myComponent->Value = 1;
world.setComponent(entity, myComponent);
```

Components can be removed from an Entity to stop that Entity from having any System that acts on that Component from acting on it anymore.

```
world.removeEntity<MyComponent>(entity);
```

## Making Use of Components in a System

To actually use the components, we can iterate through all entities that have our dependent components attached.

```
class MySystem : System {
public:
    ComponentGroup group = ComponentGroup::createComponentGroup<MyComponent>(*this);

    MySystem(World& w) : System (w) {
        std::cout << "Hello, World!";
    }

    virtual void OnUpdate(){
        for(size_t i = 0; i < group.size(); ++i){
            std::cout << group.getComponent<MyComponent>(i).Value << " at " << group.getEntity(i) << std::endl;
        }
    }
}
```

And finally, to run an update tick, you just need to call the static `World::updateActive(World::allWorlds)`. Notice that this takes a `vector<World*>` as a parameter, so it can be executed on all current worlds or even just a user-defined selection of them.
