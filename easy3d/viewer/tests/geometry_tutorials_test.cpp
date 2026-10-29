// SPDX-License-Identifier: GPL-3.0-or-later
#include <tutorials/viewer_backend.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/drawable_lines_2D.h>
#include <easy3d/util/initializer.h>
#include <iostream>
#include <stdexcept>
using namespace easy3d;
std::shared_ptr<SurfaceMesh> create_tessellation_mesh();
std::shared_ptr<SurfaceMesh> create_partition_mesh();
void create_curves_drawables(TutorialViewer&);
void require(bool ok,const char* text) {if (!ok) throw std::runtime_error(text);}
struct GeometryViewer : TutorialViewer {
    using TutorialViewer::TutorialViewer;
    using TutorialViewer::key_press_event;
    const std::string& hint() const {return hint_;}
    void hide_axes() {show_corner_axes_=false;}
};
void frame(GeometryViewer& v) {VulkanViewer::poll_events();require(v.draw_frame(),"geometry tutorial did not present");}
void probe(GeometryViewer& v,const vec3& world,bool filled,const SurfaceMesh* model) {
    const auto p=v.camera()->projectedCoordinatesOf(world);const auto hit=v.pick(int(p.x),int(p.y));
    require(bool(hit)==filled,"polygon hole/fill picking mismatch");
    if (filled) require(hit.model.get()==model && hit.face_index>=0,"polygon source-face provenance missing");
}
int main(int argc,char**) {
    try {
        initialize(false,false,false);
        VulkanViewerOptions options;options.visible=false;options.width=800;options.height=600;
        options.device.validation=options.device.synchronization_validation=argc>1;
        GeometryViewer v("Native geometry tutorials",options);v.hide_axes();v.set_background_color(vec4(1));
        if (v.device().supported_sample_counts()&4) v.set_samples(4);
        create_curves_drawables(v);require(v.drawables().size()==6,"curves tutorial lost a curve family");frame(v);
        for (const auto& d:v.drawables()) require(dynamic_cast<LinesDrawable2D*>(d.get()) && d->has_cpu_buffers(),"curve drawable did not select native CPU mode");
        require(v.hint().find("line width")!=std::string::npos,"curves hint missing");
        auto width=[&](float expected) {
            for(const auto& d:v.drawables()) require(static_cast<LinesDrawable2D*>(d.get())->line_width()==expected,"curve width shortcut failed");
        };
        width(3);const auto initial=v.capture();
        v.key_press_event(TutorialViewer::KEY_RIGHT_BRACKET,0);width(4);
        require(v.capture().rgba!=initial.rgba,"curve width shortcut did not change rendering");
        v.key_press_event(TutorialViewer::KEY_LEFT_BRACKET,0);width(3);
        for(int i=0;i<25;++i) v.key_press_event(TutorialViewer::KEY_LEFT_BRACKET,0);width(1);
        for(int i=0;i<25;++i) v.key_press_event(TutorialViewer::KEY_RIGHT_BRACKET,0);width(20);
        v.key_press_event(TutorialViewer::KEY_0,0);width(3);
        require(v.capture().rgba==initial.rgba,"curve width reset failed");
        const auto curves=v.capture();unsigned colored=0;
        for (size_t i=0;i<curves.ids.size();++i) {
            require(curves.ids[i].object==UINT32_MAX && curves.depth[i]==1,"curve became a world picking surface");
            colored+=curves.rgba[4*i]!=255 || curves.rgba[4*i+1]!=255 || curves.rgba[4*i+2]!=255;
        }
        require(colored>500,"curve families did not render");
        v.camera()->setPosition(vec3(5,6,7));v.camera()->setType(Camera::ORTHOGRAPHIC);v.camera()->setViewDirection(vec3(1,0,0));
        require(v.capture().rgba==curves.rgba,"camera movement changed screen-space curves");
        require(v.snapshot("vulkan-curves.png",1,int(v.samples()),1),"curves snapshot failed");
        v.resize(600,450);frame(v);const auto resized=v.capture();require(resized.width!=curves.width,"curves did not resize");
        for (auto id:resized.ids) require(id.object==UINT32_MAX,"resized curves produced picking IDs");
        for (unsigned kind=0;kind<2;++kind) {
            v.clear_scene();auto mesh=kind==0 ? create_tessellation_mesh() : create_partition_mesh();
            require(mesh && mesh->n_faces()>3,"geometry algorithm produced no result");v.add_model(mesh);
            v.camera()->setUpVector(vec3(0,1,0));v.camera()->setViewDirection(vec3(0,0,-1));v.fit_screen();frame(v);
            for (unsigned samples:{1u,4u}) {
                if (!(v.device().supported_sample_counts()&samples)) continue;v.set_samples(samples);frame(v);
                if (kind==0) {
                    for (auto f:mesh->faces()) require(mesh->valence(f)==3,"tessellation left polygon faces");
                    probe(v,vec3(400,100,0),true,mesh.get());probe(v,vec3(100,700,0),false,mesh.get());
                    probe(v,vec3(1850,350,0),true,mesh.get());probe(v,vec3(2000,350,0),false,mesh.get());
                } else {
                    probe(v,vec3(50,350,0),true,mesh.get());probe(v,vec3(250,350,0),true,mesh.get());
                    probe(v,vec3(250,200,0),false,mesh.get());probe(v,vec3(250,500,0),false,mesh.get());
                }
                require(v.snapshot(kind==0?"vulkan-tessellation.png":"vulkan-partition.png",1,int(samples),1),"polygon snapshot failed");
            }
        }
        require(v.device().validation_errors()==0,"geometry tutorial validation errors");
        std::cout<<"PASS: shared 601/602/603 scene builders, tessellation/partition holes, face picking, curve families, camera-independent UI, resize and MSAA\n";
    } catch(const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
