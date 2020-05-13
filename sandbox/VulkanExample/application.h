#define GLFW_INCLUDE_VULKAN


#include <vector>


struct GLFWwindow;

struct VkInstance_T;
struct VkDebugUtilsMessengerEXT_T;
struct VkPhysicalDevice_T;
struct VkDevice_T;
struct VkQueue_T;

class Application {
public:
    Application();
    void run();

private:
    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();

private:
    void createInstance();
    std::vector<const char*> getRequiredExtensions();
    bool checkValidationLayerSupport();
    void setupDebugMessenger();
    void pickPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice_T* device);
    int  findQueueFamilies(VkPhysicalDevice_T* device);
    void createLogicalDevice();

private:
    GLFWwindow* window;
    VkInstance_T* instance;
    VkDebugUtilsMessengerEXT_T* debugMessenger;
    VkPhysicalDevice_T* physicalDevice;
    VkDevice_T* logical_device_;
    VkQueue_T* graphicsQueue;
};