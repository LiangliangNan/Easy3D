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

#ifndef EASY3D_RENDERER_SHAPE_H
#define EASY3D_RENDERER_SHAPE_H

#include <vector>

#include <easy3d/core/types.h>


namespace easy3d {

    class LinesDrawable;

    /**
     * \brief A collection of functions for rendering basic shapes
     * \namespace easy3d::shape
     */
    namespace shape {

        /// \name Functions for drawing
        // @{

        /**
         * \brief Draws a wire quad defined in the screen space.
         * \param rect The quad.
         * \param color The color.
         * \param width The width of the viewer.
         * \param height The height of the viewer.
         * \param depth The depth at which the quad will be drawn. The depth value is the Normalized Device Coordinates
         *              within the range [-1.0, 1.0], corresponding to the near and far clipping planes, respectively.
         */
        void draw_quad_wire(const Rect &rect, const vec4 &color, int width, int height, float depth);

        /**
         * \brief Draws a solid quad defined in the screen space.
         * \param rect The quad.
         * \param color The color.
         * \param width The width of the viewer.
         * \param height The height of the viewer.
         * \param depth The depth at which the quad will be drawn. The depth value is the Normalized Device Coordinates
         *              within the range [-1.0, 1.0], corresponding to the near and far clipping planes, respectively.
         */
        void draw_quad_filled(const Rect &rect, const vec4 &color, int width, int height, float depth);

        /**
         * \brief Draws a solid quad defined in the screen space.
         * \param rect The quad.
         * \param texture The texture.
         * \param width The width of the viewer.
         * \param height The height of the viewer.
         * \param depth The depth at which the quad will be drawn. The depth value is the Normalized Device Coordinates
         *              within the range [-1.0, 1.0], corresponding to the near and far clipping planes, respectively.
         */
        void draw_quad_filled(const Rect &rect, unsigned int texture, int width, int height, float depth);

        /**
         * \brief Draws a full screen textured quad.
         * \param texture The texture.
         * \param depth The depth at which the quad will be drawn. The depth value is the Normalized Device Coordinates
         *              within the range [-1.0, 1.0], corresponding to the near and far clipping planes, respectively.
         */
        void draw_full_screen_quad(unsigned int texture, float depth);

        /**
         * \brief Draws a quad visualizing a depth texture in a region.
         * \details This is function is similar to draw_quad_filled(... int texture...). The only difference is that
         * a depth texture is rendered as a gray scale image. For depth textures from off-screen rendering (usually
         * from an FBO), the depth values are stored in the R component of the texture.
         * \param rect The quad.
         * \param texture The texture.
         * \param width The width of the viewer.
         * \param height The height of the viewer.
         * \param depth The depth at which the quad will be drawn. The depth value is the Normalized Device Coordinates
         *              within the range [-1.0, 1.0], corresponding to the near and far clipping planes, respectively.
         */
        void draw_depth_texture(const Rect &rect, unsigned int texture, int width, int height, float depth);

        /**
         * \brief Draws a quad defined in the screen space using a bound shader.
         * \details It binds the vertex position and UVs arrays to the given attribute array indices and draws the quad.
         * \param position_attrib The vertex attribute array index that represents position in the current shader.
         * \param texcoord_attrib The vertex attribute array index that represents 2D UVs in the current shader.
         * \param (x, y) The position (i.e., min corner) of the quad.
         * \param (w, h) The size (i.e., width and height) of the quad.
         * \param (vpw, vph) The size (i.e., width and height) of the viewport (of the viewer).
         * \param depth The depth at which the quad will be drawn. The depth value is the Normalized Device Coordinates
         *              within the range [-1.0, 1.0], corresponding to the near and far clipping planes, respectively.
         */
        void draw_quad(unsigned int position_attrib, unsigned int texcoord_attrib, int x, int y, int w, int h, int vpw,
                       int vph, float depth);

        /**
         * \brief Draws a full screen quad using the bound shader.
         * \details It binds the vertex position and UVs arrays to the given attribute array indices and draws the quad.
         * \param position_attrib The vertex attribute array index that represents position in the current shader.
         * \param texcoord_attrib The vertex attribute array index that represents 2D UVs in the current shader.
         * \param depth The depth at which the quad will be drawn. The depth value is the Normalized Device Coordinates
         *              within the range [-1.0, 1.0], corresponding to the near and far clipping planes, respectively.
         */
        void draw_full_screen_quad(unsigned int position_attrib, unsigned int texcoord_attrib, float depth);

