#include "ECSlua.hpp"

//using namespace ECSLua;
ECSLua::ECSLua(){}

ECSLua* ECSLua::instance = nullptr;

ECSLua::ECSLua(lua_State* s, const char* d) : mainState(s), dir(d){
    if(!mainState){
        throw std::runtime_error("Failed to initializa lua!");
    }

    ecsExtern = init_ECSextern();

    luaL_openlibs(mainState);

	lua_getglobal(mainState, "package");
	lua_getfield(mainState, -1, "path");
	luaL_Buffer b;
	luaL_buffinit(mainState, &b);
	luaL_addvalue(&b);
    char* luaPath = (char*)malloc(strlen(dir)+24);
    strcpy(luaPath, ";./");
    strcat(luaPath, dir);
    strcat(luaPath, "/libs/lua/?.raw;;");
	luaL_addstring(&b, luaPath);
	luaL_pushresult(&b);
	lua_setfield(mainState, -2, "path");
	lua_pop(mainState, 1);
    free(luaPath);
    //pass the ECSextern struct to lua which contains the pointers to functions
    lua_pushlightuserdata(mainState, ecsExtern);
    lua_setglobal(mainState, "ud");

    instance = this;
}
ECSLua::~ECSLua(){
    free(ecsExtern);
    lua_close(mainState);
}

lua_State* ECSLua::getState(){
    return mainState;
}

void ECSLua::executeLua(const char* filepath){
    
    lua_State* mainState = ECSLua::Instance().mainState;
    
    int result = luaL_dofile(mainState, filepath);

    if(result != 0) {
        printLuaError(mainState);
        return;
    }
}

void printLuaError(lua_State* state) {
    const char* message = lua_tostring(state, -1);
    puts(message);
    lua_pop(state, 1);
}

ECSLua& ECSLua::Instance(){
    if(instance == nullptr){
        throw std::runtime_error("ECSLua singleton returned null! Do not call Instance() before it ever gets set");
    }
    return *instance;
}



LuaSystem::LuaSystem(World& w, const char* n) : System(w), name(n){
//    Lutherie::Instance()::getState();
    std::cout << name << std::endl;
}

LuaSystem::~LuaSystem(){
    for(auto view : views){
        delete view;
    }
}

void LuaSystem::addDependency(void* cg, size_t componentType){

    LuaSystem::ComponentGroup* lg = static_cast<LuaSystem::ComponentGroup*>(cg);

//    std::cout << &lg->getParent() << std::endl;

    lg->addDependencies(componentType);

}

void LuaSystem::OnActive(){}

void LuaSystem::OnUpdate(){
              
    lua_State* state = ECSLua::Instance().getState();

    lua_getglobal(state, "Worlds");

    //index 1 is Worlds table
    lua_pushnil(state);

    //index 2 is now reserved for keys
    while(lua_next(state, 1) != 0){
        //index 3 is now world value

        lua_getfield(state, -1, "Systems");
        //index 4 is now Systems table
        lua_getfield(state, -1, name);
        //index 5 is now the correct system
        lua_getfield(state, -1, "OnUpdate");
        //index 6 is now the OnUpdate function

        lua_pushvalue(state, -2);
        //index 7 is system self

        int result = lua_pcall(state, 1, 0, 0);

        if(result != 0){
            printLuaError(state);
            lua_pop(state, lua_gettop(state));
            return;
        }

        lua_pop(state, 3);
    }

    lua_pop(state, 1);

}

void LuaSystem::notifyComponentChange() {

    for(auto view : views){
        view->updateComponents();
    }
}

void LuaSystem::ComponentGroup::updateComponents(){
        
    System::ComponentGroup::updateHelper(static_cast<LuaSystem&>(parent).world.getComponents(), localDependencies);
}

void LuaWorld::RegisterSystem(System* system) {
    inactiveSystems.emplace(inactiveSystems.end(), system);
}

void LuaComponent::setDataPtr(void* ptr){
    if(data == nullptr){
        data = ptr;
    }
}

System::ComponentGroup* LuaSystem::voidPtrToGroup(void* ptr) {
    return (System::ComponentGroup*)ptr;
}

int LuaSystem::getGroupSize(System::ComponentGroup* cg) {
    return static_cast<int>(cg->size());
}

const Entity* LuaSystem::getEntity(System::ComponentGroup* cg, int index){
    return cg->getEntity(index);
}

Component* LuaSystem::getComponent(System::ComponentGroup* cg, size_t typeCode, int index){
    return cg->getComponent(typeCode, index);
}

LuaComponent::LuaComponent(size_t componentType) : type(componentType){}

LuaComponent::~LuaComponent() {
    free(data);
}

extern "C" {
    void* createWorld() {
        return &static_cast<LuaWorld&>(World::createWorld());
    }
    
    int world_id(World* world) {
        return world->id();
    }

    void registerSystem(LuaWorld* world, System* system){
        world->RegisterSystem(system);
    }
    
    void* createSystem(World* world, const char* name){
        return new LuaSystem(*world, name);
    }
    
    void* createComponentGroup(LuaSystem* system) {
        return &system->createComponentGroup();
    }
    
    void addComponentDependency(LuaSystem* system, void* groupPtr, int componentType) {
        system->addDependency(groupPtr, componentType);
    }
    
    const Entity* createEntity(World* world) {
        return &world->createEntity();
    }
    
    void removeEntity(LuaWorld* world, Entity* entity){
        world->removeEntity(*entity);
    }
    
    void component_setDataPtr(LuaComponent* component, void* ptr){
        component->setDataPtr(ptr);
    }
    
    void* component_getData(LuaComponent* component) {
        return component->getData();
    }
    
    void* setComponent(LuaWorld* world, Entity* entity, int componentType){
        return world->setComponent(*entity, new LuaComponent(componentType));
    }
    
    void removeComponent(LuaWorld* world, Entity* entity, LuaComponent* component){
        world->removeComponent(*entity, component);
    }
    
    int group_size(LuaSystem* system, void* groupPtr) {
        return system->getGroupSize(LuaSystem::voidPtrToGroup(groupPtr));
    }
    
    const Entity* group_getEntity(LuaSystem* system, void* groupPtr, int index){
        return system->getEntity(LuaSystem::voidPtrToGroup(groupPtr), index);
    }
    
    void* group_getComponent(LuaSystem* system, void* groupPtr, int typeCode, int index){
        return system->getComponent(LuaSystem::voidPtrToGroup(groupPtr), typeCode, index);
    }   
}

ECSextern* init_ECSextern() {
    ECSextern* ecs = (ECSextern*)malloc(sizeof(struct ECSextern));

    ecs->createWorld = &createWorld;
    ecs->world_id = &world_id;
    ecs->registerSystem = &registerSystem;
    ecs->createSystem = &createSystem;
    ecs->createComponentGroup = &createComponentGroup;
    ecs->addComponentDependency = &addComponentDependency;
    ecs->createEntity = &createEntity;
    ecs->removeEntity = &removeEntity;
    ecs->component_setDataPtr = &component_setDataPtr;
    ecs->component_getData = &component_getData;
    ecs->setComponent = &setComponent;
    ecs->removeComponent = &removeComponent;
    ecs->group_size = &group_size;
    ecs->group_getEntity = &group_getEntity;
    ecs->group_getComponent = &group_getComponent;

    return ecs;
}

