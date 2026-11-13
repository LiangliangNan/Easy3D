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

#ifndef EASY_RENDERER_AMBIENT_OCCLUSION_H
#define EASY_RENDERER_AMBIENT_OCCLUSION_H

#include <vector>
#include <memory>

#include <easy3d/core/types.h>


namespace easy3d {

    class Model;
    class Camera;

#ifdef EASY3D_HAS_VULKAN

    /**
     * \brief Ground-truth ambient occlusion (GTAO), a horizon-based screen-space technique.
     * \class AmbientOcclusion easy3d/renderer/ambient_occlusion.h
     * \details In Vulkan, rendering and presentation belong to the viewer, so begin() only
     *      publishes the radius, bias and projection of this frame. The viewer, which owns
     *      the frame, inserts the corresponding full-screen shading pass after the scene.
     *
     *      Example usage:
     *          \code
     *              ao_->begin(models);
     *              draw(); // your rendering code here
     *              ao_->end();
     *          \endcode
     */
    class AmbientOcclusion
    {
    public:
        /**
         * \brief Constructor
         * \param cam The camera used in the view
         */
        explicit AmbientOcclusion(Camera* cam);
        /**
         * \brief Destructor
         */
        virtual ~AmbientOcclusion();

        /**
         * \brief Sets the sample radius (in view-space units, scaled by the scene radius). Typical value is in range [0, 4].
         * \param r The sample radius
         */
        void  set_radius(float r) { radius_ = r; }
        /**
         * \brief Returns the sample radius.
         * \return The sample radius
         */
        float radius() const { return radius_; }

        /**
         * \brief Sets the bias. Default value is 0.005.
         * \param b The bias value
         */
        void  set_bias(float b) { bias_ = b; }
        /**
         * \brief Returns the bias.
         * \return The bias value
         */
        float bias() const { return bias_; }

        /**
         * \brief Begins the ambient occlusion rendering process. In Vulkan the models are not
         *      re-rendered: the depth buffer of the frame supplies all geometry.
         * \param models The models to be rendered
         */
        void begin(const std::vector< std::shared_ptr<Model> >& models);
        /**
         * \brief Ends the ambient occlusion rendering process.
         */
        void end();

        /**
         * \brief Returns the frame parameters published by begin() for the registered camera.
         * \param camera The camera whose frame parameters are requested.
         * \param radius Receives the sample radius (already scaled by the scene radius).
         * \param bias Receives the bias value.
         * \param projection Receives the camera projection matrix.
         * \return True if parameters were published for the camera; false otherwise.
         */
        static bool frame_params(const Camera* camera, float& radius, float& bias, mat4& projection);

    protected:
        // The camera whose scene is shaded with ambient occlusion.
        Camera* camera_;

        // Effect parameters: sample radius (scaled by the scene radius) and depth bias.
        float	radius_;
        float	bias_;

    private:
        //copying disabled
        AmbientOcclusion(const AmbientOcclusion&);
        AmbientOcclusion& operator=(const AmbientOcclusion&);
    };

} // namespace easy3d

#else

    class FramebufferObject;

    /**
     * \brief Ground-truth ambient occlusion (GTAO), a horizon-based screen-space technique.
     * \class AmbientOcclusion easy3d/renderer/ambient_occlusion.h
     * \details The effect reads the scene depth of the projection buffer, so no geometry pass is needed.
     *
     *      Example usage:
     *          \code
     *              ao_->begin(models);
     *              draw(); // your rendering code here
     *              ao_->end();
     *          \endcode
     */
    class AmbientOcclusion
    {   
    public:
        /**
         * \brief Constructor
         * \param cam The camera used in the view
         */
		explicit AmbientOcclusion(Camera* cam);
        /**
         * \brief Destructor
         */
        virtual ~AmbientOcclusion();

        /**
         * \brief Sets the sample radius (in view-space units, scaled by the scene radius). Typical value is in range [0, 4].
         * \param r The sample radius
         */
        void  set_radius(float r) { radius_ = r; }
        /**
         * \brief Returns the sample radius.
         * \return The sample radius
         */
        float radius() const { return radius_; }

        /**
         * \brief Sets the bias. Default value is 0.005.
         * \param b The bias value
         */
        void  set_bias(float b) { bias_ = b; }
        /**
         * \brief Returns the bias.
         * \return The bias value
         */
        float bias() const { return bias_; }

        /**
         * \brief Begins the ambient occlusion rendering process. Subsequent drawing targets an
         *      offscreen projection buffer that end() shades and composites.
         * \param models The models to be rendered
         */
        void begin(const std::vector< std::shared_ptr<Model> >& models);
        /**
         * \brief Shades the drawn frame with ambient occlusion and composites it
         *      into the framebuffer that was bound before begin().
         */
        void end();

        /**
         * \brief Generates the ambient occlusion texture
         * \details The models are drawn into an offscreen projection buffer (only its depth is
         *      used), and the ambient occlusion term is returned without compositing.
         * \param models The models to be rendered
         * \return The ambient occlusion texture ID
         */
        virtual unsigned int generate(const std::vector< std::shared_ptr<Model> >& models);
        /**
         * \brief Returns the generated ambient occlusion texture ID
         * \return The ambient occlusion texture ID
         */
        unsigned int ssao_texture() const;

    protected:
        /**
         * \brief Initializes the ambient occlusion with the given width and height
         * \details Creates/resizes the projection FBO (color + depth texture) and the
         *      SSAO FBO (two R32F textures for the raw and blurred occlusion term).
         * \param w The width
         * \param h The height
         */
        void init(int w, int h);
        /**
         * \brief Performs the SSAO pass
         * \details Runs the horizon-based ssao/ssao shader on a full-screen quad, reading the
         *      projection depth, and writes the raw occlusion term to the first SSAO attachment.
         */
        void ssao_pass();
        /**
         * \brief Performs the blur pass
         * \details Filters the raw occlusion texture with the depth-aware ssao/blur shader
         *      into the second SSAO color attachment.
         */
        void blur_pass();
        /**
         * \brief Composites the shaded projection buffer into the framebuffer.
         * \details Multiplies the projection color by the blurred occlusion term using the
         *      shared edl/edl_compose shader, without modifying the destination depth.
         */
        void compose();

        // The camera whose scene is shaded with ambient occlusion.
        Camera* camera_;

        // Effect parameters: sample radius (scaled by the scene radius) and depth bias.
        float	radius_;
        float	bias_;

        // Offscreen render targets.
        FramebufferObject*	projection_fbo_; ///< The framebuffer object for the projected scene
        FramebufferObject*	ssao_fbo_;  ///< The framebuffer object for SSAO pass

        int width_;     ///< The projection buffer width
        int height_;    ///< The projection buffer height

    private:
        //copying disabled
		AmbientOcclusion(const AmbientOcclusion&);
        AmbientOcclusion& operator=(const AmbientOcclusion&);
    };


} // namespace easy3d


#endif // EASY3D_HAS_VULKAN

#endif	//  EASY_RENDERER_AMBIENT_OCCLUSION_H
