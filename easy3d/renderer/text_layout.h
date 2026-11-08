// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_RENDERER_TEXT_LAYOUT_H
#define EASY3D_RENDERER_TEXT_LAYOUT_H
#include <easy3d/renderer/render_device.h>

namespace easy3d { namespace graphics {

/// Horizontal alignment applied independently to each line.
enum class TextAlign { left, center, right };

/// Font and placement options for a laid-out UTF-8 string.
struct TextOptions {
    /// Index returned by TextLayoutEngine::add_font().
    std::size_t font = 0;
    /// Font size in framebuffer pixels, independent of platform DPI.
    float size = 24;
    /// Horizontal framebuffer-pixel origin.
    float x = 0; ///< Horizontal framebuffer-pixel origin.
    float y = 0; ///< Vertical framebuffer-pixel origin.
    /// Text color in RGBA.
    vec4 color = vec4(0,0,0,1);
    /// Alignment of shorter lines relative to the longest line.
    TextAlign align = TextAlign::left;
    /// Additional horizontal spacing between characters, in framebuffer pixels.
    float character_spacing = 0;
    /// Additional line spacing as a fraction of font line height; must be at least -1.
    float line_spacing = 0;
    /// Apply the font's horizontal kerning pairs when positioning glyphs.
    bool kerning = false;
    /// If true, y is the first line's top and lines proceed downward; otherwise
    /// y is the first line's bottom and lines proceed upward.
    bool upper_left = true;
};

/// Immutable draw commands and metrics produced for one text layout.
struct TextLayout {
    /// Draw commands containing glyph geometry and atlas snapshots.
    std::vector<DrawCommand> commands;
    /// Actual ink bounds in top-left framebuffer pixel coordinates.
    Rect bounds = Rect(0,0,0,0);
    /// Advance of the last line, including its alignment offset.
    float advance = 0;
    /// Font line height in framebuffer pixels.
    float line_height = 0;
};

/**
 * \brief Rasterize TrueType glyphs and produce backend-independent text commands.
 *
 * \details Layout is performed on the CPU and requires no graphics context.
 * Input is decoded as UTF-8 Unicode scalar values; a missing glyph uses the
 * font's glyph zero. This engine provides basic horizontal layout only: it
 * does not perform script shaping, bidirectional reordering, or font fallback.
 * Returned layouts own immutable geometry and atlas snapshots and remain
 * valid after the engine is destroyed or its cache is cleared. Calls on one
 * engine must be externally synchronized.
 */
class TextLayoutEngine {
public:
    /**
     * \brief Create a text layout engine.
     * \param atlas_size Requested glyph-atlas page size in pixels.
     */
    explicit TextLayoutEngine(unsigned atlas_size = 512);

    /// Destroy the engine; already returned layouts remain valid.
    ~TextLayoutEngine();

    /// Copying is disabled because the engine owns mutable font and glyph caches.
    TextLayoutEngine(const TextLayoutEngine&) = delete;
    TextLayoutEngine& operator=(const TextLayoutEngine&) = delete;

    /**
     * \brief Load a TrueType font.
     * \param filename Path to the font file.
     * \return Font index to use in TextOptions::font.
     * \throws std::runtime_error if the file is unreadable or unsupported.
     */
    std::size_t add_font(const std::string& filename);

    /// Return the loaded font names in font-index order.
    /// \return Font names.
    const std::vector<std::string>& font_names() const;

    /**
     * \brief Lay out UTF-8 text into immutable glyph draw commands.
     * \param utf8 Text to decode and lay out.
     * \param width Target framebuffer width in pixels.
     * \param height Target framebuffer height in pixels.
     * \param options Font, size, placement, spacing, and alignment options.
     * \return Glyph commands and their layout metrics.
     */
    TextLayout layout(const std::string& utf8, unsigned width, unsigned height,
                      const TextOptions& options = {});

    /**
     * \brief Release cached glyphs and atlas pages.
     * \details Loaded fonts and previously returned layouts are retained.
     */
    void clear_cache();
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} }
#endif
