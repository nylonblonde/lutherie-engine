#ifndef luthvulk_hpp
#define luthvulk_hpp

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cstring>

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers = {
  "VK_LAYER_LUNARG_standard_validation"  
};

const int WIDTH = 800;
const int HEIGHT = 600;

class Gfx {
protected: 
    GLFWwindow* window;
    
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    bool frameBufferResized = false;
    
public:
    
    virtual void initWindow();
    virtual bool windowShouldClose();
};

class VulkGfx : public Gfx {
private:
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    
    void initVulkan();
    void createInstance();
    
    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData
    );
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    void setupDebugMessenger();
public:
    VulkGfx();
    ~VulkGfx();

};

#endif //luthvulk_hpp