        /**
         * \brief Draws a polygon (line loop) in the screen space.
         * \param polygon The polygon.
         * \param color The color.
         * \param width The width of the viewer.
         * \param height The height of the viewer.
         * \param depth The depth at which the polygon will be drawn. The depth value is the Normalized Device Coordinates
         *              within the range [-1.0, 1.0], corresponding to the near and far clipping planes, respectively.
         */
        void draw_polygon_wire(const Polygon2 &polygon, const vec4 &color, int width, int height, float depth);

        /**
         * \brief Draws a filled polygon in the screen space.
         * \param polygon The polygon.
         * \param color The color.
         * \param width The width of the viewer.
         * \param height The height of the viewer.
         * \param depth The depth at which the polygon will be drawn. The depth value is the Normalized Device Coordinates
         *              within the range [-1.0, 1.0], corresponding to the near and far clipping planes, respectively.
         *
         * To make the polygon transparent, do the following
         *  \code
         *      glEnable(GL_BLEND);
         *      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
         *      draw_polygon_filled(...);
         *      glDisable(GL_BLEND);
         *  \endcode
         */
        void draw_polygon_filled(const Polygon2 &polygon, const vec4 &color, int width, int height, float depth);

        /**
         * \brief Draws the outline (the 3 big circles) of a sphere.
         * \param drawable The drawable.
         * \param mvp The model view projection matrix.
         * \param m The transformation matrix defining the sphere's location, radius, and orientation.
         * \param axes \c true will also draw the three main axes.
         */
        void draw_sphere_big_circles(LinesDrawable *drawable, const mat4 &mvp, const mat4 &m, bool axes = true);

        /**
         * \brief Draws a box.
         * \param drawable The drawable.
         * \param mvp The model view projection matrix.
         * \param m The transformation matrix defining the box's location, sizes, and orientation.
         * \param abstracted \c true to draw an abstracted version (only part of its corners).
         */
        void draw_box_wire(LinesDrawable *drawable, const mat4 &mvp, const mat4 &m, bool abstracted = false);
        // @}



        /// \name Functions for creating basic shapes (data preparation)
        // @{

        /**
         * \brief Generates data for a grid as a set of line segments.
         * \details The grid is centered at vec3(0, 0, 0) and lies on the XOY plane.
         * \param x_steps The number of subdivisions along X direction.
         * \param y_steps The number of subdivisions along Y direction.
         * \param scale The scaling factor
         * \param points The points to be returned.
         */
        void create_grid(int x_steps, int y_steps, std::vector<vec3> &points, float depth = 0.0f, float scale = 0.5f);

        /**
         * \brief Generates data for a unit circle as a set of line segments.
		 * \details The circle is centered at vec3(0, 0, 0) and lies on the XOY plane.
         * \param slices. The number of subdivisions.
         */
        void create_circle(int slices, std::vector<vec3> &points, std::vector<unsigned int> &indices);

        /**
         * \brief Generates data for a unit box as a set of line segments.
         * \details The box is centered at vec3(0, 0, 0) and is aligned with the main axes.
         * \param abstracted \c true to draw an abstracted version (only part of its corners).
         */
        void create_box(std::vector<vec3> &points, std::vector<vec3> &colors, bool abstracted = false);


        /**
         * \brief Generates data (points, normals, and colors) for a 3D sphere.
         * \param radius: the radius of the sphere.
         * \param slices: the number of subdivisions around the z axis (similar to lines of longitude).
         * \param stacks: the number of subdivisions along the z axis(similar to lines of latitude).
         */
        void create_sphere(
                const vec3 &center, double radius, int slices, int stacks, const vec3 &color,
                std::vector<vec3> &points, std::vector<vec3> &normals, std::vector<vec3> &colors
        );

        /**
         * \brief Generates data (points, normals, and colors) for a 3D checker sphere.
         * \param radius: the radius of the sphere.
         * \param slices: the number of subdivisions around the z axis (similar to lines of longitude).
         * \param stacks: the number of subdivisions along the z axis(similar to lines of latitude).
         * \param checker_size: how many tiles each checker will occupy
         * \param color1 and color2: the two interchanging colors
         */
        void create_checker_sphere(
                const vec3 &center, double radius, int slices, int stacks, int checker_size,
                const vec3 &color1, const vec3 &color2,
                std::vector<vec3> &points, std::vector<vec3> &normals, std::vector<vec3> &colors
        );

