/** \cond LICENSE
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
 \endcond */

#ifndef EASY3D_RENDERER_EYE_DOME_LIGHTING_H
#define EASY3D_RENDERER_EYE_DOME_LIGHTING_H

#ifdef EASY3D_HAS_VULKAN


namespace easy3d {

    class Camera;

    /**
     * \brief An implementation of the Eye Dome Lighting (EDL) technique.
     * \class EyeDomeLighting easy3d/renderer/eye_dome_lighting.h
     * \details This class implements an EDL shading method, which is useful for rendering point clouds
     *      without normal information.
     *
     *      In Vulkan, rendering and presentation belong to the viewer, so begin() only publishes
     *      the camera parameters for this frame. The viewer, which owns the frame, inserts the
     *      corresponding full-screen shading pass after the scene.
     *
     *      Example usage:
     *          \code
     *              edl_->begin();
     *              draw(); // your rendering code here
     *              edl_->end();
     *          \endcode
     */
    class EyeDomeLighting
    {
    public:
        /**
         * \brief Constructor
         * \param cam The camera used in the view
         */
        explicit EyeDomeLighting(Camera* cam);
        /**
         * \brief Destructor
         */
        virtual ~EyeDomeLighting();
        /**
         * \brief Begins the EDL rendering process.
         */
        void begin();
        /**
         * \brief Ends the EDL rendering process.
         */
        void end();

        /**
         * \brief Returns the frame parameters published by begin() for the registered camera.
         * \param camera The camera whose frame parameters are requested.
         * \param z_near Receives the near depth used for depth linearization.
         * \param z_far Receives the far depth used for depth linearization.
         * \param scene_size Receives the scene size (diagonal of the scene bounding box).
         * \return True if parameters were published for the camera; false otherwise.
         */
        static bool frame_params(const Camera* camera, float& z_near, float& z_far, float& scene_size);

    private:
        //copying disabled
        EyeDomeLighting(const EyeDomeLighting&);
        EyeDomeLighting& operator=(const EyeDomeLighting&);

        // The camera whose scene is shaded with EDL.
        Camera* camera_;
    };

}

#else


namespace easy3d {

    class Camera;
    class ShaderProgram;
    class FramebufferObject;

    /**
     * \brief An implementation of the Eye Dome Lighting (EDL) technique.
     * \class EyeDomeLighting easy3d/renderer/eye_dome_lighting.h
     * \details This class implements an EDL offscreen shading method, which is useful for rendering point clouds
     *      without normal information.
     *
     *      Two image resolutions are used.
     *
     *      This pass expects an initialized depth buffer and color buffer. Initialized buffers means they have been
     *      cleared with the farthest z-value and background color/gradient/transparent color.
     *      An opaque pass may have been performed right after the initialization.
     *
     *      Optimization tips:
     *          Rendering with multi-effects (e.g., shadowing, SSAO) can benefit from sharing the same geometry pass.
     *
     *      Example usage:
     *          \code
     *              edl_->begin();
     *              draw(); // your rendering code here
     *              edl_->end();
     *          \endcode
     */
    class EyeDomeLighting
    {
    public:
        /**
         * \brief Constructor
         * \param cam The camera used in the view
         */
        explicit EyeDomeLighting(Camera* cam);
        /**
         * \brief Destructor
         */
        virtual ~EyeDomeLighting();
        /**
         * \brief Begins the EDL rendering process.
         */
        void begin();
        /**
         * \brief Ends the EDL rendering process.
         */
        void end();

    protected:

        // Create/resize the required framebuffer objects: projection (color + depth),
        // full-res shading, and low-res shading/blur (size / low_res_factor_).
        void init_frame_buffers(int w, int h);

        // Load (or fetch cached) GLSL shaders for EDL shading, composition, and bilateral blur.
        void init_shader_programs();

        // Render EDL in full resolution buffer: shade from the projection depth into high_fbo_.
        void shade_high(float scene_radius);

        // Render EDL in middle resolution buffer (size / low_res_factor_) into low_fbo_.
        void shade_low();

        // Bilaterally filter the low-res shading (depth-aware kernel) into low_fbo_'s second attachment.
        void blur_low();

        // Compose color and shaded images: multiply the projection color by the mixed
        // full/low-res shading, propagating depth to the destination.
        void compose();

        // Release graphics resources: delete the three FBOs (shaders are owned by ShaderManager).
        void clear();

    private:
        //copying disabled
        EyeDomeLighting(const EyeDomeLighting&);
        EyeDomeLighting& operator=(const EyeDomeLighting&);

        // The camera whose scene is shaded with EDL.
        Camera* camera_;

        // Framebuffer object for initial projection

        // used to record scene data
        FramebufferObject*	projection_fbo_;

        //////////////////////////////////////////////////////////////////////////

        // Framebuffer objects for EDL

        // for EDL full res shading
        FramebufferObject*	high_fbo_;

        // for EDL low res shading (image size/4)
        FramebufferObject*	low_fbo_;

        //////////////////////////////////////////////////////////////////////////

        // Shader programs
        ShaderProgram* shade_program_;
        ShaderProgram* compose_program_;
        ShaderProgram* blur_program_;

        // EDL sampling: unit offsets of the 8 image-space neighbours and the low-res downscale factor.
        float neighbours_[8][4];
        float low_res_factor_; // basically 4

        //////////////////////////////////////////////////////////////////////////

        // Depth range used for depth linearization.
        float z_near_;  // near clipping plane
        float z_far_;	// far clipping plane

        // Current viewport size.
        int width_;
        int height_;
    };

}

#endif // EASY3D_HAS_VULKAN

#endif	// EASY3D_RENDERER_EYE_DOME_LIGHTING_H
