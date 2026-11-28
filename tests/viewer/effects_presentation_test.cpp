// SPDX-License-Identifier: GPL-3.0-or-later
#include "test_window.h"
#include <easy3d/viewer/offscreen.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/render_device.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/util/initializer.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace easy3d;
using namespace easy3d::graphics;

namespace {

void require(bool value, const char *message) {
    if (!value)
        throw std::runtime_error(message);
}

size_t pixel(unsigned width, unsigned x, unsigned y) {
    return size_t(y) * width + x;
}

void require_dominant(const FrameImage &image, unsigned x, unsigned y, unsigned channel) {
    const size_t offset = pixel(image.width, x, y) * 4;
    const unsigned value = image.rgba[offset + channel];
    unsigned other_max = 0;
    for (unsigned c = 0; c < 3; ++c) {
        if (c != channel)
            other_max = std::max(other_max, unsigned(image.rgba[offset + c]));
    }
    if (value < 80 || other_max > 60) {
        std::cerr << "effect pixel at " << x << "," << y << ": "
                  << int(image.rgba[offset]) << ',' << int(image.rgba[offset + 1]) << ','
                  << int(image.rgba[offset + 2]) << '\n';
        throw std::runtime_error("OIT composition lost the command viewport");
    }
}

DrawCommand quad(float x0, float x1, float y0, float y1, float z, const vec4 &color,
                 uint32_t object_id, const ivec4 &viewport = ivec4(0)) {
    std::vector<Vertex> vertices(4);
    vertices[0].position = vec3(x0, y0, z);
    vertices[1].position = vec3(x1, y0, z);
    vertices[2].position = vec3(x1, y1, z);
    vertices[3].position = vec3(x0, y1, z);
    DrawCommand result;
    result.geometry = std::make_shared<Geometry>(std::move(vertices), std::vector<uint32_t>{0, 1, 2, 0, 2, 3});
    result.color = color;
    result.object_id = object_id;
    result.viewport = viewport;
    return result;
}

bool same_ids_and_depth(const FrameImage &a, const FrameImage &b) {
    require(a.depth.size() == b.depth.size() && a.ids.size() == b.ids.size(), "effect changed readback sizes");
    for (size_t i = 0; i < a.ids.size(); ++i) {
        if (a.ids[i].object != b.ids[i].object || a.ids[i].primitive != b.ids[i].primitive ||
            std::abs(a.depth[i] - b.depth[i]) > 1e-6f)
            return false;
    }
    return true;
}

// x,y -> [0,1], z -> [0,1]. The z term offsets the projected shadow from the
// caster so both shadowed and lit receiver regions are visible.
mat4 test_light_matrix() {
    mat4 m = mat4::identity();
    m(0, 0) = .5f;
    m(0, 2) = .6f;
    m(0, 3) = .5f;
    m(1, 1) = .5f;
    m(1, 2) = .6f;
    m(1, 3) = .5f;
    m(2, 2) = .5f;
    m(2, 3) = .5f;
    return m;
}

void effect_order_and_clipping(RenderDevice &device) {
    const unsigned width = 64, height = 48;
    const vec4 background(0, 0, 0, 1);
    RenderOptions options;
    options.background = background;

    const auto far_overlay = quad(-1, 1, -1, 1, .6f, vec4(.5f, .5f, .5f, 1), 7);
    const auto near_overlay = quad(-.3f, .3f, -.3f, .3f, -.4f, vec4(.8f, .8f, .8f, 1), 8);
    const std::vector<DrawCommand> scene{far_overlay, near_overlay};

    Camera camera;
    camera.set_type(Camera::PERSPECTIVE);
    const auto ao = graphics::shape::draw_ambient_occlusion(width, height, 1.0f, 0.0f, 1.0f,
                                                            camera.projection_matrix());
    const auto edl = graphics::shape::draw_eye_dome_lighting(width, height, .1f, 100.0f, 2.0f);
    const auto depth = graphics::shape::draw_depth_texture(Rect(0, int(width), 0, int(height)), width, height);

    const auto plain = device.render(width, height, scene, options);
    const auto shaded = device.render(width, height, {ao, edl, scene[0], scene[1]}, options);
    require(plain.rgba != shaded.rgba, "settings effects were not composed into the scene");
    require(same_ids_and_depth(plain, shaded), "effect overlays changed scene picking or depth");

    // Vulkan walks overlays in submitted command order. A depth preview submitted
    // before the darkening effects must itself be darkened; composing effects by
    // category instead would apply the preview last and erase them.
    const auto preview = device.render(width, height, {depth}, options);
    const auto preview_then_effects = device.render(width, height, {depth, ao, edl, scene[0], scene[1]}, options);
    require(preview_then_effects.rgba != preview.rgba,
            "OpenGL did not preserve Vulkan's stream-ordered effect composition");

    auto flat = far_overlay;
    flat.overlay = true;
    flat.object_id = UINT32_MAX;
    flat.color = vec4(0, 1, 0, 1);
    auto depth_tested = flat;
    depth_tested.overlay_depth_test = true;
    const auto preview_scene = device.render(width, height, {depth, scene[0], scene[1]}, options);
    const auto flat_then_preview = device.render(width, height, {flat, depth, scene[0], scene[1]}, options);
    const auto preview_then_flat = device.render(width, height, {depth, flat, scene[0], scene[1]}, options);
    const size_t overlay_pixel = pixel(width, 5, height / 2) * 4;
    require(flat_then_preview.rgba == preview_scene.rgba,
            "flat overlay ordering did not preserve the following depth preview");
    require(preview_then_flat.rgba[overlay_pixel + 1] > 200 &&
                    preview_then_flat.rgba[overlay_pixel] < 10,
            "flat overlay did not replace the preceding depth preview");
    const auto depth_tested_then_preview = device.render(
        width, height, {depth_tested, depth, scene[0], scene[1]}, options);
    require(depth_tested_then_preview.rgba == preview_scene.rgba,
            "depth effects ran before Vulkan's depth-tested overlay phase");
    require(same_ids_and_depth(preview_scene, depth_tested_then_preview),
            "overlay phase ordering changed scene picking or depth");

    // Effective planes clip the depth source. A cut pixel far from the surviving
    // geometry must keep the clear color and clear depth with AO and EDL enabled.
    const auto clipped = [&](const std::vector<DrawCommand> &commands) {
        std::vector<DrawCommand> result = commands;
        for (auto &d : result)
            d.clipping_planes = {vec4(1, 0, 0, 0)};
        return result;
    };
    const auto clipped_scene = clipped(scene);
    std::vector<DrawCommand> clipped_with_effects{ao, edl};
    clipped_with_effects.insert(clipped_with_effects.end(), clipped_scene.begin(), clipped_scene.end());
    const auto clipped_plain = device.render(width, height, clipped_scene, options);
    const auto clipped_effects = device.render(width, height, clipped_with_effects, options);
    const size_t cut = pixel(width, 5, height / 2) * 4;
    for (unsigned c = 0; c < 4; ++c)
        require(std::abs(int(clipped_effects.rgba[cut + c]) - int(background[c] * 255)) <= 2,
                "a fully cut pixel received an effect contribution");
    require(clipped_effects.depth[pixel(width, 5, height / 2)] == 1.0f &&
                    clipped_effects.ids[pixel(width, 5, height / 2)].object == UINT32_MAX,
            "a fully cut pixel kept stale scene depth or IDs");
    require(clipped_effects.rgba != clipped_plain.rgba, "clipped effects were not composed");
    require(same_ids_and_depth(clipped_plain, clipped_effects),
            "clipped effect overlays changed scene picking or depth");
}

void mixed_effects_and_shadow_casters(RenderDevice &device) {
    const unsigned width = 64, height = 48;
    const vec4 background(0, 0, 0, 1);
    RenderOptions options;
    options.background = background;

    Camera camera;
    camera.set_type(Camera::PERSPECTIVE);
    const auto light = test_light_matrix();
    const auto ao = graphics::shape::draw_ambient_occlusion(width, height, 1.0f, 0.0f, 1.0f,
                                                            camera.projection_matrix());
    const auto edl = graphics::shape::draw_eye_dome_lighting(width, height, .1f, 100.0f, 2.0f);
    const auto shadow = graphics::shape::draw_shadow(width, height, light, .6f, .2f, .1f, 10.0f, 128);

    auto receiver = quad(-1, 1, -1, 1, 0.1f, vec4(.8f), UINT32_MAX);
    receiver.shadow_background = true;
    receiver.shadow_matrix = light;
    receiver.shadow_map_size = 128;
    auto caster = quad(-.8f, .8f, -.8f, .8f, -.4f, vec4(1), 21);
    caster.shadow_caster = true;
    caster.shadow_matrix = light;
    caster.shadow_map_size = 128;
    const std::vector<DrawCommand> scene{receiver, caster};

    const auto plain = device.render(width, height, scene, options);
    const auto ao_edl = device.render(width, height, {ao, edl, scene[0], scene[1]}, options);
    require(ao_edl.rgba != plain.rgba, "AO+EDL did not compose on the device path");
    require(same_ids_and_depth(plain, ao_edl), "AO+EDL changed scene picking or depth");
    const auto all_effects = device.render(width, height, {shadow, ao, edl, scene[0], scene[1]}, options);
    require(all_effects.rgba != plain.rgba, "shadow+AO+EDL did not compose on the device path");
    require(same_ids_and_depth(plain, all_effects), "shadow+AO+EDL changed scene picking or depth");

    auto transparent = quad(-1, 1, -1, 1, 0.0f, vec4(0, 0.3f, 1, .6f), 22);
    transparent.transparent = true;
    RenderOptions peeled = options;
    peeled.transparency = TransparencyMethod::peeling;
    peeled.peel_layers = 6;
    const std::vector<DrawCommand> transparent_scene{scene[0], scene[1], transparent};
    const auto oit = device.render(width, height, transparent_scene, peeled);
    const auto oit_edl = device.render(width, height, {edl, scene[0], scene[1], transparent}, peeled);
    require(oit_edl.rgba != oit.rgba, "EDL did not compose over transparent geometry");
    require(same_ids_and_depth(oit, oit_edl), "EDL changed picking or depth of transparent geometry");
    const auto oit_shadow = device.render(width, height, {shadow, scene[0], scene[1], transparent}, peeled);
    require(oit_shadow.rgba != oit.rgba, "shadow did not compose over transparent geometry");

    // Clip the visible triangles and the triangle shadow caster with one plane.
    // The overlay effect commands stay outside the scene-clip set.
    std::vector<DrawCommand> clipped_scene = scene;
    for (auto &command : clipped_scene)
        command.clipping_planes = {vec4(-1, 0, 0, 0)};
    const auto clipped_plain = device.render(width, height, clipped_scene, options);
    const auto clipped_shadow = device.render(width, height,
                                              {shadow, clipped_scene[0], clipped_scene[1]}, options);
    const size_t cut = pixel(width, width - 5, height / 2);
    for (unsigned c = 0; c < 4; ++c)
        require(std::abs(int(clipped_shadow.rgba[cut * 4 + c]) - int(background[c] * 255)) <= 2,
                "a cut pixel received a shadow contribution");
    require(clipped_shadow.depth[cut] == 1.0f && clipped_shadow.ids[cut].object == UINT32_MAX,
            "a cut pixel kept stale depth or IDs with a shadow caster");
    require(same_ids_and_depth(clipped_plain, clipped_shadow),
            "shadow changed picking or depth of clipped caster geometry");
    require(clipped_shadow.rgba != clipped_plain.rgba, "clipped caster did not cast a shadow");

    auto clipped_transparent = transparent;
    clipped_transparent.clipping_planes = {vec4(-1, 0, 0, 0)};
    const std::vector<DrawCommand> clipped_transparent_scene{clipped_scene[0], clipped_scene[1],
                                                             clipped_transparent};
    const auto clipped_oit = device.render(width, height, clipped_transparent_scene, peeled);
    const auto clipped_oit_edl = device.render(width, height,
                                               {edl, clipped_scene[0], clipped_scene[1], clipped_transparent}, peeled);
    for (unsigned c = 0; c < 4; ++c)
        require(std::abs(int(clipped_oit_edl.rgba[cut * 4 + c]) - int(background[c] * 255)) <= 2,
                "a cut transparent pixel received an effect contribution");
    require(clipped_oit_edl.depth[cut] == 1.0f && clipped_oit_edl.ids[cut].object == UINT32_MAX,
            "a cut transparent pixel kept stale depth or IDs");
    require(same_ids_and_depth(clipped_oit, clipped_oit_edl),
            "an effect changed picking or depth of clipped transparent geometry");
}

const char *method_name(TransparencyMethod method) {
    switch (method) {
        case TransparencyMethod::sorted: return "sorted";
        case TransparencyMethod::average: return "average";
        case TransparencyMethod::peeling: return "peeling";
        case TransparencyMethod::linked_lists: return "linked lists";
    }
    return "unknown";
}

void oit_cell_viewports(RenderDevice &device) {
    const unsigned width = 64, height = 32;
    auto left = quad(-1, 1, -1, 1, 0, vec4(1, 0, 0, .8f), 10, ivec4(0, 0, 32, 32));
    auto right = quad(-1, 1, -1, 1, 0, vec4(0, 1, 0, .8f), 11, ivec4(32, 0, 32, 32));
    left.transparent = right.transparent = true;

    for (auto method : {TransparencyMethod::sorted, TransparencyMethod::average,
                        TransparencyMethod::peeling, TransparencyMethod::linked_lists}) {
        RenderOptions options;
        options.background = vec4(0);
        options.transparency = method;
        options.peel_layers = 6;
        const auto frame = device.render(width, height, {left, right}, options);
        try {
            require_dominant(frame, 8, height / 2, 0);
            require_dominant(frame, 40, height / 2, 1);
            require_dominant(frame, 31, height / 2, 0);
            require_dominant(frame, 32, height / 2, 1);
        } catch (...) {
            std::cerr << "transparency method: " << method_name(method) << '\n';
            throw;
        }
        require(frame.ids[pixel(width, 8, height / 2)].object == 10 &&
                        frame.ids[pixel(width, 40, height / 2)].object == 11 &&
                        frame.ids[pixel(width, 31, height / 2)].object == 10 &&
                        frame.ids[pixel(width, 32, height / 2)].object == 11,
                "OIT composition changed per-cell scene IDs");
        if (device.validation_errors())
            throw std::runtime_error("OIT composition reported backend validation errors");
    }
}

} // namespace

int main(int argc, char **argv) {
    try {
        initialize(false, false, false);
        (void)argv;
        easy3d::DeviceOptions host_options;
        graphics::DeviceOptions device_options;
#ifdef EASY3D_HAS_VULKAN
        host_options.validation = host_options.sync_validation = argc > 1;
        device_options.validation = host_options.validation;
        device_options.sync_validation = host_options.sync_validation;
#endif
        OffScreen host(64, 48, host_options);
        auto device = create_device(device_options);
        effect_order_and_clipping(*device);
        mixed_effects_and_shadow_casters(*device);
        oit_cell_viewports(*device);
        require(device->validation_errors() == 0, "effect presentation validation errors");
        std::cout << "PASS: effect order, mixed effects, clipped effect and shadow inputs, scene queries, "
                     "OIT cell viewports and fallbacks\n";
        return EXIT_SUCCESS;
    } catch (const std::exception &e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return EXIT_FAILURE;
    }
}
