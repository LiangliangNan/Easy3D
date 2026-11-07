/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************/

#ifndef EASY3D_RENDERER_PER_PIXEL_LINKED_LISTS_OIT_H
#define EASY3D_RENDERER_PER_PIXEL_LINKED_LISTS_OIT_H

#ifdef EASY3D_HAS_VULKAN

#include <vector>
#include <easy3d/renderer/transparency.h>


namespace easy3d {

    class Camera;
    class TrianglesDrawable;

    /**
     * \brief Order-independent transparency using per-pixel linked lists.
     *
     * \class PerPixelLinkedListsOIT easy3d/renderer/per_pixel_linked_lists_oit.h
     *
     * \details This class implements order-independent transparency (OIT) using per-pixel
     *      linked lists, providing perfect transparency quality for scientific visualization
     *      and figure generation. Unlike dual depth peeling (which approximates transparency
     *      by peeling a fixed number of layers), this method stores all transparent fragments
     *      per pixel and sorts them by depth, producing correct results regardless of surface
     *      intersections or complex overlapping geometry.
     *
     *      **Algorithm Overview:**
     *
     *      The algorithm operates in two passes:
     *
     *      1. **Build Pass:** All transparent fragments are rendered. Each fragment atomically
     *         appends itself to a per-pixel linked list stored in a GPU buffer. Each list node
     *         contains the fragment's color (premultiplied alpha), depth, and a pointer to the
     *         next node. An atomic counter allocates unique node indices, and atomic
     *         compare-and-swap operations insert nodes at the head of each pixel's list.
     *
     *      2. **Resolve Pass:** A fullscreen quad is rendered. For each pixel, the linked list
     *         is traversed, fragments are sorted by depth (front-to-back), and blended using
     *         standard alpha compositing. The result is written to the output framebuffer.
     *
     *      **Memory Layout:**
     *
     *      - **Head Buffer:** One int32 per pixel, storing the index of the first node in each
     *        pixel's linked list. Initialized to -1 (empty list) each frame.
     *      - **Node Buffer:** A pool of fragment nodes. Each node contains:
     *          - vec4 color (premultiplied RGBA)
     *          - float depth
     *          - int next (index of next node, -1 if end of list)
     *        Total size: 32 bytes per node (std430 alignment pads the 24-byte payload to 32).
     *      - **Counter Buffer:** One atomic uint32, incremented to allocate node indices.
     *        Reset to 0 each frame.
     *
     *      **Memory Requirements:**
     *
     *      For a viewport of width × height pixels with an average of N fragments per pixel:
     *      - Head buffer: width × height × 4 bytes
     *      - Node buffer: width × height × N × 32 bytes
     *      - Counter buffer: 4 bytes
     *
     *      Example: 1920×1080 viewport, 8 fragments/pixel average:
     *      - Head: 8.3 MB
     *      - Nodes: 505 MB
     *      - Total: ~513 MB
     *
     *      The `max_fragments_per_pixel` parameter controls the node pool size. If the pool
     *      overflows (more fragments than allocated nodes), excess fragments are silently
     *      discarded. For typical scenes, 8-16 fragments per pixel is sufficient.
     *
     *      **Performance Characteristics:**
     *
     *      - **Build Pass:** Limited by atomic operation throughput and memory bandwidth.
     *        High fragment density causes atomic contention, reducing performance.
     *      - **Resolve Pass:** Limited by memory bandwidth (reading linked list nodes) and
     *        sorting complexity. Sorting is O(N log N) per pixel where N is the number of
     *        fragments at that pixel. For N ≤ 16, sorting is fast.
     *      - **Overall:** Typically 2-3× slower than dual depth peeling, but produces perfect
     *        results. Suitable for offline rendering and figure generation where quality is
     *        paramount.
     *
     *      **Backend Support:**
     *
     *      - **OpenGL:** Requires OpenGL 4.3+ for Shader Storage Buffer Objects (SSBOs) and
     *        atomic operations. Uses fragment shaders for both build and resolve passes.
     *      - **Vulkan:** Implemented in the Vulkan render device using storage buffers with
     *        atomic operations (head/node/counter) and a fragment-shader resolve. This is the
     *        supported path on macOS (via MoltenVK), where OpenGL is capped at 4.1 and cannot
     *        run the SSBO/atomic build pass.
     *
     *      **Usage:**
     *
     *      ```cpp
     *      auto oit = new PerPixelLinkedListsOIT(camera());
     *      oit->set_max_fragments_per_pixel(16);  // adjust based on scene complexity
     *      // In draw():
     *      std::vector<TrianglesDrawable*> surfaces = ...;
     *      oit->draw(surfaces);
     *      ```
     *
     *      **Limitations:**
     *
     *      - Requires significant GPU memory (hundreds of MB for typical viewports).
     *      - Performance degrades with high fragment density (many overlapping surfaces).
     *      - OpenGL 4.3+ required (not compatible with OpenGL 3.2).
     *      - Not suitable for real-time rendering of complex scenes; best for offline/figure
     *        generation.
     *
     * \sa DualDepthPeeling, AverageColorBlending
     */
    class PerPixelLinkedListsOIT : public Transparency
    {
    public:
        explicit PerPixelLinkedListsOIT(Camera* cam) : camera_(cam) {}
        ~PerPixelLinkedListsOIT() override = default;

