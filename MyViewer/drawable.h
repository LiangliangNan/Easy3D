#pragma once

#include <vector>
#include <string>
#include <memory>

#include <glm/glm.hpp>
#include <vk_common.h>

#define INTERLEAVED_VERTEX_ATTRIBUTES 1

namespace lve {

// todo: not needed in case of separate vertex attributes
    struct MyVertex {
        MyVertex(const glm::vec3 &p, const glm::vec3 &c) : pos(p), color(c) {}

        glm::vec3 pos;
        glm::vec3 color;
    };

    class LveDevice;
    class LveBuffer;
    class LvePipeline;
    class DescriptorAllocator;

    /// todo: Rename it to Renderer? A drawable has a pipeline, and it can actually render ALL drawables of the same type.
    class Drawable {
    public:
        Drawable(LveDevice &device, DescriptorAllocator& allocator, VkRenderPass pass);
        virtual ~Drawable();

        virtual void draw(VkCommandBuffer cmdbuffer);

        /// todo: to handle uniforms related to only this drawable.
        /// todo: common uniforms (like model/view/project matrices) should be handled separately
        void updateUniformBuffer(const glm::mat4& perspective, const glm::mat4& view);

        /// todo: use separated buffers (instead of interleaved).
        virtual void update_buffers(const std::vector<MyVertex> &vertices, const std::vector<uint32_t> &indices);

        void set_name(const std::string &name) { name_ = name; }
        const std::string &name() const { return name_; }
        void set_visible(bool b) { visible_ = b; }
        bool is_visible() const { return visible_; }

    protected:
        void prepareUniformBuffer();
        virtual void preparePipelines() = 0;

    protected:
        struct UBO {
            glm::mat4 model;
            glm::mat4 view;
            glm::mat4 projection;
        } ubo;

        struct Buffers {
#if INTERLEAVED_VERTEX_ATTRIBUTES
            std::unique_ptr<LveBuffer> vertex; // todo: allow different drawables share the same vertex buffer
#else
            vks::Buffer vertex;
            vks::Buffer color;
            vks::Buffer normal;
#endif
            uint32_t vertexCount;
            std::unique_ptr<LveBuffer> index; // todo: allow different drawables share the same vertex buffer
            uint32_t indexCount;
        } buffers;

        std::unique_ptr<LveBuffer> uniformBuffer; // todo: allow different drawables share the same vertex buffer

        /// todo: many of the following can be shared with others
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        LvePipeline* active_pipeline = nullptr;

    protected:
        LveDevice &lveDevice;
        DescriptorAllocator& descriptorAllocator;
        VkRenderPass renderPass;
        VkDescriptorSet descriptorSet;

        std::string name_;
        bool visible_;
    };

}