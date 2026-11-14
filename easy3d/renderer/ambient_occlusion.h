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
         * \brief Sets the sample radius as a fraction of the scene bounding-sphere radius.
         * \details The default is 0.045. Values in the range [0.02, 0.1] are typical: larger
         *      values capture broader occlusion but blur out contact shadows, and values above
         *      0.3 start to darken whole surfaces rather than creases and contacts.
         * \param ratio The sample radius relative to the scene radius (must be positive)
         */
        void  set_radius_ratio(float ratio) { radius_ratio_ = ratio; }
        /**
         * \brief Returns the sample radius as a fraction of the scene bounding-sphere radius.
         * \return The sample radius relative to the scene radius
         */
        float radius_ratio() const { return radius_ratio_; }

        /**
         * \brief Sets the depth bias (in view-space units). Default value is 0.005.
         * \details The bias is the smallest elevation above the tangent plane that still counts
         *      as an occluder. It suppresses self-occlusion of nearly coplanar samples and should
         *      be small compared with the world-space radius (radius_ratio() times the scene radius).
         * \param b The bias value
         */
        void  set_bias(float b) { bias_ = b; }
        /**
         * \brief Returns the depth bias.
         * \return The bias value
         */
        float bias() const { return bias_; }

        /**
         * \brief Sets the strength of the darkening, in [0, 1]. Default value is 1.
         * \details The strength interpolates between no darkening (0) and the full occlusion
         *      term (1). It scales the effect without changing the sampled neighborhood.
         * \param s The strength
         */
        void  set_intensity(float s) { intensity_ = s; }
        /**
         * \brief Returns the strength of the darkening.
         * \return The strength value
         */
        float intensity() const { return intensity_; }

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
         * \param radius_ratio Receives the sample radius as a fraction of the scene bounding-sphere radius.
         * \param bias Receives the bias value.
         * \param intensity Receives the strength of the darkening.
         * \param projection Receives the camera projection matrix.
         * \return True if parameters were published for the camera; false otherwise.
         */
        static bool frame_params(const Camera* camera, float& radius_ratio, float& bias, float& intensity,
                                 mat4& projection);

    protected:
        // The camera whose scene is shaded with ambient occlusion.
        Camera* camera_;

        // Effect parameters: sample radius relative to the scene radius, depth bias, strength.
        float	radius_ratio_;
        float	bias_;
        float	intensity_;

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
         * \brief Sets the sample radius as a fraction of the scene bounding-sphere radius.
         * \details The default is 0.045. Values in the range [0.02, 0.1] are typical: larger
         *      values capture broader occlusion but blur out contact shadows, and values above
         *      0.3 start to darken whole surfaces rather than creases and contacts.
         * \param ratio The sample radius relative to the scene radius (must be positive)
         */
        void  set_radius_ratio(float ratio) { radius_ratio_ = ratio; }
        /**
         * \brief Returns the sample radius as a fraction of the scene bounding-sphere radius.
         * \return The sample radius relative to the scene radius
         */
        float radius_ratio() const { return radius_ratio_; }

        /**
         * \brief Sets the depth bias (in view-space units). Default value is 0.005.
         * \details The bias is the smallest elevation above the tangent plane that still counts
         *      as an occluder. It suppresses self-occlusion of nearly coplanar samples and should
         *      be small compared with the world-space radius (radius_ratio() times the scene radius).
         * \param b The bias value
         */
        void  set_bias(float b) { bias_ = b; }
        /**
         * \brief Returns the depth bias.
         * \return The bias value
         */
        float bias() const { return bias_; }

        /**
         * \brief Sets the strength of the darkening, in [0, 1]. Default value is 1.
         * \details The strength interpolates between no darkening (0) and the full occlusion
         *      term (1). It scales the effect without changing the sampled neighborhood.
         * \param s The strength
         */
        void  set_intensity(float s) { intensity_ = s; }
        /**
         * \brief Returns the strength of the darkening.
         * \return The strength value
         */
        float intensity() const { return intensity_; }

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
        unsigned int ao_texture() const;

    protected:
        /**
         * \brief Initializes the ambient occlusion with the given width and height
         * \details Creates/resizes the projection FBO (color + depth texture) and the
         *      AO FBO (two R32F textures for the raw and blurred occlusion term).
         * \param w The width
         * \param h The height
         */
        void init(int w, int h);
        /**
         * \brief Performs the AO pass
         * \details Runs the horizon-based ao/ao shader on a full-screen quad, reading the
         *      projection depth, and writes the raw occlusion term to the first AO attachment.
         */
        void ao_pass();
        /**
         * \brief Performs the blur pass
         * \details Filters the raw occlusion texture with the depth-aware ao/blur shader
         *      into the second AO color attachment.
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

        // Effect parameters: sample radius relative to the scene radius, depth bias, strength.
        float	radius_ratio_;
        float	bias_;
        float	intensity_;

        // Offscreen render targets.
        FramebufferObject*	projection_fbo_; ///< The framebuffer object for the projected scene
        FramebufferObject*	ao_fbo_;  ///< The framebuffer object for AO pass

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