        /**
         * \brief Sets the maximum number of fragments per pixel.
         * \details This controls the size of the node pool buffer. Higher values handle more
         *      complex scenes but require more memory. Default is 8.
         * \param n Maximum fragments per pixel. Must be positive.
         */
        void set_max_fragments_per_pixel(int n) { max_fragments_ = n; }

        /**
         * \brief Returns the maximum number of fragments per pixel.
         * \return The current limit.
         */
        int max_fragments_per_pixel() const { return max_fragments_; }

        /**
         * \brief Renders the scene with per-pixel linked lists OIT.
         * \details The Vulkan render device builds the per-pixel fragment lists in
         *      storage buffers and resolves them into the transparency accumulation
         *      target. The last argument carries the node-pool capacity.
         * \param surfaces The transparent surfaces to render.
         */
        void draw(const std::vector<TrianglesDrawable*>& surfaces) override {
            render_frame(camera_, surfaces, 3, max_fragments_);
        }

    private:
        Camera* camera_;
        int max_fragments_ = 8;  ///< Maximum fragments per pixel (controls node pool size).
    };

}

#else

#include <vector>
#include <easy3d/renderer/transparency.h>


namespace easy3d {

    class Camera;
    class TrianglesDrawable;
    class FramebufferObject;

    /**
     * \brief Order-independent transparency using per-pixel linked lists.
     *
     * \class PerPixelLinkedListsOIT easy3d/renderer/per_pixel_linked_lists_oit.h
     *
     * \details This class implements order-independent transparency (OIT) using per-pixel
     *      linked lists, providing perfect transparency quality for scientific visualization
     *      and figure generation. Unlike dual depth peeling (which approximates transparency
     *      by peeling a fixed number of layers), this method stores all transparent fragments
     *      per pixel and sorts them by depth, producing correct results regardless of surface
     *      intersections or complex overlapping geometry.
     *
     *      **Algorithm Overview:**
     *
     *      The algorithm operates in two passes:
     *
     *      1. **Build Pass:** All transparent fragments are rendered. Each fragment atomically
     *         appends itself to a per-pixel linked list stored in a GPU buffer. Each list node
     *         contains the fragment's color (premultiplied alpha), depth, and a pointer to the
     *         next node. An atomic counter allocates unique node indices, and atomic
     *         compare-and-swap operations insert nodes at the head of each pixel's list.
     *
     *      2. **Resolve Pass:** A fullscreen quad is rendered. For each pixel, the linked list
     *         is traversed, fragments are sorted by depth (front-to-back), and blended using
     *         standard alpha compositing. The result is written to the output framebuffer.
     *
     *      **Memory Layout:**
     *
     *      - **Head Buffer:** One int32 per pixel, storing the index of the first node in each
     *        pixel's linked list. Initialized to -1 (empty list) each frame.
     *      - **Node Buffer:** A pool of fragment nodes. Each node contains:
     *          - vec4 color (premultiplied RGBA)
     *          - float depth
     *          - int next (index of next node, -1 if end of list)
     *        Total size: 32 bytes per node (std430 alignment pads the 24-byte payload to 32).
     *      - **Counter Buffer:** One atomic uint32, incremented to allocate node indices.
     *        Reset to 0 each frame.
     *
     *      **Memory Requirements:**
     *
     *      For a viewport of width × height pixels with an average of N fragments per pixel:
     *      - Head buffer: width × height × 4 bytes
     *      - Node buffer: width × height × N × 32 bytes
     *      - Counter buffer: 4 bytes
     *
     *      Example: 1920×1080 viewport, 8 fragments/pixel average:
     *      - Head: 8.3 MB
     *      - Nodes: 505 MB
     *      - Total: ~513 MB
     *
     *      The `max_fragments_per_pixel` parameter controls the node pool size. If the pool
     *      overflows (more fragments than allocated nodes), excess fragments are silently
     *      discarded. For typical scenes, 8-16 fragments per pixel is sufficient.
     *
     *      **Performance Characteristics:**
     *
     *      - **Build Pass:** Limited by atomic operation throughput and memory bandwidth.
     *        High fragment density causes atomic contention, reducing performance.
     *      - **Resolve Pass:** Limited by memory bandwidth (reading linked list nodes) and
     *        sorting complexity. Sorting is O(N log N) per pixel where N is the number of
     *        fragments at that pixel. For N ≤ 16, sorting is fast.
     *      - **Overall:** Typically 2-3× slower than dual depth peeling, but produces perfect
     *        results. Suitable for offline rendering and figure generation where quality is
     *        paramount.
     *
     *      **Backend Support:**
     *
     *      - **OpenGL:** Requires OpenGL 4.3+ for Shader Storage Buffer Objects (SSBOs) and
     *        atomic operations. Uses fragment shaders for both build and resolve passes.
     *      - **Vulkan:** Implemented in the Vulkan render device using storage buffers with
     *        atomic operations (head/node/counter) and a fragment-shader resolve. This is the
     *        supported path on macOS (via MoltenVK), where OpenGL is capped at 4.1 and cannot
     *        run the SSBO/atomic build pass.
     *
     *      **Usage:**
     *
     *      ```cpp
     *      auto oit = new PerPixelLinkedListsOIT(camera());
     *      oit->set_max_fragments_per_pixel(16);  // adjust based on scene complexity
     *      // In draw():
     *      std::vector<TrianglesDrawable*> surfaces = ...;
     *      oit->draw(surfaces);
     *      ```
     *
     *      **Limitations:**
     *
     *      - Requires significant GPU memory (hundreds of MB for typical viewports).
     *      - Performance degrades with high fragment density (many overlapping surfaces).
     *      - OpenGL 4.3+ required (not compatible with OpenGL 3.2).
     *      - Not suitable for real-time rendering of complex scenes; best for offline/figure
     *        generation.
     *
     * \sa DualDepthPeeling, AverageColorBlending
     */
    class PerPixelLinkedListsOIT : public Transparency
    {
    public:
        /**
         * \brief Constructor.
         * \param cam The camera used for rendering.
         */
        explicit PerPixelLinkedListsOIT(Camera* cam);

