#include <vk_descriptors.h>

#include <algorithm>

#include <logging.h>


namespace lve {

    struct PoolSizes {
        std::vector<std::pair<VkDescriptorType, float> > sizes = {
                {VK_DESCRIPTOR_TYPE_SAMPLER,                0.5f},
                {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.f},
                {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          4.f},
                {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1.f},
                {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1.f},
                {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1.f},
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         2.f},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         2.f},
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.f},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.f},
                {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       0.5f}
        };
    };


    DescriptorAllocator::DescriptorAllocator(VkDevice newDevice) : device(newDevice) {
        cache = std::make_unique<DescriptorLayoutCache>(device);
    }

    DescriptorAllocator::~DescriptorAllocator() {
        cache.reset();

        //delete every pool held
        for (auto p: freePools)
            vkDestroyDescriptorPool(device, p, nullptr);
        for (auto p: usedPools)
            vkDestroyDescriptorPool(device, p, nullptr);
    }

    void DescriptorAllocator::reset_pools() {
        for (auto p: usedPools)
            vkResetDescriptorPool(device, p, 0);

        freePools = usedPools;
        usedPools.clear();
        currentPool = VK_NULL_HANDLE;
    }

    bool DescriptorAllocator::allocate(VkDescriptorSet *set, VkDescriptorSetLayout layout) {
        if (currentPool == VK_NULL_HANDLE) {
            currentPool = grab_pool(1000, 0);
            usedPools.push_back(currentPool);
        }

        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.pNext = nullptr;

        allocInfo.pSetLayouts = &layout;
        allocInfo.descriptorPool = currentPool;
        allocInfo.descriptorSetCount = 1;

        VkResult allocResult = vkAllocateDescriptorSets(device, &allocInfo, set);
        switch (allocResult) {
            case VK_SUCCESS: //all good, return
                return true;
            case VK_ERROR_FRAGMENTED_POOL:
            case VK_ERROR_OUT_OF_POOL_MEMORY:
                //allocate a new pool and retry
                currentPool = grab_pool(1000, 0);
                usedPools.push_back(currentPool);
                allocResult = vkAllocateDescriptorSets(device, &allocInfo, set);
                //if it still fails then we have big issues
                if (allocResult == VK_SUCCESS)
                    return true;
                else
                    LOGE("Failed to allocate descriptor set (even with a new descriptor pool)");
                break;
            default:
                //unrecoverable error
                LOGE("Unrecognized error in calling vkAllocateDescriptorSets()");
                return false;
        }

        return false;
    }

    VkDescriptorPool DescriptorAllocator::grab_pool(int count, VkDescriptorPoolCreateFlags flags) {
        if (freePools.size() > 0) {
            VkDescriptorPool pool = freePools.back();
            freePools.pop_back();
            return pool;
        } else {
            PoolSizes descriptorSizes;
            std::vector<VkDescriptorPoolSize> sizes(descriptorSizes.sizes.size());
            for (std::size_t idx = 0; idx < sizes.size(); ++idx) {
                const auto &sz = descriptorSizes.sizes[idx];
                sizes[idx] = {sz.first, uint32_t(sz.second * count)};
            }
            VkDescriptorPoolCreateInfo pool_info = {};
            pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            pool_info.flags = flags;
            pool_info.maxSets = count;
            pool_info.poolSizeCount = (uint32_t) sizes.size();
            pool_info.pPoolSizes = sizes.data();

            VkDescriptorPool descriptorPool;
            vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptorPool);

            return descriptorPool;
        }
    }


    class DescriptorLayoutCache {
    public:
        explicit DescriptorLayoutCache(VkDevice device);

        ~DescriptorLayoutCache();

        VkDescriptorSetLayout create_descriptor_layout(VkDescriptorSetLayoutCreateInfo *info);

        struct DescriptorLayoutInfo {
            //good idea to turn this into an inlined array
            std::vector<VkDescriptorSetLayoutBinding> bindings;

            bool operator==(const DescriptorLayoutInfo &other) const;

            size_t hash() const;
        };

    private:
        struct DescriptorLayoutHash {
            std::size_t operator()(const DescriptorLayoutInfo &k) const {
                return k.hash();
            }
        };

        std::unordered_map<DescriptorLayoutInfo, VkDescriptorSetLayout, DescriptorLayoutHash> layoutCache;
        VkDevice device;
    };

    DescriptorLayoutCache::DescriptorLayoutCache(VkDevice newDevice) : device(newDevice) {
    }

    DescriptorLayoutCache::~DescriptorLayoutCache() {
        //delete every descriptor layout held
        for (auto pair: layoutCache)
            vkDestroyDescriptorSetLayout(device, pair.second, nullptr);
    }


    VkDescriptorSetLayout DescriptorLayoutCache::create_descriptor_layout(VkDescriptorSetLayoutCreateInfo *info) {
        DescriptorLayoutInfo layout_info;
        layout_info.bindings.reserve(info->bindingCount);
        bool isSorted = true;
        int32_t lastBinding = -1;
        for (uint32_t i = 0; i < info->bindingCount; i++) {
            layout_info.bindings.push_back(info->pBindings[i]);

            //check that the bindings are in strict increasing order
            if (static_cast<int32_t>(info->pBindings[i].binding) > lastBinding) {
                lastBinding = info->pBindings[i].binding;
            } else {
                isSorted = false;
            }
        }
        if (!isSorted) {
            std::sort(layout_info.bindings.begin(), layout_info.bindings.end(),
                      [](VkDescriptorSetLayoutBinding &a, VkDescriptorSetLayoutBinding &b) {
                          return a.binding < b.binding;
                      });
        }

        auto it = layoutCache.find(layout_info);
        if (it != layoutCache.end()) {
            return (*it).second;
        } else {
            VkDescriptorSetLayout layout;
            VK_CHECK(vkCreateDescriptorSetLayout(device, info, nullptr, &layout));
            //add to cache
            layoutCache[layout_info] = layout;
            return layout;
        }
    }


    DescriptorAllocator::DescriptorBuilder DescriptorAllocator::begin() {
        return DescriptorBuilder(this);
    }


    DescriptorAllocator::DescriptorBuilder &DescriptorAllocator::DescriptorBuilder::bind(
            uint32_t binding, VkDescriptorType type, VkShaderStageFlags stageFlags
    ) {
        VkDescriptorSetLayoutBinding newBinding{};
        newBinding.descriptorCount = 1;
        newBinding.descriptorType = type;
        newBinding.pImmutableSamplers = nullptr;
        newBinding.stageFlags = stageFlags;
        newBinding.binding = binding;
        bindings.push_back(newBinding);

        return *this;
    }


    DescriptorAllocator::DescriptorBuilder &DescriptorAllocator::DescriptorBuilder::write(
            uint32_t binding, VkDescriptorType type, VkDescriptorImageInfo *info
    ) {
        VkWriteDescriptorSet newWrite{};
        newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        newWrite.pNext = nullptr;
        newWrite.descriptorCount = 1;
        newWrite.descriptorType = type;
        newWrite.pImageInfo = info;
        newWrite.dstBinding = binding;
        writes.push_back(newWrite);
        return *this;
    }


    DescriptorAllocator::DescriptorBuilder &DescriptorAllocator::DescriptorBuilder::write(
            uint32_t binding, VkDescriptorType type, VkDescriptorBufferInfo *info
    ) {
        VkWriteDescriptorSet newWrite{};
        newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        newWrite.pNext = nullptr;
        newWrite.descriptorCount = 1;
        newWrite.descriptorType = type;
        newWrite.pBufferInfo = info;
        newWrite.dstBinding = binding;
        writes.push_back(newWrite);
        return *this;
    }

