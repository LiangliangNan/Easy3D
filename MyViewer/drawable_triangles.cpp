#include "drawable_triangles.h"
#include <lve_pipeline.hpp>
#include <lve_buffer.hpp>
#include <vk_shaders.h>


namespace lve {

    TrianglesDrawable::TrianglesDrawable(LveDevice &device, DescriptorAllocator& allocator, VkRenderPass pass)
            : Drawable(device, allocator, pass) {
        preparePipelines();
    }

    TrianglesDrawable::~TrianglesDrawable() {
        pipelines.solid.reset();
        pipelines.wireframe.reset();
    }


#define USE_SHADER_REFLECTION 1

    void TrianglesDrawable::preparePipelines() {
#if USE_SHADER_REFLECTION

        // todo: make ShaderManager life cycle the same as the program
        ShaderManager manager(lveDevice.device());
        auto vertexShader = manager.getShader(std::string(ASSET_DIR) + "shaders/points.vert");
        auto fragmentShader = manager.getShader(std::string(ASSET_DIR) + "shaders/points.frag");

        ShaderReflect reflect;
        reflect.addStage(vertexShader, VK_SHADER_STAGE_VERTEX_BIT);
        reflect.addStage(fragmentShader, VK_SHADER_STAGE_FRAGMENT_BIT);
        reflect.reflect(lveDevice.device());

        // setup descriptors
        const auto& reflectedBindings = reflect.getReflectedBindings();

        VkDescriptorSetLayout descriptorSetLayout;
        // todo: to be simplified
        auto bufferInfo = uniformBuffer->descriptorInfo();
        auto builder = descriptorAllocator.begin();
        for (auto& bin : reflectedBindings)
            builder.bind(bin.second.binding, bin.second.type, bin.second.stageFlags);
        builder.write(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &bufferInfo);
        builder.build(descriptorSet, descriptorSetLayout);

        const auto& constantRanges = reflect.getConstantRanges();
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(constantRanges.size());
        pipelineLayoutInfo.pPushConstantRanges = constantRanges.data();
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
        VK_CHECK(vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout));

        PipelineConfig pipelineConfig;
        // todo: only difference -> use pipeline variants
        pipelineConfig.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        pipelineConfig.bindingDescriptions = { reflect.getVertexInputBindingDescription() };
        pipelineConfig.attributeDescriptions = reflect.getVertexInputAttributeDescriptions();
        pipelines.solid = std::make_unique<LvePipeline>(
                lveDevice,
                vertexShader->module,
                fragmentShader->module,
                pipelineConfig);

        active_pipeline = pipelines.solid.get();
#else
        // setup attributes
        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
#if INTERLEAVED_VERTEX_ATTRIBUTES
        // Interleaved vertex attributes
        // One Binding (one buffer) and multiple attributes
        // Binding descriptions (todo: can be shared across drawables)
        bindingDescriptions = { {0, sizeof(MyVertex), VK_VERTEX_INPUT_RATE_VERTEX} };
        // Attribute descriptions
        // Describes memory layout and shader positions
        attributeDescriptions = {
                {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(MyVertex, pos) /*0*/                  }, // Location 0 : Position
                {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(MyVertex, color) /*sizeof(float) * 3*/}  // Location 1 : Color
        };
#else
        // Separate vertex attribute
    // Multiple bindings (for each attribute buffer) and multiple attributes
    vertices.bindingDescriptions = {
            {0, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX},    // Location 0 : Position
            {1, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX},    // Location 1 : Color
            {2, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX},    // Location 2 : Normal
//      { 3, sizeof(glm::vec2), VK_VERTEX_INPUT_RATE_VERTEX },    // Location 2 : UV
    };
    vertices.attributeDescriptions = {
            {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
            {1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0},
            {2, 2, VK_FORMAT_R32G32B32_SFLOAT, 0},    // color
//		{ 3, 3, VK_FORMAT_R32G32_SFLOAT,    0 },    // uv
    };
#endif

        // setup descriptors
        VkDescriptorSetLayout descriptorSetLayout;
        auto bufferInfo = uniformBuffer->descriptorInfo();
        descriptorAllocator.begin()
                .bind(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                .write_buffer(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &bufferInfo)
                .build(descriptorSet, descriptorSetLayout);
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        VK_CHECK(vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout));

        PipelineConfig pipelineConfig;
        // todo: only difference -> use pipeline variants
        pipelineConfig.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        pipelineConfig.bindingDescriptions = bindingDescriptions;
        pipelineConfig.attributeDescriptions = attributeDescriptions;
        pipelines.solid = std::make_unique<LvePipeline>(
                lveDevice,
                // todo: shaderStages should be cached and reused.
                std::string(ASSET_DIR) + "shaders/points.vert.spv",
                std::string(ASSET_DIR) + "shaders/points.frag.spv",
                pipelineConfig);

        active_pipeline = pipelines.solid.get();
#endif
    }

}