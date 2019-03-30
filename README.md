# Lutherie Engine

Lutherie Engine is a game development engine powered by an Entity Component System pattern written in C/C++ and supports scripting with Lua. Currently, the engine only features the ECS architecture, to which features like rendering will be added to as development continues.

Currently using [an icon](http://icongal.com/gallery/icon/109662/256/lion_young) by [mattahan](https://www.deviantart.com/mattahan) as the default AppImage icon for built projects.

## Build instructions

Lutherie Engine depends on the following libraries
- [LuaJIT](http://luajit.org)
- [Vulkan](https://vulkan.lunarg.com/sdk/home)
- [GLFW](https://github.com/glfw/glfw)

LuaJIT and GLFW are included as git submodules. When you clone the repository, you can get the submodules as well with `git clone https://github.com/nylonblonde/lutherie-engine --recurse-submodules`. 

Before building, the Vulkan SDK must be installed, and the VULKAN_SDK environment must be set to the installation path.

`cd` into the cloned repository and follow the installation commands below.

List of installation commands on Linux & Mac:

```
mkdir build
cd build
cmake ..
make
```

On Windows, use the NMake Makefiles generator instead of building a Visual Studio project and then build with nmake.

List of installation commands on Windows:

```
mkdir build
cd build
cmake -G "NMake Makefiles" ..
nmake
```

Once built, the engine will be portably contained in the Release directory (or Debug if you change the build type). On Mac, the engine will be packaged as an app, but can still utilize command line arguments if executed from the terminal, and on linux, the app will be packaged as an AppImage.

## Getting Started

Run the program with `./lutherie -o <path-to-a-project-directory>` where the path is an arbitrary folder where you want project files to go. If the directory doesn't exist, it will be created.

Go ahead and close the program, and navigate to the project directory. You can now place lua scripts in the `scripts/` subdirectory. Feel free to read about the enitity component system below, or if you are familiar enough and comfortable with ECS as a concept, take a look at the example lua script at the bottom of the readme to get started.

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

### Example script

```
require("lutherie") -- includes the lutherie lua library

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
local component = world:setComponent(entity, MyComponent)
```