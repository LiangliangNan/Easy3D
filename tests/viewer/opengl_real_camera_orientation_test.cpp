// SPDX-License-Identifier: GPL-3.0-or-later
#include <tutorials/Tutorial_205_RealCamera/viewer.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/resource.h>

#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>

using namespace easy3d;

namespace {

    class OrientationViewer final : public TutorialRealCamera {
    public:
        OrientationViewer(const std::string &title, const std::string &bundler, const std::string &cloud)
            : TutorialRealCamera(title, bundler, cloud) {}

        Rect image_rect() const { return calculate_image_rect(); }
    };

    void require(bool condition, const char *message) {
        if (!condition)
            throw std::runtime_error(message);
    }

    graphics::FrameImage frame(Viewer &viewer) {
        for (unsigned i = 0; i < 100; ++i) {
            Viewer::poll_events();
            if (viewer.draw_frame())
                return viewer.capture();
            glfwWaitEventsTimeout(.01);
        }
        throw std::runtime_error("real-camera frame did not present");
    }

    void verify_source_orientation(const OrientationViewer &viewer,
                                   const graphics::FrameImage &frame,
                                   const std::vector<unsigned char> &source,
                                   int source_width, int source_height) {
        const Rect rect = viewer.image_rect();
        for (float x_fraction : {.23f, .47f, .71f}) {
            for (float y_fraction : {.23f, .47f, .71f}) {
                const int x = int(rect.x_min() + rect.width() * x_fraction);
                const int y = int(rect.y_min() + rect.height() * y_fraction);
                require(x >= 0 && y >= 0 && x < int(frame.width) && y < int(frame.height),
                        "image probe outside viewport");
                const float u = (x + .5f - rect.x_min()) / rect.width() * source_width - .5f;
                const float v = (y + .5f - rect.y_min()) / rect.height() * source_height - .5f;
                const int x0 = int(std::floor(u));
                const int y0 = int(std::floor(v));
                const float fx = u - x0;
                const float fy = v - y0;
                for (unsigned channel = 0; channel < 3; ++channel) {
                    float expected = 0;
                    for (int dy = 0; dy < 2; ++dy) {
                        for (int dx = 0; dx < 2; ++dx) {
                            const int px = std::max(0, std::min(source_width - 1, x0 + dx));
                            const int py = std::max(0, std::min(source_height - 1, y0 + dy));
                            expected += source[(size_t(py) * source_width + px) * 4 + channel] *
                                        (dx ? fx : 1 - fx) * (dy ? fy : 1 - fy);
                        }
                    }
                    require(std::abs(frame.rgba[(size_t(y) * frame.width + x) * 4 + channel] - expected) < 4,
                            "real-camera image is flipped or incorrectly sampled");
                }
            }
        }
    }

} // namespace

int main() {
    try {
        initialize(false, false, false);
        const std::string root = resource::directory() + "/data/fountain/";
        OrientationViewer viewer("RealCamera orientation", root + "bundle.out", root + "pointcloud.ply");
        viewer.hide();

        std::vector<unsigned char> source;
        int width = 0, height = 0, channels = 0;
        require(ImageIO::load(root + "images/0000.jpg", source, width, height, channels, 4, false),
                "source image missing");
        verify_source_orientation(viewer, frame(viewer), source, width, height);
        std::cout << "PASS: OpenGL real-camera source image orientation\n";
    } catch (const std::exception &e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
