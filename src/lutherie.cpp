#include "lutherie.hpp"

Lutherie::Lutherie(){}

Lutherie::Lutherie(const char* dir, const char* sDir, const char* rDir, const char* lDir) : projectDir(dir), scriptsDir(sDir), resourcesDir(rDir), libDir(lDir) {
    
    state = luaL_newstate();
    
    if(!state){
        throw std::runtime_error("Failed to initializa lua!");
    }
    luaL_openlibs(state);

    executeLua("test.lua");
        
    initWindow();
    mainLoop();
}

Lutherie::~Lutherie(){
    lua_close(state);
}

void Lutherie::executeLua(const char* filename){
    
    char fullPath[strlen(scriptsDir) + strlen(filename)+1];
    strcpy(fullPath, scriptsDir);
    strcat(fullPath, filename);

    printf("%s\n", fullPath);
    
    int result = luaL_dofile(state, fullPath);
    
    if(result != 0) {
        printLuaError();
        return;
    }

}


void Lutherie::UpdateActiveSystems(){
    int i;
    lua_getglobal(state, "Worlds");
    
    int top = lua_gettop(state);
    for(i = 1; i <= top; i++){

        //2 pushed a nil so lua_next can populate it with table key
        lua_pushnil(state);

        //3 pushed value of key to stack
        while(lua_next(state, i) != 0){
            
            //4 pushed the inactiveSystems field from the table at -1 (top)
            lua_getfield(state, -1, "inactiveSystems");

            //5 push nil so lua_next can populate it with table key
            lua_pushnil(state);
            
            //6 pushed value of key to stack. -2 To reach inactiveSystems
            while(lua_next(state, -2) != 0){

                //7 pushed the OnUpdate field
                lua_getfield(state, -1, "OnUpdate");

                //8 pushes the value of -2 (index 6, system's table value) so we can pass it as 'self'
                lua_pushvalue(state, -2);

                //call OnUpdate with the single argument of self. This pops 7 and 8
                int result = lua_pcall(state, 1,0,0);
                if(result != 0){
                    printLuaError();
                }
                
                //pop index 6 (so it can be reused on next iteration)
                lua_pop(state, 1);
            }
            //index 5 gets popped once the loop is done

            //pop index 3 and 4 because we no-longer need inactiveSystems
            lua_pop(state, 2);
        }
        //index 2 gets popped        
    }
    
    //pop the last index when we're done
    lua_pop(state, 1);
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
        
        UpdateActiveSystems();
    }
}
