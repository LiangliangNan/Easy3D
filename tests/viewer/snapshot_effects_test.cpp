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

// A tutorial that brackets its effects inside draw(), as Tutorials 501 and 504
// do. Everything this function adds must therefore appear in a file snapshot.
struct EffectViewer : Viewer {
    using Viewer::Viewer;
    ~EffectViewer() override { delete occlusion_; delete blending_; }
    void enable_effects() { occlusion_ = new AmbientOcclusion(camera()); blending_ = new AverageColorBlending(camera()); }
    std::vector<TrianglesDrawable*> surfaces() {
        std::vector<TrianglesDrawable*> result;
        for (auto& model : models_)
            for (auto& drawable : model->renderer()->triangles_drawables())
                result.push_back(dynamic_cast<TrianglesDrawable*>(drawable.get()));
        return result;
    }
    int draws = 0;
    bool occlude = false, blend = false;
    bool draw() override {
        ++draws;
        if (occlude && occlusion_) {
            occlusion_->begin(models_);
            const bool ok = Viewer::draw();
            occlusion_->end();
            return ok;
        }
        if (blend && blending_) { blending_->draw(surfaces()); return true; }
        return Viewer::draw();
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
        Viewer::poll_events();v.draw_frame();
        const int fw = v.framebuffer_width(), fh = v.framebuffer_height();
        require(fw>0 && fh>0,"test window has no framebuffer");
        const mat4 before = v.camera()->modelViewProjectionMatrix();
        const auto live = v.capture();

        // The plain frame is the reference the effects have to change.
        const int plain_calls = v.draws;
        require(v.snapshot("vulkan-snapshot-effects.png",1,1,1),"plain snapshot failed");
        require(v.draws>plain_calls,"the file snapshot did not run the virtual draw()");
        const auto plain = read_snapshot("vulkan-snapshot-effects.png",fw,fh);

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

        // The snapshot borrows the camera and the frame path; both are back.
        require(v.camera()->screenWidth()==fw && v.camera()->screenHeight()==fh,
                "the snapshot left the camera at the output size");
        const mat4 after = v.camera()->modelViewProjectionMatrix();
        for (unsigned i=0;i<16;++i) require(after[i]==before[i],"the snapshot left the projection overridden");
        Viewer::poll_events();v.draw_frame();
        require(v.capture().rgba==live.rgba,"the snapshot changed the live frame");
        require(v.device().validation_errors()==0,"snapshot validation errors");
        std::cout<<"PASS: file snapshots run the virtual draw(), include its effects and transparency method, restore the camera and the live frame\n";
    } catch (const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
