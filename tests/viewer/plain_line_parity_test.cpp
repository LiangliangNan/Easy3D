// SPDX-License-Identifier: GPL-3.0-or-later
//
// Shared headless parity check for plain world-space lines.
//
// The same source runs in both backend trees with the same scene and the same
// expectations. Each backend renders through its own capture path (OpenGL:
// glReadPixels behind Viewer::capture(), Vulkan: the OffScreen device readback),
// so equal results on both sides mean the two backends draw the same stroke.
//
// A plain line width is a size in logical pixels: with the screen-space
// expansion (setting::lines_drawable_screen_space_width) the stroke keeps the
// requested framebuffer width along its whole length, independently of the
// projection and of the distance to the camera. The scene therefore uses a line
// that recedes from the camera, which is where a world-space line width and a
// screen-space line width disagree.
//
// The camera-space expansion is checked too: its width changes with the distance
// to the camera. Both backends support both expansions, selected by
// setting::lines_drawable_screen_space_width.
#include <easy3d/viewer/offscreen.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/setting.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace easy3d;

namespace {

    void require(bool ok, const char *message) {
        if (!ok)
            throw std::runtime_error(message);
    }

    // The requested line width in framebuffer pixels. The offscreen canvas has
    // no DPI scaling, so logical pixels and framebuffer pixels are equal.
    const float WIDTH = 50.0f;

    // Local perpendicular thickness of the rendered stroke, sampled along the
    // stroke. A constant-width stroke reports the same thickness everywhere.
    struct Stroke {
        std::vector<float> thickness;  //!< Thickness per sample, from one end to the other.
        float first = 0;               //!< Thickness near the first end.
        float last = 0;                //!< Thickness near the second end.
        unsigned pixels = 0;           //!< Number of covered pixels.
    };

    Stroke measure(const graphics::FrameImage &frame) {
        // The line is opaque red on a black background, so its pixels are exact.
        std::vector<vec2> pixels;
        for (unsigned y = 0; y < frame.height; ++y)
            for (unsigned x = 0; x < frame.width; ++x) {
                const size_t i = size_t(y) * frame.width + x;
                if (frame.rgba[4 * i] > 200 && frame.rgba[4 * i + 1] < 60 && frame.rgba[4 * i + 2] < 60)
                    pixels.emplace_back(float(x), float(y));
            }
        Stroke stroke;
        stroke.pixels = unsigned(pixels.size());
        require(stroke.pixels > 1000, "plain line did not render");

        // Principal axis of the stroke.
        vec2 mean(0, 0);
        for (const auto &p : pixels)
            mean += p;
        mean /= float(pixels.size());
        double cxx = 0, cxy = 0, cyy = 0;
        for (const auto &p : pixels) {
            const vec2 d = p - mean;
            cxx += double(d.x) * d.x;
            cxy += double(d.x) * d.y;
            cyy += double(d.y) * d.y;
        }
        cxx /= pixels.size();
        cxy /= pixels.size();
        cyy /= pixels.size();
        const double trace = cxx + cyy;
        const double det = cxx * cyy - cxy * cxy;
        const double lambda = 0.5 * (trace + std::sqrt(std::max(0.0, trace * trace - 4.0 * det)));
        vec2 dir(1, 0);
        if (std::abs(cxy) > 1e-12)
            dir = vec2(float(lambda - cyy), float(cxy));
        else if (cyy > cxx)
            dir = vec2(0, 1);
        const float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        require(length > 1e-6f, "plain line did not render a stroke");
        dir /= length;
        const vec2 normal(-dir.y, dir.x);

        // Sample the perpendicular extent of the stroke along its axis. The end
        // samples are skipped because the butt caps truncate them.
        std::vector<float> along, across;
        along.reserve(pixels.size());
        across.reserve(pixels.size());
        for (const auto &p : pixels) {
            const vec2 d = p - mean;
            along.push_back(d.x * dir.x + d.y * dir.y);
            across.push_back(d.x * normal.x + d.y * normal.y);
        }
        const float t_min = *std::min_element(along.begin(), along.end());
        const float t_max = *std::max_element(along.begin(), along.end());
        require(t_max - t_min > 10.0f, "plain line did not render a stroke");
        constexpr int samples = 20;
        std::vector<float> low(samples, 1e30f), high(samples, -1e30f);
        for (size_t i = 0; i < along.size(); ++i) {
            const int s = std::min(samples - 1, int((along[i] - t_min) / (t_max - t_min) * samples));
            low[s] = std::min(low[s], across[i]);
            high[s] = std::max(high[s], across[i]);
        }
        for (int s = 0; s < samples; ++s) {
            require(low[s] <= high[s], "plain line stroke has a gap");
            stroke.thickness.push_back(high[s] - low[s]);
        }
        float first = 0, last = 0;
        for (int s = 3; s < 8; ++s)
            first += stroke.thickness[s];
        for (int s = 12; s < 17; ++s)
            last += stroke.thickness[s];
        stroke.first = first / 5.0f;
        stroke.last = last / 5.0f;
        return stroke;
    }

