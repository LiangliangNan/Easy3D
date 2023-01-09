#pragma once

#include "lve_device.hpp"

// std
#include <string>
#include <vector>

namespace lve {

    class PipelineConfig {
    public:
        PipelineConfig();
        PipelineConfig(const PipelineConfig&) = delete;
        PipelineConfig&operator=(const PipelineConfig&) = delete;

        void enableAlphaBlending();

        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        VkPipelineViewportStateCreateInfo viewportInfo{};
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
        VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
        VkPipelineMultisampleStateCreateInfo multisampleInfo{};
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };

    class ShaderManager;
    class LvePipeline {
    public:
        LvePipeline(
                LveDevice &device,
                VkShaderModule vertShaderModule,
                VkShaderModule fragShaderModule,
                const PipelineConfig &config);

        /// This constructor is deprecated.
        /// Please use the above one in which the shader modules are not cached and can be reused.
        LvePipeline(
                LveDevice &device,
                const std::string &vertFilepath,
                const std::string &fragFilepath,
                const PipelineConfig &config);

        ~LvePipeline();

        LvePipeline(const LvePipeline &) = delete;
        LvePipeline &operator=(const LvePipeline &) = delete;

        void bind(VkCommandBuffer commandBuffer);

    private:
        static std::vector<char> readFile(const std::string &filepath);

        void createGraphicsPipeline(
                VkShaderModule vertShaderModule,
                VkShaderModule fragShaderModule,
                const PipelineConfig & config
        );

        void createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule);

        LveDevice &lveDevice;
        VkPipeline graphicsPipeline{};
        VkShaderModule vertShaderModule_{};
        VkShaderModule fragShaderModule_{};
    };
}  // namespace lve
