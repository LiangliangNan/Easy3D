
/*
* UI overlay class using ImGui
*
* Copyright (C) 2017 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include "vk_ui_overlay.h"
#include "lve_window.hpp"
#include "lve_buffer.hpp"
#include "lve_device.hpp"
#include <lve_swap_chain.hpp>
#include <vk_shaders.h>
#include <vk_initializers.h>
#include <lve_utils.hpp>
#include <logging.h>
#include "events.hpp"

#include "imgui.h"


namespace lve {
    UIOverlay::UIOverlay(const LveWindow *window, LveDevice *device_, LveSwapChain *swapChain,
                         VkSampleCountFlagBits samples)
            : window_(window), device_(device_),
              content_scale_factor_(window->get_content_scale_factor()) {
        init();
        prepareResources();
        // todo: use a valid pipeline cache?
        preparePipeline(VK_NULL_HANDLE, swapChain->getRenderPass(), swapChain->getImageFormat(), swapChain->getDepthFormat(), samples);
    }

    UIOverlay::~UIOverlay() {
        freeResources();

        if (ImGui::GetCurrentContext()) {
            ImGui::DestroyContext();
        }
    }

    void UIOverlay::init() {
        // Init ImGui
        ImGui::CreateContext();
        // Color scheme
        ImGuiStyle &style = ImGui::GetStyle();
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.005f, 0.005f, 0.005f, 0.94f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.0f, 0.0f, 0.0f, 0.1f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
        style.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.2f, 0.0f, 0.8f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.1f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
        style.Colors[ImGuiCol_Button] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);

        // Set window border thickness
        style.WindowBorderSize = 1.0f;

        // Global scale
        // Set ImGui style scale factor to handle retina and other HiDPI displays
        // Note: Scaling the fonts is done separately
        style.ScaleAllSizes(content_scale_factor_);

        // Dimensions
        ImGuiIO &io = ImGui::GetIO();
        const auto extent = window_->getExtent();
        io.DisplaySize = ImVec2((float) extent.width * content_scale_factor_,
                                (float) extent.height * content_scale_factor_);
        io.FontGlobalScale = 1.0f; // scale this results in blurry texts (so we scale the font size)
        io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

        // Enable keyboard navigation
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.KeyMap[ImGuiKey_Space] = static_cast<int>(Key::Space);
        io.KeyMap[ImGuiKey_Enter] = static_cast<int>(Key::Enter);
        io.KeyMap[ImGuiKey_LeftArrow] = static_cast<int>(Key::Left);
        io.KeyMap[ImGuiKey_RightArrow] = static_cast<int>(Key::Right);
        io.KeyMap[ImGuiKey_UpArrow] = static_cast<int>(Key::Up);
        io.KeyMap[ImGuiKey_DownArrow] = static_cast<int>(Key::Down);
        io.KeyMap[ImGuiKey_Tab] = static_cast<int>(Key::Tab);
    }

    /** Prepare all vulkan resources required to render the UI overlay */
    void UIOverlay::prepareResources() {
        shaders_ = {
                vkutil::load_shader(device_->device(), std::string(RESOURCE_DIR) + "shaders/base/uioverlay.vert.spv",
                                    VK_SHADER_STAGE_VERTEX_BIT),
                vkutil::load_shader(device_->device(), std::string(RESOURCE_DIR) + "shaders/base/uioverlay.frag.spv",
                                    VK_SHADER_STAGE_FRAGMENT_BIT)
        };

        ImGuiIO &io = ImGui::GetIO();

        // Create font texture
        unsigned char *fontData;
        int texWidth, texHeight;

        const std::string filename = std::string(RESOURCE_DIR) + "fonts/Roboto-Medium.ttf";
        io.Fonts->AddFontFromFileTTF(filename.c_str(), 16.0f * content_scale_factor_);
        io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);

        // Create target image for copy
        // TODO: add an 'Image' class (ref: Vulkan official samples)
        VkImageCreateInfo imageInfo = vkinit::image_create_info(
                VK_FORMAT_R8G8B8A8_UNORM,
                VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                {static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1}
        );
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageInfo.extent.width = texWidth;
        imageInfo.extent.height = texHeight;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VK_CHECK(vkCreateImage(device_->device(), &imageInfo, nullptr, &fontImage_));
        VkMemoryRequirements memReqs;
        vkGetImageMemoryRequirements(device_->device(), fontImage_, &memReqs);
        VkMemoryAllocateInfo memAllocInfo{};
        memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memAllocInfo.allocationSize = memReqs.size;
        memAllocInfo.memoryTypeIndex = device_->findMemoryType(memReqs.memoryTypeBits,
                                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VK_CHECK(vkAllocateMemory(device_->device(), &memAllocInfo, nullptr, &fontMemory_));
        VK_CHECK(vkBindImageMemory(device_->device(), fontImage_, fontMemory_, 0));

        // Image view
        // TODO: add an 'ImageView' class (ref: Vulkan official samples)
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = fontImage_;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.layerCount = 1;
        VK_CHECK(vkCreateImageView(device_->device(), &viewInfo, nullptr, &fontView_));

        // Staging buffers for font data upload
        LveBuffer stagingBuffer(*device_,
                                sizeof(char),
                                texWidth * texHeight * 4,
                                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        stagingBuffer.map();
        stagingBuffer.writeToBuffer(fontData);
        stagingBuffer.unmap();

        // Copy buffer data to font image
        VkCommandBuffer copyCmd = device_->beginSingleTimeCommands();

        // Prepare for transfer
        tools::setImageLayout(
                copyCmd,
                fontImage_,
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_PIPELINE_STAGE_HOST_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT);

        // Copy
        VkBufferImageCopy bufferCopyRegion = {};
        bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        bufferCopyRegion.imageSubresource.layerCount = 1;
        bufferCopyRegion.imageExtent.width = texWidth;
        bufferCopyRegion.imageExtent.height = texHeight;
        bufferCopyRegion.imageExtent.depth = 1;

        vkCmdCopyBufferToImage(
                copyCmd,
                stagingBuffer.getBuffer(),
                fontImage_,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &bufferCopyRegion
        );

        // Prepare for shader read
        tools::setImageLayout(
                copyCmd,
                fontImage_,
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

        device_->endSingleTimeCommands(copyCmd);

        stagingBuffer.destroy();

        // Font texture Sampler
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        VK_CHECK(vkCreateSampler(device_->device(), &samplerInfo, nullptr, &sampler_));

        // Descriptor pool
        std::vector<VkDescriptorPoolSize> poolSizes = {
                {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}
        };
        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = 1;
        VK_CHECK(vkCreateDescriptorPool(device_->device(), &descriptorPoolInfo, nullptr, &descriptorPool_));

        // Descriptor set layout
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
                {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}
        };
        VkDescriptorSetLayoutCreateInfo descriptorLayout{};
        descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorLayout.pBindings = setLayoutBindings.data();
        descriptorLayout.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        VK_CHECK(vkCreateDescriptorSetLayout(device_->device(), &descriptorLayout, nullptr, &descriptorSetLayout_));

        // Descriptor set
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool_;
        allocInfo.pSetLayouts = &descriptorSetLayout_;
        allocInfo.descriptorSetCount = 1;
        VK_CHECK(vkAllocateDescriptorSets(device_->device(), &allocInfo, &descriptorSet_));
        VkDescriptorImageInfo fontDescriptor{};
        fontDescriptor.sampler = sampler_;
        fontDescriptor.imageView = fontView_;
        fontDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
                vkinit::write_descriptor_image(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptorSet_,
                                               &fontDescriptor, 0)
        };
        vkUpdateDescriptorSets(device_->device(), static_cast<uint32_t>(writeDescriptorSets.size()),
                               writeDescriptorSets.data(), 0, nullptr);
    }

    /** Prepare a separate pipeline for the UI overlay rendering decoupled from the main application */
    void UIOverlay::preparePipeline(VkPipelineCache pipelineCache, VkRenderPass renderPass,
                                    VkFormat colorFormat, VkFormat depthFormat, VkSampleCountFlagBits samples) {
        // Pipeline layout
        // Push constants for UI rendering parameters
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushConstBlock);

        auto pipelineLayoutCreateInfo = vkinit::pipeline_layout_create_info(&descriptorSetLayout_, 1);
        pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
        VK_CHECK(vkCreatePipelineLayout(device_->device(), &pipelineLayoutCreateInfo, nullptr, &pipelineLayout_));

        // Setup graphics pipeline for UI rendering
        auto inputAssemblyState = vkinit::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        auto rasterizationState = vkinit::rasterization_state_create_info(VK_POLYGON_MODE_FILL);

        // Enable blending
        auto blendAttachmentState = vkinit::color_blend_attachment_state();
        blendAttachmentState.blendEnable = VK_TRUE;
        blendAttachmentState.colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                VK_COLOR_COMPONENT_A_BIT;
        blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
        blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo colorBlendState{};
        colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendState.attachmentCount = 1;
        colorBlendState.pAttachments = &blendAttachmentState;

        auto depthStencilState = vkinit::depth_stencil_create_info(false, false, VK_COMPARE_OP_ALWAYS);

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;
        viewportState.flags = 0;

        auto multisampleState = vkinit::multisampling_state_create_info(samples);

        std::vector<VkDynamicState> dynamicStateEnables = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.pDynamicStates = dynamicStateEnables.data();
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
        dynamicState.flags = 0;

        VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.layout = pipelineLayout_;
        pipelineCreateInfo.renderPass = renderPass;
        pipelineCreateInfo.flags = 0;
        pipelineCreateInfo.basePipelineIndex = -1;
        pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

        pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
        pipelineCreateInfo.pRasterizationState = &rasterizationState;
        pipelineCreateInfo.pColorBlendState = &colorBlendState;
        pipelineCreateInfo.pMultisampleState = &multisampleState;
        pipelineCreateInfo.pViewportState = &viewportState;
        pipelineCreateInfo.pDepthStencilState = &depthStencilState;
        pipelineCreateInfo.pDynamicState = &dynamicState;
        pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaders_.size());
        pipelineCreateInfo.pStages = shaders_.data();
        pipelineCreateInfo.subpass = 0;

