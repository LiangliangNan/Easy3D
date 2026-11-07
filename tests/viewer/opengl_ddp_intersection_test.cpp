// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/dual_depth_peeling.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/initializer.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <cmath>
using namespace easy3d;
namespace {
void require(bool ok, const char* text) { if (!ok) throw std::runtime_error(text); }

struct PeelViewer : Viewer {
    using Viewer::Viewer;
    ~PeelViewer() override { delete peeling_; }
    void enable() { peeling_ = new DualDepthPeeling(camera()); }
    int draws = 0;
    bool peel = false;
    bool draw() override {
        ++draws;
        if (peel && peeling_) {
            std::vector<TrianglesDrawable*> surfaces;
            for (auto& m : models_)
                for (auto& d : m->renderer()->triangles_drawables())
                    surfaces.push_back(dynamic_cast<TrianglesDrawable*>(d.get()));
            peeling_->draw(surfaces);
            return true;
        }
        return Viewer::draw();
    }
    DualDepthPeeling* peeling_ = nullptr;
};

void add_cube(SurfaceMesh& mesh, const vec3& center, float h) {
    const vec3 corners[8] = {
        center + vec3(-h,-h,-h), center + vec3( h,-h,-h),
        center + vec3( h, h,-h), center + vec3(-h, h,-h),
        center + vec3(-h,-h, h), center + vec3( h,-h, h),
        center + vec3( h, h, h), center + vec3(-h, h, h)
    };
    SurfaceMesh::Vertex v[8];
    for (int i = 0; i < 8; ++i) v[i] = mesh.add_vertex(corners[i]);
    const int quads[6][4] = {
        {4,5,6,7}, {1,0,3,2}, {5,1,2,6}, {0,4,7,3}, {7,6,2,3}, {0,1,5,4}
    };
    for (const auto& q : quads) {
        const std::vector<SurfaceMesh::Vertex> face = {v[q[0]], v[q[1]], v[q[2]], v[q[3]]};
        require(mesh.add_face(face).is_valid(), "cannot build cube fixture");
    }
}

std::vector<unsigned char> read(const std::string& file, int width, int height) {
    std::vector<unsigned char> data; int w = 0, h = 0, channels = 0;
    require(ImageIO::load(file, data, w, h, channels, 4, false), "snapshot file could not be read");
    require(w == width && h == height, "snapshot size does not match the request");
    require(data.size() == size_t(width) * size_t(height) * 4, "snapshot data does not match its size");
    return data;
}

size_t black_pixels(const std::vector<unsigned char>& p) {
    size_t n = 0;
    for (size_t i = 0; i + 3 < p.size(); i += 4)
        if (p[i] < 8 && p[i+1] < 8 && p[i+2] < 8) ++n;
    return n;
}
}

int main() {
    try {
        initialize(false, false, false);
        PeelViewer v("DDP intersection seam", 640, 480, 1, false, false, true);
        v.enable();
        
        // Two intersecting boxes: one axis-aligned, one rotated 45 degrees
        auto mesh1 = std::make_shared<SurfaceMesh>();
        add_cube(*mesh1, vec3(0, 0, 0), 1.0f);
        require(v.add_model(mesh1), "scene missing");
        
        auto mesh2 = std::make_shared<SurfaceMesh>();
        add_cube(*mesh2, vec3(0.5f, 0.3f, 0.2f), 0.8f);
        require(v.add_model(mesh2), "scene missing");
        
        // Configure both boxes: transparent, flat shading, lit, high contrast colors
        auto* faces1 = mesh1->renderer()->get_triangles_drawable("faces");
        require(faces1, "cube1 has no faces drawable");
        faces1->set_coloring_method(State::UNIFORM_COLOR);
        faces1->set_color(vec4(1.0f, 0.0f, 0.0f, 1.0f));  // Red
        faces1->set_opacity(0.7f);
        faces1->set_lighting(true);
        faces1->set_lighting_two_sides(true);
        faces1->set_distinct_back_color(false);
        faces1->set_smooth_shading(false);
        
        auto* faces2 = mesh2->renderer()->get_triangles_drawable("faces");
        require(faces2, "cube2 has no faces drawable");
        faces2->set_coloring_method(State::UNIFORM_COLOR);
        faces2->set_color(vec4(0.0f, 0.0f, 1.0f, 1.0f));  // Blue
        faces2->set_opacity(0.7f);
        faces2->set_lighting(true);
        faces2->set_lighting_two_sides(true);
        faces2->set_distinct_back_color(false);
        faces2->set_smooth_shading(false);
        
        v.fit_screen();
        Viewer::poll_events(); v.draw_frame();
        int fw = 0, fh = 0; v.framebuffer_size(fw, fh);
        require(fw > 0 && fh > 0, "test window has no framebuffer");
        
        // Snapshot with DDP
        v.peel = true;
        require(v.snapshot("opengl-ddp-intersection.png", 1, 1, 1), "DDP snapshot failed");
        const auto img = read("opengl-ddp-intersection.png", fw, fh);
        
        std::cout << "Image size: " << fw << "x" << fh << "\n";
        std::cout << "Black pixels: " << black_pixels(img) << "\n";
        std::cout << "Snapshot saved to opengl-ddp-intersection.png\n";
        
        v.peel = false;
        return 0;
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}