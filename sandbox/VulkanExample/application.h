#define GLFW_INCLUDE_VULKAN

struct GLFWwindow;
struct VkInstance_T;

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

private:
    GLFWwindow* window;
    VkInstance_T* instance;
};