        /**
         * \brief Prepares data (points, normals, and colors) for a 3D cylinder defined by two 3D points s and t.
         */
        void create_cylinder(
                double radius, int slices, const vec3 &s, const vec3 &t, const vec3 &color,
                std::vector<vec3> &points, std::vector<vec3> &normals, std::vector<vec3> &colors
        );

        /**
         * \brief Prepares data (points, normals, and colors) for a 3D cone defined by two 3D points b and t.
         * s is the base center and t is the tip.
         */
        void create_cone(
                double radius, int slices, const vec3 &s, const vec3 &t, const vec3 &color,
                std::vector<vec3> &points, std::vector<vec3> &normals, std::vector<vec3> &colors
        );

        /**
         * \brief Prepares data for representing a torus.
         * \param major_radius The radius of major circle.
         * \param minor_radius The radius of minor circle.
         * \param major_slices The number of subdivisions along the major circle (suggested value 50).
         * \param minor_slices The number of subdivisions along the minor circle (suggested value 20).
         * \param points Returns the points.
         * \param normals Returns the normals.
         */
        void create_torus(double major_radius, double minor_radius, int major_slices, int minor_slices,
                          std::vector<vec3> &points, std::vector<vec3> &normals
        );

        /**
         * \brief Generates data (points) for representing a camera in the 3D world as a set of lines.
         * \param width The width of the camera. A good value can be 5% of the scene radius, or 10% of the
         *      character height (in walking mode).
         * \param fov The vertical field of view of the camera (in radians).
         * \param hw_ratio The aspect ratio of the base quad defined as height/width (default 0.6).
         */
        void create_camera(std::vector<vec3> &points, float width, float fov, float hw_ratio = 0.6f);

        /**
         * \brief Prepares data (points) for representing a camera in the 3D world as a set of triangles.
         * \param width The width of the camera. A good value can be 5% of the scene radius, or 10% of the
         *      character height (in walking mode).
         * \param fov The vertical field of view of the camera (in radians).
         * \param hw_ratio The aspect ratio of the base quad defined as height/width (default 0.6).
         */
        void create_camera(std::vector<vec3> &points, std::vector<unsigned int> &indices, float width, float fov,
                           float hw_ratio = 0.6f);
        // @}


        // create an image plane (useful for rendering images in 3D using camera extrinsic parameters)
//        void create_image_plane(const easy3d::mat3 &R, const easy3d::vec3 &t) {
//            if (!image_plane_) {
//                image_plane_ = new TrianglesDrawable("image_plane");
//                image_plane_->set_visible(show_images_);
//                image_plane_->update_texcoord_buffer({vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 1)});
//                image_plane_->update_index_buffer({0, 2, 1, 0, 3, 2});
//                add_drawable(image_plane_);
//
//                if (!texture_1_)
//                    texture_1_ = Texture::create(resource::directory() + "/data/image_1.png");
//
//                if (texture_1_) {
//                    image_plane_->set_texture(texture_1_);
//                    image_plane_->set_use_texture(true);
//                }
//            }
//
//            if (!view_frustum_) {
//                view_frustum_ = new LinesDrawable("view_frustum");
//                view_frustum_->set_visible(show_images_);
//                add_drawable(view_frustum_);
//            }
//
//            const float fov = camera()->fieldOfView();
//            const float hw_ratio = static_cast<float>(height()) / width();
//            const float halfWidth = camera()->sceneRadius() * 2;  // large enough to put the image plane behind the object
//            const float halfHeight = halfWidth * hw_ratio;
//            const float dist = halfHeight / std::tan(fov * 0.5);
//
//            // coordinates in camera frame
//            const vec3 c(0.0f, 0.0f, 0.0f);  // camera center
//            std::vector<vec3> corners = {
//                    vec3(-halfWidth, -halfHeight, -dist),
//                    vec3(halfWidth, -halfHeight, -dist),
//                    vec3(halfWidth, halfHeight, -dist),
//                    vec3(-halfWidth, halfHeight, -dist)
//            };
//
//            Box3 box = current_model()->bounding_box();
//            // convert to world coordinate system
//            for (auto &p : corners) {
//                p = camera()->worldCoordinatesOf(p);
//                box.grow(p);
//            }
//            box.grow(camera()->position());
//            camera()->setSceneBoundingBox(box);
//
//            image_plane_->update_vertex_buffer(corners);
//
//            corners.push_back(camera()->position());
//            view_frustum_->update_vertex_buffer(corners);
//            view_frustum_->update_index_buffer({0, 4, 1, 4, 2, 4, 3, 4});
//        }

    }

}

#endif  // EASY3D_RENDERER_SHAPE_H
