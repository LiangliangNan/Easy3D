// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vk_common.h>

#include <vector>
#include <array>
#include <string>
#include <unordered_map>

#include <vk_descriptors.h>


namespace lve {

    // todo: delete it
    namespace vkutil {
        VkPipelineShaderStageCreateInfo load_shader(VkDevice device, const std::string& file_path, VkShaderStageFlagBits stage);
    }

    struct ShaderModule {
        std::vector<uint32_t> code;
        VkShaderModule module = VK_NULL_HANDLE;
    };

    //holds all information for a given shader set for pipeline
    class ShaderReflect {
    public:
        ShaderReflect() = default;

        /// Input shaderModule should be created by
        void addStage(ShaderModule *shaderModule, VkShaderStageFlagBits stage);

        void reflect(VkDevice device);

        // vertex input. The results can be used to create VkPipelineVertexInputStateCreateInfo
        //      VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        //      vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        //      vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        //      vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        //      vertexInputInfo.vertexBindingDescriptionCount = 1;
        //      vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        // A real application would probably derive this information from its mesh format(s); a similar mechanism
        // could be used to ensure mesh/shader compatibility.
        VkVertexInputBindingDescription getVertexInputBindingDescription() const { return bindingDescription; }
        const std::vector<VkVertexInputAttributeDescription>& getVertexInputAttributeDescriptions() const {
            return attributeDescriptions;
        }

        // reflected bindings
        struct ReflectedBinding {
            uint32_t set;
            uint32_t binding;
            VkDescriptorType type;
            VkShaderStageFlags stageFlags;
        };
        // The results are the bindings from all stages.
        // They can be used to create descriptor set layouts (you can also use getDescriptorSetLayoutCreateInfos())
        const std::unordered_map<std::string, ReflectedBinding>& getReflectedBindings() {
            return reflectedBindings;
        };

        // The results can be used to create descriptor set layouts (you can also use getReflectedBindings())
        //      VkDescriptorSetLayout layout;
        //      VK_CHECK(vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &layout));
        const std::vector<VkDescriptorSetLayoutCreateInfo>& getDescriptorSetLayoutCreateInfos() const {
            return descriptorSetLayoutCreateInfos;
        }

        // The results can be used to fill VkPipelineLayoutCreateInfo (to create pipeline layout):
        //        VkPipelineLayoutCreateInfo pipelineLayoutInfo = vkinit::pipeline_layout_create_info(nullptr, 0);
        //        pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(constant_ranges.size());
        //        pipelineLayoutInfo.pPushConstantRanges = constant_ranges.data();
        //        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
        //        pipelineLayoutInfo.pSetLayouts = setLayouts.data();
        //        vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout);
        // The descriptor set layouts for VkPipelineLayoutCreateInfo can be created using the results from
        // getDescriptorSetLayoutCreateInfos().
        const std::vector<VkPushConstantRange>& getConstantRanges() const { return constantRanges; }

        // Get all the shader stages. The results can be used to fill VkGraphicsPipelineCreateInfo (to be used for
        // vkCreateGraphicsPipelines)
        const std::vector<VkPipelineShaderStageCreateInfo>& getShaderStages() const { return shaderStages; }

    private:
        void clear();

    private:
        // vertex input
        VkVertexInputBindingDescription bindingDescription {};
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

        // bindings from all stages.
        std::unordered_map<std::string, ReflectedBinding> reflectedBindings;

        // can be used to create descriptor set layouts
        std::vector<VkDescriptorSetLayoutCreateInfo> descriptorSetLayoutCreateInfos;

        // can be used to fill VkPipelineLayoutCreateInfo (to create pipeline layout).
        std::vector<VkPushConstantRange> constantRanges;

        // all the shader stages (can be used to fill VkGraphicsPipelineCreateInfo)
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

    private:
        struct ShaderStage {
            ShaderModule *shaderModule;
            VkShaderStageFlagBits stage;
        };
        std::vector<ShaderStage> stages;
    };


    class ShaderManager {
    public:
        ShaderManager(VkDevice device) : _device{device} {}
        ~ShaderManager();

        /// Requests a shader module from a file (can be either GLSL or SPIR-V file). 
        /// Returns nullptr it errors.
        /// note: This function immediately returns the shader module if it already exists in the cache. Otherwise, it will load the
        ///     shader module from the file and store it in the cache (so it can be reused in the future).
        ShaderModule *getShader(const std::string &file_name);

        /// Loads a shader module from shader module from a file (can be either GLSL or SPIR-V file). 
        /// Returns empty shader code and VK_NULL_HANDLE if it errors.
        /// note: the shader module loaded by this function is not cached. Client code should use getShader() that can cache and 
        /// reuse shader modules.
        static ShaderModule loadShaderModule(VkDevice device, const std::string& file_name);

    private:
        VkDevice _device;
        std::unordered_map<std::string, ShaderModule> module_cache;
    };

}