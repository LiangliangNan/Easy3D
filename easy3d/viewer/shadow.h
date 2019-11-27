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

#ifndef EASY3D_SHADOW_H
#define EASY3D_SHADOW_H

#include <vector>
#include <easy3d/core/types.h>


namespace easy3d {

    class Camera;
    class Frustum;
    class TrianglesDrawable;
    class FramebufferObject;

    // Code can be simplified by ommittting frustum and use the Camera class instead.
    // check line 611, void Camera::setFOVToFitScene(), in camera.cpp.

    // Optimization tips: rendering with multi-effects (e.g., shadowing, SSAO)
    // can benefit from sharing the same geometry pass.

    // implemented the standard shadow map (hard shadow) algorithm.
    class Shadow
    {
    public:
        Shadow(Camera* cam);
        virtual ~Shadow();

        // A background plane perpendicular to the light direction and
        // is placed at the far plane of the light frustum. This only
        // works for directional lights.
        bool ortho_background() const { return orhto_background_; }
        void set_ortho_background(bool b) { orhto_background_ = b; }
        void set_background_color(const vec3& c) { background_color_ = c; }

        // assumed to be a square shadow map. Defauly: 1024.
        int shadow_map_size() const { return shadow_map_size_; }
        void set_shadow_map_size(int size) { shadow_map_size_ = size; }

        // the distance of the light source to the scene scene (w.r.t the
        // scene radius). Value must be > 1.0. Default: 50 (large enough
        // to mimic the directional light used in the default viewer).
        float light_distance() const { return light_distance_; }
        void set_light_distance(float dist);

        // the darkness of the shadow region. Values must be in [0, 1.0].
        // (0: no shadow at all; 1: completely dark/black). Default: 0.6.
        float darkness() const { return darkness_; }
        void set_darkness(float darkness);

        void draw(const std::vector<TrianglesDrawable*>& surfaces);

    protected:
        virtual void ensure_fbo();
        virtual void shadow_map_pass(const std::vector<TrianglesDrawable*>& surfaces);
        virtual void render_pass(const std::vector<TrianglesDrawable*>& surfaces);

        void clear();
        void init();

        // visualize the shadow map/frustum (useful for debugging)
        void draw_shadow_map(int w, int h);
        void draw_light_frustum();

        void compute_camera_frustum();
        void compute_light_frustum();

        // A background plane perpendicular to the light direction and
        // is placed at the far plane of the light frustum. This only
        // works for directional lights. It might be more natural to
        // have a *real* ground, i.e. up right, contacting the object.
        virtual void update_ortho_background();

    protected:
        Camera*	camera_;

        FramebufferObject*	fbo_;

        Frustum* camera_frustum_;
        Frustum* light_frustum_;

        vec3    light_pos_;

        int		shadow_map_size_;

        // the shadow is cast onto a minimum plane orthogonal to the light direction.
        bool	orhto_background_;
        TrianglesDrawable* background_;
        vec3    background_color_;

        float	light_distance_;	// for perspective light frustum only
        float	darkness_;

        mat4	shadow_matrix_;		// clip to texture transformation is contained
        mat4	light_view_matrix_;
        mat4	light_projection_matrix_;

    private:
        //copying disabled
        Shadow(const Shadow&);
        Shadow& operator=(const Shadow&);
    };

}

#endif // EASY3D_SHADOW_H
