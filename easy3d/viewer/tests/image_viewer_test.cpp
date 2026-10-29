// SPDX-License-Identifier: GPL-3.0-or-later
#include <tutorials/Tutorial_306_ImageViewer/viewer.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/initializer.h>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <iostream>
using namespace easy3d;
void require(bool value,const char* message) { if (!value) throw std::runtime_error(message); }
void frame(TutorialImageViewer& viewer) {
    for (unsigned attempt=0;attempt<100;++attempt) {
        VulkanViewer::poll_events();
        if (viewer.draw_frame()) return;
        glfwWaitEventsTimeout(0.01);
    }
    throw std::runtime_error("image viewer did not present within bounded retries");
}
void unpickable(const graphics::FrameImage& image) {
    for (size_t i=0;i<image.ids.size();++i)
        require(image.ids[i].object==UINT32_MAX && image.ids[i].primitive==UINT32_MAX && image.depth[i]==1,
                "screen-space image created a scene picking hit");
}
struct ImageViewer : TutorialImageViewer {
    using TutorialImageViewer::TutorialImageViewer;
    void hide_logo() {show_easy3d_logo_=false;}
};
int main(int argc,char**) {
    try {
        initialize(false,false,false);
        std::vector<unsigned char> wide(4*2*4,255),tall(2*4*4,255);
        for (unsigned i=0;i<8;++i) {
            wide[4*i]=i<4 ? 255 : 0; wide[4*i+1]=0; wide[4*i+2]=i<4 ? 0 : 255;
            tall[4*i+2]=0;
        }
        require(ImageIO::save("vulkan-image-wide.png",wide,4,2,4,false) &&
                ImageIO::save("vulkan-image-tall.png",tall,2,4,4,false),"could not save image-viewer fixtures");
        VulkanViewerOptions options; options.width=160; options.height=120;
        options.device.validation=options.device.synchronization_validation=argc>1;
        ImageViewer viewer("Native image viewer test","vulkan-image-wide.png",options);viewer.hide_logo();
        viewer.set_background_color(vec4(0,1,0,1));
        const bool msaa=(viewer.device().supported_sample_counts() & 4)!=0;
        if (msaa) viewer.set_samples(4);
        frame(viewer);
        const auto fitted=viewer.capture();
        const size_t top=(fitted.height/4)*fitted.width+fitted.width/2;
        const size_t bottom=(3*fitted.height/4)*fitted.width+fitted.width/2;
        require(fitted.rgba[top*4]>250 && fitted.rgba[top*4+2]<5 &&
                fitted.rgba[bottom*4+2]>250 && fitted.rgba[bottom*4]<5,"image viewer flipped the image");
        require(fitted.rgba[(fitted.width/2)*4+1]==255,"wide image did not preserve its aspect ratio/letterbox");
        unpickable(fitted);
        bool found=true;
        viewer.point_under_pixel(fitted.width/2,fitted.height/2,found);
        require(!found,"image overlay participated in world-point picking");
        auto scroll=glfwSetScrollCallback(viewer.window(),nullptr); glfwSetScrollCallback(viewer.window(),scroll);
        scroll(viewer.window(),0,-2); frame(viewer);
        const auto zoomed=viewer.capture();
        const size_t left=(zoomed.height/2)*zoomed.width+1;
        require(zoomed.rgba[4*left+1]==255 && fitted.rgba[4*left+1]<5,"wheel callback did not zoom image in framebuffer pixels");
        auto key=glfwSetKeyCallback(viewer.window(),nullptr); glfwSetKeyCallback(viewer.window(),key);
        key(viewer.window(),GLFW_KEY_F,0,GLFW_PRESS,0); frame(viewer);
        require(viewer.capture().rgba==fitted.rgba,"fit-image shortcut did not restore the view");
        viewer.resize(220,160); frame(viewer);
        auto resized=viewer.capture();
        require(resized.width!=fitted.width && resized.height!=fitted.height,"image test did not resize the framebuffer");
        require(resized.rgba[((resized.height/2)*resized.width+1)*4+1]==255,"resize did not recenter image at its current scale");
        key(viewer.window(),GLFW_KEY_F,0,GLFW_PRESS,0); frame(viewer);
        resized=viewer.capture();
        require(resized.rgba[((resized.height/2)*resized.width+1)*4+1]<5,"fit after resize used stale viewport dimensions");
        auto drop=glfwSetDropCallback(viewer.window(),nullptr); glfwSetDropCallback(viewer.window(),drop);
        const char* file="vulkan-image-tall.png";
        drop(viewer.window(),1,&file); frame(viewer);
        const auto reloaded=viewer.capture();
        const size_t center=(reloaded.height/2)*reloaded.width+reloaded.width/2;
        require(reloaded.rgba[4*center]==255 && reloaded.rgba[4*center+1]==255 && reloaded.rgba[4*center+2]==0,
                "image drop did not replace image pixels");
        require(reloaded.rgba[((reloaded.height/2)*reloaded.width+1)*4+1]==255 &&
                reloaded.rgba[((reloaded.height/2)*reloaded.width+1)*4]==0,"tall image did not fit with horizontal letterboxing");
        unpickable(reloaded);
        if (msaa) { viewer.set_samples(1); frame(viewer); unpickable(viewer.capture()); }
        require(viewer.device().validation_errors()==0,"native image viewer reported validation errors");
        std::cout << "PASS: native image orientation, fit/zoom/resize/reload, HiDPI, MSAA, no picking hits\n";
    } catch (const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
