#include "drawable.h"

#include <array>

#include <lve_device.hpp>
#include <lve_buffer.hpp>
#include <vk_descriptors.h>
#include <lve_pipeline.hpp>


namespace lve {

    Drawable::Drawable(LveDevice &device, DescriptorAllocator& allocator, VkRenderPass pass)
            : lveDevice(device), descriptorAllocator(allocator), renderPass(pass), name_("untitled"), visible_(true) {
        prepareUniformBuffer();
    }

    Drawable::~Drawable() {
        // Clean up vulkan resources
        uniformBuffer.reset();
#if INTERLEAVED_VERTEX_ATTRIBUTES
        buffers.vertex.reset();
#else
        buffers.vertex.reset();
        buffers.color.reset();
        buffers.normal.reset();
#endif
        buffers.index.reset();
        vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
    }


    void Drawable::update_buffers(const std::vector<MyVertex> &vertices, const std::vector<uint32_t> &indices) {
#if INTERLEAVED_VERTEX_ATTRIBUTES
        // Vertex buffer
        buffers.vertex = std::make_unique<LveBuffer>(
                lveDevice,
                sizeof(MyVertex),
                static_cast<uint32_t>(vertices.size()),
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        buffers.vertex->map();
        buffers.vertex->writeToBuffer((void*)vertices.data());
#else
        // todo: remove the steps obtaining coordinates and colors
        std::vector<glm::vec3> points, colors;
        for (const auto& v : vertices) {
            points.push_back(v.pos);
            colors.push_back(v.color);
        }

        // Position buffer
        size_t positionBufferSize = points.size() * sizeof(glm::vec3);
        vulkanDevice->createBuffer(
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                &buffers.vertex,
                positionBufferSize,
                const_cast<glm::vec3*>(points.data()));

        // Color buffer
        size_t colorBufferSize = colors.size() * sizeof(glm::vec3);
        vulkanDevice->createBuffer(
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                &buffers.color,
                colorBufferSize,
                const_cast<glm::vec3*>(colors.data()));

        // Normal buffer
    //    size_t normalBufferSize = normals.size() * sizeof(glm::vec3);
    //    vulkanDevice->createBuffer(
    //            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    //            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
    //            &buffers.normal,
    //            normalBufferSize,
    //            const_cast<glm::vec3*>(normals.data()));
#endif

        // Index buffer
        if (!indices.empty()) {
            buffers.index = std::make_unique<LveBuffer>(
                    lveDevice,
                    sizeof(uint32_t),
                    indices.size(),
                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            buffers.index->map();
            buffers.index->writeToBuffer((void*)indices.data());
        }

        buffers.indexCount = static_cast<uint32_t>(indices.size());
        buffers.vertexCount = static_cast<uint32_t>(vertices.size());
    }

    void Drawable::draw(VkCommandBuffer cmdbuffer) {
        if (!visible_)
            return;

        vkCmdBindDescriptorSets(cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0,
                                nullptr);
        active_pipeline->bind(cmdbuffer);

#if INTERLEAVED_VERTEX_ATTRIBUTES
        VkBuffer bufs[] = {buffers.vertex->getBuffer()};
        // Using interleaved attribute bindings only requires one buffer to be bound
        VkDeviceSize offsets[1] = {0};
        vkCmdBindVertexBuffers(cmdbuffer, 0, 1, bufs, offsets);
#else
        // Using separate vertex attribute bindings requires binding multiple attribute buffers
        VkDeviceSize offsets[3] = {0, 0, 0};
        std::array<VkBuffer, 3> bufs = {buffers.vertex.buffer, buffers.color.buffer, buffers.normal.buffer};
        vkCmdBindVertexBuffers(cmdbuffer, 0, static_cast<uint32_t>(bufs.size()), bufs.data(), offsets);
#endif
        if (buffers.indexCount > 0) {// for index draws
            vkCmdBindIndexBuffer(cmdbuffer, buffers.index->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(cmdbuffer, buffers.indexCount, 1, 0, 0, 0);
        } else { // for non-index draws
            vkCmdDraw(cmdbuffer, buffers.vertexCount, 1, 0, 0);
        }
    }


    void Drawable::updateUniformBuffer(const glm::mat4& perspective, const glm::mat4& view) {
        ubo.projection = perspective;
        ubo.view = view;
        ubo.model = glm::mat4(1.0f);
        uniformBuffer->writeToBuffer(&ubo);
    }


    void Drawable::prepareUniformBuffer() {
        uniformBuffer = std::make_unique<LveBuffer>(
                lveDevice, sizeof(ubo), 1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        uniformBuffer->map();
    }

}