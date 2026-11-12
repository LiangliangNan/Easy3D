// SPDX-License-Identifier: GPL-3.0-or-later
#include "test_window.h"
#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/ambient_occlusion.h>
#include <easy3d/renderer/average_color_blending.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/resource.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace easy3d;
void require(bool ok,const char* text) {if (!ok) throw std::runtime_error(text);}

// A file snapshot is only observable through the image it wrote.
std::vector<unsigned char> read_snapshot(const std::string& file,int width,int height) {
    std::vector<unsigned char> data; int w=0,h=0,channels=0;
    require(ImageIO::load(file,data,w,h,channels),"snapshot file could not be read");
    require(w==width && h==height,"snapshot size does not match the request");
    require(channels==4,"snapshot is not an RGBA image");
    require(data.size()==size_t(width)*size_t(height)*4,"snapshot data does not match its size");
    return data;
}

// The hint and the corner-axes Z arrow are pure blue; the gray knot and the
// pale/white backgrounds never are.
size_t count_blue(const std::vector<unsigned char>& pixels) {
    size_t count=0;
    for (size_t i=0;i+3<pixels.size();i+=4)
        if (pixels[i]<64 && pixels[i+1]<64 && pixels[i+2]>192) ++count;
    return count;
}

// A tutorial that brackets its effects inside draw(), as Tutorials 501 and 504
// do. Everything this function adds must therefore appear in a file snapshot.
struct EffectViewer : Viewer {
    using Viewer::Viewer;
    ~EffectViewer() override { delete occlusion_; delete blending_; }
    void enable_effects() { occlusion_ = new AmbientOcclusion(camera()); blending_ = new AverageColorBlending(camera()); }
    std::vector<TrianglesDrawable*> surfaces() const {
        std::vector<TrianglesDrawable*> result;
        for (auto& model : models_)
            for (auto& drawable : model->renderer()->triangles_drawables())
                result.push_back(dynamic_cast<TrianglesDrawable*>(drawable.get()));
        return result;
    }
    mutable int draws = 0;
    bool occlude = false, blend = false;
    void draw() const override {
        ++draws;
        if (occlude && occlusion_) {
            occlusion_->begin(models_);
            Viewer::draw();
            occlusion_->end();
            return;
        }
        if (blend && blending_) { blending_->draw(surfaces()); return; }
        Viewer::draw();
    }
    AmbientOcclusion* occlusion_ = nullptr;
    AverageColorBlending* blending_ = nullptr;
};

int main(int argc,char**) {
    try {
        initialize(false,false,false);
        TestWindow options;options.visible=false;options.width=320;options.height=240;
        options.device.validation=options.device.sync_validation=argc>1;
        EffectViewer v("Snapshot effects", options.width, options.height, options.samples, false, options.visible, options.resizable, options.device);
        v.enable_effects();
        require(v.add_model(resource::directory()+"/data/torusknot.obj"),"scene missing");
        for (auto* surface : v.surfaces()) surface->set_opacity(.5f);
        // A blue hint exercises the Viewer HUD: it belongs to the live frame and
        // capture(), but a file snapshot must not carry it (as on OpenGL).
        v.set_usage("Snapshot effects","No HUD in file snapshots");
        Viewer::poll_events();v.draw_frame();
        const int fw = v.framebuffer_width(), fh = v.framebuffer_height();
        require(fw>0 && fh>0,"test window has no framebuffer");
        const mat4 before = v.camera()->model_view_projection_matrix();
        const auto live = v.capture();

        // The plain frame is the reference the effects have to change.
        const int plain_calls = v.draws;
        require(v.snapshot("vulkan-snapshot-effects.png",1,1,1),"plain snapshot failed");
        require(v.draws>plain_calls,"the file snapshot did not run the virtual draw()");
        const auto plain = read_snapshot("vulkan-snapshot-effects.png",fw,fh);

        // The live frame carries the HUD (blue hint, corner axes); the file
        // snapshot of the same scene must be pixel-identical to the HUD-free
        // plain frame, on either backend.
        require(count_blue(live.rgba)>count_blue(plain),"the live frame lost its HUD");
        require(v.snapshot("vulkan-snapshot-effects-hud.png",1,1,1),"HUD snapshot failed");
        require(read_snapshot("vulkan-snapshot-effects-hud.png",fw,fh)==plain,
                "the file snapshot carried the Viewer HUD");

        // Viewer::capture() deliberately stays a raw scene frame: it renders the
        // command list, not draw(), so the same effects must not appear in it.
        v.occlude = true; v.blend = true;
        require(v.capture().rgba==live.rgba,"capture() is no longer a raw scene frame");
        v.occlude = false; v.blend = false;

        // Ambient occlusion is injected by Viewer::draw() from the parameters
        // published inside the tutorial's draw() override.
        v.occlude = true;
        require(v.snapshot("vulkan-snapshot-effects-ao.png",1,1,1),"occluded snapshot failed");
        require(read_snapshot("vulkan-snapshot-effects-ao.png",fw,fh)!=plain,
                "ambient occlusion is missing from the file snapshot");
        v.occlude = false;

        // The transparency method is chosen inside draw() as well, and reaches
        // the device through the same capture.
        v.blend = true;
        require(v.snapshot("vulkan-snapshot-effects-blended.png",1,1,1),"blended snapshot failed");
        require(read_snapshot("vulkan-snapshot-effects-blended.png",fw,fh)!=plain,
                "the transparency method of draw() is missing from the file snapshot");
        v.blend = false;

        // A scaled frame keeps the same content policy at another size, which
        // also exercises the camera override and its restoration.
        require(v.snapshot("vulkan-snapshot-effects-scaled.png",2,1,1),"scaled snapshot failed");
        read_snapshot("vulkan-snapshot-effects-scaled.png",fw*2,fh*2);

        // The default background is the viewer's current color, not white. The
        // top-right corner is clear of the centered scene and of every HUD element.
        require(v.snapshot("vulkan-snapshot-effects-default-bg.png",1,1),"default-background snapshot failed");
        const auto default_bg = read_snapshot("vulkan-snapshot-effects-default-bg.png",fw,fh);
        const size_t corner = (size_t(fw)-1)*4;
        const vec4 bg = v.background_color();
        for (unsigned c=0;c<4;++c) {
            const int diff = int(default_bg[corner+c]) - int(bg[c]*255);
            require(diff>=-2 && diff<=2,"the default snapshot background is not the viewer color");
        }
        require(plain[corner]==255 && plain[corner+1]==255 && plain[corner+2]==255,
                "the explicit white background was ignored");

        // The snapshot borrows the camera and the frame path; both are back. The camera
        // carries logical window dimensions on both backends (Batch 46), not the framebuffer.
        require(v.camera()->screen_width()==v.width() && v.camera()->screen_height()==v.height(),
                "the snapshot left the camera at the output size");
        const mat4 after = v.camera()->model_view_projection_matrix();
        for (unsigned i=0;i<16;++i) require(after[i]==before[i],"the snapshot left the projection overridden");
        Viewer::poll_events();v.draw_frame();
        require(v.capture().rgba==live.rgba,"the snapshot changed the live frame");
        require(v.device().validation_errors()==0,"snapshot validation errors");
        std::cout<<"PASS: file snapshots run the virtual draw(), include its effects and transparency method, exclude the Viewer HUD, default to the viewer background, restore the camera and the live frame\n";
    } catch (const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
