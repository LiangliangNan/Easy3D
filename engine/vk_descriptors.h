// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vk_common.h>

#include <vector>
#include <memory>
#include <unordered_map>


namespace lve {

    /**
     * @brief Allocates and writes descriptor sets (and their descriptor set layouts, if needed) automatically.
     * @details It internally manages descriptor pools (creates new descriptor pools once they get filled) and
     *      caches descriptor set layouts to avoid creating duplicated layouts.
     *
     *      This implementation is taken from the link below (by significantly reducing the APIs):
     *          - https://vkguide.dev/docs/extra-chapter/abstracting_descriptors/
     *
     * @note This implementation is not thread safe. A thread safe implementation can be found at
     *          - https://github.com/vblanco20-1/Vulkan-Descriptor-Allocator
     */


    class DescriptorAllocator {
    public:
        explicit DescriptorAllocator(VkDevice device);
        ~DescriptorAllocator();

        /**
         * Use DescriptorBuilder to create VkDescriptorSet and (optionally) VkDescriptorSetLayout.
         * The optional VkDescriptorSetLayout will be cached by the allocator.
         * Usage:
         *      VkDescriptorSet GlobalSet;
         *      descriptorAllocator.begin()
         *          .bind_buffer(0, &dynamicInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT )
         *        	.bind_buffer(1, &dynamicInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT)
         *        	.build(GlobalSet);
         *      VkDescriptorSet ObjectDataSet;
         *      descriptorAllocator.begin()
         *        	.bind_buffer(0, &objectBufferInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
         *       	.build(ObjectDataSet);
         *      VkDescriptorSet ImageSet;
         *      descriptorAllocator.begin()
         *        	.bind_image(0, &imageBufferInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
         *        	.build(ImageSet);
         */
        class DescriptorBuilder {
        public:
            DescriptorBuilder(DescriptorAllocator* alloc) : allocator(alloc) {}
            DescriptorBuilder &bind(uint32_t binding, VkDescriptorType type, VkShaderStageFlags flags);
            DescriptorBuilder &write(uint32_t binding, VkDescriptorType type, VkDescriptorBufferInfo *info);
            DescriptorBuilder &write(uint32_t binding, VkDescriptorType type, VkDescriptorImageInfo *info);
            bool build(VkDescriptorSet &set, VkDescriptorSetLayout &layout);
            bool build(VkDescriptorSet &set);

        private:
            DescriptorAllocator* allocator;
            std::vector<VkWriteDescriptorSet> writes;
            std::vector<VkDescriptorSetLayoutBinding> bindings;
        };

        DescriptorBuilder begin();

        /// Other usage: create descriptor set layout from VkDescriptorSetLayoutCreateInfo. In this case, the input
        /// create info is hashed, and the created descriptor set layout is cached and can be reused.
    private:
        bool allocate(VkDescriptorSet *set, VkDescriptorSetLayout layout);
        void reset_pools();
        VkDescriptorPool grab_pool(int count, VkDescriptorPoolCreateFlags flags = 0);

    private:
        VkDevice device;
        VkDescriptorPool currentPool{VK_NULL_HANDLE};
        std::vector<VkDescriptorPool> usedPools;
        std::vector<VkDescriptorPool> freePools;

        std::unique_ptr<class DescriptorLayoutCache> cache{nullptr};
    };

}

