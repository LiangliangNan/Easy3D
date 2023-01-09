#include "drawable_lines.h"
#include <lve_pipeline.hpp>


namespace lve {

    LinesDrawable::LinesDrawable(LveDevice &device, DescriptorAllocator& allocator, VkRenderPass pass)
            : Drawable(device, allocator, pass) {
        preparePipelines();
    }

    LinesDrawable::~LinesDrawable() {
        pipelines.plain.reset();
        pipelines.cylinders.reset();
        pipelines.cones.reset();
    }

    void LinesDrawable::preparePipelines() {
//        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
//
//        PipelineConfig pipelineConfig;
//        pipelineConfig.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST; // todo: only difference -> use pipeline variants
//        pipelineConfig.renderPass = renderPass;
//        pipelineConfig.pipelineLayout = pipelineLayout;
//        pipelineConfig.bindingDescriptions = vertices.bindingDescriptions;
//        pipelineConfig.attributeDescriptions = vertices.attributeDescriptions;
//        pipelines.plain = std::make_unique<LvePipeline>(
//                lveDevice,
//                // todo: shaderStages should be cached and reused.
//                std::string(ASSET_DIR) + "shaders/points.vert.spv",
//                std::string(ASSET_DIR) + "shaders/points.frag.spv",
//                pipelineConfig);
//
//        active_pipeline = pipelines.plain.get();
    }

}