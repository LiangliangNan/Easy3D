#pragma once

#include <memory>
#include <vector>
#include <cassert>

#include <vk_common.h>


namespace lve {

    class LveSwapChain;

    class LveRenderer {
    public:
        LveRenderer(class LveWindow &window, class LveDevice &device, bool vsync);
        ~LveRenderer();

        LveRenderer(const LveRenderer &) = delete;
        LveRenderer &operator=(const LveRenderer &) = delete;

        LveSwapChain* getSwapChain() const { return lveSwapChain.get(); }
        VkRenderPass getSwapChainRenderPass() const;

        float getAspectRatio() const;

        bool isFrameInProgress() const { return isFrameStarted; }

        VkCommandBuffer getCurrentCommandBuffer() const {
            assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
            return commandBuffers[currentFrameIndex];
        }

        int getFrameIndex() const {
            assert(isFrameStarted && "Cannot get frame index when frame not in progress");
            return currentFrameIndex;
        }

        VkCommandBuffer beginFrame();
        void endFrame();
        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

    private:
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapChain();

    private:
        LveWindow &lveWindow;
        LveDevice &lveDevice;
        bool vsync;
        std::unique_ptr<LveSwapChain> lveSwapChain;
        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t currentImageIndex;
        int currentFrameIndex{0};
        bool isFrameStarted{false};
    };

}  // namespace lve
