local ffi = require("ffi")

ffi.cdef[[
	
]]

local C = ffi.C

--print("Hello, world!")

Class = {}

function Class:Derived(obj) 
    obj = obj or {}
    obj.super = self
    setmetatable(obj, self)
    self.__index = self
    return obj
end

local World = {
    __index = World,
    new = function()
        self = setmetatable({
                inactiveSystems = {},
                activeSystems = {},
                id = 0,
            }, World)
        return self
    end
}

Worlds = {}

function CreateWorld(...)
    local arg = {...}
    world = World.new()
    for i,v in ipairs(arg) do
        table.insert(world.inactiveSystems, v())
    end
    world.id = #Worlds+1
    table.insert(Worlds, world)
    return Worlds[world.id]
end

function copy(obj)
    if(type(obj) ~= "table") then return obj end
    local res = {}
    for k,v in pairs(obj) do
        res[copy(k)] = copy(v) 
    end
    return res
end

function System()
    local private = {
        dependencies = {true}
    }

    self = Class:Derived{
        getDependencies = function() 
            return copy(private.dependencies)    
        end,
    
        OnUpdate = function(self)
          print "You forgot to assign OnUpdate"
        end,
    
        ComponentGroup = function(self, ...)
            local arg = {...}
--            assert(type(v) == "string", "ComponentGroup can only take strings as argument")
            for i,v in ipairs(arg) do
                table.insert(private.dependencies, v)    
            end
        end
    }

    return self
end

System = System()

local Component = Class:Derived{
    __index = Component
}

function MeshComponent()
    return Component:Derived {
        value = 0
    }
end

function MeshSystem(...)
    self = System:Derived {
        OnUpdate = function(self)
--            print "hheyyy"    
        end
    }
    print(#self.getDependencies())
    return self
end

function RenderSystem(...)
    self = System:Derived {
        componentGroup = self.ComponentGroup(MeshComponent),
        OnUpdate = function(self)
--            self.super.OnUpdate()
        end
    }
    return self
end

local world = CreateWorld(MeshSystem, RenderSystem)

function TestPrivacy(...)
    local private = {
        privateTest = 546548
    }
    return System:Derived {
        getTest = function() return private.privateTest end
    }
end

--print(world.inactiveSystems[1].dependencies)

--local world = CreateWorld("MeshSystem", "RenderSystem");
--print(world.id);
--
--local world = CreateWorld("MeshSystem", "RenderSystem");
--world.id = ecs.getWorldId(world.super);
--print(world.id);

--local world = ecs.createWorld()
--
--local entity = ecs.createEntity(world);
--
--local entity = ecs.createEntity(world);
--
--local meshSystem = MeshSystem(world)
--
--component = ecs.newComponent(world, entity)
--
--local renderSystem = RenderSystem(world)
--renderSystem:OnUpdate()


