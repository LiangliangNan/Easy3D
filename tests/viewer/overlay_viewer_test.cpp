// SPDX-License-Identifier: GPL-3.0-or-later
#include <tests/test.h>
#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/text_layout.h>
#include <easy3d/util/resource.h>
#include <easy3d/fileio/image_io.h>
#include <tutorials/Tutorial_306_ImageViewer/viewer.h>
#include <tutorials/Tutorial_309_TextRendering/viewer.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <cmath>
using namespace easy3d;
using test::require;
#ifndef EASY3D_HAS_VULKAN
#include <easy3d/renderer/opengl.h>
#endif
struct AnnotationView:Viewer {using Viewer::Viewer;using Viewer::draw;void hide_logo(){show_easy3d_logo_=false;}};
struct ImageView:TutorialImageViewer {using TutorialImageViewer::TutorialImageViewer;using TutorialImageViewer::mouse_scroll_event;using TutorialImageViewer::drop_event;};
struct TextView:TutorialTextRendering {using TutorialTextRendering::TutorialTextRendering;using TutorialTextRendering::key_press_event;};
struct Pixels {
    std::vector<unsigned char> rgba;int w=0,h=0,c=0;
    explicit Pixels(const std::string& file) {require(ImageIO::load(file,rgba,w,h,c,4,false),"cannot read overlay snapshot");}
    const unsigned char* at(int x,int y) const {return rgba.data()+4*(size_t(y)*w+x);}
};
void frame(Viewer& v) {require(v.run_frames(2)==0,"overlay viewer frame failed");}
int main(int argc,char**) {return test::run("shared annotations and image/text tutorials",[&] {
#ifdef EASY3D_HAS_VULKAN
    VulkanOptions options;options.validation=options.sync_validation=argc>1;
#else
    OpenGLOptions options;
#endif
    AnnotationView v("Annotations",161,121,4,false,false,true,options);v.set_usage("");v.hide_logo();
    v.camera()->setViewDirection(vec3(0,0,-1));v.camera()->setUpVector(vec3(0,1,0));
    v.camera()->setType(Camera::ORTHOGRAPHIC);v.camera()->setSceneBoundingBox(vec3(-1),vec3(1));v.camera()->showEntireScene();
    const int w=v.framebuffer_width(),h=v.framebuffer_height();
    std::vector<graphics::Vertex> vertices(4);
    const vec3 p[]={vec3(0,0,0),vec3(float(w),0,0),vec3(float(w),float(h),0),vec3(0,float(h),0)};
    const vec2 uv[]={vec2(0,0),vec2(1,0),vec2(1,1),vec2(0,1)};
    for(unsigned i=0;i<4;++i) {vertices[i].position=p[i];vertices[i].texcoord=uv[i];}
    graphics::DrawCommand d;d.geometry=std::make_shared<graphics::Geometry>(vertices,std::vector<uint32_t>{0,1,2,0,2,3});
    d.model_view_projection=graphics::screen_space_projection(w,h);d.color=vec4(1,0,0,.5f);
    // Clip to the right half; its local MVP covers the viewport, not the complete canvas.
    d.viewport=ivec4(w/2,0,w-w/2,h);v.set_overlays({d});frame(v);
    bool found=true;v.point_under_pixel(v.width()*3/4,v.height()/2,found);require(!found,"annotation wrote scene depth");
    require(!v.pick(w*3/4,h/2),"annotation produced a picking source");
    for(float scale:{.5f,1.0f,1.5f,2.0f}) {
        require(v.snapshot("annotation.png",scale,4,1,true),"annotation snapshot failed");Pixels image("annotation.png");
        require(image.w==int(std::ceil(w*scale)) && image.h==int(std::ceil(h*scale)),"annotation snapshot dimensions differ");
        const auto left=image.at(image.w/4,image.h/2),right=image.at(3*image.w/4,image.h/2);
        require(left[0]>250 && left[1]>250 && left[2]>250,"annotation escaped its viewport");
        require(right[0]>250 && std::abs(int(right[1])-127)<=2 && std::abs(int(right[2])-127)<=2,"annotation alpha/viewport differs across snapshot tiles");
    }

    d.viewport=ivec4(21,13,43,37);v.set_overlays({d});frame(v);
    require(v.snapshot("clip-edge.png",1.5f,1,1,true),"fractional viewport snapshot failed");Pixels edge("clip-edge.png");
    require(edge.at(95,48)[1]<130 && edge.at(96,48)[1]>250,"scaled viewport right edge differs from Vulkan rounding");
    d.viewport=ivec4(w/2,0,w-w/2,h);v.set_overlays({d});frame(v);
    require(v.snapshot("alpha.png",1.5f,4,2,true),"transparent annotation snapshot failed");Pixels alpha("alpha.png");
    const auto empty=alpha.at(alpha.w/4,alpha.h/2),translucent=alpha.at(3*alpha.w/4,alpha.h/2);
    require(empty[3]==0 && translucent[0]>250 && translucent[1]<3 && translucent[2]<3 &&
            std::abs(int(translucent[3])-128)<=2,"transparent annotation output is not straight alpha");
    d.viewport=ivec4(0);d.color=vec4(1);
    d.texture=std::make_shared<graphics::Image>(2,2,std::vector<unsigned char>{255,0,0,255,0,255,0,255,0,0,255,255,255,255,255,255});
    v.set_overlays({d});frame(v);
    require(v.snapshot("texture.png",1.5f,4,1,true),"texture annotation snapshot failed");Pixels texture("texture.png");
    const auto red=texture.at(texture.w/4,texture.h/4),blue=texture.at(texture.w/4,3*texture.h/4);
    require(red[0]>250 && red[1]<3 && red[2]<3 && blue[0]<3 && blue[1]<3 && blue[2]>250,"annotation texture orientation or tiled UVs differ");
#ifndef EASY3D_HAS_VULKAN
    GLuint unpack=0,sampler=0;glGenBuffers(1,&unpack);glBindBuffer(GL_PIXEL_UNPACK_BUFFER,unpack);glBufferData(GL_PIXEL_UNPACK_BUFFER,16,nullptr,GL_STATIC_DRAW);
    glPixelStorei(GL_UNPACK_ALIGNMENT,8);glPixelStorei(GL_UNPACK_ROW_LENGTH,13);glPixelStorei(GL_UNPACK_SKIP_ROWS,2);
    glActiveTexture(GL_TEXTURE3);glViewport(3,4,31,27);glColorMask(GL_FALSE,GL_TRUE,GL_FALSE,GL_TRUE);glDepthFunc(GL_GREATER);
    if(GLEW_VERSION_3_3 || GLEW_ARB_sampler_objects) {glGenSamplers(1,&sampler);glBindSampler(0,sampler);}
    d.geometry=std::make_shared<graphics::Geometry>(vertices,std::vector<uint32_t>{0,1,2,0,2,3});
    v.set_overlays({d});v.draw();
    GLint active=0,vp[4],binding=0,row=0,skip=0,alignment=0,depth=0;GLboolean mask[4];
    glGetIntegerv(GL_ACTIVE_TEXTURE,&active);glGetIntegerv(GL_VIEWPORT,vp);glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING,&binding);
    glGetIntegerv(GL_UNPACK_ROW_LENGTH,&row);glGetIntegerv(GL_UNPACK_SKIP_ROWS,&skip);glGetIntegerv(GL_UNPACK_ALIGNMENT,&alignment);
    glGetIntegerv(GL_DEPTH_FUNC,&depth);glGetBooleanv(GL_COLOR_WRITEMASK,mask);
    require(active==GL_TEXTURE3 && vp[0]==3 && vp[1]==4 && vp[2]==31 && vp[3]==27 && GLuint(binding)==unpack &&
            row==13 && skip==2 && alignment==8 && depth==GL_GREATER && !mask[0] && mask[1] && !mask[2] && mask[3],"annotations leaked OpenGL state");
    if(sampler) {GLint bound=0;glActiveTexture(GL_TEXTURE0);glGetIntegerv(GL_SAMPLER_BINDING,&bound);require(GLuint(bound)==sampler,"annotations leaked sampler binding");glBindSampler(0,0);glDeleteSamplers(1,&sampler);}
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER,0);glDeleteBuffers(1,&unpack);glPixelStorei(GL_UNPACK_ALIGNMENT,4);glPixelStorei(GL_UNPACK_ROW_LENGTH,0);glPixelStorei(GL_UNPACK_SKIP_ROWS,0);
    glActiveTexture(GL_TEXTURE0);glViewport(0,0,w,h);glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);glDepthFunc(GL_LESS);
