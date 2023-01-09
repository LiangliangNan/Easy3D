#pragma once

#include "drawable.h"


namespace lve {

    class TrianglesDrawable : public Drawable {
    public:
        TrianglesDrawable(LveDevice &device, DescriptorAllocator &allocator, VkRenderPass pass);
        ~TrianglesDrawable() override;

    protected:
        void preparePipelines() override;

    private:
        struct Pipelines {
            std::unique_ptr<LvePipeline> solid;
            std::unique_ptr<LvePipeline> wireframe;
        } pipelines;
    };

}
