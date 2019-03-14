#ifndef ECSlua_hpp
#define ECSlua_hpp

#include <luajit-2.0/lua.hpp>
#include <ECS.hpp>
#include <list>

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
//    public:
        void addDependencies(){}
//
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
    
    virtual void OnUpdate();
    virtual void OnActive();
    void notifyComponentChange();
};

class LuaWorld;

class LuaWorld : public World {
public:
    void RegisterSystem(System* system);
};

#endif /* ECSlua_hpp */