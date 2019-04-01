#ifndef luthvulk_hpp
#define luthvulk_hpp

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <map>
#include <optional>
#include <set>

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
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue;
    VkSurfaceKHR surface;
    VkQueue presentQueue;
    
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        
        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };
    
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    
    void initVulkan();
    void createInstance();
    
    bool checkValidationLayerSupport();
    
    std::vector<const char*> getRequiredExtensions();
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData
    );
    
    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    
    //Debug messenger methods
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    void setupDebugMessenger();
    
    //Physical device methods
    void pickPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    
    //logical device methods
    void createLogicalDevice();
    
    //Presentation methods
    void createSurface();
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    
    //QueueFamily methods
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);    
    
    
public:
    VulkGfx();
    ~VulkGfx();

};

#endif //luthvulk_hpp