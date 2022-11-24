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

#ifndef EASY3D_RENDERER_DUAL_DEPTH_PEELING_H
#define EASY3D_RENDERER_DUAL_DEPTH_PEELING_H

#include <vector>
#include <easy3d/renderer/transparency.h>


namespace easy3d {

    /**
     * \brief Transparency effect using dual depth peeling.
     *
     * \class DualDepthPeeling easy3d/renderer/dual_depth_peeling.h
     *
     * \details DualDepthPeeling implements the dual depth peeling algorithm described in
     * [Order independent transparency with dual depth peeling]
     * (http://developer.download.nvidia.com/SDK/10.5/opengl/src/dual_depth_peeling/doc/DualDepthPeeling.pdf)
     * by L. Bavoil, K. Myers. February 2008.
     *
     * Depth peeling is traditionally used to perform order independent
     * transparency (OIT) with N geometry passes for N transparency
     * layers. Dual depth peeling enables peeling N transparency layers
     * in N/2+1 passes, by peeling from the front and the back
     * simultaneously using a min-max depth buffer. It performs peeling
     * and blends on the fly. The pass occurs in several stages:
     * 1. Copy the current (opaque geometry) depth buffer into a texture.
     * 2. Initialize the min-max depth buffer from the opaque depth
     *    texture and the translucent geometry.
     * 3. Peel the nearest and farthest fragments:
     *		3a. Blend fragments that match the nearest depth of the
     *          min-max depth buffer into the front buffer.
     *		3b. Write the far depth fragments into a temporary buffer.
     *		3c. Extract the next set of min/max depth values for the
     *          next peel.
     *		3d. Blend the temporary far fragment texture (3b) into an
     *          accumulation texture.
     *		3e. Go back to 3a and repeat until the maximum number of
     *          peels is met, or the desired occlusion ratio is satisfied.
     * 4. If the occlusion ratio != 0 (i.e. we hit the maximum number of
     *    peels before finishing), alpha blend the remaining fragments
     *    in-between the near and far accumulation textures.
     * 5. Blend all accumulation buffers over the opaque color buffer to
     *    produce the final image.
     *
     * Related articals or methods:
     *  - [VTK Technical Highlight: Dual Depth Peeling](https://blog.kitware.com/vtk-technical-highlight-dual-depth-peeling/)
     *  - [OpenGL 4.0+ ABuffer V2.0: Linked lists of fragment pages](https://blog.icare3d.org/2010/07/opengl-40-abuffer-v20-linked-lists-of.html)
     *
     * About antialiasing:
     *    Dual depth peeling won't work with a multisample FBO, so for better
     *    rendering quality, it is possible to add a "Fast Approximate Anti-Aliasing"
     *    (FXAA) step for post antialiasing.
     *
     * Optimization tips: rendering with multi-effects (e.g., shadowing, SSAO)
     * can benefit from sharing the same geometry pass.
     */

    class Camera;
    class TrianglesDrawable;
    class FramebufferObject;

    class DualDepthPeeling : public Transparency
    {
    public:
        /// \brief Constructor
        /// \param cam The camera used in the view
        explicit DualDepthPeeling(Camera* cam);
        ~DualDepthPeeling() override;

        /// \brief Sets the maximum number of peeling layers.
        /// \details It has to be a positive value. Default value is 4. A special value of 0 means no maximum limit.
        void set_max_peels(int n) { max_peels_ = n; }

        /// \brief Returns the maximum number of peeling layers.
        int  max_peels() const { return max_peels_; }

        /// \brief Renders the scene (a set of surfaces) with transparency effect
        void draw(const std::vector<TrianglesDrawable*>& surfaces) override;


    protected:

        // Allocate and configure FBOs, initialize variables, query, etc.
        void init(int w, int h);

        // Initialize Min - Max Depth Buffer
        void init_depth_buffers(const std::vector<TrianglesDrawable*>& surfaces);

        // Render the opaque pass geometry.
        void geometry_pass(const std::vector<TrianglesDrawable*>& surfaces);

        // Render the scene to produce the next set of peels.
        void peel(const std::vector<TrianglesDrawable*>& surfaces);

        void blend_final_image();

        // Swap the src/dest render targets:
        void swap_targets();

        bool peeling_done() const;

        void init_occlusion_query();
        void delete_occlusion_query_id();

        void start_occlusion_query() const;
        void end_occlusion_query();

        // Release all FBOs, shader programs.
        void clear();

    private:
        Camera* camera_;

        int width_;
        int height_;

        // Defines the maximum number of peeling layers. Initial value is 4.
        // A special value of 0 means no maximum limit. It has to be a positive value.
        int max_peels_;

        // The stages of this multi-pass DualDepthPeeling pass
        enum DualDepthPeelingStage {
            DDP_InitDepth,
            DDP_Peel,
            DDP_Blend,
            DDP_Inactive = -1,
        };
        DualDepthPeelingStage stage_;

        FramebufferObject* fbo_;

        // Name the textures used by this render pass. These are indexes into this->Textures
        enum TextureName {
            DepthA = 0,		// RG32F min-max depth buffer
            DepthB,			// RG32F min-max depth buffer
            BackTemp,		// RGBA8 back-to-front peeling buffer
            Back,			// RGBA8 back-to-front accumulation buffer
            FrontA,			// RGBA8 front-to-back peeling buffer
            FrontB			// RGBA8 front-to-back accumulation buffer
        };
        TextureName front_source_;		// The current front source buffer
        TextureName front_destination_; // The current front destination buffer
        TextureName depth_source_;		// The current depth source buffer
        TextureName depth_destination_; // The current depth destination buffer

        int current_peel_;
        int num_geom_passes_; // Debug info, counts number of geometry passes.

        bool use_occlusion_query_;
        unsigned int occlusion_query_Id_;
        unsigned int num_written_pixels_;
        unsigned int occlusion_threshold_;

        float bkg_color_[4];

    private:

        /* Methods to block default compiler methods.
         * The compiler automatically generates the following methods.
         *  Since the default compiler implementation is generally not what
         *  you want (for all but the most simple classes), we usually
         *  put the declarations of these methods in the private section
         *  and never implement them. This prevents the compiler from
         *  implementing an incorrect "default" behavior without us
         *  knowing. (See Scott Meyers book, "Effective C++")
         */
        //copying disabled
        DualDepthPeeling(const DualDepthPeeling&);
        DualDepthPeeling& operator=(const DualDepthPeeling&);
    };

}


#endif // EASY3D_RENDERER_DUAL_DEPTH_PEELING_H
