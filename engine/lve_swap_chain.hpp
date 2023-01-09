#pragma once

#include "lve_device.hpp"

// vulkan headers
#include <vk_common.h>

// std lib headers
#include <memory>
#include <string>
#include <vector>

namespace lve {

    class LveSwapChain {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 3;

        LveSwapChain(LveDevice &deviceRef, const VkExtent2D& windowExtent, bool vsync);
        LveSwapChain(LveDevice &deviceRef, const VkExtent2D& windowExtent, std::shared_ptr<LveSwapChain> previous, bool vsync);

        ~LveSwapChain();

        LveSwapChain(const LveSwapChain &) = delete;
        LveSwapChain &operator=(const LveSwapChain &) = delete;

        VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }

        VkRenderPass getRenderPass() { return renderPass; }

        VkImageView getImageView(int index) { return swapChainImageViews[index]; }

        size_t imageCount() { return swapChainImages.size(); }

        VkFormat getImageFormat() { return swapChainImageFormat; }
        VkFormat getDepthFormat() { return swapChainDepthFormat; }

        VkExtent2D getExtent() { return swapChainExtent; }

        uint32_t width() { return swapChainExtent.width; }
        uint32_t height() { return swapChainExtent.height; }

        float extentAspectRatio() {
            return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
        }

        VkFormat findDepthFormat();
        VkResult acquireNextImage(uint32_t *imageIndex);
        VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

        bool compareSwapFormats(const LveSwapChain &swapChain) const {
            return swapChain.swapChainDepthFormat == swapChainDepthFormat &&
                   swapChain.swapChainImageFormat == swapChainImageFormat;
        }

    private:
        void init(bool vsync);
        void createSwapChain(bool vsync);
        void createImageViews();
        void createDepthResources();
        void createRenderPass();
        void createFramebuffers();
        void createSyncObjects();

        // Helper functions
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(bool vsync, const std::vector<VkPresentModeKHR> &availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

        VkFormat swapChainImageFormat;
        VkFormat swapChainDepthFormat;
        VkExtent2D swapChainExtent;

        std::vector<VkFramebuffer> swapChainFramebuffers;
        VkRenderPass renderPass;

        std::vector<VkImage> depthImages;
        std::vector<VkDeviceMemory> depthImageMemorys;
        std::vector<VkImageView> depthImageViews;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;

        LveDevice &device;
        VkExtent2D windowExtent;

        VkSwapchainKHR swapChain;
        std::shared_ptr<LveSwapChain> oldSwapChain;

        std::vector<VkSemaphore> imageAvailableSemaphores; //todo: only one is enough. Check Sascha Willems's examples
        std::vector<VkSemaphore> renderFinishedSemaphores; //todo: only one is enough. Check Sascha Willems's examples
        std::vector<VkFence> inFlightFences; //todo: only one is enough. Check Sascha Willems's examples
        std::vector<VkFence> imagesInFlight; //todo: only one is enough. Check Sascha Willems's examples
        size_t currentFrame = 0;
    };

}  // namespace lve
