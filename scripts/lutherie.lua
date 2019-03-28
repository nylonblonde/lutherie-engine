local ffi = require("ffi")

ffi.cdef[[
    typedef struct World World;
    typedef struct System {
        World* world;
    } System;
    typedef struct ComponentGroup {
        System* parent;
    } ComponentGroup;
    typedef struct Component Component;
    typedef struct Entity Entity;
    typedef struct ECSextern {
        World* (*createWorld)();
        int (*world_id)(void*);
        void (*registerSystem)(World*, System*);
        System* (*createSystem)(World*, const char*);
        ComponentGroup* (*createComponentGroup)(System*);
        void (*addComponentDependency)(System*, void*, int);
        const Entity* (*createEntity)(World*);
        void (*removeEntity)(World*, Entity*);
        void (*component_setDataPtr)(Component*, void*);
        Component* (*component_getData)(Component*);
        Component* (*setComponent)(World*, Entity*, int);
        void (*removeComponent)(World*, Entity*, Component*);
        int (*group_size)(System*, void*);
        Entity* (*group_getEntity)(System*, void*, int);
        Component* (*group_getComponent)(System*, void*, int, int);
    } ECSextern;

    void free(void*);
]]

local ecs = ffi.cast("struct ECSextern*", ud);

local C = ffi.C

local systemNames = {}
local componentNames = {}
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
        Components = {},
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
            data = component:new()
            data.embedded = ecs.setComponent(self, ffi.cast("struct Entity*", entity), component.type)
            ecs.component_setDataPtr(data.embedded, data.self)
--            assert(data.embedded ~= NULL, "Cannot set a component on an entity that hasn't been created or has been destroyed")
--            print(data.embedded)
            return data
        end,
        removeComponent = function(self, entity, comp)
            ecs.removeComponent(self, ffi.cast("struct Entity*", entity), comp);
        end,
    }
}

local group_mt = {
    __index = {
        size = function(self)
            return ecs.group_size(self.parent, self)
        end,
        getComponentArray = function(self, component)
            local array = {}
            local size = self:size()
            local component = component()
            local typeCode = component.type
            local ctype = component.ctype
			-- print(component.ctype)
            for i=1, size do
--                entity = ecs.group_getEntity(self.parent, self, i-1)
--                print(entity, typeCode, self.parent.world)

--                assert(self.parent.world.Components[entity])
                array[i] = ffi.cast(ctype,
--                    ecs.group_getComponent(self.parent, self, typeCode, i-1).data
--
                    ecs.group_getComponent(self.parent, self, typeCode, i-1):getData()
                    ):getProxy()
            end
            return array
        end,
    }
}

local component_mt = {
    __index = {
        getData = function(self)
            return ecs.component_getData(self)
        end
    }
}

-- local string_mt = {
--     __tostring = function(v) 
--         return ffi.string(v.val)
--     end
        
-- }

ffi.metatype("World", world_mt)
ffi.metatype("ComponentGroup", group_mt)
ffi.metatype("Component", component_mt)
-- ffi.metatype("struct string", string_mt)

function getInvertedTable(table, target)
    target = target or {}
    for k,v in next, table, nil do
        target[v] = k
    end
    
    return target
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
                if(#systemNames == 0) then getInvertedTable(Systems, systemNames) end
                
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
        Components[self.type] = function()
            if(self.ctype) then return Components[self.type] end
            
            local ctypename = "struct Component"..self.type
            local typedef = "typedef "..ctypename.." { "
            local newObj = {}

            typedef = typedef.."void* embedded; "
            for k,v in next, obj, nil do
                if type(k)=="string" and type(v)=="number" then
                    typedef = typedef.."int "..k.."; "
                    newObj[k] = v
                elseif type(k)=="string" and type(v)=="string" then
                    typedef = typedef.."const char* "..k.."; "
                    newObj[k] = v
                end
            end
            typedef = typedef.."} Component"..self.type..";"

            obj.new = function(obj)
                local proxy = {}
                ffi.cdef (typedef)

                local data = ffi.new(ctypename, newObj)

                setmetatable(proxy, {
                    __index = function(t,k)
                        if(k == "self") then return data end
                        
                        local retVal = data[k]
                        
                        if(type(retVal) == "cdata" and ffi.typeof(retVal) == ffi.typeof("const char *")) then retVal = ffi.string(retVal) end
                            
                        return retVal
                    end,
                    __newindex = function(t,k,v)
                        print("setting: ", k, v)
                        data[k] = v
                    end
                })

                ffi.metatype(data, {
                    __index = {
                        getProxy = function(self)
                            return proxy     
                        end
                    }      
                })
                
                self.ctype = ffi.typeof(ctypename.."&")
                
--                data.val = "yo"
                print("proxy", proxy.self)
            
                return proxy
            end
            Components[self.type] = obj
            return obj
        end
        return Components[self.type]
    end
}