#pragma once

#include "lve_device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace lve {


    ///todo: https://vkguide.dev/docs/extra-chapter/abstracting_descriptors/
    //
    //Creating and managing descriptor sets is one of the most painful things about vulkan. Creating an abstraction that simplifies is is really important, and will improve the workflow a lot.
    //I’m going to show you a simple way to create a thin abstraction over descriptor sets, that makes it easier to handle them.
    //The end result will look like this.
    //VkDescriptorSet GlobalSet;
    //vkutil::DescriptorBuilder::begin(_descriptorLayoutCache, _descriptorAllocator)
    //	.bind_buffer(0, &dynamicInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT )
    //	.bind_buffer(1, &dynamicInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT)
    //	.build(GlobalSet);
    //
    //VkDescriptorSet ObjectDataSet;
    //vkutil::DescriptorBuilder::begin(_descriptorLayoutCache, _descriptorAllocator)
    //	.bind_buffer(0, &objectBufferInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
    //	.build(ObjectDataSet);
    //
    //VkDescriptorSet ImageSet;
    //vkutil::DescriptorBuilder::begin(_descriptorLayoutCache, _descriptorAllocator)
    //		.bind_image(0, &imageBufferInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
    //		.build(ImageSet);
    //
    //The implementation of this abstraction will be completely standalone, and you can find its code on the Engine branch of the example code.
    //https://github.com/vblanco20-1/vulkan-guide/blob/engine/extra-engine/vk_descriptors.h
    //https://github.com/vblanco20-1/vulkan-guide/blob/engine/extra-engine/vk_descriptors.cpp

    // see also
    //      - https://github.com/asc-community/VulkanAbstractionLayer/blob/master/VulkanAbstractionLayer/DescriptorCache.h
    //      - https://github.com/asc-community/VulkanAbstractionLayer/blob/master/VulkanAbstractionLayer/DescriptorBinding.h

    class LveDescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(LveDevice &lveDevice) : lveDevice{lveDevice} {}

            Builder &addBinding(
                    uint32_t binding,
                    VkDescriptorType descriptorType,
                    VkShaderStageFlags stageFlags,
                    uint32_t count = 1);

            std::unique_ptr<LveDescriptorSetLayout> build() const;

        private:
            LveDevice &lveDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        LveDescriptorSetLayout(
                LveDevice &lveDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);

        ~LveDescriptorSetLayout();

        LveDescriptorSetLayout(const LveDescriptorSetLayout &) = delete;

        LveDescriptorSetLayout &operator=(const LveDescriptorSetLayout &) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
        LveDevice &lveDevice;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class LveDescriptorWriter;
    };

    class LveDescriptorPool {
    public:
        class Builder {
        public:
            Builder(LveDevice &lveDevice) : lveDevice{lveDevice} {}

            Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);

            Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);

            Builder &setMaxSets(uint32_t count);

            std::unique_ptr<LveDescriptorPool> build() const;

        private:
            LveDevice &lveDevice;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        LveDescriptorPool(
                LveDevice &lveDevice,
                uint32_t maxSets,
                VkDescriptorPoolCreateFlags poolFlags,
                const std::vector<VkDescriptorPoolSize> &poolSizes);

        ~LveDescriptorPool();

        LveDescriptorPool(const LveDescriptorPool &) = delete;
        LveDescriptorPool &operator=(const LveDescriptorPool &) = delete;

        void allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;
        void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;

        void resetPool();

    private:
        LveDevice &lveDevice;
        VkDescriptorPool descriptorPool;

        friend class LveDescriptorWriter;
    };

    class LveDescriptorWriter {
    public:
        LveDescriptorWriter(LveDescriptorSetLayout &setLayout, LveDescriptorPool &pool);

        LveDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
        LveDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

        VkDescriptorSet build();
        void overwrite(VkDescriptorSet &set);

    private:
        LveDescriptorSetLayout &setLayout;
        LveDescriptorPool &pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

}  // namespace lve
