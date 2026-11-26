// SPDX-License-Identifier: GPL-3.0-or-later
//
// Portable rendering of the shared KeyframeInterpolator guides.
//
// Both backends cover the same ordinary TrianglesDrawable/LinesDrawable scene
// geometry. This checks that the path and keyframe cameras render, that deleted
// path geometry disappears, that camera markers are lit (unlit markers would be
// the exact uniform color on every interior pixel, while lit facets add ambient
// and specular terms even where the diffuse term is zero), and that the shared
// scene-glyph drawable renders and unregisters cleanly.
#include <easy3d/viewer/viewer.h>
#include <tests/viewer/test_window.h>   // window_options()
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/frame.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/keyframe_interpolator.h>
#include <easy3d/util/initializer.h>
#include <iostream>
#include <memory>
#include <stdexcept>

using namespace easy3d;

namespace {
    void require(bool ok, const char *message) {
        if (!ok)
            throw std::runtime_error(message);
    }

    struct GuideViewer : Viewer {
        using Viewer::Viewer;
        // The lit corner axes are also red and would satisfy the marker/lit pixel tests.
        void hide_corner_axes() { show_corner_axes_ = false; }
    };

    struct Counts {
        unsigned markers = 0;  //!< Pixels shaded with the red marker color, lit or unlit.
        unsigned lit = 0;      //!< Marker pixels carrying the ambient/specular grey term.
        unsigned path = 0;     //!< Pixels of the blue path line.
    };

    Counts count(const graphics::FrameImage &frame) {
        Counts counts;
        for (unsigned y = 0; y < frame.height; ++y)
            for (unsigned x = 0; x < frame.width; ++x) {
                const size_t i = (size_t(y) * frame.width + x) * 4;
                const unsigned r = frame.rgba[i], g = frame.rgba[i + 1], b = frame.rgba[i + 2];
                if (r > 100 && g < 100 && b < 100)
                    ++counts.markers;
                if (r > 100 && g > 8 && b > 8 && g < 200 && b < 200)
                    ++counts.lit;
                if (b > 200 && r < 150 && g < 200)
                    ++counts.path;
            }
        return counts;
    }
}

int main() {
    try {
        initialize(false, true, false);
        GuideViewer viewer("Camera path guides", window_options(480, 360, 4));
        viewer.hide();
        viewer.hide_corner_axes();
        viewer.set_usage("");
        viewer.set_background_color(vec4(0, 0, 0, 1));
        viewer.camera()->set_position(vec3(20, 20, 200));
        viewer.camera()->look_at(vec3(20, 20, 0));
        viewer.camera()->set_scene_radius(200);

        const float camera_width = 20;
        KeyframeInterpolator path(viewer.camera()->frame());
        require(path.add_keyframe(Frame(vec3(0, 0, 0), quat())), "first key failed");
        require(path.add_keyframe(Frame(vec3(40, 0, 0), quat())), "second key failed");
        require(path.add_keyframe(Frame(vec3(40, 40, 0), quat())), "third key failed");

        const auto bare = count(viewer.capture(4));

        // Scene glyphs take part in the portable scene pass, where the depth pass
        // hides each glyph's back side and picking maps a triangle to its keyframe.
        auto glyphs = std::make_shared<TrianglesDrawable>("keyframe cameras");
        require(!path.update_cameras_drawable(*glyphs, viewer.camera(), 0.0f),
                "a non-positive glyph width must be refused");
        require(path.update_cameras_drawable(*glyphs, viewer.camera(), camera_width, vec4(1, 0, 0, 1)),
                "filling the scene glyphs failed");
        require(KeyframeInterpolator::camera_glyph_triangles() > 0, "camera glyphs have no triangles");
        viewer.add_drawable(glyphs);
        const auto cameras = count(viewer.capture(4));
        require(cameras.markers > 200, "keyframe cameras did not render");
        require(cameras.lit > 100, "keyframe cameras are not lit");
        viewer.delete_drawable(glyphs.get());
        const auto cleared = count(viewer.capture(4));
        require(cleared.markers == bare.markers, "scene camera glyphs stayed after removal");

        auto line = std::make_shared<LinesDrawable>("camera path");
        require(path.update_path_drawable(*line), "filling the camera path failed");
        viewer.add_drawable(line);
        const auto path_frame = count(viewer.capture(4));
        require(path_frame.path > 100, "camera path did not render");
        require(path_frame.markers == bare.markers, "camera path drew marker geometry");

        path.delete_path();
        require(!path.update_path_drawable(*line), "deleted camera path still produced geometry");
        line->update_vertex_buffer(std::vector<vec3>{});
        line->disable_element_buffer();
        const auto deleted = count(viewer.capture(4));
        require(deleted.markers == bare.markers && deleted.path == bare.path, "deleted path still rendered");
        viewer.delete_drawable(line.get());

        std::cout << "PASS: camera path and lit keyframe cameras render and clear on delete\n";
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
