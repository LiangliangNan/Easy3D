// SPDX-License-Identifier: GPL-3.0-or-later
#include <tutorials/Tutorial_306_ImageViewer/viewer.h>
#include <tutorials/Tutorial_307_CrossSection/viewer.h>
#include <tutorials/Tutorial_309_TextRendering/viewer.h>
#include <tutorials/Tutorial_401_ModelPicker/viewer.h>
#include <tutorials/Tutorial_402_FacePicker/viewer.h>
#include <tutorials/Tutorial_403_PointSelection/viewer.h>
#include <tutorials/Tutorial_404_VirtualScanner/viewer.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/util/initializer.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
using namespace easy3d;
void require(bool ok,const char* message) { if (!ok) throw std::runtime_error(message); }
template<class Base> struct Inspect : Base {
    using Base::Base;
    const std::string& manual() const { return this->manual_; }
    const std::string& hint() const { return this->hint_; }
    void direct_hint(const std::string& text) { this->hint_=text; }
};
struct Console {
    std::ostringstream text;
    std::streambuf* previous;
    Console() : previous(std::cout.rdbuf(text.rdbuf())) {}
    ~Console() { std::cout.rdbuf(previous); }
};
void same_picking(const graphics::FrameImage& a,const graphics::FrameImage& b) {
    require(a.depth==b.depth && a.ids.size()==b.ids.size(),"usage HUD changed depth");
    for (size_t i=0;i<a.ids.size();++i)
        require(a.ids[i].object==b.ids[i].object && a.ids[i].primitive==b.ids[i].primitive,"usage HUD changed picking IDs");
}
int main(int argc,char**) {
    try {
        initialize(false,false,false);
        VulkanViewerOptions options; options.visible=false;options.width=480;options.height=300;
        options.device.validation=options.device.synchronization_validation=argc>1;
        Inspect<VulkanViewer> v("Usage HUD",options);
        require(!v.manual().empty() && v.hint().empty(),"default viewer usage missing");
        const auto baseline=v.capture();
        for (unsigned samples:{1u,4u}) {
            if (!(v.device().supported_sample_counts()&samples)) continue;
            v.set_samples(samples);
            v.set_usage("Custom manual","first hint line\nsecond hint line");
            const auto labeled=v.capture();same_picking(baseline,labeled);
            unsigned blue=0;for (size_t i=0;i<labeled.rgba.size();i+=4)
                blue+=labeled.rgba[i+2]>labeled.rgba[i]+20 && labeled.rgba[i+2]>labeled.rgba[i+1]+20;
            require(blue>50,"multiline blue usage hint was not rendered");
            v.set_overlays({});require(v.capture().rgba==labeled.rgba,"application overlay update erased usage hint");
            v.clear_scene();require(v.capture().rgba==labeled.rgba,"clearing scene lost persistent usage hint");
            v.direct_hint("replacement hint");require(v.capture().rgba!=labeled.rgba,"direct protected hint assignment kept stale text");
            v.set_usage("default","");require(v.manual()=="Custom manual","default sentinel reset custom manual");
            require(v.capture().rgba==baseline.rgba,"empty hint left stale text");
        }
        auto key=glfwSetKeyCallback(v.window(),nullptr);glfwSetKeyCallback(v.window(),key);
        {
            Console console;
            require(v.run_frames(1)==0,"usage startup frame failed");
            require(console.text.str()=="Custom manual\n","startup manual was not printed once");
            require(v.run_frames(1)==0 && console.text.str()=="Custom manual\n","startup manual repeated");
            key(v.window(),GLFW_KEY_F1,0,GLFW_PRESS,0);
            require(console.text.str()=="Custom manual\nCustom manual\n","F1 did not show current manual");
            v.set_usage("","");key(v.window(),GLFW_KEY_F1,0,GLFW_PRESS,0);
            require(console.text.str()=="Custom manual\nCustom manual\n","empty manual showed fallback help");
        }
        v.set_usage("Resized","resize-aware hint");const auto before=v.capture();
        v.resize(520,320);VulkanViewer::poll_events();v.draw_frame();const auto resized=v.capture();
        require(resized.width!=before.width && resized.rgba!=before.rgba,"usage HUD did not rebuild after resize");
        require(ImageIO::save("vulkan-usage-hint.png",resized.rgba,resized.width,resized.height,4,false),"usage artifact failed");
        require(v.device().validation_errors()==0,"usage HUD validation errors");
        // Every currently ported viewer with custom OpenGL text must retain it.
        {
            Inspect<TutorialVirtualScanner> scanner("scanner",options);
            require(scanner.manual().find("Virtual Scanner usage")!=std::string::npos &&
                    scanner.hint()=="press 'Space' to perform scanning\npress 'n' to toggle Gaussian noise", "scanner usage missing");
            const auto with_hint=scanner.capture();scanner.set_usage("default","");
            require(scanner.capture().rgba!=with_hint.rgba,"scanner hint not visible");
            require(scanner.device().validation_errors()==0,"scanner hint validation errors");
        }
        {Inspect<TutorialPointSelection> x("selection",options);require(x.hint().find("select (left)")!=std::string::npos,"selection hint missing");}
        {
            const bool enabled=ClippingPlane::instance()->is_enabled();
            {Inspect<CrossSection> x("clipping",options);require(x.hint().find("clipping plane")!=std::string::npos,"clipping hint missing");}
            ClippingPlane::instance()->set_enabled(enabled);
        }
        {Inspect<TutorialTextRendering> x("text",options);require(x.manual().find("Text Rendering usage")!=std::string::npos,"text manual missing");}
        {Inspect<TutorialModelPicker> x("models",options);require(x.hint().find("pick/unpick a model")!=std::string::npos,"model picker hint missing");}
        {Inspect<TutorialFacePicker> x("faces",options);require(x.hint().find("on a face")!=std::string::npos,"face picker hint missing");}
        require(ImageIO::save("vulkan-usage-image.png",std::vector<unsigned char>(4*16,255),4,4,4,false),"image fixture failed");
        {Inspect<TutorialImageViewer> x("image","vulkan-usage-image.png",options);
         require(x.manual().empty() && x.hint().find("open an image")!=std::string::npos,"image viewer usage missing");}
        std::cout << "PASS: manuals, startup/F1, all ported hints, empty/default semantics, HUD, resize and MSAA\n";
    } catch (const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
