#include "lutherie.hpp"

Lutherie::Lutherie(){}

Lutherie::Lutherie(const char* dir) : projectDir(dir) {
    
    state = luaL_newstate();
    
    if(!state){
        throw std::runtime_error("Failed to initializa lua!");
    }
    luaL_openlibs(state);

    executeLua("/Users/lyon/Documents/lutherie-engine/scripts/test.lua");
    
    initWindow();
    mainLoop();
}

Lutherie::~Lutherie(){
    lua_close(state);
}

void Lutherie::executeLua(const char* filename){
    
    int result = luaL_loadfile(state, filename);
    
    if(result != 0) {
        printLuaError();
        return;
    }
    
    result = lua_pcall(state, 0, LUA_MULTRET, 0);
    
    if(result != 0) {
        printLuaError();
        return;
    }
}

void Lutherie::printLuaError() {
    const char* message = lua_tostring(state, -1);
    puts(message);
    lua_pop(state, 1);
}

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
        
        World::updateActive(World::allWorlds);
    }
}
