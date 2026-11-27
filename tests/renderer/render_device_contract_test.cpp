// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/renderer/render_device.h>
#include <easy3d/renderer/shape.h>

#include <cmath>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

using namespace easy3d;
using namespace easy3d::graphics;

static_assert(std::is_standard_layout<DrawCommand>::value,
              "DrawCommand must retain its 2.8 layout compatibility");
static_assert(sizeof(DrawCommand) == 616,
              "DrawCommand must retain its frozen 616-byte 2.8 layout");
static_assert(offsetof(DrawCommand, model_view_projection) == 32 &&
              offsetof(DrawCommand, model) == 172 &&
              offsetof(DrawCommand, clipping_planes) == 392 &&
              offsetof(DrawCommand, depth_range_min) == 610 &&
              offsetof(DrawCommand, depth_range_max) == 612,
              "DrawCommand members moved within the frozen 2.8 layout");

namespace {

    void require(bool value, const char *message) {
        if (!value)
            throw std::runtime_error(message);
    }

    template<class Function>
    void rejects(Function function) {
        bool rejected = false;
        try {
            function();
        } catch (const std::invalid_argument &) {
            rejected = true;
        }
        require(rejected, "invalid factory input was accepted");
    }

    class RecordingDevice final : public RenderDevice {
    public:
        const std::string &name() const override { return name_; }
        const std::string &driver_version() const override { return driver_; }
        std::uint32_t supported_sample_counts(PointStyle) const override { return 1; }
        std::uint32_t supported_sample_counts(LineStyle) const override { return 1; }

        FrameImage render(std::uint32_t width,
                          std::uint32_t height,
                          const std::vector<DrawCommand> &,
                          const vec4 &background,
                          unsigned samples,
                          TransparencyMethod transparency,
                          unsigned peel_layers,
                          FrameReadback readback) override {
            this->background = background;
            this->samples = samples;
            this->transparency = transparency;
            this->peel_layers = peel_layers;
            this->readback = readback;
            FrameImage frame;
            frame.width = width;
            frame.height = height;
            return frame;
        }

        ScenePixel sample_pixel(std::uint32_t, std::uint32_t, const std::vector<DrawCommand> &,
                                std::uint32_t, std::uint32_t) override {
            return {};
        }

        std::vector<FrameImage::Id> sample_ids(std::uint32_t, std::uint32_t,
                                               const std::vector<DrawCommand> &, std::uint32_t,
                                               std::uint32_t, std::uint32_t, std::uint32_t) override {
            return {};
        }

        bool present(std::uint32_t, std::uint32_t, const std::vector<DrawCommand> &, const vec4 &,
                     unsigned, TransparencyMethod, unsigned) override {
            return false;
        }

        unsigned validation_errors() const override { return 0; }
        std::vector<std::uint32_t> select_points(const PointSelectionRequest &request) override {
            return request.words;
        }
        std::uint64_t point_upload_count() const override { return 0; }
        std::uint32_t oit_dropped_fragments() const override { return 0; }

        vec4 background = vec4(0);
        unsigned samples = 0;
        TransparencyMethod transparency = TransparencyMethod::sorted;
        unsigned peel_layers = 0;
        FrameReadback readback = FrameReadback::color;

    private:
        std::string name_ = "RecordingDevice";
        std::string driver_ = "contract-test";
    };

    void test_render_options() {
        RecordingDevice device;
        RenderDevice &api = device;
        RenderOptions options;
        require(options.background == vec4(1) && options.samples == 1 &&
                options.transparency == TransparencyMethod::sorted && options.peel_layers == 4 &&
                options.readback == FrameReadback::color_depth_ids,
                "render option defaults differ from the frozen contract");

        options.background = vec4(.1f, .2f, .3f, .4f);
        options.samples = 4;
        options.transparency = TransparencyMethod::peeling;
        options.peel_layers = 9;
        options.readback = FrameReadback::color;
        const FrameImage frame = api.render(31, 17, {}, options);
        require(frame.width == 31 && frame.height == 17, "options overload changed frame dimensions");
        require(device.background == options.background && device.samples == options.samples &&
                device.transparency == options.transparency && device.peel_layers == options.peel_layers &&
                device.readback == options.readback,
                "options overload did not forward every field");
        const FrameImage legacy_empty_braces = api.render(31, 17, {}, {});
        require(legacy_empty_braces.width == 31 && device.background == vec4(0),
                "legacy empty-braces render call changed overload resolution");
    }