bool DescriptorAllocator::DescriptorBuilder::build(VkDescriptorSet &set, VkDescriptorSetLayout &layout) {
        //build layout first
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.pNext = nullptr;

        layoutInfo.pBindings = bindings.data();
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layout = allocator->cache->create_descriptor_layout(&layoutInfo);

        //allocate descriptor
        if (!allocator->allocate(&set, layout))
            return false;

        //write descriptor
        for (VkWriteDescriptorSet &w: writes)
            w.dstSet = set;

        vkUpdateDescriptorSets(allocator->device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
        return true;
    }


    bool DescriptorAllocator::DescriptorBuilder::build(VkDescriptorSet &set) {
        VkDescriptorSetLayout layout;
        return build(set, layout);
    }


    bool DescriptorLayoutCache::DescriptorLayoutInfo::operator==(const DescriptorLayoutInfo &other) const {
        if (other.bindings.size() != bindings.size()) {
            return false;
        } else {
            //compare each of the bindings is the same. Bindings are sorted so they will match
            for (int i = 0; i < bindings.size(); i++) {
                if (other.bindings[i].binding != bindings[i].binding) {
                    return false;
                }
                if (other.bindings[i].descriptorType != bindings[i].descriptorType) {
                    return false;
                }
                if (other.bindings[i].descriptorCount != bindings[i].descriptorCount) {
                    return false;
                }
                if (other.bindings[i].stageFlags != bindings[i].stageFlags) {
                    return false;
                }
            }
            return true;
        }
    }

    std::size_t DescriptorLayoutCache::DescriptorLayoutInfo::hash() const {
        std::size_t result = std::hash<std::size_t>()(bindings.size());

        for (const VkDescriptorSetLayoutBinding &b: bindings) {
            //pack the binding data into a single int64. Not fully correct but it is ok
            std::size_t binding_hash = b.binding | b.descriptorType << 8 | b.descriptorCount << 16 | b.stageFlags << 24;

            //shuffle the packed binding data and xor it with the main hash
            result ^= std::hash<std::size_t>()(binding_hash);
        }

        return result;
    }

}