# Lutherie Engine

Lutherie Engine is a game development engine powered by an Entity Component System pattern written in C/C++ and supports scripting with Lua. Currently, the engine only features the ECS architecture, to which features like rendering will be added to as development continues.

## Build instructions

Lutherie Engine depends on the following libraries
- [LuaJIT](http://luajit.org/git/luajit-2.0.git)
- [Vulkan](https://vulkan.lunarg.com/sdk/home)
- [GLFW](https://github.com/glfw/glfw)

Ensure that you have the shared libraries of GLFW installed rather than static. You can follow their build instructions to ensure shared libraries are built if compiling from source.

From the root directory, make a build directory, cd into it, and run `cmake ..` and then `make`.

cmake will require some arguments to determine where the dependencies are located if pkgconfig cannot find the dependencies. Check CMakeLists.txt for arguments you may have to pass if they are not installed in the default installation locations.

Vulkan will require you set a VULKAN_SDK environment variable with a path to where the SDK is located in order to find it. Alternatively, you can manually set the library directories and include directories with cmake definition arguments, ie: `cmake -DVulkan_LIBRARY_DIRS:PATH=~/vulkan/macOS/lib -DVulkan_INCLUDE_DIRS:PATH=~/vulkan/macOS/include ..` as opposed to just `cmake ..` below.

List of installation commands:

```
mkdir build
cd build
cmake ..
make
```

Once built, you can run the engine with `./lutherie`, which will output arguments for usage.

## Getting Started

Run the program from your build directory with `./lutherie -o <path-to-a-project-directory>` where the path is an arbitrary folder where you want project files to go. If the directory doesn't exist, it will be created.

Go ahead and close the program, and navigate to the project directory. You can place lua scripts in the `scripts/` subdirectory.

## ECS in C++

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

## Scripting in Lua

# Example script

```require("lutherie") -- includes the lutherie lua library

-- Defining a component struct
MyComponent = Component:Derived {
    val = 100;   
} 

-- Defining a System factory function
function Systems.MySystem()
    return System:Derived {
        group = self:createComponentGroup(MyComponent),
        OnUpdate = function(self)
            myComponents = self.group:getComponentArray(MyComponent)
            for i=1, self.group:size() do
                print(myComponents[i].val) 
            end
        end
    }
end

-- Creating a world, passing System factory functions which will register Systems to World
local world = World:createWorld(Systems.MySystem)
-- Creating an Entity to which we can assign components
local entity = world:createEntity()
-- Setting a component to an Entity
local component = world:setComponent(entity, MyComponent)```