#if defined(VK_KHR_dynamic_rendering)
        // SRS - if we are using dynamic rendering (i.e. renderPass null), must define color, depth and stencil attachments at pipeline create time
        VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo = {};
        if (renderPass == VK_NULL_HANDLE) {
            pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
            pipelineRenderingCreateInfo.colorAttachmentCount = 1;
            pipelineRenderingCreateInfo.pColorAttachmentFormats = &colorFormat;
            pipelineRenderingCreateInfo.depthAttachmentFormat = depthFormat;
            pipelineRenderingCreateInfo.stencilAttachmentFormat = depthFormat;
            pipelineCreateInfo.pNext = &pipelineRenderingCreateInfo;
        }
#endif

        // Vertex bindings an attributes based on ImGui vertex definition
        std::vector<VkVertexInputBindingDescription> vertexInputBindings = {
                {0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX}
        };
        std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
                {0, 0, VK_FORMAT_R32G32_SFLOAT,  offsetof(ImDrawVert, pos)},    // Location 0: Position
                {1, 0, VK_FORMAT_R32G32_SFLOAT,  offsetof(ImDrawVert, uv)},    // Location 1: UV
                {2, 0, VK_FORMAT_R8G8B8A8_UNORM, offsetof(ImDrawVert, col)},    // Location 0: Color
        };
        VkPipelineVertexInputStateCreateInfo vertexInputState = vkinit::vertex_input_state_create_info();
        vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size());
        vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
        vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
        vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes.data();

        pipelineCreateInfo.pVertexInputState = &vertexInputState;

        VK_CHECK(vkCreateGraphicsPipelines(device_->device(), pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipeline_));
    }

    /** Update vertex and index buffer containing the imGui elements when required */
    bool UIOverlay::updateBuffers() {
        ImDrawData *imDrawData = ImGui::GetDrawData();
        bool updateCmdBuffers = false;

        if (!imDrawData) { return false; };

        // Note: Alignment is done inside buffer creation
        VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
        VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

        // Update buffers only if vertex or index count has been changed compared to current buffer size
        if ((vertexBufferSize == 0) || (indexBufferSize == 0)) {
            return false;
        }

        // Vertex buffer
        if ((vertexBuffer_.get() == VK_NULL_HANDLE) || (vertexCount_ != imDrawData->TotalVtxCount)) {
            if (vertexBuffer_) {
                device_->waitIdle();
                vertexBuffer_->destroy();
            }
            vertexBuffer_ = std::make_unique<LveBuffer>(*device_, sizeof(ImDrawVert), imDrawData->TotalVtxCount,
                                                        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            vertexCount_ = imDrawData->TotalVtxCount;
            vertexBuffer_->map();
            updateCmdBuffers = true;
//            LOGI("vertexCount: {}", vertexCount_);
        }

        // Index buffer
        if ((indexBuffer_.get() == VK_NULL_HANDLE) || (indexCount_ < imDrawData->TotalIdxCount)) {
            if (indexBuffer_) {
                device_->waitIdle();
                indexBuffer_->destroy();
            }
            indexBuffer_.reset();
            indexBuffer_ = std::make_unique<LveBuffer>(*device_, sizeof(ImDrawIdx), imDrawData->TotalIdxCount,
                                                       VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            indexCount_ = imDrawData->TotalIdxCount;
            indexBuffer_->map();
            updateCmdBuffers = true;
//            LOGI("indexCount: {}", indexCount_);
        }

        // Upload data
        ImDrawVert *vtxDst = (ImDrawVert *) vertexBuffer_->getMappedMemory();
        ImDrawIdx *idxDst = (ImDrawIdx *) indexBuffer_->getMappedMemory();

        for (int n = 0; n < imDrawData->CmdListsCount; n++) {
            const ImDrawList *cmd_list = imDrawData->CmdLists[n];
            memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
            memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
            vtxDst += cmd_list->VtxBuffer.Size;
            idxDst += cmd_list->IdxBuffer.Size;
        }

        // Flush to make writes visible to GPU
        vertexBuffer_->flush();
        indexBuffer_->flush();

        return updateCmdBuffers;
    }

    void UIOverlay::draw(const VkCommandBuffer commandBuffer) {
        if (dirty_) {
            updateBuffers();
        }
        const auto &extent = window_->getExtent();
        const float width = static_cast<float>(extent.width) * content_scale_factor_;
        const float height = static_cast<float>(extent.height) * content_scale_factor_;
        const VkViewport viewport{0, 0, width, height, 0.0f, 1.0f};
        const VkRect2D scissor{{0, 0},{static_cast<uint32_t>(width), static_cast<uint32_t>(height)}};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        ImDrawData *imDrawData = ImGui::GetDrawData();
        int32_t vertexOffset = 0;
        int32_t indexOffset = 0;

        if ((!imDrawData) || (imDrawData->CmdListsCount == 0)) {
            return;
        }

        ImGuiIO &io = ImGui::GetIO();

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, 1,
                                &descriptorSet_,
                                0, nullptr);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);

        pushConstBlock_.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
        pushConstBlock_.translate = glm::vec2(-1.0f);
        VkDeviceSize offsets[1] = {0};

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer_->getBuffer(), offsets);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer_->getBuffer(), 0, VK_INDEX_TYPE_UINT16);

        for (int32_t i = 0; i < imDrawData->CmdListsCount; i++) {
            const ImDrawList *cmd_list = imDrawData->CmdLists[i];
            for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++) {
                // fixme: weird!!! The overlay will not show if vkCmdPushConstants() is called outside the for loop
                vkCmdPushConstants(commandBuffer, pipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstBlock), &pushConstBlock_);

                const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[j];
                VkRect2D scissorRect;
                scissorRect.offset.x = std::max((int32_t) (pcmd->ClipRect.x), 0);
                scissorRect.offset.y = std::max((int32_t) (pcmd->ClipRect.y), 0);
                scissorRect.extent.width = (uint32_t) (pcmd->ClipRect.z - pcmd->ClipRect.x);
                scissorRect.extent.height = (uint32_t) (pcmd->ClipRect.w - pcmd->ClipRect.y);
                vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);
                vkCmdDrawIndexed(commandBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
                indexOffset += pcmd->ElemCount;
            }
            vertexOffset += cmd_list->VtxBuffer.Size;
        }
    }

    void UIOverlay::onResize(uint32_t width, uint32_t height) {
        ImGuiIO &io = ImGui::GetIO();
        io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));
    }

    void UIOverlay::freeResources() {
        for (auto &shader: shaders_)
            vkDestroyShaderModule(device_->device(), shader.module, nullptr);

        vertexBuffer_.reset();
        indexBuffer_.reset();
        vkDestroyImageView(device_->device(), fontView_, nullptr);
        vkDestroyImage(device_->device(), fontImage_, nullptr);
        vkFreeMemory(device_->device(), fontMemory_, nullptr);
        vkDestroySampler(device_->device(), sampler_, nullptr);
        vkDestroyDescriptorSetLayout(device_->device(), descriptorSetLayout_, nullptr);
        vkDestroyDescriptorPool(device_->device(), descriptorPool_, nullptr);
        vkDestroyPipelineLayout(device_->device(), pipelineLayout_, nullptr);
        vkDestroyPipeline(device_->device(), pipeline_, nullptr);
    }

    bool UIOverlay::header(const char *caption) {
        return ImGui::CollapsingHeader(caption, ImGuiTreeNodeFlags_DefaultOpen);
    }

    bool UIOverlay::checkBox(const char *caption, bool *value) {
        bool res = ImGui::Checkbox(caption, value);
        if (res) { dirty_ = true; };
        return res;
    }

    bool UIOverlay::checkBox(const char *caption, int32_t *value) {
        bool val = (*value == 1);
        bool res = ImGui::Checkbox(caption, &val);
        *value = val;
        if (res) { dirty_ = true; };
        return res;
    }

    bool UIOverlay::radioButton(const char *caption, bool value) {
        bool res = ImGui::RadioButton(caption, value);
        if (res) { dirty_ = true; };
        return res;
    }

    bool UIOverlay::inputFloat(const char *caption, float *value, float step) {
        bool res = ImGui::InputFloat(caption, value, step, step * 10.0f);
        if (res) { dirty_ = true; };
        return res;
    }

    bool UIOverlay::sliderFloat(const char *caption, float *value, float min, float max) {
        bool res = ImGui::SliderFloat(caption, value, min, max);
        if (res) { dirty_ = true; };
        return res;
    }

    bool UIOverlay::sliderInt(const char *caption, int32_t *value, int32_t min, int32_t max) {
        bool res = ImGui::SliderInt(caption, value, min, max);
        if (res) { dirty_ = true; };
        return res;
    }

    bool UIOverlay::comboBox(const char *caption, int32_t *itemindex, std::vector<std::string> items) {
        if (items.empty()) {
            return false;
        }
        std::vector<const char *> charitems;
        charitems.reserve(items.size());
        for (size_t i = 0; i < items.size(); i++) {
            charitems.push_back(items[i].c_str());
        }
        uint32_t itemCount = static_cast<uint32_t>(charitems.size());
        bool res = ImGui::Combo(caption, itemindex, &charitems[0], itemCount, itemCount);
        if (res) { dirty_ = true; };
        return res;
    }

    bool UIOverlay::button(const char *caption) {
        bool res = ImGui::Button(caption);
        if (res) { dirty_ = true; };
        return res;
    }

    void UIOverlay::text(const char *formatstr, ...) {
        va_list args;
        va_start(args, formatstr);
        ImGui::TextV(formatstr, args);
        va_end(args);
    }

    void UIOverlay::showOptionsWindow(uint32_t last_fps, std::function<void()> body) {
        ImGui::NewFrame();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5);
        ImGui::SetNextWindowPos(ImVec2(10 * content_scale_factor_, 10 * content_scale_factor_));
        ImGui::SetNextWindowSize(ImVec2(0, 0));
        ImGui::Begin("Application Info & Options", nullptr,
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImGui::TextUnformatted(device_->properties.deviceName);
        ImGui::Text("%.2f ms/frame (%.1d fps)", (1000.0f / last_fps), last_fps);
        ImGui::PushItemWidth(110.0f * content_scale_factor_);

        body();

        ImGui::PopItemWidth();
        ImGui::End();
        ImGui::PopStyleVar();
        ImGui::Render();
    }

    bool UIOverlay::onKeyEvent(const KeyEvent &event) {
        auto &io = ImGui::GetIO();
        if (event.get_action() == KeyAction::Down)
            io.KeysDown[static_cast<int>(event.get_code())] = true;
        else if (event.get_action() == KeyAction::Up)
            io.KeysDown[static_cast<int>(event.get_code())] = false;
        return false;
    }

    bool UIOverlay::onMouseEvent(const MouseEvent& event) {
        auto &io = ImGui::GetIO();
        auto capture_move_event = false;
        io.MousePos = ImVec2{event.get_pos_x() * content_scale_factor_, event.get_pos_y() * content_scale_factor_};
        auto button_id = static_cast<int>(event.get_button());
        if (event.get_action() == MouseAction::Down)
            io.MouseDown[button_id] = true;
        else if (event.get_action() == MouseAction::Up)
            io.MouseDown[button_id] = false;
        else if (event.get_action() == MouseAction::Move)
            capture_move_event = io.WantCaptureMouse;
        return capture_move_event;
    }

    bool UIOverlay::capturesMouse() const {
        ImGuiIO &io = ImGui::GetIO();
        return io.WantCaptureMouse && visible_;
    }

    bool UIOverlay::capturesKeyboard() const {
        ImGuiIO &io = ImGui::GetIO();
        return io.WantCaptureKeyboard && visible_;
    }

}
