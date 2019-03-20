#include "lutherie.hpp"
#include <chrono>

using namespace ECS;

//static void newState(){
//    mainState = &luaL_newstate();
//}

Lutherie::Lutherie(const char* dir, const char* sDir, const char* rDir, const char* lDir) : projectDir(dir), scriptsDir(sDir), resourcesDir(rDir), libDir(lDir), ecs(new ECSLua(luaL_newstate())) {
    
#ifdef LUTHERIE_MAC
	const char* filename = "test-actual.lua";
	char fullPath[strlen(scriptsDir) + strlen(filename)+1];
	std::string fullPath = std::string(sDir) + std::string(filename);
	strcpy(fullPath, scriptsDir);
	strcat(fullPath, filename);
	ecs->executeLua(fullPath);
#else
	for (std::filesystem::directory_entry it : std::filesystem::recursive_directory_iterator(sDir)) {
		std::cout << it.path() << std::endl;
		ecs->executeLua(it.path().string().c_str());
	}
#endif
    initWindow();
    mainLoop();
}

Lutherie::Lutherie() {}
Lutherie::~Lutherie(){
	delete ecs;
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
        
        auto start = std::chrono::high_resolution_clock::now();
        World::updateActive(World::allWorlds);
        auto end = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration_cast<std::chrono::duration<double>>(end-start).count();
//        std::cout << duration << std::endl;
//        UpdateActiveSystems();
    }
}


