#pragma once

#include "drawable.h"


namespace lve {

    class PointsDrawable : public Drawable {
    public:
        PointsDrawable(LveDevice &device, DescriptorAllocator &allocator, VkRenderPass pass);
        ~PointsDrawable() override;

    protected:
        void preparePipelines() override;

    private:
        struct Pipelines { ;
            std::unique_ptr<LvePipeline> plain;
            std::unique_ptr<LvePipeline> spheres; // render points as spheres
        } pipelines;
    };

}
