// SPDX-License-Identifier: GPL-3.0-or-later
#include <tests/test.h>
#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/fileio/image_io.h>
#include <cmath>
using namespace easy3d;
using test::require;
struct HudView:Viewer {using Viewer::Viewer;void hide_hud(){show_easy3d_logo_=false;set_usage("");}};
struct Pixels {
    int w=0,h=0,c=0;std::vector<unsigned char> rgba;
    explicit Pixels(const std::string& path){require(ImageIO::load(path,rgba,w,h,c,4,false),"cannot load HUD capture");}
    const unsigned char* at(int x,int y)const{return rgba.data()+4*(size_t(y)*w+x);}
};
// Rotated so the silhouette stays diagonal at any framebuffer size. Axis-aligned
// edges can land between the 4x sample positions, yielding no partially covered
// pixels at all and making the coverage checks depend on the window resolution.
// The radius keeps the horizontal reach at the quad's center close to the
// previously axis-aligned 0.8 while its corners stay inside the viewport.
graphics::DrawCommand quad(float z,const vec4& color) {
    std::vector<graphics::Vertex> vertices(4);
    const float radius=.75f,angle=.35f,c=std::cos(angle),s=std::sin(angle);
    const float corners[4][2]={{-1,-1},{1,-1},{1,1},{-1,1}};
    for(unsigned i=0;i<4;++i) {
        const float x=radius*corners[i][0],y=radius*corners[i][1];
        vertices[i].position=vec3(x*c-y*s,x*s+y*c,z);vertices[i].normal=vec3(0,0,1);
    }
    graphics::DrawCommand d;d.geometry=std::make_shared<graphics::Geometry>(vertices,std::vector<uint32_t>{0,1,2,0,2,3});
    d.color=color;d.overlay_independent_depth=true;return d;
}
void near_color(const unsigned char* p,int r,int g,int b,int a,const char* message){
    require(std::abs(int(p[0])-r)<=3 && std::abs(int(p[1])-g)<=3 && std::abs(int(p[2])-b)<=3 && std::abs(int(p[3])-a)<=3,message);
}
int main(int argc,char**) {return test::run("independent 3D HUD depth, lighting and captures",[&]{
#ifdef EASY3D_HAS_VULKAN
    VulkanOptions options;options.validation=options.sync_validation=argc>1;
#else
    OpenGLOptions options;
#endif
    HudView v("Independent HUD",161,121,4,false,false,true,options);v.hide_hud();
    v.camera()->set_view_direction(vec3(0,0,-1));v.camera()->set_up_vector(vec3(0,1,0));
    v.camera()->set_type(Camera::ORTHOGRAPHIC);v.camera()->set_scene_bounding_box(vec3(-1),vec3(1));v.camera()->show_entire_scene();
    auto scene=std::make_shared<TrianglesDrawable>("scene");
    scene->update_vertex_buffer({vec3(-10,-10,0),vec3(10,-10,0),vec3(0,10,0)});
    scene->set_uniform_coloring(vec4(0,0,1,1));scene->set_lighting(false);v.add_drawable(scene);
    const int w=v.framebuffer_width(),h=v.framebuffer_height();
    const auto before=v.pick(v.width()/2,v.height()/2);require(bool(before),"missing source scene");
    auto front=quad(-.5f,vec4(1,0,0,.5f)),back=quad(.5f,vec4(0,1,0,1));
    auto flat=quad(0,vec4(0,1,0,.25f));flat.overlay_independent_depth=false;
    // The rear opaque quad is submitted last, but must fail HUD depth testing.
    // The flat overlay is submitted first, but must composite after the 3D HUD.
    v.set_overlays({flat,front,back});require(v.run_frames(2)==0,"HUD frame failed");
    const auto after=v.pick(v.width()/2,v.height()/2);
    require(after.drawable==before.drawable && std::abs(after.depth-before.depth)<1e-6f && distance(after.position,before.position)<1e-5f,"HUD changed scene picking/depth");
    for(int samples:{1,4}) for(float scale:{1.f,1.5f,2.f}){
        require(v.snapshot("hud.png",scale,samples,1,true),"HUD snapshot failed");Pixels image("hud.png");
        near_color(image.at(image.w/2,image.h/2),96,64,96,255,"HUD ordering/depth/alpha or tile composition differs");
        near_color(image.at(0,0),0,0,255,255,"HUD escaped its projected bounds");
    }
    scene->set_visible(false);v.set_overlays({front,back});
    require(v.snapshot("transparent.png",1.5f,4,2,true),"transparent HUD snapshot failed");Pixels transparent("transparent.png");
    near_color(transparent.at(transparent.w/2,transparent.h/2),255,0,0,128,"HUD transparent export is not straight alpha");
    near_color(transparent.at(0,0),0,0,0,0,"HUD target clear leaked into transparent background");
    front.color=vec4(1);front.lighting=true;v.set_overlays({front});
    require(v.snapshot("lit.png",1,4,1,true),"lit HUD snapshot failed");Pixels lit("lit.png");
    const int bright=int(std::lround(255*(.2+.8/std::sqrt(1.34))));
    near_color(lit.at(lit.w/2,lit.h/2),bright,bright,bright,255,"HUD basic diffuse lighting differs");
    front.normal_matrix(2,2)=0;front.normal_matrix(0,2)=1;v.set_overlays({front});
    require(v.snapshot("normal.png",1.5f,4,1,true),"transformed HUD snapshot failed");Pixels normal("normal.png");
    const int dark=int(std::lround(255*(.2+.8*.3/std::sqrt(1.34))));
    near_color(normal.at(normal.w/2,normal.h/2),dark,dark,dark,255,"HUD ignored normal transform");
    front.normal_matrix=mat3::identity();front.material_lighting=true;front.light=vec3(0,0,-1);
    front.eye=vec3(0,0,4);front.ambient=front.specular=vec4(0);front.color=vec4(1,0,0,1);
    for(bool two_sides:{false,true}) {
        front.lighting_two_sides=two_sides;v.set_overlays({front});
        require(v.snapshot("material.png",1,4,1,true),"material HUD snapshot failed");Pixels material("material.png");
        near_color(material.at(material.w/2,material.h/2),two_sides?255:0,0,0,255,"HUD material two-sided lighting differs");
    }
    front.light=vec3(0,0,1);front.color=vec4(.5f,0,0,1);front.ambient=vec4(0,.1f,0,0);
    front.specular=vec4(0,0,.25f,0);front.shininess=0;v.set_overlays({front});
    require(v.snapshot("specular.png",1,4,1,true),"specular HUD snapshot failed");Pixels specular("specular.png");
    near_color(specular.at(specular.w/2,specular.h/2),128,26,64,255,"HUD ambient/specular material differs");
    front.light=vec3(0,0,-1);front.ambient=front.specular=vec4(0);
    front.distinct_back_color=true;front.back_color=vec4(0,1,0,1);
    auto reversed=front.geometry->indices();for(size_t i=0;i<reversed.size();i+=3)std::swap(reversed[i],reversed[i+1]);
    front.geometry=std::make_shared<graphics::Geometry>(front.geometry->vertices(),reversed);v.set_overlays({front});
    require(v.snapshot("backside.png",1.5f,4,1,true),"backside HUD snapshot failed");Pixels backside("backside.png");
    near_color(backside.at(backside.w/2,backside.h/2),0,255,0,255,"HUD ignored distinct back color or face orientation");
    front.material_lighting=false;front.lighting=false;front.color=vec4(0,0,0,1);v.set_overlays({front});
    size_t edge_samples[2]={};unsigned sample_index=0;
    for(int samples:{1,4}) {
        require(v.snapshot("edges.png",1,samples,1,true),"HUD edge snapshot failed");Pixels edges("edges.png");
        for(size_t i=0;i<size_t(edges.w)*(edges.h/3)*4;i+=4) if(edges.rgba[i]>3 && edges.rgba[i]<252)++edge_samples[sample_index];
        ++sample_index;
    }
    require(edge_samples[1]>edge_samples[0],"HUD MSAA silhouette coverage was lost during composition");
    // Coincident scene/HUD silhouettes must blend each covered sample before
    // resolving. An early HUD resolve produces unequal red/blue edge channels.
    front=quad(-.5f,vec4(1,0,0,.5f));v.set_overlays({front});
    std::vector<vec3> silhouette;
    const auto inv=inverse(v.camera()->model_view_projection_matrix());
    for(unsigned i:{0u,1u,2u,0u,2u,3u}) {
        vec3 p=front.geometry->vertices()[i].position;p.z=0;silhouette.push_back(inv*p);
    }
    scene->update_vertex_buffer(silhouette);scene->set_visible(true);
    require(v.snapshot("correlated.png",1,4,1,true),"correlated MSAA HUD snapshot failed");Pixels correlated("correlated.png");
    size_t partial=0;
    for(int y=0;y<correlated.h/3;++y) for(int x=0;x<correlated.w;++x) {
        const auto p=correlated.at(x,y);
        if(p[1]>3 && p[1]<252) {++partial;require(std::abs(int(p[0])-int(p[2]))<=3,"HUD resolved before blending correlated scene samples");}
    }
    require(partial>0,"correlated MSAA test has no partial-coverage pixels");
    scene->set_visible(false);front.color=vec4(1,0,0,1);
    // Geometry stays the same while viewport, lighting, depth and foreground metadata change.
    front.lighting=false;front.viewport=ivec4(w/2,0,w-w/2,h);v.set_overlays({front},true);
    v.resize(201,151);require(v.run_frames(2)==0,"HUD resize frame failed");
    require(v.snapshot("resized.png",1.5f,4,1,true),"resized HUD snapshot failed");Pixels resized("resized.png");
    near_color(resized.at(resized.w*3/4,resized.h/2),255,0,0,255,"HUD resize/cache metadata differs");
    near_color(resized.at(resized.w/4,resized.h/8),255,255,255,255,"resized HUD escaped its viewport");
    bool found=true;v.point_under_pixel(v.width()*3/4,v.height()/2,found);require(!found,"3D HUD leaked into surface query");
    v.set_overlays({});require(v.run_frames(1)==0,"HUD clear frame failed");
    {
        MultiViewer grid(1,2,"Independent grid HUD",201,151,4,false,false,true,options);
        grid.set_usage("");grid.set_division_visible(false);
        front.viewport=ivec4(0);front.color=vec4(1,0,0,.5f);back.color=vec4(0,1,0,1);
        for(bool foreground:{false,true}) {
            grid.set_overlays({front,back},foreground);require(grid.run_frames(2)==0,"grid HUD frame failed");
            require(grid.snapshot("grid.png",1.5f,4,1,true),"grid HUD snapshot failed");Pixels image("grid.png");
            near_color(image.at(image.w/2,image.h/2),255,128,128,255,"grid HUD depth/alpha composition differs");
            require(!grid.pick(grid.width()/2,grid.height()/2),"grid HUD produced a scene source");
        }
        require(grid.validation_errors()==0,"grid HUD validation errors");
    }
    require(v.validation_errors()==0,"HUD validation errors");
});}
