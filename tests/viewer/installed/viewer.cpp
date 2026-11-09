// SPDX-License-Identifier: GPL-3.0-or-later
// Public interactive API only: no Vulkan SDK, GLFW or OpenGL headers/calls.
#include <easy3d/viewer/viewer_base.h>
#include <easy3d/viewer/viewer.h>
// Legacy headers remain includable even when their classes have no Vulkan counterpart.
#include <easy3d/renderer/ambient_occlusion.h>
#include <easy3d/renderer/average_color_blending.h>
#include <easy3d/renderer/dual_depth_peeling.h>
#include <easy3d/renderer/eye_dome_lighting.h>
#include <easy3d/renderer/shadow.h>
#include <easy3d/renderer/soft_shadow.h>
#include <easy3d/renderer/transparency.h>
#include <easy3d/gui/picker.h>
#include <easy3d/gui/picker_model.h>
#include <easy3d/gui/picker_point_cloud.h>
#include <easy3d/gui/picker_surface_mesh.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/initializer.h>
#include <cmath>
#include <iostream>
#include <stdexcept>
int main() {
    using namespace easy3d;
    initialize(false, false, false);
    const auto require = [](bool ok, const char* message) { if (!ok) throw std::runtime_error(message); };
    try {
#ifdef EASY3D_HAS_VULKAN
        const VulkanOptions options{};
#else
        const OpenGLOptions options{};
#endif
        std::shared_ptr<ViewerBase> viewer = std::make_shared<Viewer>(
            "Installed Viewer", 181, 151, 1, false, false, true, options);
        auto mesh = std::make_shared<SurfaceMesh>();
        const auto a = mesh->add_vertex(vec3(0, -1, -1)), b = mesh->add_vertex(vec3(0, 1, -1));
        const auto c = mesh->add_vertex(vec3(0, 0, 1)); mesh->add_triangle(a, b, c); viewer->add_model(mesh);
        auto faces = mesh->renderer()->get_triangles_drawable("faces");
        faces->set_uniform_coloring(vec4(.8f, .2f, .1f, 1)); faces->set_lighting(false);
        viewer->set_usage("", ""); viewer->fit_screen();
        unsigned ticks = 0, before = 0, after = 0;
        ViewerCallbacks callbacks;
        callbacks.pre_draw = [&](ViewerBase* host) { require(host == viewer.get(), "callback received another viewer"); ++before; };
        callbacks.post_draw = [&](ViewerBase*) { ++after; };
        viewer->set_callbacks(std::move(callbacks));
        bool found=false;
        const auto hit=viewer->point_under_pixel(viewer->width()/2,viewer->height()/2,found);
        require(found && std::abs(hit.x)<1e-4f, "installed viewer surface query failed");
        require(distance(viewer->unproject_point(viewer->project_point(hit)),hit)<1e-4f,
                "installed viewer coordinate conversion failed");
        require(before==0 && after==0, "installed viewer query invoked frame callbacks");
        viewer->set_animation_callback([&](ViewerBase* host) { ++ticks; host->set_animation(false); return true; });
        viewer->set_animation(true);
        require(viewer->run_frames(2) == 0 && ticks == 1 && viewer->width() == 181 && viewer->height() == 151 && viewer->samples() == 1 && before >= 2 && after == 2,
                "installed viewer loop/dimensions/animation/callbacks failed");
        const auto frame_callbacks = before + after;
        require(viewer->snapshot("installed-viewer.png", 1, 4, 1), "installed viewer snapshot failed");
        require(before + after == frame_callbacks, "installed viewer snapshot invoked frame callbacks");
        int w, h, channels; std::vector<unsigned char> rgba;
        require(ImageIO::load("installed-viewer.png", rgba, w, h, channels, 4, false) &&
                w == viewer->framebuffer_width() && h == viewer->framebuffer_height(), "installed viewer output dimensions differ");
        const auto center = (size_t(h / 2) * w + w / 2) * 4;
        require(std::abs(int(rgba[center]) - 204) <= 2 && std::abs(int(rgba[center+1]) - 51) <= 2,
                "installed viewer material/default camera did not render the model");
        MultiViewer grid(1,2,"Installed MultiViewer",181,151,1,false,false,true,options);
        auto shared=std::make_shared<SurfaceMesh>();
        auto ga=shared->add_vertex(vec3(0,-1,-1)),gb=shared->add_vertex(vec3(0,1,-1)),gc=shared->add_vertex(vec3(0,0,1));
        shared->add_triangle(ga,gb,gc);grid.add_model(shared);
        shared->renderer()->get_triangles_drawable("faces")->set_lighting(false);
        grid.assign(0,0,shared.get());grid.assign(0,1,shared.get());grid.set_division_visible(false);grid.fit_screen();
        require(grid.run_frames(2)==0 && grid.snapshot("installed-grid.png",1.25f,4,0),"installed MultiViewer failed");
        ViewerBase& portable=grid;
        for(int x:{grid.width()/4,3*grid.width()/4}) {
            bool hit=false;auto p=portable.point_under_pixel(x,grid.height()/2,hit);
            require(hit && std::abs(p.x)<1e-4f,"installed MultiViewer cell query failed");
        }
        require(grid.models().size()==1 && grid.validation_errors()==0,"installed MultiViewer ownership/validation failed");
        std::cout << "Viewer and MultiViewer rendered using the configured backend" << '\n';
        return viewer->validation_errors() ? 1 : 0;
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << '\n'; return 1;
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
