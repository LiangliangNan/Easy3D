// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/renderer/text_layout.h>
#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include <3rd_party/stb/stb_truetype.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iterator>
#include <map>
#include <tuple>
#include <stdexcept>

namespace easy3d { namespace graphics {
namespace {
std::vector<std::vector<int>> decode(const std::string& text) {
    std::vector<std::vector<int>> lines(1);
    for (std::size_t i=0;i<text.size();) {
        const unsigned char first=text[i++];
        unsigned code=first, count=0, minimum=0;
        if (first>=0xc2 && first<=0xdf) { code=first&31; count=1; minimum=0x80; }
        else if (first>=0xe0 && first<=0xef) { code=first&15; count=2; minimum=0x800; }
        else if (first>=0xf0 && first<=0xf4) { code=first&7; count=3; minimum=0x10000; }
        else if (first>=0x80) throw std::invalid_argument("invalid UTF-8 text");
        if (count>text.size()-i) throw std::invalid_argument("truncated UTF-8 text");
        for (unsigned j=0;j<count;++j) {
            const unsigned char next=text[i++];
            if ((next&0xc0)!=0x80) throw std::invalid_argument("invalid UTF-8 continuation");
            code=(code<<6)|(next&63);
        }
        if (code<minimum || code>0x10ffff || (code>=0xd800 && code<=0xdfff))
            throw std::invalid_argument("invalid Unicode scalar value");
        if (code=='\r' || code=='\n') {
            if (code=='\r' && i<text.size() && text[i]=='\n') ++i;
            lines.emplace_back();
        } else if (code=='\t') lines.back().insert(lines.back().end(),4,' ');
        else lines.back().push_back(int(code));
    }
    return lines;
}
}
struct TextLayoutEngine::Impl {
    struct Font {
        std::vector<unsigned char> bytes;
        stbtt_fontinfo info{};
        int ascent=0, descent=0, gap=0;
    };
    struct Glyph { int page=-1, x=0,y=0,w=0,h=0,dx=0,dy=0; };
    struct Page {
        unsigned x=1,y=1,row=0;
        std::vector<unsigned char> rgba;
        std::shared_ptr<const Image> snapshot;
    };
    unsigned size;
    std::vector<std::unique_ptr<Font>> fonts;
    std::vector<std::string> names;
    std::vector<Page> pages;
    std::map<std::tuple<std::size_t,float,int>,Glyph> glyphs;
    explicit Impl(unsigned s) : size(s) {}
    Glyph glyph(std::size_t font, float pixels, int index, float scale) {
        const auto key=std::make_tuple(font,pixels,index);
        const auto cached=glyphs.find(key);
        if (cached!=glyphs.end()) return cached->second;
        Glyph g;
        int x1,y1;
        stbtt_GetGlyphBitmapBox(&fonts[font]->info,index,scale,scale,&g.dx,&g.dy,&x1,&y1);
        g.w=x1-g.dx; g.h=y1-g.dy;
        if (g.w>0 && g.h>0) {
            if (unsigned(g.w)+2>size || unsigned(g.h)+2>size)
                throw std::length_error("glyph exceeds text atlas page; reduce size or use a larger atlas");
            // Shelf packing with a transparent one-texel gutter for linear filtering.
            if (!pages.empty()) {
                auto& p=pages.back();
                if (p.x+g.w+1>size) { p.x=1; p.y+=p.row+2; p.row=0; }
            }
            if (pages.empty() || pages.back().y+g.h+1>size) {
                Page p; p.rgba.resize(std::size_t(size)*size*4);
                for (std::size_t i=0;i<p.rgba.size();i+=4) p.rgba[i]=p.rgba[i+1]=p.rgba[i+2]=255;
                pages.push_back(std::move(p));
            }
            auto& p=pages.back();
            g.page=int(pages.size()-1); g.x=int(p.x); g.y=int(p.y);
            std::vector<unsigned char> mask(std::size_t(g.w)*g.h);
            stbtt_MakeGlyphBitmap(&fonts[font]->info,mask.data(),g.w,g.h,g.w,scale,scale,index);
            for (int y=0;y<g.h;++y) for (int x=0;x<g.w;++x)
                p.rgba[((g.y+y)*std::size_t(size)+g.x+x)*4+3]=mask[y*std::size_t(g.w)+x];
            p.x+=g.w+2; p.row=std::max(p.row,unsigned(g.h)); p.snapshot.reset();
        }
        glyphs.emplace(key,g);
        return g;
    }
};
TextLayoutEngine::TextLayoutEngine(unsigned size) {
    if (size<32 || size>4096) throw std::invalid_argument("text atlas size must be in [32,4096]");
    impl_.reset(new Impl(size));
}
TextLayoutEngine::~TextLayoutEngine() = default;
const std::vector<std::string>& TextLayoutEngine::font_names() const { return impl_->names; }
void TextLayoutEngine::clear_cache() { impl_->glyphs.clear(); impl_->pages.clear(); }
std::size_t TextLayoutEngine::add_font(const std::string& file) {
    std::ifstream stream(file,std::ios::binary);
    if (!stream) throw std::runtime_error("cannot read font: "+file);
    std::unique_ptr<Impl::Font> font(new Impl::Font);
    font->bytes.assign(std::istreambuf_iterator<char>(stream),{});
    if (font->bytes.size()<16) throw std::runtime_error("font file is too short: "+file);
    const int offset=stbtt_GetFontOffsetForIndex(font->bytes.data(),0);
    if (offset<0 || std::size_t(offset)+12>font->bytes.size() ||
        !stbtt_InitFont(&font->info,font->bytes.data(),offset))
        throw std::runtime_error("unsupported font: "+file);
    stbtt_GetFontVMetrics(&font->info,&font->ascent,&font->descent,&font->gap);
    if (font->ascent<=font->descent) throw std::runtime_error("invalid font metrics: "+file);
    impl_->names.push_back(file.substr(file.find_last_of("/\\")+1));
    impl_->fonts.push_back(std::move(font));
    return impl_->fonts.size()-1;
}
TextLayout TextLayoutEngine::layout(const std::string& text,unsigned width,unsigned height,const TextOptions& o) {
    const auto projection=screen_space_projection(width,height);
    if (o.font>=impl_->fonts.size()) throw std::out_of_range("text font index is not loaded");
    for (float v : {o.size,o.x,o.y,o.character_spacing,o.line_spacing,o.color.x,o.color.y,o.color.z,o.color.w})
        if (!std::isfinite(v)) throw std::invalid_argument("non-finite text option");
    if (o.size<=0 || o.size>4096 || o.line_spacing < -1)
        throw std::invalid_argument("invalid text size or line spacing");
    if (o.align!=TextAlign::left && o.align!=TextAlign::center && o.align!=TextAlign::right)
        throw std::invalid_argument("invalid text alignment");
    auto lines=decode(text);
    const auto& f=*impl_->fonts[o.font];
    const float scale=stbtt_ScaleForPixelHeight(&f.info,o.size);
    std::vector<float> widths;
    float longest=0;
    for (auto& line:lines) {
        float x=0;
        for (std::size_t i=0;i<line.size();++i) {
            line[i]=stbtt_FindGlyphIndex(&f.info,line[i]);
            int advance,bearing; stbtt_GetGlyphHMetrics(&f.info,line[i],&advance,&bearing);
            if (i) x+=o.character_spacing+(o.kerning ? scale*stbtt_GetGlyphKernAdvance(&f.info,line[i-1],line[i]) : 0);
            x+=advance*scale;
        }
        if (!std::isfinite(x)) throw std::length_error("text line advance overflow");
        widths.push_back(x); longest=std::max(longest,x);
    }
    struct Quad { Impl::Glyph glyph; float x,y; };
    std::vector<Quad> quads;
    TextLayout result;
    result.line_height=(f.ascent-f.descent+f.gap)*scale*(1+o.line_spacing);
    if (!std::isfinite(result.line_height)) throw std::length_error("text line height overflow");
    bool has_ink=false;
    for (std::size_t l=0;l<lines.size();++l) {
        float x=o.x+(o.align==TextAlign::left ? 0 : (longest-widths[l])*(o.align==TextAlign::center ? 0.5f : 1));
        const float baseline=o.upper_left ? o.y+f.ascent*scale+l*result.line_height :
            height-o.y+f.descent*scale-l*result.line_height;
        for (std::size_t i=0;i<lines[l].size();++i) {
            const int index=lines[l][i];
            if (i) x+=o.character_spacing+(o.kerning ? scale*stbtt_GetGlyphKernAdvance(&f.info,lines[l][i-1],index) : 0);
            const auto g=impl_->glyph(o.font,o.size,index,scale);
            if (g.page>=0) {
                const float left=x+g.dx,top=baseline+g.dy;
                if (!std::isfinite(left) || !std::isfinite(top)) throw std::length_error("text position overflow");
                quads.push_back({g,left,top});
                if (!has_ink) { result.bounds=Rect(left,left+g.w,top,top+g.h); has_ink=true; }
                else result.bounds=Rect(std::min(result.bounds.x_min(),left),std::max(result.bounds.x_max(),left+g.w),
                    std::min(result.bounds.y_min(),top),std::max(result.bounds.y_max(),top+g.h));
            }
            int advance,bearing; stbtt_GetGlyphHMetrics(&f.info,index,&advance,&bearing); x+=advance*scale;
        }
        result.advance=x-o.x;
    }
    // Consecutive glyphs sharing a page batch together; preserve order across pages.
    std::size_t begin=0;
    while (begin<quads.size()) {
        const int page=quads[begin].glyph.page;
        std::vector<Vertex> vertices; std::vector<uint32_t> indices;
        do {
            const auto& q=quads[begin]; const auto& g=q.glyph;
            const uint32_t base=uint32_t(vertices.size());
            const float u=float(g.x)/impl_->size,v=float(g.y)/impl_->size;
            const float du=float(g.w)/impl_->size,dv=float(g.h)/impl_->size;
            Vertex p; p.position=vec3(q.x,q.y,0); p.texcoord=vec2(u,v); vertices.push_back(p);
            p.position=vec3(q.x+g.w,q.y,0); p.texcoord=vec2(u+du,v); vertices.push_back(p);
            p.position=vec3(q.x+g.w,q.y+g.h,0); p.texcoord=vec2(u+du,v+dv); vertices.push_back(p);
            p.position=vec3(q.x,q.y+g.h,0); p.texcoord=vec2(u,v+dv); vertices.push_back(p);
            indices.insert(indices.end(),{base,base+1,base+2,base,base+2,base+3});
            ++begin;
        } while (begin<quads.size() && quads[begin].glyph.page==page);
        auto& atlas=impl_->pages[page];
        if (!atlas.snapshot) {
            Sampler sampler; sampler.filter=TextureFilter::linear;
            atlas.snapshot=std::make_shared<Image>(impl_->size,impl_->size,atlas.rgba,sampler);
        }
        DrawCommand command;
        command.geometry=std::make_shared<Geometry>(std::move(vertices),std::move(indices));
        command.texture=atlas.snapshot; command.model_view_projection=projection; command.color=o.color; command.overlay=true;
        result.commands.push_back(std::move(command));
    }
    return result;
}
} }
