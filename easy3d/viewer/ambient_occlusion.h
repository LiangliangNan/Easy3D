/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work, 
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan. 
*           Easy3D: a lightweight, easy-to-use, and efficient C++ 
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef EASY_AMBIENT_OCCLUSION_H
#define EASY_AMBIENT_OCCLUSION_H


#include <string>
#include <vector>

#include <easy3d/core/types.h>


namespace easy3d {

    class Model;
    class Camera;
    class FramebufferObject;
	
	// AmbientOcclusion implements the traditional Screen Space Ambient Occlusion technique. 
    //
    // Optimization tips: rendering with multi-effects (e.g., shadowing, SSAO)
    // can benefit from sharing the same geometry pass.


    class AmbientOcclusion
    {   
    public:
		AmbientOcclusion(Camera* cam);
        virtual ~AmbientOcclusion();

        // used to compute sample radius (in pixels [0, 4]?).
        float radius() const { return radius_; }
        void  set_radius(float r) { radius_ = r; }

        float bias() const { return bias_; }
        void  set_bias(float b) { bias_ = b; }

        // generate and return the ssao texture
        virtual unsigned int generate(const std::vector<Model*>& models);
        // return the ssao texture
        unsigned int ssao_texture() const;

		// show the occlusion texture on the screen region
		void  draw_occlusion(int x, int y, int w, int h);

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

        int     width_;
        int     height_;

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


#endif	//  EASY_AMBIENT_OCCLUSION_H
