#pragma once

#include <string>
#include <vector>

#include <vk_common.h>

#include <events.hpp>


struct GLFWwindow;

namespace lve {

    class Application;
    class LveWindow {
    public:
        LveWindow(Application *app, const std::string &title = "Untitled", uint32_t width = 800, uint32_t height = 600, bool resizable = true);
        LveWindow(const LveWindow &) = delete;
        LveWindow &operator=(const LveWindow &) = delete;
        ~LveWindow();

        bool shouldClose() const;
        void close() const;

        bool isVisible() const;
        bool isFocused() const;

        /**
         * @brief Attempts to change the size of the application's window
         * @param width The preferred window width
         * @param height The preferred window height
         */
        void resize(uint32_t width, uint32_t height);

        VkExtent2D getExtent() const { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }

        bool wasWindowResized() { return framebufferResized; }

        void resetWindowResizedFlag() { framebufferResized = false; }

        /**
         * @return The dot-per-inch scale factor
         */
        float get_dpi_factor() const;

        /**
         * @return The scale factor for systems with heterogeneous window and pixel coordinates
         */
        float get_content_scale_factor() const;

        /**
         * @brief Handles the processing of all underlying window events
         */
        virtual void processEvents();

        /**
         * @brief Gets a handle from the platform's Vulkan surface
         * @param instance A Vulkan instance
         * @param window The pointer of the window
         * @returns A VkSurfaceKHR handle (VK_NULL_HANDLE if failed), for use by the application
         */
        VkSurfaceKHR createSurface(VkInstance instance) const;

        /**
         * @brief Returns an array of names of Vulkan instance extensions required for the window.
         *      If successful, the list will always contain VK_KHR_surface, so if you don't require any additional
         *      extensions you can pass this list directly to the VkInstanceCreateInfo struct.
         */
        std::vector<const char *> getInstanceExtensions() const;

    private:

        /**
         * @brief Handles the creation of the window
         * @param properties Preferred window configuration
         */
        void createWindow(const std::string& title, bool resizable);

    private:
        Application *application;
        uint32_t width;
        uint32_t height;
        std::string title;
        bool framebufferResized = false;
        GLFWwindow *window;
    };
}  // namespace lve
