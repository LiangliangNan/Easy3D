// SPDX-License-Identifier: GPL-3.0-or-later
#include "test_window.h"
#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/ambient_occlusion.h>
#include <easy3d/renderer/average_color_blending.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/renderer/soft_shadow.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/resource.h>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace easy3d;
void require(bool ok,const char* text) {if (!ok) throw std::runtime_error(text);}

std::string output_path(const std::string& directory,const std::string& file) {
    return directory.empty() || directory=="." ? file : directory+"/"+file;
}

// A file snapshot is only observable through the image it wrote.
std::vector<unsigned char> read_snapshot(const std::string& file,int width,int height) {
    std::vector<unsigned char> data; int w=0,h=0,channels=0;
    require(ImageIO::load(file,data,w,h,channels),"snapshot file could not be read");
    require(w==width && h==height,"snapshot size does not match the request");
    require(channels==4,"snapshot is not an RGBA image");
    require(data.size()==size_t(width)*size_t(height)*4,"snapshot data does not match its size");
    return data;
}

void write_rgba(const std::string& file,const std::vector<unsigned char>& rgba,int width,int height) {
    std::ofstream output(file.c_str(),std::ios::binary);
    require(bool(output),"could not create comparison image");
    output << "EASY3D_RGBA 1 " << width << ' ' << height << '\n';
    output.write(reinterpret_cast<const char*>(rgba.data()),std::streamsize(rgba.size()));
    require(bool(output),"could not write comparison image");
}

// The hint and the corner-axes Z arrow are pure blue; the room and the
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
    ~EffectViewer() override { delete occlusion_; delete blending_; delete shadow_; }
    void enable_effects() {
        occlusion_ = new AmbientOcclusion(camera());
        blending_ = new AverageColorBlending(camera());
        shadow_ = new SoftShadow(camera());
        shadow_->set_virtual_background_color(background_color());
    }
    std::vector<TrianglesDrawable*> surfaces() const {
        std::vector<TrianglesDrawable*> result;
        for (auto& model : models_)
            for (auto& drawable : model->renderer()->triangles_drawables())
                result.push_back(dynamic_cast<TrianglesDrawable*>(drawable.get()));
        return result;
    }
    mutable int draws = 0;
    bool occlude = false, blend = false, cast_shadow = false;
    void draw() const override {
        ++draws;
        if (occlude && occlusion_) {
            occlusion_->begin(models_);
            Viewer::draw();
            occlusion_->end();
            return;
        }
        if (cast_shadow && shadow_) {
            shadow_->begin(surfaces());
            Viewer::draw();
            shadow_->end();
            return;
        }
        if (blend && blending_) { blending_->draw(surfaces()); return; }
        Viewer::draw();
    }
    AmbientOcclusion* occlusion_ = nullptr;
    AverageColorBlending* blending_ = nullptr;
    SoftShadow* shadow_ = nullptr;
};

