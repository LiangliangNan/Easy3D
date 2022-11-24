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

#ifndef EASY3D_RENDERER_EYE_DOME_LIGHTING_H
#define EASY3D_RENDERER_EYE_DOME_LIGHTING_H


#include <string>

namespace easy3d {


    /**
     * \brief An implementation of the Eye Dome Lighting (EDL) technique.
     *
     * \class EyeDomeLighting easy3d/renderer/eye_dome_lighting.h
     *
     * \details This class implements an EDL offscreen shading method, which is useful  for rendering point clouds
     * without normal information.
     *
     * Two image resolutions are used.
     *
     * This pass expects an initialized depth buffer and color buffer. Initialized buffers means they have been
     * cleared with the farthest z-value and background color/gradient/transparent color.
     * An opaque pass may have been performed right after the initialization.
     *
     * Optimization tips:
     * Rendering with multi-effects (e.g., shadowing, SSAO) an benefit from sharing the same geometry pass.
     *
     * Example usage:
     *      \code
     * 		if (camera()->type() == Camera::PERSPECTIVE)
     *			edl_->begin(width(), height(), camera()->zNear(), camera()->zFar());
     *		else {
     *			float r = camera()->sceneRadius();
     *			edl_->begin(width(), height(), 2.0f * r, 10.0f * r);
     *		}
     *      draw(); // your rendering code here
     *      edl_->end(camera()->sceneRadius());
     *      \endcode
     */

    class Camera;
    class ShaderProgram;
    class FramebufferObject;

    class EyeDomeLighting
    {
    public:
        /// \brief Constructor
        /// \param cam The camera used in the view
        explicit EyeDomeLighting(Camera* cam);
        virtual ~EyeDomeLighting();

        void begin();
        void end();

    protected:

        // Initialization of required framebuffer objects
        void init_frame_buffers(int w, int h);

        // Initialization of required GLSL shaders
        void init_shader_programs();

        // Render EDL in full resolution buffer
        void shade_high(float scene_radius);

        // Render EDL in middle resolution buffer
        void shade_low();

        // Render EDL in middle resolution buffer
        void blur_low();

        // Compose color and shaded images
        void compose();

        // Release graphics resources
        void clear();

    private:
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

        float neighbours_[8][4];
        float low_res_factor_; // basically 4

        //////////////////////////////////////////////////////////////////////////

        float z_near_;  // near clipping plane
        float z_far_;	// far clipping plane

        int width_;
        int height_;

    private:
        //copying disabled
        EyeDomeLighting(const EyeDomeLighting&);
        EyeDomeLighting& operator=(const EyeDomeLighting&);
    };

}

#endif	// EASY3D_RENDERER_EYE_DOME_LIGHTING_H
