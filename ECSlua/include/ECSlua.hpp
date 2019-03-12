#ifndef ECSlua_hpp
#define ECSlua_hpp

#include <luajit-2.0/lua.hpp>
#include <ECS.hpp>
using namespace ECS;

void printLuaError(lua_State* state);

class ECSLua {
private:
    static ECSLua* instance;
    lua_State* mainState;
    
    ECSLua();
public:
    static ECSLua& Instance();
    
    ECSLua(ECSLua const&) = delete;
    void operator=(ECSLua const&) = delete;
    
    void executeLua(const char* filepath);
    
    lua_State* getState();
    ECSLua(lua_State* s);
    ~ECSLua();
};

class LuaComponent : public Component {
private:
    const size_t type;
public:
    LuaComponent(size_t componentType);
    size_t getType() const {
        return type;
    }
};

class LuaSystem : public System {
protected:
    class ComponentGroup;
private:
    const char* name;
    std::vector<ComponentGroup> componentGroups = {};
    std::vector<ComponentGroup*> views;
protected:
    class ComponentGroup : public System::ComponentGroup {
    public:         
        ComponentGroup(const System::ComponentGroup& base) : System::ComponentGroup(base) {}
//        virtual void updateComponents();
    public:
        void addDependencies(){}

        template<typename... Ts>
        //get table id from lua to treat as the componentType
        void addDependencies(size_t componentType, Ts... args){
            dynamic_cast<LuaSystem&>(parent).dependencies.insert(componentType);
            localDependencies.insert(componentType);
            addDependencies(args...);
            std::cout << "dependencies " << dynamic_cast<LuaSystem&>(parent).dependencies.size() << std::endl;
        }
        
    };
public:
    LuaSystem(World& w, const char* name);
    ~LuaSystem();
    
    template <typename... Ts>
    ComponentGroup& createComponentGroup(Ts... args){
//        componentGroups.emplace(componentGroups.end(), ComponentGroup::createComponentGroup(*this));
        ComponentGroup cg = ComponentGroup::createComponentGroup(*this);

        cg.addDependencies(args...);
        componentGroups.push_back(cg);
        return *&componentGroups.back();
    }
    
    static ComponentGroup* voidPtrToGroup(void* ptr);
    
    void addDependency(ComponentGroup* cg, size_t componentType);
    
    virtual void OnUpdate();
    virtual void OnActive();
    virtual void notifyComponentChange();
};

class LuaWorld;

class LuaWorld : public World {
public:
    void RegisterSystem(System* system);
};

#endif /* ECSlua_hpp */
