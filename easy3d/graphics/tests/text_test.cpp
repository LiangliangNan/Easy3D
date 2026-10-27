// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/graphics/text_renderer.h>
#include <easy3d/util/resource.h>
#include <cmath>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>
using namespace easy3d;
using namespace easy3d::graphics;
void require(bool value,const char* message) { if (!value) throw std::runtime_error(message); }
template<class F> void rejects(F f) {
    bool threw=false; try { f(); } catch (const std::exception&) { threw=true; }
    require(threw,"invalid text input accepted");
}
float x_at(const TextLayout& layout,unsigned glyph) {
    return layout.commands.front().geometry->vertices().at(glyph*4).position.x;
}
float y_at(const TextLayout& layout,unsigned glyph) {
    return layout.commands.front().geometry->vertices().at(glyph*4).position.y;
}
std::size_t glyph_count(const TextLayout& layout) {
    std::size_t count=0;
    for (const auto& d:layout.commands) count+=d.geometry->vertices().size()/4;
    return count;
}
int main(int argc,char** argv) {
    try {
        graphics::TextRenderer text(256);
        const std::string font=resource::directory()+"/fonts/en_Vera.ttf";
        require(text.add_font(font)==0,"font index mismatch");
        require(text.add_font(resource::directory()+"/fonts/en_Cousine-Regular.ttf")==1,"second font index mismatch");
        TextOptions o; o.size=32; o.x=8.25f; o.y=6.5f;
        const auto single=text.layout("F",160,120,o);
        require(single.commands.size()==1 && glyph_count(single)==1,"glyph batching failed");
        require(single.bounds.x_min()>=o.x && single.bounds.y_min()>=o.y,"top-left text metrics incorrect");
        const auto shared=text.layout("F",160,120,o);
        require(shared.commands[0].texture==single.commands[0].texture,"unchanged atlas not shared");
        const auto saved=single.commands[0].texture->rgba();
        text.layout("gXYZ",160,120,o);
        require(single.commands[0].texture->rgba()==saved,"new glyph mutated old atlas snapshot");
        require(text.layout("F",160,120,o).commands[0].texture!=single.commands[0].texture,"new atlas was not published");
        const auto av=text.layout("AV",160,120,o);
        o.kerning=true;
        const auto kerned=text.layout("AV",160,120,o);
        require(kerned.advance<av.advance && x_at(kerned,1)<x_at(av,1),"kerning has no effect on advances/geometry");
        o.kerning=false; o.character_spacing=3;
        const auto spaced=text.layout("AV",160,120,o);
        require(std::abs(spaced.advance-av.advance-3)<1e-4f && std::abs(x_at(spaced,1)-x_at(av,1)-3)<1e-4f,
                "character spacing disagrees with advances");
        o.character_spacing=0;
        auto left=text.layout("FFF\nF",160,120,o);
        require(std::abs(x_at(left,0)-x_at(left,3))<1e-4f && y_at(left,3)>y_at(left,0),"left alignment/newline failed");
        const float expected=(text.layout("FFF",160,120,o).advance-single.advance)*0.5f;
        o.align=TextAlign::center;
        auto centered=text.layout("FFF\r\nF",160,120,o);
        require(glyph_count(centered)==4 && std::abs(x_at(centered,3)-x_at(left,3)-expected)<1e-4f,"CRLF/center alignment failed");
        o.align=TextAlign::right;
        auto right=text.layout("FFF\nF",160,120,o);
        require(std::abs(x_at(right,3)-x_at(left,3)-2*expected)<1e-4f,"right alignment failed");
        o.line_spacing=0.5f;
        const auto extra=text.layout("F\nF",160,120,o);
        require(std::abs(extra.line_height-left.line_height*1.5f)<1e-4f,"line spacing failed");
        o.upper_left=false; o.line_spacing=0;
        const auto bottom=text.layout("F\nF",160,120,o);
        require(y_at(bottom,1)<y_at(bottom,0) && bottom.bounds.y_max()<120-o.y,"bottom origin/multiline direction failed");
        o=TextOptions(); o.size=24;
        const auto unicode=text.layout(u8"Aé中😀",160,120,o);
        require(glyph_count(unicode)==4,"UTF-8 codepoints, including non-BMP, were split into bytes");
        require(text.layout(" \t\n",160,120,o).commands.empty() && text.layout("",160,120,o).commands.empty(),"whitespace emitted ink");
        const float proportional=text.layout("iiii",160,120,o).advance;
        o.font=1;
        require(std::abs(text.layout("iiii",160,120,o).advance-proportional)>1,"font selection ignored");
        o.font=0;
        rejects([&]{ text.layout(std::string("\xc0\xaf",2),160,120,o); });
        rejects([&]{ text.layout(std::string("\xed\xa0\x80",3),160,120,o); });
        rejects([&]{ text.layout(std::string("\xf0\x9f",2),160,120,o); });
        rejects([&]{ text.layout(std::string("\xf4\x90\x80\x80",4),160,120,o); });
        rejects([&]{ text.layout("F",0,120,o); });
        o.font=9; rejects([&]{ text.layout("F",160,120,o); }); o.font=0;
        o.size=std::numeric_limits<float>::quiet_NaN(); rejects([&]{ text.layout("F",160,120,o); });
        o.size=0; rejects([&]{ text.layout("F",160,120,o); }); o.size=24;
        o.line_spacing=-2; rejects([&]{ text.layout("F",160,120,o); }); o.line_spacing=0;
        rejects([&]{ text.add_font("/missing/font.ttf"); });
        graphics::TextRenderer tiny(32); tiny.add_font(font);
        o.size=200; rejects([&]{ tiny.layout("W",160,120,o); });
        graphics::TextRenderer paged(64); paged.add_font(font); o.size=24;
        auto pages=paged.layout("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",1000,100,o);
        std::set<const Image*> atlases;
        for (const auto& command:pages.commands) atlases.insert(command.texture.get());
        require(atlases.size()>1 && glyph_count(pages)==52,"multi-page atlas lost glyphs");
        text.clear_cache();
        require(single.commands[0].texture->rgba()==saved,"cache clear invalidated retained text");
        if (argc>1 && std::string(argv[1])=="--cpu-only") {
            std::cout << "PASS: CPU text layout, UTF-8, fonts, kerning/spacing/origins, immutable multi-page atlases\n";
            return 0;
        }
        DeviceOptions options; options.validation=options.synchronization_validation=argc>1;
        auto device=create_vulkan_device(options);
        // Independent bilinear coverage reference for a positioned glyph. This
        // catches atlas row/UV flips, pixel offsets, dark fringes and alpha errors.
        auto commands=single.commands; commands[0].color=vec4(1,0,0,0.6f);
        const auto image=device->render(160,120,commands,vec4(0));
        const auto& quad=commands[0].geometry->vertices(); const auto& atlas=*commands[0].texture;
        unsigned coverage=0;
        for (unsigned y=0;y<120;++y) for (unsigned x=0;x<160;++x) {
            float alpha=0;
            if (x+0.5f>=quad[0].position.x && x+0.5f<quad[2].position.x &&
                y+0.5f>=quad[0].position.y && y+0.5f<quad[2].position.y) {
                const float tx=quad[0].texcoord.x*atlas.width()+(x+0.5f-quad[0].position.x)-0.5f;
                const float ty=quad[0].texcoord.y*atlas.height()+(y+0.5f-quad[0].position.y)-0.5f;
                const int ix=int(std::floor(tx)),iy=int(std::floor(ty));
                const float fx=tx-ix,fy=ty-iy;
                for (int j=0;j<2;++j) for (int i=0;i<2;++i)
                    alpha+=atlas.rgba()[((iy+j)*atlas.width()+ix+i)*4+3]*(i ? fx : 1-fx)*(j ? fy : 1-fy)*0.6f;
            }
            const auto pixel=(y*160+x)*4;
            require(std::abs(image.rgba[pixel]-alpha)<=2 && image.rgba[pixel+1]==0 && image.rgba[pixel+2]==0 &&
                    std::abs(image.rgba[pixel+3]-alpha)<=2,"text raster differs from independent coverage/alpha reference");
            coverage+=image.rgba[pixel]>0;
        }
        require(coverage>50,"glyph has no ink");
        std::vector<Vertex> v(3);
        v[0].position=vec3(-1,-1,0); v[1].position=vec3(3,-1,0); v[2].position=vec3(-1,3,0);
        DrawCommand scene; scene.geometry=std::make_shared<Geometry>(v); scene.object_id=73; scene.color=vec4(0,0,1,1);
        for (unsigned samples : {1u,4u}) {
            if (!(device->supported_sample_counts()&samples)) continue;
            const auto baseline=device->render(160,120,{scene},vec4(0),samples);
            auto draws=commands; draws.push_back(scene);
            const auto composed=device->render(160,120,draws,vec4(0),samples);
            unsigned changed=0;
            for (std::size_t i=0;i<baseline.ids.size();++i) {
                require(composed.ids[i].object==baseline.ids[i].object && composed.ids[i].primitive==baseline.ids[i].primitive &&
                        composed.depth[i]==baseline.depth[i],"text/MSAA changed scene IDs or depth");
                changed+=composed.rgba[4*i]>0;
                require(composed.rgba[4*i+3]==255,"text changed opaque scene alpha");
            }
            require(changed>50,"text did not composite over scene");
            const auto only=device->render(1000,100,pages.commands,vec4(0),samples);
            for (std::size_t i=0;i<only.ids.size();++i)
                require(only.ids[i].object==UINT32_MAX && only.ids[i].primitive==UINT32_MAX && only.depth[i]==1,"text created picking hits");
        }
        auto resized=text.layout("F",240,180,TextOptions());
        device->render(240,180,resized.commands,vec4(0));
        require(device->validation_errors()==0,"text validation errors");
        std::cout << "PASS: CPU text layout/UTF-8, fonts, kerning/spacing/alignment/origins, immutable multi-page atlases, Vulkan coverage/alpha, MSAA, picking/depth\n";
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
