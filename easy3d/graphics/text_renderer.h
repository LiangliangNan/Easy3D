// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_GRAPHICS_TEXT_RENDERER_H
#define EASY3D_GRAPHICS_TEXT_RENDERER_H
#include <easy3d/graphics/render_device.h>

namespace easy3d { namespace graphics {

enum class TextAlign { left, center, right };
struct TextOptions {
    std::size_t font = 0;
    float size = 24; // framebuffer pixels, independent of platform DPI
    float x = 0, y = 0;
    vec4 color = vec4(0,0,0,1);
    TextAlign align = TextAlign::left; // shorter lines align within the longest line
    float character_spacing = 0; // framebuffer pixels between characters
    float line_spacing = 0; // additional fraction of the font's line height; >= -1
    bool kerning = false;
    // Top origin: y is the top of the first font box, subsequent lines go down.
    // Bottom origin: y is the bottom of that box, subsequent lines go up.
    bool upper_left = true;
};
struct TextLayout {
    std::vector<DrawCommand> commands;
    Rect bounds = Rect(0,0,0,0); // actual ink bounds in top-left framebuffer pixels
    float advance = 0; // last line's advance, including alignment offset
    float line_height = 0;
};

// CPU TrueType rasterization/layout, no graphics context or device required.
// UTF-8 is decoded to Unicode scalar values; missing glyphs use the font's glyph 0.
// This is basic horizontal layout, without shaping, bidi or font fallback.
// Layouts own immutable geometry/atlas snapshots and outlive this object. New
// glyphs never mutate a previously returned layout. Externally synchronize use.
class TextRenderer {
public:
    explicit TextRenderer(unsigned atlas_size = 512);
    ~TextRenderer();
    TextRenderer(const TextRenderer&) = delete;
    TextRenderer& operator=(const TextRenderer&) = delete;
    // Loads trusted font files. Throws on unreadable/unsupported fonts.
    std::size_t add_font(const std::string& filename);
    const std::vector<std::string>& font_names() const;
    TextLayout layout(const std::string& utf8, unsigned width, unsigned height,
                      const TextOptions& options = {});
    // Release cached glyph sizes/pages. Fonts and previously returned layouts survive.
    void clear_cache();
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} }
#endif
