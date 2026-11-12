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

    // Code can be simplified by omitting frustum and use the Camera class instead.
    // check line 611, void Camera::set_fov_to_fit_scene(), in camera.cpp.

    // Optimization tips: rendering with multi-effects (e.g., shadowing, SSAO)
    // can benefit from sharing the same geometry pass.

#ifdef EASY3D_HAS_VULKAN

    /// \brief Shadow implements the standard shadow map (hard shadow) algorithm.
    /// \class Shadow easy3d/renderer/shadow.h
    /// \details In Vulkan, rendering and presentation belong to the viewer, so begin()
    ///      only publishes the shadow frame of this camera. The viewer marks the
    ///      corresponding scene commands, appends the virtual background and inserts
    ///      the full-screen shadow pass after the scene.
    ///
    ///      Example usage:
    ///          \code
    ///              shadow_->begin(surfaces);
    ///              draw(); // your rendering code here
    ///              shadow_->end();
    ///          \endcode
    class Shadow
    {
    public:
        /// \brief Create a shadow mapper for a camera.
        /// \param cam Camera whose scene is shadowed.
        explicit Shadow(Camera* cam);
        /// Destroy the shadow mapper.
        virtual ~Shadow();

        /// Shadow allows to have a virtual background as the shadow receiver. The virtual background plane is
        /// perpendicular to the light direction and is placed at the far plane of the light frustum.
        /// \note This only works for directional lights.
        /// \return True when the virtual background receiver is enabled.
        bool virtual_background() const { return virtual_background_; }
        /// Enable or disable the virtual background. \see virtual_background()
        /// \param b True to enable the receiver.
        void set_virtual_background(bool b) { virtual_background_ = b; }

        /// Query the virtual background color.
        /// \return Receiver color in RGBA.
        const vec4& virtual_background_color() const { return virtual_background_color_; }
        /// Set the virtual background color.
        /// \param c Receiver color in RGBA.
        void set_virtual_background_color(const vec4& c) { virtual_background_color_ = c; }

        /// Query the size of the shadow map. The shadow is assumed to be square.
        /// \return Shadow-map side length in pixels.
        int shadow_map_size() const { return shadow_map_size_; }
        /// Set the square shadow-map size in pixels. The default is 1024.
        /// \param size Shadow-map side length in pixels.
        void set_shadow_map_size(int size) { shadow_map_size_ = size; }

        /// The distance of the light source to the scene (w.r.t. the scene radius).
        /// \return Light-frustum distance factor.
        float light_distance() const { return light_distance_; }
        /// Set the light-frustum distance relative to the scene radius.
        /// \param dist Requested distance factor; values below 1.1 are clamped to 1.1. Default is 50.
        void set_light_distance(float dist);

        /// Query the darkness of the shadow region. The values is in [0, 1.0] (0: no shadow at all; 1: completely
        /// dark/black). \see set_darkness(float darkness).
        /// \return Shadow darkness in [0,1].
        float darkness() const { return darkness_; }
        /// Set shadow darkness in [0,1] (0 is no darkening, 1 is black). Default is 0.6.
        /// \param darkness Requested shadow darkness; clamped to [0,1].
        void set_darkness(float darkness);

        /// Register the surfaces that cast shadows for the next draw on this camera.
        /// \param surfaces Triangle drawables to use as shadow casters.
        /// \details Pair every call with end().
        void begin(const std::vector<TrianglesDrawable*>& surfaces);
        /// End the shadow registration started by begin().
        void end();
        /// Return the active shadow registration for a camera, or nullptr.
        /// \param camera Camera to query.
        /// \return Active shadow or nullptr.
        static Shadow* active(const Camera* camera);

        /// Return the registered shadow-caster drawables, or an empty list.
        /// \return Registered drawable pointers.
        const std::vector<TrianglesDrawable*>& surfaces() const { return surfaces_; }

        // Shadow frame parameters published by begin() for the registered camera.
        struct FrameData {
            /// Transform from world positions to shadow texture coordinates.
            mat4 shadow_matrix;                 // world position -> shadow texture coordinates
            /// World-space corners of the virtual background receiver.
            std::vector<vec3> background_corners;
            /// Virtual background RGBA color.
            vec4 background_color = vec4(0);
            /// Shadow map size in pixels.
            unsigned shadow_map_size = 1024;
            /// Shadow darkness in [0,1].
            float darkness = 0;
            /// Shadow softness; zero denotes hard shadows.
            float softness = 0;
            /// Near plane of the light frustum.
            float light_near = 1;
            /// Far plane of the light frustum.
            float light_far = 2;
        };
        /// Return published shadow parameters for a camera, if available.
        /// \param camera Camera whose active shadow frame is requested.
        /// \param data Receives the frame data when one is available.
        /// \return True when frame data was written; otherwise false.
        static bool frame_data(const Camera* camera, FrameData& data);

    protected:
        // SoftShadow overrides this with its penumbra softness.
        virtual float frame_softness() const { return 0.0f; }

        // Build a perspective frustum from the camera (FOV widened by 0.2 rad to
        // avoid screen-border artifacts); it bounds the light frustum.
        void compute_camera_frustum();
        // Fit the light's view/projection frustum around the camera frustum and
        // derive the world->shadow-texture matrix.
        void compute_light_frustum();
        // Recompute the virtual background receiver corners at the light far plane.
        void update_virtual_background();

        // The camera whose scene is shadowed.
        Camera* camera_;
        // Camera and light frustums (owned; created on first use).
        Frustum* camera_frustum_;
        Frustum* light_frustum_;
        // Light position derived from the scene center and the light distance.
        vec3    light_pos_;
        // Square shadow-map side length in pixels.
        int     shadow_map_size_;
        // Virtual background receiver state and color (directional lights only).
        bool    virtual_background_;
        vec4    virtual_background_color_;
        // Light distance factor (relative to the scene radius) and shadow darkness.
        float   light_distance_;
        float   darkness_;
        // Light matrices; shadow_matrix_ also contains the clip->texture transform.
        mat4    shadow_matrix_;        // clip to texture transformation is contained
        mat4    light_view_matrix_;
        mat4    light_projection_matrix_;
        // World-space corners of the virtual background receiver.
        std::vector<vec3> background_corners_;
        // Shadow-caster drawables registered by begin().
        std::vector<TrianglesDrawable*> surfaces_;

    private:
        //copying disabled
        Shadow(const Shadow&);
        Shadow& operator=(const Shadow&);
    };

} // namespace easy3d