int main(int argc,char** argv) {
    try {
        bool validation=false;
        std::string output_dir=".";
        for (int i=1;i<argc;++i) {
            const std::string argument=argv[i];
            if (argument=="--validation")
                validation=true;
            else if (argument=="--output-dir" && i+1<argc)
                output_dir=argv[++i];
            else
                throw std::invalid_argument("unexpected snapshot-effects test argument: "+argument);
        }
        initialize(false,false,false);
#ifdef EASY3D_HAS_VULKAN
        VulkanOptions options;
        options.validation=options.sync_validation=validation;
#else
        OpenGLOptions options;
        (void)validation;
#endif
        const int width=320,height=240,samples=1;
        EffectViewer v("Snapshot effects",width,height,samples,false,false,true,options);
        v.enable_effects();
        require(v.add_model(resource::directory()+"/data/room.obj"),"scene missing");
        v.camera()->set_up_vector(vec3(0,1,0));
        v.camera()->set_view_direction(vec3(0,0,1));
        v.fit_screen();
        // Keep texture/material selection from obscuring backend effect parity.
        for (auto* surface : v.surfaces()) {
            surface->set_coloring_method(State::UNIFORM_COLOR);
            surface->set_color(vec4(.7f,.5f,.3f,1.0f));
        }
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
        require(v.snapshot(output_path(output_dir,"cross-backend-plain.png"),1,1,1),"plain snapshot failed");
        require(v.draws>plain_calls,"the file snapshot did not run the virtual draw()");
        const auto plain = read_snapshot(output_path(output_dir,"cross-backend-plain.png"),fw,fh);
        write_rgba(output_path(output_dir,"plain.rgba"),plain,fw,fh);

        // The live frame carries the HUD (blue hint, corner axes); the file
        // snapshot of the same scene must be pixel-identical to the HUD-free
        // plain frame, on either backend.
        require(count_blue(live.rgba)>count_blue(plain),"the live frame lost its HUD");
        require(v.snapshot(output_path(output_dir,"cross-backend-hud.png"),1,1,1),"HUD snapshot failed");
        require(read_snapshot(output_path(output_dir,"cross-backend-hud.png"),fw,fh)==plain,
                "the file snapshot carried the Viewer HUD");

        // Viewer::capture() deliberately stays a raw scene frame: it renders the
        // command list, not draw(), so the same effects must not appear in it.
        v.occlude = true; v.blend = true;
        require(v.capture().rgba==live.rgba,"capture() is no longer a raw scene frame");
        v.occlude = false; v.blend = false;

        // Ambient occlusion is injected by Viewer::draw() from the parameters
        // published inside the tutorial's draw() override.
        v.occlude = true;
        require(v.snapshot(output_path(output_dir,"cross-backend-ao.png"),1,1,1),"occluded snapshot failed");
        const auto ao=read_snapshot(output_path(output_dir,"cross-backend-ao.png"),fw,fh);
        require(ao!=plain,
                "ambient occlusion is missing from the file snapshot");
        write_rgba(output_path(output_dir,"ambient-occlusion.rgba"),ao,fw,fh);
        v.occlude = false;

        v.cast_shadow = true;
        require(v.snapshot(output_path(output_dir,"cross-backend-shadow.png"),1,1,1),"shadow snapshot failed");
        const auto shadow=read_snapshot(output_path(output_dir,"cross-backend-shadow.png"),fw,fh);
        require(shadow!=plain,"soft shadows are missing from the file snapshot");
        write_rgba(output_path(output_dir,"soft-shadow.rgba"),shadow,fw,fh);
        v.cast_shadow = false;

        // The transparency method is chosen inside draw() as well, and reaches
        // the device through the same capture.
        v.blend = true;
        for (auto* surface : v.surfaces()) surface->set_opacity(.5f);
        require(v.snapshot(output_path(output_dir,"cross-backend-blended.png"),1,1,1),"blended snapshot failed");
        const auto blended=read_snapshot(output_path(output_dir,"cross-backend-blended.png"),fw,fh);
        require(blended!=plain,
                "the transparency method of draw() is missing from the file snapshot");
        write_rgba(output_path(output_dir,"average-transparency.rgba"),blended,fw,fh);
        v.blend = false;
        for (auto* surface : v.surfaces()) surface->set_opacity(1.0f);

        // A scaled frame keeps the same content policy at another size, which
        // also exercises the camera override and its restoration.
        v.occlude = true;
        require(v.snapshot(output_path(output_dir,"cross-backend-scaled.png"),2,4,1),"scaled snapshot failed");
        const auto scaled=read_snapshot(output_path(output_dir,"cross-backend-scaled.png"),fw*2,fh*2);
        write_rgba(output_path(output_dir,"scaled-msaa.rgba"),scaled,fw*2,fh*2);
        v.occlude = false;

        // The default background is the viewer's current color, not white. The
        // top-right corner is clear of the centered scene and of every HUD element.
        require(v.snapshot(output_path(output_dir,"cross-backend-default-bg.png"),1,1),"default-background snapshot failed");
        const auto default_bg = read_snapshot(output_path(output_dir,"cross-backend-default-bg.png"),fw,fh);
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
#ifdef EASY3D_HAS_VULKAN
        require(v.device().validation_errors()==0,"snapshot validation errors");
#endif
        std::cout<<"PASS: OpenGL/Vulkan effect and snapshot images written to "<<output_dir<<'\n';
    } catch (const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
