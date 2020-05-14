#define GLFW_INCLUDE_VULKAN


#include <vector>
#include <optional>


struct GLFWwindow;

struct VkInstance_T;
struct VkDebugUtilsMessengerEXT_T;
struct VkSurfaceKHR_T;
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
    void createSurface();
    void pickPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice_T* device);

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice_T* device);

    void createLogicalDevice();

private:
    GLFWwindow* window;
    VkInstance_T* instance;
    VkDebugUtilsMessengerEXT_T* debugMessenger;
    VkSurfaceKHR_T* surface;
    VkPhysicalDevice_T* physicalDevice;
    VkDevice_T* logical_device_;
    VkQueue_T* graphicsQueue;
    VkQueue_T* presentQueue;
};