#else

    class FramebufferObject;

    /// \brief Shadow implements the standard shadow map (hard shadow) algorithm.
    /// \class Shadow easy3d/renderer/shadow.h
    class Shadow
    {
    public:
        /// \brief Create a shadow mapper for a camera.
        /// \param cam Camera whose scene is shadowed.
        explicit Shadow(Camera* cam);
        /// Destroy the shadow mapper.
        virtual ~Shadow();

        /// Shadow allows to have a virtual background as the shadow receiver. The virtual background plane is
        /// perpendicular to the light direction and is placed at the far plane of the light frustum.
        /// \note This only works for directional lights.
        /// \return True when the virtual background receiver is enabled.
        bool virtual_background() const { return virtual_background_; }
        /// Enable or disable the virtual background. \see virtual_background()
        /// \param b True to enable the receiver.
        void set_virtual_background(bool b) { virtual_background_ = b; }

        /// Query the virtual background color.
        /// \return Receiver color in RGBA.
        const vec4& virtual_background_color() const { return virtual_background_color_; }
        /// Set the virtual background color.
        /// \param c Receiver color in RGBA.
        void set_virtual_background_color(const vec4& c) { virtual_background_color_ = c; }

        /// Query the size of the shadow map. The shadow is assumed to be square.
        /// \return Shadow-map side length in pixels.
        int shadow_map_size() const { return shadow_map_size_; }
        /// Set the square shadow-map size in pixels. The default is 1024.
        /// \param size Shadow-map side length in pixels.
        void set_shadow_map_size(int size) { shadow_map_size_ = size; }

        /// The distance of the light source to the scene (w.r.t. the scene radius).
        /// \return Light-frustum distance factor.
        float light_distance() const { return light_distance_; }
        /// Set the light-frustum distance relative to the scene radius.
        /// \param dist Requested distance factor; values below 1.1 are clamped to 1.1. Default is 50.
        void set_light_distance(float dist);

        /// Query the darkness of the shadow region. The values is in [0, 1.0] (0: no shadow at all; 1: completely
        /// dark/black). \see set_darkness(float darkness).
        /// \return Shadow darkness in [0,1].
        float darkness() const { return darkness_; }
        /// Set shadow darkness in [0,1] (0 is no darkening, 1 is black). Default is 0.6.
        /// \param darkness Requested shadow darkness; clamped to [0,1].
        void set_darkness(float darkness);

        /// Render the supplied shadow casters and scene surfaces.
        /// \param surfaces Triangle drawables to render.
        void draw(const std::vector<TrianglesDrawable*>& surfaces);

        /// Register the surfaces that cast shadows for the next draw on this camera.
        /// \param surfaces Triangle drawables to use as shadow casters.
        /// \details Pair every call with end().
        void begin(const std::vector<TrianglesDrawable*>& surfaces);
        /// End the shadow registration started by begin().
        void end();
        /// Return the active shadow registration for a camera, or nullptr.
        /// \param camera Camera to query.
        /// \return Active shadow or nullptr.
        static Shadow* active(const Camera* camera);
        /// Return the registered shadow-caster drawables.
        /// \return Registered drawable pointers.
        const std::vector<TrianglesDrawable*>& surfaces() const { return surfaces_; }

    protected:
        // Create/resize the shadow-map framebuffer to shadow_map_size_.
        virtual void ensure_fbo();
        // Render the shadow casters into the shadow map (depth only) from the light frustum.
        virtual void shadow_map_pass(const std::vector<TrianglesDrawable*>& surfaces);
        // Render the scene with shadow comparison (PCF; PCSS in SoftShadow).
        virtual void render_pass(const std::vector<TrianglesDrawable*>& surfaces);

        // Load the shadow shaders and allocate resources.
        void init();
        // Release framebuffer objects and shaders.
        void clear();

        // Build a perspective frustum from the camera (FOV widened by 0.2 rad).
        void compute_camera_frustum();
        // Fit the light's view/projection frustum around the camera frustum.
        void compute_light_frustum();

        // A background plane perpendicular to the light direction and
        // is placed at the far plane of the light frustum. This only
        // works for directional lights. It might be more natural to
        // have a *real* ground, i.e. up right, contacting the object.
        virtual void update_virtual_background();

        // visualize the frustum (useful for debugging)
        void draw_light_frustum();

        // The camera whose scene is shadowed.
        Camera*	camera_;
        // Shadow-map framebuffer (depth texture).
        FramebufferObject*	fbo_;
        // Camera and light frustums (owned; created on first use).
        Frustum* camera_frustum_;
        Frustum* light_frustum_;
        // Light position derived from the scene center and the light distance.
        vec3    light_pos_;
        // Square shadow-map side length in pixels.
        int		shadow_map_size_;
        // the shadow is cast onto a minimum plane orthogonal to the light direction.
        bool	virtual_background_;
        vec4    virtual_background_color_;
        // Drawable rendering the virtual background receiver plane.
        TrianglesDrawable* virtual_background_drawable_;
        float	light_distance_;	// for perspective light frustum only
        float	darkness_;
        // Light matrices; shadow_matrix_ also contains the clip->texture transform.
        mat4	shadow_matrix_;		// clip to texture transformation is contained
        mat4	light_view_matrix_;
        mat4	light_projection_matrix_;

    private:
        //copying disabled
        Shadow(const Shadow&);
        Shadow& operator=(const Shadow&);

        // Shadow-caster drawables registered by begin().
        std::vector<TrianglesDrawable*> surfaces_;
    };

}

#endif // EASY3D_HAS_VULKAN

#endif // EASY3D_RENDERER_SHADOW_H
