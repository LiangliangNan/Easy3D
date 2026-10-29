// SPDX-License-Identifier: GPL-3.0-or-later
// Ordinary consumer: no backend-specific classes, SDK headers or conditionals.
#include <easy3d/viewer/offscreen.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/util/initializer.h>
#include <easy3d/fileio/image_io.h>
#include <iostream>
int main() {
    easy3d::initialize(false,false,false);
    easy3d::OffScreen renderer(101,77);
    auto mesh=std::make_shared<easy3d::SurfaceMesh>();
    auto a=mesh->add_vertex(easy3d::vec3(0,-1,-1)),b=mesh->add_vertex(easy3d::vec3(0,1,-1)),c=mesh->add_vertex(easy3d::vec3(0,0,1));
    mesh->add_triangle(a,b,c);renderer.add_model(mesh);
    mesh->renderer()->get_triangles_drawable("faces")->set_uniform_coloring(easy3d::vec4(1,.4f,0,1));
    const bool result=renderer.render("installed-unified.png",1,4);
    std::cout<<"Unified OffScreen rendered using "<<(renderer.backend()==easy3d::Backend::Vulkan?"Vulkan":"OpenGL")<<'\n';
    std::vector<unsigned char> rgba;int w,h,channels;
    if(!result || !easy3d::ImageIO::load("installed-unified.png",rgba,w,h,channels,4,false) || w!=101 || h!=77) return 1;
    unsigned colored=0;for(size_t i=0;i<rgba.size();i+=4) colored+=rgba[i]!=255 || rgba[i+1]!=255 || rgba[i+2]!=255;
    return colored>100?0:1;
}
