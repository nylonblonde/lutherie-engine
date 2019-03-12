#include "lutherie.hpp"
#include <chrono>

Lutherie::Lutherie(){}
using namespace ECS;

//static void newState(){
//    mainState = &luaL_newstate();
//}

Lutherie::Lutherie(const char* dir, const char* sDir, const char* rDir, const char* lDir) : projectDir(dir), scriptsDir(sDir), resourcesDir(rDir), libDir(lDir) {
    
//    newState();
//    std::cout << mainState << std::endl;
//    
//    if(!mainState){
//        throw std::runtime_error("Failed to initializa lua!");
//    }
//    luaL_openlibs(mainState);
//
//    executeLua("test.lua");
//    
    
    char filename[] = "test.lua";
    char fullPath[strlen(scriptsDir) + strlen(filename)+1];
    strcpy(fullPath, scriptsDir);
    strcat(fullPath, filename);
    ecs.executeLua(fullPath);
    
    initWindow();
    mainLoop();
}

Lutherie::~Lutherie(){
//    lua_close(mainState);
}

//void Lutherie::executeLua(const char* filename){
//    

//
//    printf("%s\n", fullPath);
//    
//    int result = luaL_dofile(mainState, fullPath);
//    
//    if(result != 0) {
//        printLuaError();
//        return;
//    }
//    
//    lua_getglobal(mainState, "_G");
//    lua_pushnil(mainState);
//    while(lua_next(mainState, 1) != 0){
//
//        lua_pop(mainState, 1);
//    }
//
//}

//void Lutherie::UpdateActiveSystems(){
//    int i;
//        return;
//
//    lua_getglobal(mainState, "getWorlds");
//    
//    int result = lua_pcall(mainState, 0,1,0);
//    if(result != 0){
//        printLuaError();
//        return;
//    }
//
//    //2 pushed a nil so lua_next can populate it with table key
//    lua_pushnil(mainState);
//
//    //3 pushed value of key to stack
//    while(lua_next(mainState, 1) != 0){
//
//        //4 pushed the inactiveSystems field from the table at -1 (top)
//        lua_getfield(mainState, -1, "getInactiveSystems");
//
//        result = lua_pcall(mainState,0,1,0);
//        if(result != 0){
//            printLuaError();
//            return;
//        }
//
//        //5 push nil so lua_next can populate it with table key
//        lua_pushnil(mainState);
//
//        //6 pushed value of key to stack. -2 To reach inactiveSystems
//        while(lua_next(mainState, -2) != 0){
//
//            //7 pushed the OnUpdate field
//            lua_getfield(mainState, -1, "OnUpdate");
//
//            //8 pushes the value of -2 (index 6, system's table value) so we can pass it as 'self'
//            lua_pushvalue(mainState, -2);
//
//            //call OnUpdate with the single argument of self. This pops 7 and 8
//            result = lua_pcall(mainState, 1,0,0);
//            if(result != 0){
//                printLuaError();
//                return;
//            }
//
//            //pop index 6 (so it can be reused on next iteration)
//            lua_pop(mainState, 1);
//        }
//        //index 5 gets popped once the loop is done
//
//        //pop index 3 and 4 because we don't need inactiveSystems anymore
//        lua_pop(mainState, 2);
//    }
//    //index 2 gets popped        
//    
//    //pop the last index when we're done
//    lua_pop(mainState, 1);
//}

//void Lutherie::printLuaError() {
//    const char* message = lua_tostring(mainState, -1);
//    puts(message);
//    lua_pop(mainState, 1);
//}

Lutherie& Lutherie::Instance(){
    static Lutherie instance;
    return instance;
}

void Lutherie::initWindow(){
    glfwInit();
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
    window = glfwCreateWindow(WIDTH, HEIGHT, "Lutherie", nullptr, nullptr);
    
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void Lutherie::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<Lutherie*>(glfwGetWindowUserPointer(window));
    
    app->frameBufferResized = true;
}

void Lutherie::mainLoop(){
    using namespace ECS;

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        
        auto start = std::chrono::high_resolution_clock::now();
        World::updateActive(World::allWorlds);
        auto end = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration_cast<std::chrono::duration<double>>(end-start).count();
//        std::cout << duration << std::endl;
//        UpdateActiveSystems();
    }
}


