#define GLFW_INCLUDE_VULKAN


#include <vector>


struct GLFWwindow;

struct VkInstance_T;
struct VkDebugUtilsMessengerEXT_T;

class Application {
public:
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


private:
    GLFWwindow* window;
    VkInstance_T* instance;
    VkDebugUtilsMessengerEXT_T* debugMessenger;
};