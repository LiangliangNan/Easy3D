// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_RENDERER_BACKENDS_OPENGL_RENDER_DEVICE_H
#define EASY3D_RENDERER_BACKENDS_OPENGL_RENDER_DEVICE_H

#include <easy3d/renderer/render_device.h>
#include <memory>

namespace easy3d {
    namespace graphics {

        class EASY3D_DEPRECATED("use RenderDevice returned by create_device()") OpenGLDevice
            : public RenderDevice {
        public:
            using RenderDevice::render;
            OpenGLDevice();
            ~OpenGLDevice() override;

            const std::string &name() const override;
            const std::string &driver_version() const override;
            std::uint32_t supported_sample_counts(PointStyle style = PointStyle::plain) const override;
            std::uint32_t supported_sample_counts(LineStyle style) const override;
            EASY3D_DEPRECATED("use render(width, height, commands, const RenderOptions&)")
            FrameImage render(std::uint32_t width,
                              std::uint32_t height,
                              const std::vector<DrawCommand> &commands,
                              const vec4 &background = vec4(1, 1, 1, 1),
                              unsigned samples = 1,
                              TransparencyMethod transparency = TransparencyMethod::sorted,
                              unsigned peel_layers = 4,
                              FrameReadback readback = FrameReadback::color_depth_ids) override;
            ScenePixel sample_pixel(std::uint32_t width,
                                    std::uint32_t height,
                                    const std::vector<DrawCommand> &commands,
                                    std::uint32_t x,
                                    std::uint32_t y) override;
            std::vector<FrameImage::Id> sample_ids(std::uint32_t width,
                                                   std::uint32_t height,
                                                   const std::vector<DrawCommand> &commands,
                                                   std::uint32_t x,
                                                   std::uint32_t y,
                                                   std::uint32_t region_width,
                                                   std::uint32_t region_height) override;
            EASY3D_DEPRECATED("window presentation belongs to the private host adapter")
            bool present(std::uint32_t width,
                         std::uint32_t height,
                         const std::vector<DrawCommand> &commands,
                         const vec4 &background = vec4(1, 1, 1, 1),
                         unsigned samples = 1,
                         TransparencyMethod transparency = TransparencyMethod::sorted,
                         unsigned peel_layers = 4) override;
            unsigned validation_errors() const override;
            std::vector<std::uint32_t> select_points(const PointSelectionRequest &request) override;
            std::uint64_t point_upload_count() const override;
            std::uint32_t oit_dropped_fragments() const override;

        private:
            struct Impl;
            std::unique_ptr<Impl> impl_;
        };

    } // namespace graphics
} // namespace easy3d

#endif
