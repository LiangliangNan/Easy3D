// SPDX-License-Identifier: GPL-3.0-or-later
#include <tutorials/Tutorial_309_TextRendering/viewer_vulkan.h>
#include <easy3d/util/initializer.h>
#include <iostream>
using namespace easy3d;
void require(bool value,const char* message) { if (!value) throw std::runtime_error(message); }
void frame(TutorialTextRendering& viewer) {
    for (unsigned i=0;i<100;++i) {
        VulkanViewer::poll_events();
        if (viewer.draw_frame()) return;
        glfwWaitEventsTimeout(0.01);
    }
    throw std::runtime_error("text viewer did not present");
}
void same_picking(const graphics::FrameImage& a,const graphics::FrameImage& b) {
    require(a.depth==b.depth && a.ids.size()==b.ids.size(),"text controls changed depth");
    for (std::size_t i=0;i<a.ids.size();++i)
        require(a.ids[i].object==b.ids[i].object && a.ids[i].primitive==b.ids[i].primitive,"text controls changed picking IDs");
}
int main(int argc,char**) {
    try {
        initialize(false,false,false);
        VulkanViewerOptions options; options.width=900; options.height=800;
        options.device.validation=options.device.synchronization_validation=argc>1;
        TutorialTextRendering viewer("Native text viewer test",options);
        require(viewer.add_model(resource::directory()+"/data/sphere.obj")!=nullptr,"could not load tutorial sphere");
        viewer.fit_screen();
        if (viewer.device().supported_sample_counts()&4) viewer.set_samples(4);
        frame(viewer);
        const auto baseline=viewer.capture();
        auto key=glfwSetKeyCallback(viewer.window(),nullptr); glfwSetKeyCallback(viewer.window(),key);
        auto press=[&](int code) { key(viewer.window(),code,0,GLFW_PRESS,0); frame(viewer); };
        for (const auto pair: {std::pair<int,int>{GLFW_KEY_EQUAL,GLFW_KEY_MINUS},
                              {GLFW_KEY_PERIOD,GLFW_KEY_COMMA},{GLFW_KEY_UP,GLFW_KEY_DOWN},
                              {GLFW_KEY_L,GLFW_KEY_C},{GLFW_KEY_R,GLFW_KEY_C},
                              {GLFW_KEY_O,GLFW_KEY_O},{GLFW_KEY_SPACE,GLFW_KEY_SPACE}}) {
            std::cout << "checking shortcut " << pair.first << " / " << pair.second << std::endl;
            press(pair.first);
            const auto edited=viewer.capture();
            require(edited.rgba!=baseline.rgba,"text shortcut did not affect rendered text");
            same_picking(baseline,edited);
            press(pair.second);
            const auto restored=viewer.capture();
            unsigned differences=0,maximum=0;
            for (std::size_t i=0;i<baseline.rgba.size();++i) {
                unsigned d=unsigned(std::abs(int(restored.rgba[i])-int(baseline.rgba[i])));
                differences+=d>0; maximum=std::max(maximum,d);
            }
            if (differences) std::cout << "restored differences " << differences << " max " << maximum << std::endl;
            require(restored.rgba==baseline.rgba,"text shortcut did not restore original text");
        }
        viewer.resize(760,700); frame(viewer);
        const auto resized=viewer.capture();
        require(resized.width!=baseline.width && resized.height!=baseline.height,"text viewer did not resize");
        require(viewer.snapshot("vulkan-text.png"),"could not save text tutorial snapshot");
        viewer.set_samples(1); frame(viewer);
        const auto single=viewer.capture(); same_picking(resized,single);
        bool found=false; viewer.point_under_pixel(single.width/2,single.height/2,found);
        require(found,"text overlays blocked sphere world-point picking");
        require(viewer.device().validation_errors()==0,"text viewer validation errors");
        std::cout << "PASS: native text controls, resize/HiDPI, MSAA, sphere world-point picking, snapshot\n";
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
