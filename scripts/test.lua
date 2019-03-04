local ffi = require("ffi")

ffi.cdef[[
	
]]

local C = ffi.C

--print("Hello, world!")

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

local Class = {}

function Class:Derived(obj) 
    obj = obj or {}
    obj.super = self
    setmetatable(obj, self)
    self.__index = self
    return obj
end

local Worlds = {}

local function EntityFactory()
    local factory = {
        id = 0
    }
    
    return {
        new = function()
            local private = {
                id = factory.id+1
            }
            factory.id = private.id
            return {
                getId = function() return private.id end
            }
        end,
    }
end

local World = {
    new = function(...) 
        local private = {
            components = {},
            inactiveSystems = {},
            activeSystems = {},
            Entity = EntityFactory()
        }
        
        local Entity = EntityFactory()
        
        local arg = {...}
        
        for i,v in ipairs(arg) do
            assert(type(v) == "function", "CreateWorld must take functions as arguments")
            table.insert(private.inactiveSystems, v())
        end
        
        self = {
            
            getComponents = function() 
                return copy(private.components)
            end,
            
            getInactiveSystems = function() 
--                print(#private.inactiveSystems)
                return copy(private.inactiveSystems)
            end,
            
            getActiveSystems = function() 
                return copy(private.activeSystems)
            end,
            
            createEntity = function()
                entity = private.Entity.new()
                private.components[entity.getId()] = {1,2,3,4,5}
                return entity
            end,
            
            id = #Worlds+1
        }
        
        table.insert(Worlds, self)
        return Worlds[self.id]
    end
}

function CreateWorld(...)
    return World.new(...)
end

function getWorlds()
    return Worlds;
end


local function SystemFactory()
    local factory = {
        dependencies = {true}
    }

    return Class:Derived{
        
        getDependencies = function() 
            return copy(factory.dependencies)    
        end,
    
        OnUpdate = function(self)
          print "You forgot to assign OnUpdate"
        end,
    
        ComponentGroup = function(self, ...)
            local arg = {...}
            local private = {
                entities = {},
                components = {},
                dependencies = {}
            }
            
            for i,v in ipairs(arg) do
                assert(type(v) == "function", "ComponentGroup must take functions as arguments")
                table.insert(factory.dependencies, v)    
            end
            
            return {
                getDependencies = function() return copy(private.dependencies) end,
            }
        end
    }

end

local System = SystemFactory()

local Component = Class:Derived{}

local world = CreateWorld(MeshSystem, RenderSystem)

function MeshComponent()
    return Component:Derived {
        value = 0
    }
end

function MeshSystem()
    self = System:Derived {
        OnUpdate = function(self)
--            print "hheyyy"    
        end
    }
--    print(#self.getDependencies())
    return self
end

function RenderSystem()
    self = System:Derived {
        componentGroup = self:ComponentGroup(MeshComponent),
        OnUpdate = function(self)
--            self.super.OnUpdate()
        end
    }
    return self
end

local x = os.clock()
for i=1, 10000 do 
    local entity = world.createEntity()
--    for k,v in next, world.getComponents()[entity.getId()], nil do
----        print(k,v)
--    end
    local world2 = CreateWorld(MeshSystem, RenderSystem)
    local entity2 = world2.createEntity()
--    print(entity.getId(), entity2.getId())
end
--print(#Worlds)
print("elapsed time:", os.clock()-x)