    void test_value_defaults() {
        const DeviceOptions device;
        require(!device.validation && !device.sync_validation && !device.profiling &&
                device.device_name.empty() && device.use_native_large_points,
                "device option defaults differ from the frozen contract");
        const DeviceCapabilities capabilities;
        require(!capabilities.shadow && !capabilities.ambient_occlusion && !capabilities.eye_dome_lighting &&
                capabilities.transparency_methods.empty() && !capabilities.native_large_points,
                "capability defaults differ from the frozen contract");
        require(Backend::opengl != Backend::vulkan, "backend vocabulary is not distinct");
        const DrawCommand command;
        require(command.depth_range() == vec2(0, 1), "draw command default depth range is not [0,1]");
    }

    void test_scene_factory() {
        std::vector<Vertex> vertices(3);
        vertices[0].position = vec3(-1, -1, 0);
        vertices[1].position = vec3(1, -1, 0);
        vertices[2].position = vec3(0, 1, 0);
        const mat4 transform = mat4::translation(vec3(2, 3, 4));
        const DrawCommand command = graphics::shape::draw_triangles(vertices, {}, transform, vec4(.2f, .4f, .6f, 1), 23);
        require(command.geometry && command.geometry->topology() == Topology::triangles &&
                command.geometry->vertices().size() == 3 && command.geometry->indices().empty(),
                "scene triangle factory lost geometry");
        require(command.model_view_projection == transform && command.color == vec4(.2f, .4f, .6f, 1) &&
                command.object_id == 23 && !command.overlay,
                "scene triangle factory changed command state");

        const DrawCommand indexed = graphics::shape::draw_triangles(vertices, {2, 1, 0});
        require(indexed.geometry->indices() == std::vector<std::uint32_t>({2, 1, 0}),
                "scene triangle factory lost indices");
        rejects([&] { graphics::shape::draw_triangles(std::vector<Vertex>(2)); });
    }

    void test_overlay_factories() {
        const auto texture = std::make_shared<Image>(2, 1, std::vector<unsigned char>{
                0, 255, 0, 255, 0, 255, 0, 255});
        const Rect rect(2, 10, 3, 7);
        const DrawCommand textured = graphics::shape::draw_quad_textured(rect, texture, 32, 16, vec4(.5f, 1, 1, 1));
        require(textured.texture == texture && textured.overlay && !textured.overlay_depth_test,
                "textured quad factory changed overlay state");
        require(textured.geometry && textured.geometry->topology() == Topology::triangles &&
                textured.geometry->indices() == std::vector<std::uint32_t>({0, 1, 2, 0, 2, 3}),
                "textured quad factory lost triangle geometry");
        require(textured.geometry->vertices()[0].position == vec3(2, 3, 0) &&
                textured.geometry->vertices()[0].texcoord == vec2(0, 0) &&
                textured.geometry->vertices()[2].texcoord == vec2(1, 1),
                "textured quad factory changed top-left coordinates or UVs");
        const vec4 top_left = textured.model_view_projection * vec4(2, 3, 0, 1);
        const vec4 bottom_right = textured.model_view_projection * vec4(10, 7, 0, 1);
        require(std::abs(top_left.x + .875f) < 1e-6f && std::abs(top_left.y - .625f) < 1e-6f &&
                std::abs(bottom_right.x - -.375f) < 1e-6f && std::abs(bottom_right.y - .125f) < 1e-6f,
                "textured quad factory changed screen-space projection");
        rejects([&] { graphics::shape::draw_quad_textured(rect, {}, 32, 16); });
        rejects([&] { graphics::shape::draw_quad_textured(rect, texture, 0, 16); });

        const DrawCommand filled = graphics::shape::draw_quad_filled(rect, vec4(1, 0, 0, 1), 32, 16);
        require(filled.overlay && !filled.texture, "filled quad factory changed overlay state");
        const DrawCommand lines = graphics::shape::draw_lines_2d({vec2(1, 2), vec2(3, 4)}, vec4(1), 32, 16, 2);
        require(lines.overlay && lines.geometry->vertices().size() == 6,
                "screen-line factory did not produce an overlay ribbon");
        const DrawCommand box = graphics::shape::draw_box_wire(mat4::identity(), 32, 16);
        require(box.overlay && box.overlay_depth_test, "3D overlay factory lost depth-tested state");
    }

} // namespace

int main() {
    try {
        test_render_options();
        test_value_defaults();
        test_scene_factory();
        test_overlay_factories();
        std::cout << "PASS: portable device options and command factories\n";
    } catch (const std::exception &e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
