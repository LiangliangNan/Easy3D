// SPDX-License-Identifier: GPL-3.0-or-later
#include "api_smoke.h"

#include <easy3d/renderer/render_device.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace {

class NullDevice : public easy3d::graphics::RenderDevice {
public:
    const std::string& name() const override { return name_; }
    const std::string& driver_version() const override { return driver_; }

    std::uint32_t supported_sample_counts(easy3d::graphics::PointStyle) const override { return 1; }
    std::uint32_t supported_sample_counts(easy3d::graphics::LineStyle) const override { return 1; }

    easy3d::graphics::FrameImage render(
        std::uint32_t width,
        std::uint32_t height,
        const std::vector<easy3d::graphics::DrawCommand>&,
        const easy3d::vec4&,
        unsigned,
        easy3d::graphics::TransparencyMethod,
        unsigned,
        easy3d::graphics::FrameReadback) override {
        easy3d::graphics::FrameImage frame;
        frame.width = width;
        frame.height = height;
        return frame;
    }

    easy3d::graphics::ScenePixel sample_pixel(
        std::uint32_t, std::uint32_t, const std::vector<easy3d::graphics::DrawCommand>&,
        std::uint32_t, std::uint32_t) override {
        return {};
    }

    std::vector<easy3d::graphics::FrameImage::Id> sample_ids(
        std::uint32_t, std::uint32_t, const std::vector<easy3d::graphics::DrawCommand>&,
        std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t) override {
        return {};
    }

    bool present(
        std::uint32_t, std::uint32_t, const std::vector<easy3d::graphics::DrawCommand>&,
        const easy3d::vec4&, unsigned, easy3d::graphics::TransparencyMethod, unsigned) override {
        return false;
    }

    unsigned validation_errors() const override { return 0; }

    std::vector<std::uint32_t> select_points(
        const easy3d::graphics::PointSelectionRequest& request) override {
        return request.words;
    }

    std::uint64_t point_upload_count() const override { return 0; }
    std::uint32_t oit_dropped_fragments() const override { return 0; }

private:
    std::string name_ = "NullDevice";
    std::string driver_ = "compile-smoke";
};

}  // namespace

void smoke_render_device() {
    NullDevice device;
    easy3d::graphics::RenderDevice& api = device;

    std::vector<easy3d::graphics::Vertex> vertices(3);
    vertices[0].position = easy3d::vec3(-1, -1, 0);
    vertices[1].position = easy3d::vec3(1, -1, 0);
    vertices[2].position = easy3d::vec3(0, 1, 0);
    auto geometry = std::make_shared<easy3d::graphics::Geometry>(
        vertices, std::vector<std::uint32_t>{0, 1, 2}, easy3d::graphics::Topology::triangles);

    easy3d::graphics::Sampler sampler;
    sampler.wrap = easy3d::graphics::TextureWrap::repeat;
    sampler.filter = easy3d::graphics::TextureFilter::linear;
    auto image = std::make_shared<easy3d::graphics::Image>(
        1, 1, std::vector<unsigned char>{255, 255, 255, 255}, sampler);

    easy3d::graphics::DrawCommand command;
    command.geometry = geometry;
    command.texture = image;
    command.object_id = 7;
    command.point_style = easy3d::graphics::PointStyle::plain;
    command.line_style = easy3d::graphics::LineStyle::plain;

    const std::vector<easy3d::graphics::DrawCommand> commands{command};
    api.render(32, 32, commands);
    api.render(32, 32, commands, easy3d::vec4(1), 1,
               easy3d::graphics::TransparencyMethod::sorted, 4,
               easy3d::graphics::FrameReadback::color);
    api.present(32, 32, commands);
    api.sample_pixel(32, 32, commands, 0, 0);
    api.sample_ids(32, 32, commands, 0, 0, 1, 1);
    api.supported_sample_counts();
    api.supported_sample_counts(easy3d::graphics::LineStyle::plain);

    easy3d::graphics::PointSelectionRequest selection;
    selection.points = std::make_shared<easy3d::graphics::PointData>(
        std::vector<easy3d::vec3>{easy3d::vec3(0, 0, 0)});
    selection.polygon = {easy3d::vec2(0, 0), easy3d::vec2(1, 0), easy3d::vec2(0, 1)};
    selection.words = {0};
    selection.width = 32;
    selection.height = 32;
    api.select_points(selection);
    api.validation_errors();
    api.point_upload_count();
    api.oit_dropped_fragments();
    api.name();
    api.driver_version();
}
