local ffi = require("ffi")

ffi.cdef[[
	typedef struct World World;
    typedef struct System System;
    typedef struct ComponentGroup {
        System* parent;
    } ComponentGroup;
    typedef struct Component Component;
    typedef struct Entity Entity;

    World* createWorld();
    int world_id(World* world);

    System* createSystem(World* world, const char* name);
    void registerSystem(World* world, System* system);

    Entity* createEntity(World* world);
    void removeEntity(World* world, Entity* entity);

    Component* setComponent(World* world, Entity* entity, int componentType);
    void removeComponent(World* world, Entity* entity, Component* component);

    ComponentGroup* createComponentGroup(System* system);
    void addComponentDependency(System* system, ComponentGroup* componentGroup, int componentType);
    int group_size(System* parent, ComponentGroup* componentGroup);
    
]]

local ecs = ffi.load("ECSlua")

local C = ffi.C

local systemNames = {}
--TODO: use metamethods to disallow new writes to these tables that aren't done through the proper channels somehow
Systems = {}
Components = {}
Worlds = {}

local function copy(obj, copies)
    copies = copies or {}
    local objType = type(obj)
    local res
    if objType == 'table' then
        if copies[obj] then
            res = copies[obj]
        else
            res = {}
            for k,v in next, obj, nil do
                --prevent recursively copying forever when __index == self. Not sure if this essentially breaks inheritance, but it doesn't seem to
                if(obj == v) then
                    copies[obj] = v    
                end
                
                res[copy(k, copies)] = copy(v, copies)  
            end
            copies[obj] = res
            setmetatable(res, copy(getmetatable(obj), copies))
        end
    else
       res = obj 
    end
    return res
end

local world_mt = {
    __index = {
        Systems = {},

        id = function(self)
            return ecs.world_id(self)
        end,
        createEntity = function(self) 
            local entity = ecs.createEntity(self) 
            return entity
        end,
        removeEntity = function(self, entity)
            ecs.removeEntity(self, entity)
        end,
        setComponent = function(self, entity, comp)
            assert(type(comp) == "function", "setComponent(self, entity, comp) requires function argument")
            local component = comp()
            assert(type(component) == "table", "Component function must return a table value")
            assert(type(component.type) == "number", "Component has no type number")
            component = ecs.setComponent(self, entity, component.type)
            assert(component ~= NULL, "Cannot set a component on an entity that hasn't been created or has been destroyed")
            return component
        end,
        removeComponent = function(self, entity, comp)
            ecs.removeComponent(self, entity, comp);
        end,
    }
}

local group_mt = {
    parent = nil,
    __index = {
        size = function(self)
            return ecs.group_size(self.parent, self)
        end
    }
}

ffi.metatype("World", world_mt)
ffi.metatype("ComponentGroup", group_mt)

function getInvertedTable(table, target)
    target = target or {}
    for k,v in next, table, nil do
        target[v] = k
    end
    
    return target
end

local function setSystemNamesTable()
    return getInvertedTable(Systems, systemNames)
end

function World()

    return {
        createWorld = function(self, ...)

            local world = ecs.createWorld()

            args = { n = select("#", ...), ...}
        
            for i=1,args.n do
                local k = i
                local v = args[i]
                
                assert(v ~= nil, "createWorld argument " .. k .. " is a nil value")
                
                --if this is the first time a world is being built, go ahead and call setSystemNamesTable
                if(#systemNames == 0) then setSystemNamesTable() end
                
                local name = systemNames[v]
                
                local system = v()
                system.embedded = ecs.createSystem(world, name)
                ecs.registerSystem(world, system.embedded)
                
                local names = getInvertedTable(system, names)
                
                for k,v in next, system.ComponentGroups, nil do
                    if names[v] then system[names[v]] = v.init(system, unpack(v.components)) end
                end
                
                world.Systems[name] = system
            end

            Worlds[world:id()+1] = world            
            return world
        end, --createWorld
    }
end

World = World()

function System()
    self = {
        Derived = function (self, obj)
            obj = obj or {}
            obj.super = self
            
            setmetatable(obj, self)
            self.__index = self
            return obj
        end
    }
    self.ComponentGroups = {}
    self.embedded = nil
    self.OnUpdate = function(self)
    end
    self.createComponentGroup = function(self, ...)
        _self = {
           init = function(self, ...)
                local componentGroup = ecs.createComponentGroup(self.embedded)
                args = {n = select("#", ...), ...}
                local component 
                for i=1, args.n do
                    component = args[i]()
                    ecs.addComponentDependency(self.embedded, componentGroup, component.type) 
                end
                componentGroup.parent = self.embedded
                return componentGroup
            end, --returnFunction
            components = {...}
        }
        table.insert(self.ComponentGroups, _self)
        return _self
    end

    return self
end

System = System()

Component = {
    Derived = function(self, obj)
        obj = obj or {}
        self.type = #Components+1
        setmetatable(obj, self)
        self.__index = self
        Components[self.type] = function() return obj end
        return Components[self.type]
    end
}

