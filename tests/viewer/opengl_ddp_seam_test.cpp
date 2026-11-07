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
using namespace easy3d;
namespace {
void require(bool ok, const char* text) { if (!ok) throw std::runtime_error(text); }

// A tutorial that brackets dual depth peeling inside draw(), as Tutorial_504
// does, and counts how many times the snapshot path invokes that draw().
struct PeelViewer : Viewer {
    using Viewer::Viewer;
    ~PeelViewer() override { delete peeling_; }
    void enable() { peeling_ = new DualDepthPeeling(camera()); }
    mutable int draws = 0;
    bool peel = false;
    void draw() const override {
        ++draws;
        if (peel && peeling_) {
            std::vector<TrianglesDrawable*> surfaces;
            for (auto& m : models_)
                for (auto& d : m->renderer()->triangles_drawables())
                    surfaces.push_back(dynamic_cast<TrianglesDrawable*>(d.get()));
            peeling_->draw(surfaces);
            return;
        }
        Viewer::draw();
    }
    DualDepthPeeling* peeling_ = nullptr;
};

// Two intersecting cubes, the scene reported in issue #219.
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

// Lit object pixels: neither the white background nor black.
size_t lit_coverage(const std::vector<unsigned char>& p) {
    size_t n = 0;
    for (size_t i = 0; i + 3 < p.size(); i += 4)
        if (!(p[i] > 245 && p[i+1] > 245 && p[i+2] > 245) && !(p[i] < 8 && p[i+1] < 8 && p[i+2] < 8)) ++n;
    return n;
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
        PeelViewer v("DDP flat-normal seam", 320, 240, 1, false, false, true);
        v.enable();
        auto mesh = std::make_shared<SurfaceMesh>();
        add_cube(*mesh, vec3(0, 0, 0), 1.0f);
        add_cube(*mesh, vec3(0.6f, 0.4f, 0.3f), 1.0f);
        require(v.add_model(mesh), "scene missing");
        auto* faces = mesh->renderer()->get_triangles_drawable("faces");
        require(faces, "cube has no faces drawable");
        faces->set_coloring_method(State::UNIFORM_COLOR);
        faces->set_color(vec4(0.7f, 0.7f, 0.7f, 1.0f));
        faces->set_opacity(0.5f);
        faces->set_lighting(true);
        faces->set_lighting_two_sides(true);
        faces->set_distinct_back_color(false);
        // Flat shading forces the peel shader to derive the face normal from
        // dFdx/dFdy of the world position -- the path that produced NaN normals
        // and black seams when evaluated after the peel early-returns (#219).
        faces->set_smooth_shading(false);
        v.fit_screen();
        Viewer::poll_events(); v.draw_frame();
        int fw = 0, fh = 0; v.framebuffer_size(fw, fh);
        require(fw > 0 && fh > 0, "test window has no framebuffer");
        const size_t frame = size_t(fw) * size_t(fh);

        // background=1 forces white, so any black pixel is a shading artifact
        // rather than the viewer color. A correct flat-shaded peel lights the
        // faces; the bug collapses wide bands along edges/intersections to black.
        v.peel = true;
        const int calls = v.draws;
        require(v.snapshot("opengl-ddp-flat.png", 1, 1, 1), "flat-shaded DDP snapshot failed");
        require(v.draws > calls, "the DDP snapshot did not run the virtual draw()");
        const auto flat = read("opengl-ddp-flat.png", fw, fh);
        const size_t coverage = lit_coverage(flat);
        require(coverage > frame / 20, "flat-shaded DDP rendered no lit surface");
        require(black_pixels(flat) < frame / 200, "flat-shaded DDP produced black derivative-normal seams (issue #219)");

        // Smooth shading uses the provided vertex normals and never touches the
        // derivatives, so it is the reference the flat path must keep up with in
        // coverage (faceted lighting, not a collapsed black image).
        faces->set_smooth_shading(true);
        require(v.snapshot("opengl-ddp-smooth.png", 1, 1, 1), "smooth-shaded DDP snapshot failed");
        const auto smooth = read("opengl-ddp-smooth.png", fw, fh);
        const size_t smooth_coverage = lit_coverage(smooth);
        require(smooth_coverage > frame / 20, "smooth-shaded DDP rendered no lit surface");
        require(coverage > smooth_coverage / 2, "flat shading collapsed relative to smooth shading");
        v.peel = false;

        std::cout << "PASS: flat-shaded dual depth peeling lights intersecting transparent cubes without black derivative-normal seams (#219)\n";
        return 0;
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}