    void report(const char *projection, const Stroke &stroke) {
        std::cout << projection << ": pixels " << stroke.pixels << ", thickness";
        for (float t : stroke.thickness)
            std::cout << ' ' << t;
        std::cout << " (first " << stroke.first << ", last " << stroke.last << ")\n";
    }

    void check(const char *projection, const Stroke &stroke) {
        // The stroke must keep its width along the whole line.
        require(std::abs(stroke.first - stroke.last) <= 2.0f,
                "plain line width changes along the line (projection-dependent thickness)");
        // ... and it must keep the requested width in framebuffer pixels.
        const float middle = stroke.thickness[stroke.thickness.size() / 2];
        require(std::abs(middle - WIDTH) <= 3.0f, "plain line did not keep the requested pixel width");
    }

    void check_camera_space(const char *projection, const Stroke &stroke) {
        // The camera-space expansion uses a world-space radius, so the width on
        // screen changes with the distance to the camera.
        float low = stroke.thickness[0], high = stroke.thickness[0];
        for (float t : stroke.thickness) {
            low = std::min(low, t);
            high = std::max(high, t);
        }
        require(high - low > 10.0f, "camera-space expansion did not change the width with the distance");
    }
}

int main(int argc, char **argv) {
    try {
        initialize(false, false, false);
        OffScreen renderer(320, 240);
        renderer.set_background_color(vec4(0, 0, 0, 1));
        // A line that recedes from the camera: its two ends are at very different
        // distances, so a world-space width and a screen-space width differ.
        auto line = std::make_shared<LinesDrawable>("plain line");
        line->update_vertex_buffer({vec3(-0.6f, 0, 1.5f), vec3(0.6f, 0, -1.5f)});
        line->set_uniform_coloring(vec4(1, 0, 0, 1));
        line->set_line_width(WIDTH);
        line->set_lighting(false);
        renderer.add_drawable(line);
        auto *camera = renderer.camera();
        camera->setSceneBoundingBox(vec3(-1), vec3(1));
        camera->setPosition(vec3(0, 0, 3));
        camera->setViewDirection(vec3(0, 0, -1));
        camera->setUpVector(vec3(0, 1, 0));
        auto measure_projection = [&](Camera::Type type) {
            camera->setType(type);
            return measure(renderer.capture(1));
        };
        // Screen-space expansion: the stroke keeps the requested pixel width.
        setting::lines_drawable_screen_space_width = true;
        const auto perspective = measure_projection(Camera::PERSPECTIVE);
        const auto orthographic = measure_projection(Camera::ORTHOGRAPHIC);
        report("perspective (screen space)", perspective);
        report("orthographic (screen space)", orthographic);
        // Both projections must draw the same stroke.
        check("perspective (screen space)", perspective);
        check("orthographic (screen space)", orthographic);
        // Camera-space expansion: the world-space radius makes the width follow
        // the distance to the camera. Both backends support this mode.
        setting::lines_drawable_screen_space_width = false;
        const auto camera_space = measure_projection(Camera::PERSPECTIVE);
        report("perspective (camera space)", camera_space);
        check_camera_space("perspective (camera space)", camera_space);        std::cout << "PASS: plain line keeps a constant pixel width in both projections\n";
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