        /**
         * \brief Destructor. Releases GPU resources.
         */
        ~PerPixelLinkedListsOIT() override;

        /**
         * \brief Sets the maximum number of fragments per pixel.
         * \details This controls the size of the node pool buffer. Higher values handle more
         *      complex scenes but require more memory. Default is 8.
         * \param n Maximum fragments per pixel. Must be positive.
         */
        void set_max_fragments_per_pixel(int n) { max_fragments_ = n; }

        /**
         * \brief Returns the maximum number of fragments per pixel.
         * \return The current limit.
         */
        int max_fragments_per_pixel() const { return max_fragments_; }

        /**
         * \brief Renders the scene with per-pixel linked lists OIT.
         * \param surfaces The transparent surfaces to render.
         */
        void draw(const std::vector<TrianglesDrawable*>& surfaces) override;

    private:
        Camera* camera_;
        int max_fragments_;  ///< Maximum fragments per pixel (controls node pool size).

        // OpenGL resources
        unsigned int head_buffer_;      ///< SSBO: per-pixel head pointers (int32 per pixel).
        unsigned int node_buffer_;      ///< SSBO: fragment node pool (32 bytes per node with std430 padding).
        unsigned int counter_buffer_;   ///< Atomic counter buffer for node allocation.
        FramebufferObject* fbo_;        ///< Framebuffer for the build pass offscreen rendering.
        int width_;                     ///< Current viewport width.
        int height_;                    ///< Current viewport height.
        int node_count_;                ///< Current number of allocated nodes.

        /**
         * \brief Initializes GPU resources for the given viewport size.
         * \param w Viewport width in pixels.
         * \param h Viewport height in pixels.
         */
        void init(int w, int h);

        /**
         * \brief Build pass: renders all transparent fragments into per-pixel linked lists.
         * \param surfaces The transparent surfaces to render.
         */
        void build_lists(const std::vector<TrianglesDrawable*>& surfaces);

        /**
         * \brief Resolve pass: sorts and blends the linked lists, writing to the output.
         */
        void resolve();

        /**
         * \brief Releases all GPU resources.
         */
        void clear();
    };

}

#endif // EASY3D_HAS_VULKAN

#endif // EASY3D_RENDERER_PER_PIXEL_LINKED_LISTS_OIT_H
