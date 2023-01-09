// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vk_common.h>


namespace lve {

    namespace vkinit {

        VkCommandPoolCreateInfo command_pool_create_info(uint32_t queueFamilyIndex, VkCommandPoolResetFlags flags = 0);

        VkCommandBufferAllocateInfo command_buffer_allocate_info(VkCommandPool pool, uint32_t count = 1,
                                                                 VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        VkCommandBufferBeginInfo command_buffer_begin_info(VkCommandBufferUsageFlags flags = 0);

        VkFramebufferCreateInfo framebuffer_create_info(VkRenderPass renderPass, VkExtent2D extent);

        VkFenceCreateInfo fence_create_info(VkFenceCreateFlags flags = 0);

        VkSemaphoreCreateInfo semaphore_create_info(VkSemaphoreCreateFlags flags = 0);

        VkSubmitInfo submit_info(VkCommandBuffer *cmd);

        VkPresentInfoKHR present_info();

        VkRenderPassBeginInfo
        renderpass_begin_info(VkRenderPass renderPass, VkExtent2D windowExtent, VkFramebuffer framebuffer);

        VkPipelineShaderStageCreateInfo
        pipeline_shader_stage_create_info(VkShaderStageFlagBits stage, VkShaderModule shaderModule);

        VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info();

        VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info(VkPrimitiveTopology topology);

        VkPipelineRasterizationStateCreateInfo rasterization_state_create_info(VkPolygonMode polygonMode,
                                                                               VkCullModeFlags cullMode = VK_CULL_MODE_NONE,
                                                                               VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE);

        //multisampling defaulted to no multisampling (1 sample per pixel)
        VkPipelineMultisampleStateCreateInfo multisampling_state_create_info(VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);

        VkPipelineColorBlendAttachmentState color_blend_attachment_state();

        VkPipelineLayoutCreateInfo pipeline_layout_create_info(const VkDescriptorSetLayout* pSetLayouts,
                                                               uint32_t setLayoutCount = 1);

        VkImageCreateInfo image_create_info(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);

        VkImageViewCreateInfo imageview_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);

        VkPipelineDepthStencilStateCreateInfo
        depth_stencil_create_info(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp);

        VkDescriptorSetLayoutBinding
        descriptorset_layout_binding(VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t binding);

        VkWriteDescriptorSet
        write_descriptor_buffer(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorBufferInfo *bufferInfo,
                                uint32_t binding);

        VkWriteDescriptorSet
        write_descriptor_image(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorImageInfo *imageInfo,
                               uint32_t binding);

        VkSamplerCreateInfo
        sampler_create_info(VkFilter filters, VkSamplerAddressMode samplerAdressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);

        VkBufferMemoryBarrier buffer_barrier(VkBuffer buffer, uint32_t queue);

        VkImageMemoryBarrier
        image_barrier(VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout,
                      VkImageLayout newLayout, VkImageAspectFlags aspectMask);
    }

}

