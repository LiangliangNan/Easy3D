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

#ifndef EASY_RENDERER_AMBIENT_OCCLUSION_H
#define EASY_RENDERER_AMBIENT_OCCLUSION_H

#include <vector>
#include <memory>

#include <easy3d/core/types.h>


namespace easy3d {

    class Model;
    class Camera;
    class FramebufferObject;

    /**
     * \brief Traditional Screen Space Ambient Occlusion (SSAO) technique.
     * \class AmbientOcclusion easy3d/renderer/ambient_occlusion.h
     * \details Optimization tip: rendering with multi-effects (e.g., shadowing, SSAO) can benefit from using a shared
     *      geometry pass.
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
         * \brief Sets the sample radius (in pixels). Typical value is in range [0, 4].
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
         * \brief Generates the SSAO texture
         * \param models The models to be rendered
         * \return The SSAO texture ID
         */
        virtual unsigned int generate(const std::vector< std::shared_ptr<Model> >& models);
        /**
         * \brief Returns the generated SSAO texture ID
         * \return The SSAO texture ID
         */
        unsigned int ssao_texture() const;

    protected:
        /**
         * \brief Initializes the SSAO with the given width and height
         * \param w The width
         * \param h The height
         */
        void init(int w, int h);
        /**
         * \brief Performs the geometry pass
         * \param models The models to be rendered
         */
        void geometry_pass(const std::vector< std::shared_ptr<Model> >& models);
        /**
         * \brief Performs the SSAO pass
         */
        void ssao_pass();
        /**
         * \brief Performs the blur pass
         */
        void blur_pass();
        /**
         * \brief Generates noise texture
         * \param width The width of the noise texture
         * \param height The height of the noise texture
         */
        void generate_noise(int width, int height);

    protected:
        Camera* camera_;

        float	radius_;
        float	bias_;

        FramebufferObject*	geom_fbo_;  ///< The framebuffer object for geometry pass
        FramebufferObject*	ssao_fbo_;  ///< The framebuffer object for SSAO pass

        std::vector<vec3> ssao_kernel_;     ///< The SSAO kernel
        unsigned int	  noise_texture_;   ///< The noise texture

    private:
        //copying disabled
		AmbientOcclusion(const AmbientOcclusion&);
        AmbientOcclusion& operator=(const AmbientOcclusion&);
    };


} // namespace easy3d


#endif	//  EASY_RENDERER_AMBIENT_OCCLUSION_H
