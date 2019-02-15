#ifndef lutherie_hpp
#define lutherie_hpp

#ifdef LUTHERIE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>

//#include <assimp/Importer.hpp>
//#include <assimp/scene.h>

//#include <glm/vec3.hpp>
//#include <glm/vec4.hpp>
//#include <glm/mat4x4.hpp>

#include "ECS.hpp"

const int WIDTH = 800;
const int HEIGHT = 600;

using namespace ECS;

class Lutherie {
public:
    
    static bool importAll3D(){
        return false;
    }
    
    static bool import3D(const std::string& pFile){
//        Assimp::Importer importer;
//
//        const aiScene scene = importer.ReadFile(pFile);
//
//        if(!scene){
//            throw std::runtime_error(importer.GetErrorString());
//            return false;
//        }

        return true;
    }
    
    static Lutherie& Instance();
    
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
