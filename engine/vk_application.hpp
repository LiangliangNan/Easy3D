#pragma once

#include <vk_common.h>

#include <application.hpp>
#include <glm/glm.hpp>


namespace lve {

    class LveDevice;
    class LveRenderer;
    class DescriptorAllocator;
    class UIOverlay;
    class Camera;

    class VulkanApplication : public Application {
    public:
        explicit VulkanApplication(const std::string &name = "VulkanApplication", bool vsync = true);
        ~VulkanApplication();

        int run() override;

        void update() override;

        /** @brief (Virtual) Called when the camera view has changed */
        virtual void viewChanged() {}

    protected:
        virtual void draw(VkCommandBuffer commandBuffer) {}
        virtual void fillUI() {}

        void onResize(uint32_t width, uint32_t height) override;
        void onKeyEvent(const KeyEvent&) override;
        void onMouseEvent(const MouseEvent&) override;

    protected:
        std::unique_ptr<LveDevice> lveDevice{nullptr};
        std::unique_ptr<LveRenderer> lveRenderer{nullptr};
        std::unique_ptr<DescriptorAllocator> descriptorAllocator{nullptr};
        std::unique_ptr<UIOverlay> uiOverlay{nullptr};

        /// todo: resolve mouse button/pos/dx/dy in the Window callback.
        std::unique_ptr<Camera> camera_{nullptr};
        struct {
            bool left = false;
            bool right = false;
            bool middle = false;
        } mouse_buttons_;
        glm::vec2 mousePos;
        bool paused = false;    // for animation
    };

}  // namespace lve
