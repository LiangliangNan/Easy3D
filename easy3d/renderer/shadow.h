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

#ifndef EASY3D_RENDERER_SHADOW_H
#define EASY3D_RENDERER_SHADOW_H

#include <vector>
#include <easy3d/core/types.h>


namespace easy3d {

    class Camera;
    class Frustum;
    class TrianglesDrawable;
    class FramebufferObject;

    // Code can be simplified by omitting frustum and use the Camera class instead.
    // check line 611, void Camera::setFOVToFitScene(), in camera.cpp.

    // Optimization tips: rendering with multi-effects (e.g., shadowing, SSAO)
    // can benefit from sharing the same geometry pass.

    /// \brief Shadow implements the standard shadow map (hard shadow) algorithm.
    /// \class Shadow easy3d/renderer/shadow.h
    class Shadow
    {
    public:
        /// \brief Constructor
        /// \param cam The camera used in the view
        explicit Shadow(Camera* cam);
        virtual ~Shadow();

        /// Shadow allows to have a virtual background as the shadow receiver. The virtual background plane is
        /// perpendicular to the light direction and is placed at the far plane of the light frustum.
        /// \note This only works for directional lights.
        bool virtual_background() const { return virtual_background_; }
        /// Enable/Disable the virtual background. \see virtual_background()
        void set_virtual_background(bool b) { virtual_background_ = b; }

        /// Query the virtual background color.
        const vec4& virtual_background_color(const vec4& c) { return virtual_background_color_; }
        /// Set the virtual background color.
        void set_virtual_background_color(const vec4& c) { virtual_background_color_ = c; }

        /// Query the size of the shadow map. The shadow is assumed to be square.
        int shadow_map_size() const { return shadow_map_size_; }
        /// Set/Change the size of the shadow map. The shadow is assumed to be square. Default: 1024 by 1024.
        void set_shadow_map_size(int size) { shadow_map_size_ = size; }

        /// The distance of the light source to the scene scene (w.r.t the scene radius).
        float light_distance() const { return light_distance_; }
        /// Set/Change the distance of the light source to the scene scene (w.r.t the scene radius). Default value is
        /// 50 (large enough to mimic the directional light used in the default viewer).
        /// \note Value must be > 1.0.
        void set_light_distance(float dist);

        /// Query the darkness of the shadow region. The values is in [0, 1.0] (0: no shadow at all; 1: completely
        /// dark/black). \see set_darkness(float darkness).
        float darkness() const { return darkness_; }
        /// Set the darkness of the shadow region. Values must be in [0, 1.0] (0: no shadow at all; 1: completely
        /// dark/black). The default value is 0.6.
        void set_darkness(float darkness);

        /// Rendering the surfaces.
        void draw(const std::vector<TrianglesDrawable*>& surfaces);

    protected:
        virtual void ensure_fbo();
        virtual void shadow_map_pass(const std::vector<TrianglesDrawable*>& surfaces);
        virtual void render_pass(const std::vector<TrianglesDrawable*>& surfaces);

        void clear();
        void init();

        // visualize the frustum (useful for debugging)
        void draw_light_frustum();

        void compute_camera_frustum();
        void compute_light_frustum();

        // A background plane perpendicular to the light direction and
        // is placed at the far plane of the light frustum. This only
        // works for directional lights. It might be more natural to
        // have a *real* ground, i.e. up right, contacting the object.
        virtual void update_virtual_background();

    protected:
        Camera*	camera_;

        FramebufferObject*	fbo_;

        Frustum* camera_frustum_;
        Frustum* light_frustum_;

        vec3    light_pos_;

        int		shadow_map_size_;

        // the shadow is cast onto a minimum plane orthogonal to the light direction.
        bool	virtual_background_;
        vec4    virtual_background_color_;
        TrianglesDrawable* virtual_background_drawable_;

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

#endif // EASY3D_RENDERER_SHADOW_H
