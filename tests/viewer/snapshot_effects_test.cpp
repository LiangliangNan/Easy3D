// SPDX-License-Identifier: GPL-3.0-or-later
#include "test_window.h"
#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/ambient_occlusion.h>
#include <easy3d/renderer/average_color_blending.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/renderer/shadow.h>
#include <easy3d/renderer/soft_shadow.h>
#include <easy3d/renderer/texture.h>
#ifndef EASY3D_HAS_VULKAN
#include <easy3d/renderer/opengl.h>
#endif
#include <easy3d/util/initializer.h>
#include <easy3d/util/resource.h>
#include <fstream>
#include <iostream>
#include <memory>
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

// The scene of this test is opaque, so every pixel of a snapshot taken over an
// opaque background must be opaque. A pass that scales the alpha of the geometry it
// darkens (the virtual background receiver of the shadow passes used to) stores a
// semi-transparent shadow, which then blends with whatever shows the image.
bool all_opaque(const std::vector<unsigned char>& rgba) {
    for (size_t i=3;i<rgba.size();i+=4)
        if (rgba[i]!=255) return false;
    return true;
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
    ~EffectViewer() override { delete occlusion_; delete blending_; delete shadow_; delete hard_shadow_; }
    void enable_effects() {
        occlusion_ = new AmbientOcclusion(camera());
        blending_ = new AverageColorBlending(camera());
        shadow_ = new SoftShadow(camera());
        shadow_->set_virtual_background_color(background_color());
        hard_shadow_ = new Shadow(camera());
        hard_shadow_->set_virtual_background_color(background_color());
    }
    std::vector<TrianglesDrawable*> surfaces() const {
        std::vector<TrianglesDrawable*> result;
        for (auto& model : models_)
            for (auto& drawable : model->renderer()->triangles_drawables())
                result.push_back(dynamic_cast<TrianglesDrawable*>(drawable.get()));
        return result;
    }
    mutable int draws = 0;
    bool occlude = false, blend = false, cast_shadow = false, cast_hard_shadow = false;
    std::vector<TrianglesDrawable*> blend_surfaces;
    // PaintCanvas's sequence: generate the AO term, hand it to the drawables,
    // then draw. OpenGL only, since generate() exists on that backend alone.
    bool mapple_ao = false;
    void draw() const override {
        ++draws;
#ifndef EASY3D_HAS_VULKAN
        if (mapple_ao && occlusion_) {
            const unsigned int texture = occlusion_->generate(models_);
            for (auto* surface : surfaces())
                surface->set_ao_texture(texture);
            Viewer::draw();
            return;
        }
#endif
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
        if (cast_hard_shadow && hard_shadow_) {
            hard_shadow_->begin(surfaces());
            Viewer::draw();
            hard_shadow_->end();
            return;
        }
        if (blend && blending_) {
            blending_->draw(blend_surfaces.empty() ? surfaces() : blend_surfaces);
            return;
        }
        Viewer::draw();
    }
    AmbientOcclusion* occlusion_ = nullptr;
    AverageColorBlending* blending_ = nullptr;
    SoftShadow* shadow_ = nullptr;
    Shadow* hard_shadow_ = nullptr;
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
        DeviceOptions options;
        options.validation=options.sync_validation=validation;
#else
        DeviceOptions options;
        (void)validation;
#endif
        {
        const int width=320,height=240,samples=1;
        EffectViewer v("Snapshot effects",window_options(width,height,samples,false,false,true),options);
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
        require(all_opaque(plain),"the plain snapshot is not opaque");
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
        require(all_opaque(ao),"the ambient-occlusion snapshot is not opaque");
        write_rgba(output_path(output_dir,"ambient-occlusion.rgba"),ao,fw,fh);
        v.occlude = false;

        v.cast_shadow = true;
        require(v.snapshot(output_path(output_dir,"cross-backend-shadow.png"),1,1,1),"shadow snapshot failed");
        const auto shadow=read_snapshot(output_path(output_dir,"cross-backend-shadow.png"),fw,fh);
        require(shadow!=plain,"soft shadows are missing from the file snapshot");
        // The shadow darkens the virtual background receiver's color only: the image
        // must stay opaque, otherwise a viewer composites the shadow with its own
        // background and the shadow looks washed out.
        require(all_opaque(shadow),"the shadow snapshot is not opaque");
        write_rgba(output_path(output_dir,"soft-shadow.rgba"),shadow,fw,fh);
        v.cast_shadow = false;

        // The hard-shadow pass draws the receiver through its own shader, which had
        // the same alpha defect: its visibility is the shadow factor as well.
        v.cast_hard_shadow = true;
        require(v.snapshot(output_path(output_dir,"cross-backend-hard-shadow.png"),1,1,1),"hard-shadow snapshot failed");
        const auto hard=read_snapshot(output_path(output_dir,"cross-backend-hard-shadow.png"),fw,fh);
        require(hard!=plain,"hard shadows are missing from the file snapshot");
        require(all_opaque(hard),"the hard-shadow snapshot is not opaque");
        v.cast_hard_shadow = false;

        // The transparency method is chosen inside draw() as well, and reaches
        // the device through the same capture.
        v.blend = true;
        for (auto* surface : v.surfaces()) surface->set_opacity(.5f);
        require(v.snapshot(output_path(output_dir,"cross-backend-blended.png"),1,1,1),"blended snapshot failed");
        const auto blended=read_snapshot(output_path(output_dir,"cross-backend-blended.png"),fw,fh);
        require(blended!=plain,
                "the transparency method of draw() is missing from the file snapshot");
        require(all_opaque(blended),"the transparency snapshot is not opaque");
        write_rgba(output_path(output_dir,"average-transparency.rgba"),blended,fw,fh);
        v.blend = false;
        for (auto* surface : v.surfaces()) surface->set_opacity(1.0f);

        // A scaled frame keeps the same content policy at another size, which
        // also exercises the camera override and its restoration.
        v.occlude = true;
        require(v.snapshot(output_path(output_dir,"cross-backend-scaled.png"),2,4,1),"scaled snapshot failed");
        const auto scaled=read_snapshot(output_path(output_dir,"cross-backend-scaled.png"),fw*2,fh*2);
        require(all_opaque(scaled),"the scaled snapshot is not opaque");
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

        // Both backends enforce the sample-count rule the same way: 1 is the request
        // for no multisampling and 0 is a caller error, not a silent fallback - only
        // the constructors accept 0 as the OpenGL spelling of the disabled case.
        require(!v.snapshot(output_path(output_dir,"cross-backend-zero-samples.png"),1,0,1),
                "a zero snapshot sample count was accepted");

        // A count that is a valid power of two but beyond this device is a quality
        // request, so it steps down instead of failing the frame: the snapshot is the
        // image of the count it stepped down to.
        {
        const int maximum = v.supported_sample_count(64);
        // The silent capability query and the resolving one agree: a count resolves to
        // itself exactly when it is offered, which is what an interface listing the
        // supported counts relies on.
        const unsigned mask = v.supported_sample_mask();
        require(mask & 1u,"the sample mask does not offer the single-sample count");
        for (int count = 1; count <= 64; count <<= 1)
            require((bool(mask & unsigned(count))) == (v.supported_sample_count(count) == count),
                    "the sample mask and the resolution of that count disagree");
        require(v.supported_sample_count(0)==1 && v.supported_sample_count(1)==1,
                "the no-multisampling request is not reported as 1");
        require(v.supported_sample_count(maximum)==maximum,"the supported count is not its own answer");
        require(v.supported_sample_count(3)==v.supported_sample_count(2),
                "a count between two powers of two is not stepped down");
        if (maximum<64) {
            const int above=maximum*2;
            require(v.supported_sample_count(above)==maximum,
                    "an unaffordable count does not step down to the supported maximum");
            require(v.snapshot(output_path(output_dir,"cross-backend-stepped.png"),1,above,1),
                    "an unaffordable snapshot sample count failed the snapshot");
            const auto stepped=read_snapshot(output_path(output_dir,"cross-backend-stepped.png"),fw,fh);
            require(v.snapshot(output_path(output_dir,"cross-backend-supported-max.png"),1,maximum,1),
                    "snapshot at the supported maximum failed");
            require(stepped==read_snapshot(output_path(output_dir,"cross-backend-supported-max.png"),fw,fh),
                    "a stepped-down snapshot is not the image of the count it stepped down to");
            require(v.capture(unsigned(above)).rgba==v.capture(unsigned(maximum)).rgba,
                    "capture() does not step an unaffordable sample count down");
        }
        bool malformed=false;
        try { v.capture(3u); } catch (const std::invalid_argument&) { malformed=true; }
        require(malformed,"a non-power-of-two capture sample count was accepted");
        }

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
        }

#ifndef EASY3D_HAS_VULKAN
        {
        // Mapple enables AO on the drawables before any AO term exists, then
        // hands them the generated texture on every frame. An enabled flag with
        // a zero texture used to bind an unloadable unit 1, which macOS
        // reported and which the surface shader sampled as black.
        EffectViewer v("AO texture handoff",window_options(320,240,1,false,false,true),options);
        v.enable_effects();
        require(v.add_model(resource::directory()+"/data/room.obj"),"AO handoff scene missing");
        v.camera()->set_up_vector(vec3(0,1,0));
        v.camera()->set_view_direction(vec3(0,0,1));
        v.fit_screen();
        for (auto* surface : v.surfaces()) {
            surface->set_coloring_method(State::UNIFORM_COLOR);
            surface->set_color(vec4(.7f,.5f,.3f,1.0f));
        }
        Viewer::poll_events();v.draw_frame();
        const int fw=v.framebuffer_width(),fh=v.framebuffer_height();
        require(v.snapshot(output_path(output_dir,"opengl-ao-handoff-plain.png"),1,1,1),
                "AO handoff plain snapshot failed");
        const auto plain=read_snapshot(output_path(output_dir,"opengl-ao-handoff-plain.png"),fw,fh);
        require(v.snapshot(output_path(output_dir,"opengl-ao-handoff-plain-scaled.png"),2,1,1),
                "scaled AO handoff plain snapshot failed");
        const auto plain_scaled=read_snapshot(output_path(output_dir,"opengl-ao-handoff-plain-scaled.png"),fw*2,fh*2);

        for (auto* surface : v.surfaces())
            surface->enable_ao(true);   // the AO texture is still 0
        while (glGetError()!=GL_NO_ERROR) {}
        require(v.snapshot(output_path(output_dir,"opengl-ao-handoff-flag-only.png"),1,1,1),
                "AO flag-only snapshot failed");
        const auto flagged=read_snapshot(output_path(output_dir,"opengl-ao-handoff-flag-only.png"),fw,fh);
        require(flagged==plain,"an enabled AO flag without a texture changed the frame");
        require(glGetError()==GL_NO_ERROR,"the AO flag-only frame left a GL error");

        v.mapple_ao=true;
        require(v.snapshot(output_path(output_dir,"opengl-ao-handoff.png"),1,1,1),
                "AO handoff snapshot failed");
        const auto ao=read_snapshot(output_path(output_dir,"opengl-ao-handoff.png"),fw,fh);
        require(ao!=plain,"the generated AO term did not reach the frame");
        // The scaled snapshot re-creates the AO textures, so the drawables hold
        // ids that no longer exist while generate() rebuilds the term.
        require(v.snapshot(output_path(output_dir,"opengl-ao-handoff-scaled.png"),2,1,1),
                "scaled AO handoff snapshot failed");
        const auto ao_scaled=read_snapshot(output_path(output_dir,"opengl-ao-handoff-scaled.png"),fw*2,fh*2);
        require(ao_scaled!=plain_scaled,"the re-created AO term did not reach the scaled frame");
        require(glGetError()==GL_NO_ERROR,"the AO handoff left a GL error");
        }
#endif

        {
        EffectViewer v("Textured average transparency",window_options(320,240,1,false,false,true),options);
        v.enable_effects();
        auto mesh=std::make_shared<SurfaceMesh>();
        auto a=mesh->add_vertex(vec3(-2,-1,0)), b=mesh->add_vertex(vec3(0,-1,0));
        auto c=mesh->add_vertex(vec3(0,1,0)), d=mesh->add_vertex(vec3(-2,1,0));
        auto e=mesh->add_vertex(vec3(0,-1,0)), f=mesh->add_vertex(vec3(2,-1,0));
        auto g=mesh->add_vertex(vec3(2,1,0)), h=mesh->add_vertex(vec3(0,1,0));
        mesh->add_triangle(a,b,c); mesh->add_triangle(a,c,d);
        mesh->add_triangle(e,g,f); mesh->add_triangle(e,h,g);
        auto texcoords=mesh->add_vertex_property<vec2>("v:texcoord");
        texcoords[a]=texcoords[e]=vec2(0,0);
        texcoords[b]=texcoords[f]=vec2(1,0);
        texcoords[c]=texcoords[g]=vec2(1,1);
        texcoords[d]=texcoords[h]=vec2(0,1);
        std::unique_ptr<Texture> texture(Texture::create(
            {255,0,0,255, 0,255,0,128, 0,0,255,64, 255,255,0,255},
            2,2,4,Texture::CLAMP_TO_EDGE,Texture::NEAREST));
        require(texture!=nullptr,"textured transparency image could not be created");
        require(v.add_model(mesh)!=nullptr,"textured transparency mesh could not be added");
        auto* faces=mesh->renderer()->get_triangles_drawable("faces");
        require(faces!=nullptr,"textured transparency drawable is missing");
        faces->set_texture_coloring(State::VERTEX,"v:texcoord",texture.get());
        faces->set_opacity(.5f);
        faces->set_lighting(true);
        faces->set_lighting_two_sides(false);
        faces->set_distinct_back_color(true);
        faces->set_back_color(vec4(1,0,1,1));
        faces->set_smooth_shading(true);
        v.camera()->set_type(Camera::ORTHOGRAPHIC);
        v.camera()->set_view_direction(vec3(0,0,-1));
        v.camera()->set_up_vector(vec3(0,1,0));
        v.fit_screen();
        const int fw=v.framebuffer_width(), fh=v.framebuffer_height();
        require(v.snapshot(output_path(output_dir,"cross-backend-average-textured-plain.png"),1,1,1),
                "textured plain snapshot failed");
        const auto plain=read_snapshot(output_path(output_dir,"cross-backend-average-textured-plain.png"),fw,fh);
        v.blend=true;
        v.blend_surfaces={faces};
        require(v.snapshot(output_path(output_dir,"cross-backend-average-textured.png"),1,1,1),
                "textured average-transparency snapshot failed");
        const auto image=read_snapshot(output_path(output_dir,"cross-backend-average-textured.png"),fw,fh);
        require(image!=plain,"textured average transparency did not change the plain frame");
        write_rgba(output_path(output_dir,"average-textured-transparency.rgba"),image,fw,fh);
#ifdef EASY3D_HAS_VULKAN
        require(v.device().validation_errors()==0,"textured average-transparency validation errors");
#endif
        }

        std::cout<<"PASS: OpenGL/Vulkan effect and snapshot images written to "<<output_dir<<'\n';
    } catch (const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
