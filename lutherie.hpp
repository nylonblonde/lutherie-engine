#ifndef lutherie_hpp
#define lutherie_hpp

#ifdef LUTHERIE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>

#include "ECS.hpp"

const int WIDTH = 800;
const int HEIGHT = 600;

using namespace ECS;

class Lutherie {
public:
    
    static Lutherie& Instance() {
        static Lutherie instance;
        return instance;
    }
       
    Lutherie(Lutherie const&) = delete;
    void operator=(Lutherie const&) = delete;
        
    Lutherie();
    
private:
    GLFWwindow* window;
    bool frameBufferResized = false;
    
    void initWindow();
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    
    void mainLoop();
    void gameLoop();
};

#endif /* lutherie_hpp */
