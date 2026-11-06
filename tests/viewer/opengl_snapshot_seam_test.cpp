// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/ambient_occlusion.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/resource.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace easy3d;
namespace {
void require(bool ok, const char* text) { if (!ok) throw std::runtime_error(text); }

// A tutorial that brackets ambient occlusion inside draw(), as Tutorial_501
// does, and counts how many times the snapshot path invokes that draw().
struct AOViewer : Viewer {
    using Viewer::Viewer;
    ~AOViewer() override { delete occlusion_; }
    void enable() { occlusion_ = new AmbientOcclusion(camera()); }
    int draws = 0;
    bool occlude = false;
    bool draw() override {
        ++draws;
        if (occlude && occlusion_) {
            occlusion_->begin(models_);
            const bool ok = Viewer::draw();
            occlusion_->end();
            return ok;
        }
        return Viewer::draw();
    }
    AmbientOcclusion* occlusion_ = nullptr;
};

std::vector<unsigned char> read(const std::string& file, int width, int height) {
    std::vector<unsigned char> data; int w = 0, h = 0, channels = 0;
    require(ImageIO::load(file, data, w, h, channels, 4, false), "snapshot file could not be read");
    require(w == width && h == height, "snapshot size does not match the request");
    require(data.size() == size_t(width) * size_t(height) * 4, "snapshot data does not match its size");
    return data;
}
}

int main() {
    try {
        initialize(false, false, false);
        AOViewer v("AO snapshot seam", 320, 240, 1, false, false, true);
        v.enable();
        require(v.add_model(resource::directory() + "/data/torusknot.obj"), "scene missing");
        v.fit_screen();
        Viewer::poll_events(); v.draw_frame();
        int fw = 0, fh = 0; v.framebuffer_size(fw, fh);
        require(fw > 0 && fh > 0, "test window has no framebuffer");
        const mat4 before = v.camera()->modelViewProjectionMatrix();
        const int cam_w = v.camera()->screenWidth(), cam_h = v.camera()->screenHeight();

        // A 2x snapshot is larger than the window. Within the GL size limits it
        // must render as ONE whole-image offscreen pass -- draw() called once --
        // not a 2x2 grid of window-sized tiles. Tiling crops each tile's depth
        // buffer, so the screen-space AO loses its neighbors across tile borders
        // and the result seams along the middle horizontal/vertical lines. The
        // single pass keeps the full depth buffer, so the effect is continuous.
        const int plain_calls = v.draws;
        require(v.snapshot("opengl-ao-seam-plain.png", 2, 1, 0), "plain scaled snapshot failed");
        require(v.draws == plain_calls + 1, "the plain in-limit snapshot was tiled");
        const auto plain = read("opengl-ao-seam-plain.png", fw * 2, fh * 2);

        v.occlude = true;
        const int ao_calls = v.draws;
        require(v.snapshot("opengl-ao-seam-ao.png", 2, 1, 0), "occluded scaled snapshot failed");
        require(v.draws == ao_calls + 1, "the occluded in-limit snapshot was tiled instead of rendered as one whole-image pass");
        const auto ao = read("opengl-ao-seam-ao.png", fw * 2, fh * 2);
        require(ao != plain, "ambient occlusion is missing from the scaled snapshot");
        v.occlude = false;

        // The snapshot borrows the camera and the frame path; both are back.
        require(v.camera()->screenWidth() == cam_w && v.camera()->screenHeight() == cam_h,
                "the snapshot left the camera at the output size");
        const mat4 after = v.camera()->modelViewProjectionMatrix();
        for (unsigned i = 0; i < 16; ++i) require(after[i] == before[i], "the snapshot left the projection overridden");

        std::cout << "PASS: an in-limit scaled AO snapshot renders as one continuous whole-image pass (no tile seams) and restores the camera\n";
        return 0;
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}