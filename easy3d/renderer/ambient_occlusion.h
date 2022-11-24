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


#include <string>
#include <vector>

#include <easy3d/core/types.h>


namespace easy3d {

    class Model;
    class Camera;
    class FramebufferObject;

    /**
     * \brief Traditional Screen Space Ambient Occlusion (SSAO) technique.
     * \class AmbientOcclusion easy3d/renderer/ambient_occlusion.h
     * \details Optimization tip: rendering with multi-effects (e.g., shadowing, SSAO) can benefit from using a shared
     * geometry pass.
     */
    class AmbientOcclusion
    {   
    public:
        /// \brief Constructor
        /// \param cam The camera used in the view
		explicit AmbientOcclusion(Camera* cam);
        virtual ~AmbientOcclusion();

        /// \brief Sets the sample radius (in pixels). Typical value is in range [0, 4].
        void  set_radius(float r) { radius_ = r; }
        /// \brief Returns the sample radius.
        float radius() const { return radius_; }

        /// \brief Sets the bias. Default value is 0.005.
        void  set_bias(float b) { bias_ = b; }
        /// \brief Returns the bias.
        float bias() const { return bias_; }

        /// \brief Generates the SSAO texture
        /// \return The SSAO texture ID
        virtual unsigned int generate(const std::vector<Model*>& models);
        /// \brief Returns the generated SSAO texture ID
        unsigned int ssao_texture() const;

    protected:
        void init(int w, int h);

        void geometry_pass(const std::vector<Model*>& models);
        void ssao_pass();
        void blur_pass();

        void generate_noise(int width, int height);

    protected:
        Camera* camera_;

        float	radius_;
        float	bias_;

        FramebufferObject*	geom_fbo_;
        FramebufferObject*	ssao_fbo_;

        std::vector<vec3> ssao_kernel_;
        unsigned int	  noise_texture_;

    private:
        //copying disabled
		AmbientOcclusion(const AmbientOcclusion&);
        AmbientOcclusion& operator=(const AmbientOcclusion&);
    };


} // namespace easy3d


#endif	//  EASY_RENDERER_AMBIENT_OCCLUSION_H
