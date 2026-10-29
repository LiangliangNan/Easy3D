// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer.h>
#include <easy3d/viewer/vulkan_offscreen.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/opengl.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/setting.h>
#include <iostream>
#include <stdexcept>
using namespace easy3d;
void require(bool ok,const char* message) {if(!ok) throw std::runtime_error(message);}
int main() {
    try {
        initialize(false,false,false);Viewer host("OpenGL/Vulkan material reference");
        graphics::DeviceOptions options;options.validation=options.synchronization_validation=true;
        VulkanOffScreen native(160,120,options);
        require(native.background_color()==setting::background_color,"offscreen background ignores shared settings");
        auto* camera=native.camera();camera->setType(Camera::ORTHOGRAPHIC);
        camera->setSceneBoundingBox(vec3(-1),vec3(1));camera->setViewDirection(vec3(0,0,-1));camera->showEntireScene();
        GLuint fbo,color,depth;glGenFramebuffers(1,&fbo);glBindFramebuffer(GL_FRAMEBUFFER,fbo);
        glGenTextures(1,&color);glBindTexture(GL_TEXTURE_2D,color);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,160,120,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,color,0);
        glGenRenderbuffers(1,&depth);glBindRenderbuffer(GL_RENDERBUFFER,depth);
        glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,160,120);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,depth);
        require(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE,"reference framebuffer incomplete");
        glViewport(0,0,160,120);glDisable(GL_BLEND);glDisable(GL_CULL_FACE);glDisable(GL_MULTISAMPLE);glDisable(GL_FRAMEBUFFER_SRGB);
        glEnable(GL_DEPTH_TEST);glDepthFunc(GL_LESS);
        auto compare=[&](Drawable& gl,const std::shared_ptr<Drawable>& vk,const std::vector<vec3>& probes) {
            // The adapter must forward State changes without a vertex reupload.
            for(int mode=0;mode<5;++mode) {
                for(auto* d:{&gl,vk.get()}) {
                    d->set_lighting(mode!=3);d->set_lighting_two_sides(mode!=1);d->set_distinct_back_color(mode!=2);
                    d->set_back_color(mode==4?vec4(.2f,.8f,.1f,1):vec4(.8f,.4f,.4f,1));
                    d->set_uniform_coloring(mode==4?vec4(.2f,.7f,.9f,1):vec4(.6f,.6f,1,1));
                    d->set_material(mode==4?State::Material(vec4(.1f),vec4(.15f),8):State::Material(vec4(.05f),vec4(.4f),64));
                }
                const auto bg=setting::background_color;glClearColor(bg.x,bg.y,bg.z,bg.w);glClearDepth(1);
                glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);gl.draw(camera);
                std::vector<unsigned char> reference(160*120*4);glReadPixels(0,0,160,120,GL_RGBA,GL_UNSIGNED_BYTE,reference.data());
                for(unsigned samples:{1u,4u}) {
                if (!(native.device().supported_sample_counts()&samples)) continue;
                const auto image=native.capture(samples);
                for(const auto& p:probes) {
                    const auto screen=camera->projectedCoordinatesOf(p);const int x=int(screen.x),y=int(screen.y);
                    const auto a=(size_t(y)*160+x)*4,b=(size_t(119-y)*160+x)*4;
                    for(unsigned c=0;c<3;++c) if(std::abs(int(image.rgba[a+c])-int(reference[b+c]))>4) {
                        std::cerr<<"mode "<<mode<<" point "<<p<<" channel "<<c<<" Vulkan "<<int(image.rgba[a+c])<<" OpenGL "<<int(reference[b+c])<<'\n';
                        throw std::runtime_error("material rendering differs from OpenGL reference");
                    }
                    require(image.ids[size_t(y)*160+x].object!=UINT32_MAX,"material shading lost picking IDs");
                }
                }
            }
        };
        {
            PointsDrawable gl("reference points");auto vk=std::make_shared<PointsDrawable>("native points");native.add_drawable(vk);
            for(auto* d:{&gl,vk.get()}) {d->update_vertex_buffer({vec3(-.4f,0,0),vec3(.4f,0,0)});d->update_normal_buffer({vec3(0,0,1),vec3(0,0,-1)});d->set_point_size(11);}
            compare(gl,vk,{vec3(-.4f,0,0),vec3(.4f,0,0)});native.clear_scene();
        }
        {
            TrianglesDrawable gl("reference faces");auto vk=std::make_shared<TrianglesDrawable>("native faces");native.add_drawable(vk);
            for(auto* d:{&gl,vk.get()}) {
                d->update_vertex_buffer({vec3(-.7f,-.4f,0),vec3(-.1f,-.4f,0),vec3(-.4f,.4f,0),vec3(.1f,-.4f,0),vec3(.4f,.4f,0),vec3(.7f,-.4f,0)});
                d->update_normal_buffer({vec3(0,0,1),vec3(0,0,1),vec3(0,0,1),vec3(0,0,-1),vec3(0,0,-1),vec3(0,0,-1)});d->set_smooth_shading(true);
            }
            const std::vector<vec3> probes{vec3(-.4f,-.1f,0),vec3(.4f,-.1f,0)};
            compare(gl,vk,probes);
            gl.set_smooth_shading(false);vk->set_smooth_shading(false);compare(gl,vk,probes);
            camera->setViewDirection(vec3(.3f,0,-1));camera->showEntireScene();compare(gl,vk,probes);
            native.clear_scene();
        }
        {
            PointsDrawable gl("points without normals");auto vk=std::make_shared<PointsDrawable>("points without normals");native.add_drawable(vk);
            for(auto* d:{&gl,vk.get()}) {d->update_vertex_buffer({vec3(0)});d->set_point_size(11);}
            compare(gl,vk,{vec3(0)});native.clear_scene();
        }
        auto cloud=std::make_shared<PointCloud>();cloud->add_vertex(vec3(0));native.add_model(cloud);
        const auto defaults=cloud->renderer()->get_points_drawable("vertices");
        require(defaults->point_size()==setting::point_cloud_vertices_size &&
            defaults->impostor_type()==(setting::point_cloud_vertices_impostors?PointsDrawable::SPHERE:PointsDrawable::PLAIN),
            "native host overrides shared drawable defaults");
        glBindFramebuffer(GL_FRAMEBUFFER,0);glDeleteFramebuffers(1,&fbo);glDeleteTextures(1,&color);glDeleteRenderbuffers(1,&depth);
        require(glGetError()==GL_NO_ERROR,"reference rendering reported OpenGL error");
        require(native.device().validation_errors()==0,"material rendering reported validation errors");
        std::cout<<"PASS: OpenGL/Vulkan point and surface lighting, two-sided/back colors, material, unlit and shared background\n";
    }catch(const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