#endif
    graphics::TextLayoutEngine text; text.add_font(resource::directory()+"/fonts/en_Roboto-Medium.ttf");
    graphics::TextOptions o;o.x=10;o.y=10;o.size=24;o.color=vec4(0,0,0,1);
    auto layout=text.layout(u8"Easy3D — Vulkan",w,h,o);v.set_overlays(layout.commands,true);frame(v);
    require(v.snapshot("text-overlay.png",1,1,1,true),"foreground text snapshot failed");
    Pixels text_image("text-overlay.png");size_t dark=0;
    for(size_t i=0;i<text_image.rgba.size();i+=4) if(text_image.rgba[i]<200 && text_image.rgba[i+1]<200 && text_image.rgba[i+2]<200) ++dark;
    require(dark>20,"foreground font atlas was not rendered");
    v.set_overlays({});frame(v);require(v.validation_errors()==0,"annotation validation errors");
    {
        MultiViewer grid(1,2,"Shared grid annotations",201,151,4,false,false,true,options);
        grid.set_usage("");grid.set_division_visible(false);
        for(bool foreground:{false,true}) {
            grid.set_overlays({d},foreground);frame(grid);
            require(grid.snapshot("grid-annotations.png",1.5f,4,1,true),"multi-view annotation snapshot failed");
            Pixels image("grid-annotations.png");
            const auto red=image.at(image.w/4,image.h/4),blue=image.at(image.w/4,3*image.h/4);
            require(red[0]>250 && red[1]<3 && blue[2]>250 && blue[0]<3,"multi-view annotation viewport/foreground differs");
            bool found=true;grid.point_under_pixel(grid.width()/4,grid.height()/2,found);
            require(!found,"multi-view annotation changed the scene query");
        }
    }
    const std::string input="input.png";
    require(ImageIO::save(input,std::vector<unsigned char>{255,0,0,255,0,255,0,255,0,0,255,255,255,255,255,255},2,2,4,false),"cannot create image fixture");
    {
        ImageView image("Shared image tutorial",input);frame(image);
        image.mouse_scroll_event(0,0,0,2);frame(image);
        require(!image.drop_event({"missing-image.png"}),"invalid drop replaced image");
        require(image.drop_event({input}),"image drop failed");image.resize(201,151);frame(image);
        require(image.snapshot("image-tutorial.png",1.5f,4,1,true),"shared image tutorial snapshot failed");
    }
    {
        TextView text_view("Shared text tutorial");frame(text_view);
        for(int key:{GLFW_KEY_EQUAL,GLFW_KEY_PERIOD,GLFW_KEY_UP,GLFW_KEY_R,GLFW_KEY_O,GLFW_KEY_SPACE}) require(text_view.key_press_event(key,0),"text control was not handled");
        text_view.resize(501,401);frame(text_view);
        require(text_view.snapshot("text-tutorial.png",1.5f,4,1,true),"shared text tutorial snapshot failed");
    }
});}
