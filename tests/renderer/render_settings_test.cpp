// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/renderer/render_settings.h>

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

using namespace easy3d;
using namespace easy3d::graphics;

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
        require(rejected, "invalid render settings were accepted");
    }

    template<class Function>
    void rejects_configured(Function configure) {
        RenderSettings settings;
        configure(settings);
        rejects([&] { validate_render_settings(settings); });
    }

    void test_defaults_and_copy() {
        const RenderSettings settings;
        require(!settings.shadow.enabled && settings.shadow.map_size == 1024 &&
                settings.shadow.light_distance == 50.0f && settings.shadow.darkness == 0.6f &&
                settings.shadow.softness == 0.0f && !settings.shadow.virtual_background &&
                settings.shadow.virtual_background_color == vec4(0.8f),
                "shadow defaults differ from the frozen contract");
        require(!settings.ambient_occlusion.enabled && settings.ambient_occlusion.radius_ratio == 0.045f &&
                settings.ambient_occlusion.bias == 0.005f && settings.ambient_occlusion.intensity == 1.0f,
                "ambient-occlusion defaults differ from the frozen contract");
        require(!settings.eye_dome_lighting && !settings.transparency.enabled &&
                settings.transparency.method == TransparencyMethod::sorted &&
                settings.transparency.layers == 4 && settings.clipping_planes.empty(),
                "render-setting defaults differ from the frozen contract");
        validate_render_settings(settings);

        RenderSettings copy = settings;
        copy.shadow.enabled = true;
        copy.ambient_occlusion.enabled = true;
        copy.eye_dome_lighting = true;
        copy.transparency.enabled = true;
        copy.transparency.method = TransparencyMethod::linked_lists;
        copy.transparency.layers = 8;
        copy.clipping_planes.push_back(vec4(0, 2, 0, -1));
        validate_render_settings(copy);
        require(settings.clipping_planes.empty() && !copy.clipping_planes.empty(),
                "render-setting copies share clipping storage");
    }

    void test_valid_boundaries() {
        RenderSettings settings;
        settings.shadow.map_size = 1;
        settings.shadow.light_distance = 1.1f;
        settings.shadow.darkness = 0;
        settings.shadow.softness = 1;
        settings.shadow.virtual_background = true;
        settings.shadow.virtual_background_color = vec4(1, 0.5f, 0, 1);
        settings.ambient_occlusion.radius_ratio = std::numeric_limits<float>::min();
        settings.ambient_occlusion.bias = 0;
        settings.ambient_occlusion.intensity = 0;
        settings.transparency.enabled = true;
        settings.transparency.layers = 1;
        settings.clipping_planes = {vec4(0, 3, 0, -1), vec4(0, 0, -4, 2)};
        validate_render_settings(settings);
        require(settings.clipping_planes[0] == vec4(0, 3, 0, -1),
                "validation unexpectedly mutates clipping planes");
    }

    void test_invalid_shadow() {
        rejects_configured([](RenderSettings &settings) { settings.shadow.map_size = 0; });
        rejects_configured([](RenderSettings &settings) { settings.shadow.map_size = 3; });
        rejects_configured([](RenderSettings &settings) {
            settings.shadow.map_size = 1u << (sizeof(unsigned) * 8 - 1);
        });
        rejects_configured([](RenderSettings &settings) { settings.shadow.light_distance = 1.09f; });
        rejects_configured([](RenderSettings &settings) { settings.shadow.light_distance = std::numeric_limits<float>::infinity(); });
        rejects_configured([](RenderSettings &settings) { settings.shadow.darkness = -0.01f; });
        rejects_configured([](RenderSettings &settings) { settings.shadow.darkness = 1.01f; });
        rejects_configured([](RenderSettings &settings) { settings.shadow.softness = -0.01f; });
        rejects_configured([](RenderSettings &settings) { settings.shadow.softness = 1.01f; });
        rejects_configured([](RenderSettings &settings) {
            settings.shadow.virtual_background_color.w = std::numeric_limits<float>::quiet_NaN();
        });
    }

    void test_invalid_ambient_occlusion() {
        rejects_configured([](RenderSettings &settings) { settings.ambient_occlusion.radius_ratio = 0; });
        rejects_configured([](RenderSettings &settings) { settings.ambient_occlusion.radius_ratio = -1; });
        rejects_configured([](RenderSettings &settings) { settings.ambient_occlusion.bias = -0.001f; });
        rejects_configured([](RenderSettings &settings) {
            settings.ambient_occlusion.bias = std::numeric_limits<float>::infinity();
        });
        rejects_configured([](RenderSettings &settings) { settings.ambient_occlusion.intensity = -0.01f; });
        rejects_configured([](RenderSettings &settings) { settings.ambient_occlusion.intensity = 1.01f; });
    }

    void test_invalid_transparency_and_clipping() {
        rejects_configured([](RenderSettings &settings) {
            settings.transparency.enabled = true;
            settings.transparency.layers = 0;
        });
        rejects_configured([](RenderSettings &settings) {
            settings.transparency.enabled = true;
            settings.transparency.method = static_cast<TransparencyMethod>(99);
        });
        rejects_configured([](RenderSettings &settings) {
            settings.clipping_planes = {vec4(0, 1, 0, 0), vec4(1, 0, 0, 0), vec4(0, 0, 1, 0)};
        });
        rejects_configured([](RenderSettings &settings) { settings.clipping_planes = {vec4(0, 0, 0, 1)}; });
        rejects_configured([](RenderSettings &settings) {
            settings.clipping_planes = {vec4(0, std::numeric_limits<float>::infinity(), 0, 0)};
        });
    }

    void test_capabilities() {
        RenderSettings settings;
        graphics::DeviceCapabilities capabilities;
        capabilities.transparency_methods = {TransparencyMethod::sorted};
        validate_render_settings(settings, capabilities);

        settings.shadow.enabled = true;
        rejects([&] { validate_render_settings(settings, capabilities); });
        capabilities.shadow = true;
        validate_render_settings(settings, capabilities);

        settings.ambient_occlusion.enabled = true;
        rejects([&] { validate_render_settings(settings, capabilities); });
        capabilities.ambient_occlusion = true;
        validate_render_settings(settings, capabilities);

        settings.eye_dome_lighting = true;
        rejects([&] { validate_render_settings(settings, capabilities); });
        capabilities.eye_dome_lighting = true;
        validate_render_settings(settings, capabilities);

        settings.transparency.enabled = true;
        settings.transparency.method = TransparencyMethod::peeling;
        rejects([&] { validate_render_settings(settings, capabilities); });
        settings.transparency.enabled = false;
        validate_render_settings(settings, capabilities);
        settings.transparency.enabled = true;
        capabilities.transparency_methods.push_back(TransparencyMethod::peeling);
        validate_render_settings(settings, capabilities);
    }

} // namespace

int main() {
    try {
        test_defaults_and_copy();
        test_valid_boundaries();
        test_invalid_shadow();
        test_invalid_ambient_occlusion();
        test_invalid_transparency_and_clipping();
        test_capabilities();
        std::cout << "render settings contract passed" << std::endl;
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << std::endl;
        return 1;
    }
}
