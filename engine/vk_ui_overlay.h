/*
* UI overlay class using ImGui
*
* Copyright (C) 2017 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#pragma once

#include <memory>
#include <vector>
#include <functional>

#include "glm/glm.hpp"

#include "vk_common.h"


namespace lve
{
    class LveWindow;
    class KeyEvent;
    class MouseEvent;
    class LveDevice;
    class LveSwapChain;
    class LveBuffer;

	class UIOverlay 
	{
	public:
        UIOverlay(const LveWindow* window, LveDevice* device, LveSwapChain* swapChain, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
		~UIOverlay();

        bool isVisible() const { return visible_; }
        void setVisible(bool b) { visible_ = b; dirty_ = true; }

		bool header(const char* caption);
		bool checkBox(const char* caption, bool* value);
		bool checkBox(const char* caption, int32_t* value);
		bool radioButton(const char* caption, bool value);
		bool inputFloat(const char* caption, float* value, float step);
		bool sliderFloat(const char* caption, float* value, float min, float max);
		bool sliderInt(const char* caption, int32_t* value, int32_t min, int32_t max);
		bool comboBox(const char* caption, int32_t* itemindex, std::vector<std::string> items);
		bool button(const char* caption);
		void text(const char* formatstr, ...);

        /**
         * @brief Shows an options window, to be filled using the fill function.
         * @param last_fps The frame rate computed last time.
         * @param func The fill function with ImGui commands defining the body of the window.
         */
        void showOptionsWindow(uint32_t last_fps, std::function<void()> func);

        /** @brief Adds the drawing commands for the ImGui overlay to the given command buffer */
        void draw(const VkCommandBuffer commandBuffer);

        void onResize(uint32_t width, uint32_t height);

        /**
         * A true value indicates that the event has been captured and should not be passed to the main UI.
         */
        bool onKeyEvent(const KeyEvent&);
        bool onMouseEvent(const MouseEvent&);

        // Indicates whether mouse inputs have already been captured by the UI overlay.
        // If true, do not dispatch them to your main game/application.
        // Note: in both cases, always pass on keyboard inputs to imgui.
        bool capturesMouse() const;

        // Indicates whether keyboard inputs have already been captured by the UI overlay.
        // If true, do not dispatch them to your main game/application.
        // Note: in both cases, always pass on mouse inputs to imgui.
        bool capturesKeyboard() const;

    private:
        void init();
        void prepareResources();
        void preparePipeline(VkPipelineCache pipelineCache, VkRenderPass renderPass,
                             VkFormat colorFormat, VkFormat depthFormat, VkSampleCountFlagBits samples);

        void freeResources();
        bool updateBuffers();

    private:
        LveDevice* device_;

        std::unique_ptr<LveBuffer> vertexBuffer_;
        std::unique_ptr<LveBuffer> indexBuffer_;
        int32_t vertexCount_ = 0;
        int32_t indexCount_ = 0;

        std::vector<VkPipelineShaderStageCreateInfo> shaders_;

        VkDescriptorPool descriptorPool_;
        VkDescriptorSetLayout descriptorSetLayout_;
        VkDescriptorSet descriptorSet_;
        VkPipelineLayout pipelineLayout_;
        VkPipeline pipeline_;

        VkDeviceMemory fontMemory_ = VK_NULL_HANDLE;
        VkImage fontImage_ = VK_NULL_HANDLE;
        VkImageView fontView_ = VK_NULL_HANDLE;
        VkSampler sampler_;

        struct PushConstBlock {
            glm::vec2 scale;
            glm::vec2 translate;
        } pushConstBlock_;

        bool visible_ = true;
        bool dirty_ = true;

    protected:
        const LveWindow* window_;
        ///  Scale factor to apply due to a difference between the window and GL pixel sizes
        float content_scale_factor_{1.0f};
    };
}
