#include "lutherie.hpp"
#include <chrono>
//using namespace ECS;

Lutherie::Lutherie(const char* dir, const char* sDir, const char* rDir, const char* lDir) : projectDir(dir), scriptsDir(sDir), resourcesDir(rDir), libDir(lDir), ecs(new ECSLua(luaL_newstate(), dir)) {
    
#if defined(LUTHERIE_MAC) || defined(__unix__)
    
    char * libScripts = new char[strlen(libDir)+12];
    strcpy(libScripts, libDir);
    strcat(libScripts, "lua/scripts/");
    
    std::cout << libScripts << std::endl;
    
    void (*f)(const char*) = &ECSLua::executeLua;
    fs::doOnFilesInDir(libScripts, f);    
    fs::doOnFilesInDir(scriptsDir, f);
    
	delete[] libScripts;

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

	glfwDestroyWindow(window);
	glfwTerminate();
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

    }
}


