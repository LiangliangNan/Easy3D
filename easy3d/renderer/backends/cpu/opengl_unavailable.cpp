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

#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/drawable_lines_2D.h>
#include <easy3d/util/setting.h>
#include <easy3d/renderer/shape.h>
#include <stdexcept>
namespace easy3d {
namespace { [[noreturn]] void unavailable() { throw std::logic_error("OpenGL operation unavailable in this build; submit CPU drawables through VulkanOffScreen"); } }
void PointsDrawable::draw(const Camera*) const { unavailable(); }
void LinesDrawable::draw(const Camera*) const { unavailable(); }
void TrianglesDrawable::draw(const Camera*) const { unavailable(); }
void LinesDrawable2D::draw(const Camera*) const { unavailable(); }
namespace shape { void draw_quad_wire(const Rect &rect, const vec4 &color, int width, int height, float depth) { unavailable(); } }
namespace shape { void draw_quad_filled(const Rect &rect, const vec4 &color, int width, int height, float depth) { unavailable(); } }
namespace shape { void draw_quad_filled(const Rect &rect, unsigned int texture, int width, int height, float depth) { unavailable(); } }
namespace shape { void draw_full_screen_quad(unsigned int texture, float depth) { unavailable(); } }
namespace shape { void draw_depth_texture(const Rect &rect, unsigned int texture, int width, int height, float depth) { unavailable(); } }
namespace shape { void draw_quad(unsigned int positionAttrib, unsigned int texcoordAttrib, int x, int y, int w, int h,
                       int vpw, int vph, float depth) { unavailable(); } }
namespace shape { void draw_full_screen_quad(unsigned int positionAttrib, unsigned int texcoordAttrib, float depth) { unavailable(); } }
namespace shape { void draw_polygon_wire(const Polygon2 &polygon, const vec4 &color, int width, int height, float depth) { unavailable(); } }
namespace shape { void draw_polygon_filled(const Polygon2 &polygon, const vec4 &color, int width, int height, float depth) { unavailable(); } }
namespace shape { void draw_sphere_big_circles(LinesDrawable *drawable, const mat4 &mvp, const mat4 &m, bool axes) { unavailable(); } }
namespace shape { void draw_box_wire(LinesDrawable *drawable, const mat4 &mvp, const mat4 &m, bool abstracted) { unavailable(); } }
void Drawable::opengl_update_vertex_buffer(const std::vector<vec3> &vertices, bool dynamic){ unavailable(); }
void Drawable::opengl_update_color_buffer(const std::vector<vec3> &colors, bool dynamic){ unavailable(); }
void Drawable::opengl_update_normal_buffer(const std::vector<vec3> &normals, bool dynamic){ unavailable(); }
void Drawable::opengl_update_texcoord_buffer(const std::vector<vec2> &texcoords, bool dynamic){ unavailable(); }
void Drawable::opengl_update_element_buffer(const std::vector<unsigned int> &indices){ unavailable(); }
void Drawable::gl_draw() const{ unavailable(); }
void Drawable::opengl_initialize() { unavailable(); }
void Drawable::opengl_release_buffers() { unavailable(); }
void Drawable::opengl_disable_element_buffer() { unavailable(); }
void LinesDrawable2D::update_vertex_buffer(const std::vector<vec2>&, int, int, bool) { unavailable(); }
}
