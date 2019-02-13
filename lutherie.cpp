#include "lutherie.hpp"

Lutherie::Lutherie(){
    initWindow();
    mainLoop();
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
