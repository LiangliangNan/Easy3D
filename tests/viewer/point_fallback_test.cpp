// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/core/point_cloud.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/initializer.h>
#include <easy3d/viewer/offscreen.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <utility>

using namespace easy3d;

namespace {
struct PointRender {
    graphics::FrameImage frame;
    std::array<std::uint32_t, 3> coverage{};
};

void require(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}

PointRender render_points(bool native_large_points, bool validation) {
    VulkanOptions options;
    options.validation = validation;
    options.sync_validation = validation;
    options.use_native_large_points = native_large_points;
    OffScreen viewer(257, 193, options);
    auto cloud = std::make_shared<PointCloud>();
    cloud->add_vertex(vec3(-0.7f, -0.5f, 0));
    cloud->add_vertex(vec3(0, 0, 0));
    cloud->add_vertex(vec3(0.7f, 0.5f, 0));
    viewer.add_model(cloud);
    viewer.camera()->setViewDirection(vec3(0, 0, -1));
    viewer.camera()->setUpVector(vec3(0, 1, 0));
    viewer.camera()->setType(Camera::ORTHOGRAPHIC);
    viewer.fit_screen();
    auto *points = cloud->renderer()->get_points_drawable("vertices");
    require(points, "point-cloud vertex drawable missing");
    points->set_lighting(false);
    points->set_uniform_coloring(vec4(1, 0, 0, 1));
    points->set_point_size(17);
    auto frame = viewer.capture();

    PointRender result;
    result.frame = std::move(frame);
    for (size_t i = 0; i < result.frame.ids.size(); ++i) {
        const auto id = result.frame.ids[i];
        if (id.object == UINT32_MAX)
            continue;
        require(id.primitive < result.coverage.size(), "point fallback changed source primitive IDs");
        ++result.coverage[id.primitive];
        require(result.frame.depth[i] >= 0 && result.frame.depth[i] <= 1,
                "point fallback produced invalid depth");
    }
    for (auto count : result.coverage)
        require(count > 0, "point fallback omitted a source point");
    require(viewer.device().validation_errors() == 0, "point fallback reported Vulkan validation errors");
    return result;
}
} // namespace

int main(int argc, char **) {
    try {
        initialize(false, false, false);
        const bool validation = argc > 1;
        const auto native = render_points(true, validation);
        const auto expanded = render_points(false, validation);
        require(native.frame.width == expanded.frame.width && native.frame.height == expanded.frame.height,
                "native and expanded point renders have different dimensions");
        require(native.frame.ids.size() == expanded.frame.ids.size(),
                "native and expanded point ID buffers differ in size");
        for (size_t primitive = 0; primitive < native.coverage.size(); ++primitive) {
            const auto native_count = native.coverage[primitive];
            const auto expanded_count = expanded.coverage[primitive];
            const auto difference = native_count > expanded_count ? native_count - expanded_count
                                                                   : expanded_count - native_count;
            require(difference <= std::max(4u, std::max(native_count, expanded_count) / 5),
                    "native and expanded point coverage differ beyond the 20% tolerance");
        }
        bool compared_pixels = false;
        for (size_t i = 0; i < native.frame.ids.size(); ++i) {
            const auto a = native.frame.ids[i];
            const auto b = expanded.frame.ids[i];
            if (a.object != b.object || a.primitive != b.primitive)
                continue;
            require(std::abs(native.frame.depth[i] - expanded.frame.depth[i]) < 1e-5f,
                    "native and expanded point depths differ at a shared pixel");
            for (size_t channel = 0; channel < 4; ++channel)
                require(std::abs(int(native.frame.rgba[4 * i + channel]) -
                                 int(expanded.frame.rgba[4 * i + channel])) <= 1,
                        "native and expanded point colors differ at a shared pixel");
            compared_pixels = true;
        }
        require(compared_pixels, "native and expanded point paths have no common coverage");
        std::cout << "Native and portable expanded Vulkan point paths rendered all source points\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
