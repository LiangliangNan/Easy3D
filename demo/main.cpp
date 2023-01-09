
#include <iostream>
#include <stdexcept>

#include "vk_application.hpp"
#include "first_app.hpp"

int main() {
    lve::FirstApp app("EngineDemo");
//    lve::VulkanApplication app("EngineDemo");
    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}