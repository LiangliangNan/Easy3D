#pragma once

#include "drawable.h"


namespace lve {

    class LinesDrawable : public Drawable {
    public:
        LinesDrawable(LveDevice &device, DescriptorAllocator &allocator, VkRenderPass pass);
        ~LinesDrawable() override;

    protected:
        void preparePipelines() override;

    private:
        struct Pipelines {
            std::unique_ptr<LvePipeline> plain;
            std::unique_ptr<LvePipeline> cylinders; // render lines as cylinders
            std::unique_ptr<LvePipeline> cones; // render lines as cones
        } pipelines;
    };

}