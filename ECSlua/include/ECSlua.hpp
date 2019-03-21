#ifndef ECSlua_hpp
#define ECSlua_hpp

#include <lua.hpp>
#include <ECS.hpp>
#include <list>

#include "ecslua_export.h"

using namespace ECS;

void printLuaError(lua_State* state);

class ECSLua {
private:
    static ECSLua* instance;
    lua_State* mainState;
    
    ECSLua();

public:
    static ECSLua& Instance();
    
	ECSLua(lua_State* s);
	~ECSLua();
	ECSLua(ECSLua const&) = delete;
	void operator=(ECSLua const&) = delete;
    static void executeLua(const char* filepath);
    void* ecsExtern;
    lua_State* getState();
    
};

class LuaComponent : public Component {
private:
    const size_t type;
    void* data = nullptr;
public:
    LuaComponent(size_t componentType);
    ~LuaComponent();
    size_t getType() const {
        return type;
    }
    void* getData() const {
        return data;
    }
    void setDataPtr(void* ptr);
};

class LuaSystem : public System {

private:
    const char* name;
    std::list<ComponentGroup> componentGroups = {};
protected:
    class ComponentGroup : public System::ComponentGroup {
    public:         
        ComponentGroup(const System::ComponentGroup& base) : System::ComponentGroup(base) {
            static_cast<LuaSystem&>(parent).views.push_back(this);
        }
        void updateComponents();
        void addDependencies(){}

        template<typename... Ts>
        void addDependencies(size_t componentType, Ts... args){
            std::cout << &parent << std::endl;
            static_cast<LuaSystem&>(parent).dependencies.insert(componentType);

            localDependencies.insert(componentType);
//            std::cout << localDependencies.size() << std::endl;
            
            addDependencies(args...);
        }
//        
//        void getComponent(size_t componentType);
//        
    };
protected:
    std::vector<ComponentGroup*> views;

public:
    LuaSystem(World& w, const char* name);
    ~LuaSystem();
    
    template <typename... Ts>
    System::ComponentGroup& createComponentGroup(Ts... args){
//        std::cout << "run once" << std::endl;
        System::ComponentGroup* cg = new ComponentGroup(ComponentGroup::createComponentGroup(*this));        
//        componentGroups.push_back(cg);
        return *cg;
//        return *&componentGroups.back();
    }
    
    static System::ComponentGroup* voidPtrToGroup(void* ptr);
    
    void addDependency(void* cg, size_t componentType);
    int getGroupSize(System::ComponentGroup* cg);
    const Entity* getEntity(System::ComponentGroup* cg, int index);
    Component* getComponent(System::ComponentGroup* cg, size_t typeCode, int index);
    
    virtual void OnUpdate();
    virtual void OnActive();
    void notifyComponentChange();
};

class LuaWorld;

class LuaWorld : public World {
public:
    void RegisterSystem(System* system);
};

extern "C" {
    void* createWorld();
    
    int world_id(World* world);

    void registerSystem(LuaWorld* world, System* system);
    
    void* createSystem(World* world, const char* name);
    
    void* createComponentGroup(LuaSystem* system);
    
    void addComponentDependency(LuaSystem* system, void* groupPtr, int componentType);
    
    const Entity* createEntity(World* world);
    
    void removeEntity(LuaWorld* world, Entity* entity);
    
    void component_setDataPtr(LuaComponent* component, void* ptr);
    
    void* component_getData(LuaComponent* component);
    
    void* setComponent(LuaWorld* world, Entity* entity, int componentType);
    
    void removeComponent(LuaWorld* world, Entity* entity, LuaComponent* component);
    
    int group_size(LuaSystem* system, void* groupPtr);
    
    const Entity* group_getEntity(LuaSystem* system, void* groupPtr, int index);
    
    void* group_getComponent(LuaSystem* system, void* groupPtr, int typeCode, int index);

    typedef struct ECSextern {

        void* (*createWorld)();
        int (*world_id)(World*);
        void (*registerSystem)(LuaWorld*, System*);
        void* (*createSystem)(World*, const char*);
        void* (*createComponentGroup)(LuaSystem*);
        void (*addComponentDependency)(LuaSystem*, void*, int);
        const Entity* (*createEntity)(World*);
        void (*removeEntity)(LuaWorld*, Entity*);
        void (*component_setDataPtr)(LuaComponent*, void*);
        void* (*component_getData)(LuaComponent*);
        void* (*setComponent)(LuaWorld*, Entity*, int);
        void (*removeComponent)(LuaWorld*, Entity*, LuaComponent*);
        int (*group_size)(LuaSystem*, void*);
        const Entity* (*group_getEntity)(LuaSystem*, void*, int);
        void* (*group_getComponent)(LuaSystem*, void*, int, int);

    } ECSextern;
}

#endif /* ECSlua_hpp */
