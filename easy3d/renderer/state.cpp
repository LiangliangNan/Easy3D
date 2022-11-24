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

#include <easy3d/renderer//state.h>
#include <easy3d/util/logging.h>


namespace easy3d {


    State::Material::Material(const vec4 &ambi, const vec4 &spec, float shin)
            : ambient(ambi), specular(spec), shininess(shin) {
    }


    State::State()
            : visible_(true), selected_(false), coloring_method_(UNIFORM_COLOR), color_(0.8f, 0.8f, 0.8f, 1.0f),
              property_location_(VERTEX), property_name_("uniform color"), lighting_(true),
              lighting_two_sides_(false), distinct_back_color_(true), back_color_(vec4(1, 0, 0, 1)),
              texture_(nullptr), texture_repeat_(1.0f), texture_fractional_repeat_(0.0f), ssao_enabled_(false),
              clamp_range_(true), clamp_lower_(0.05f), clamp_upper_(0.05f), plane_clip_discard_primitive_(false),
              highlight_(false), highlight_range_(-1, -1) {
    }


    State::State(const State &s)
            : visible_(s.is_visible()), selected_(s.is_selected()), coloring_method_(s.coloring_method()),
              color_(s.color()), property_location_(s.property_location()),
              property_name_(s.property_name()), lighting_(s.lighting()),
              lighting_two_sides_(s.lighting_two_sides()),
              distinct_back_color_(s.distinct_back_color()), back_color_(s.back_color()),
              texture_(s.texture()), texture_repeat_(s.texture_repeat()),
              texture_fractional_repeat_(s.texture_fractional_repeat()),
              ssao_enabled_(s.is_ssao_enabled()), clamp_range_(s.clamp_range()),
              clamp_lower_(s.clamp_lower()), clamp_upper_(s.clamp_upper()),
              material_(s.material()), plane_clip_discard_primitive_(s.plane_clip_discard_primitive()),
              highlight_(s.highlight()), highlight_range_(s.highlight_range()) {
    }


    State &State::operator=(const State &s) {
        visible_ = s.is_visible();
        selected_ = s.is_selected();
        coloring_method_ = s.coloring_method();
        color_ = s.color();
        property_location_ = s.property_location();
        property_name_ = s.property_name();
        lighting_ = s.lighting();
        lighting_two_sides_ = s.lighting_two_sides();
        distinct_back_color_ = s.distinct_back_color();
        back_color_ = s.back_color();
        texture_ = s.texture();
        texture_repeat_ = s.texture_repeat();
        texture_fractional_repeat_ = s.texture_fractional_repeat();
        ssao_enabled_ = s.is_ssao_enabled();
        clamp_range_ = s.clamp_range();
        clamp_lower_ = s.clamp_lower();
        clamp_upper_ = s.clamp_upper();
        material_ = s.material();
        plane_clip_discard_primitive_ = s.plane_clip_discard_primitive();
        highlight_ = s.highlight();
        highlight_range_ = s.highlight_range();
        return *this;
    }

    void State::set_uniform_coloring(const vec4 &color) {
        coloring_method_ = UNIFORM_COLOR;
        color_ = color;
        property_location_ = VERTEX;
        property_name_ = "uniform color";
    }


    void State::set_property_coloring(Location color_location, const std::string &color_name) {
        coloring_method_ = COLOR_PROPERTY;
        property_location_ = color_location;
        property_name_ = color_name;
    }


    void State::set_texture_coloring(Location texcoord_location, const std::string &texcoord_name,
                                     const Texture *texture, float repeat, float repeat_fraction) {
        coloring_method_ = TEXTURED;
        property_location_ = texcoord_location;
        property_name_ = texcoord_name;
        texture_ = texture;
        texture_repeat_ = repeat;
        texture_fractional_repeat_ = repeat_fraction;
    }


    void State::set_scalar_coloring(Location scalar_location, const std::string &scalar_name,
                                    const Texture *texture, float clamp_lower, float clamp_upper) {
        coloring_method_ = SCALAR_FIELD;
        property_location_ = scalar_location;
        property_name_ = scalar_name;
        texture_ = texture;
        texture_repeat_ = 1.0f;
        texture_fractional_repeat_ = 0.0f;
        clamp_lower_ = clamp_lower;
        clamp_upper_ = clamp_upper;
    }


    void State::set_coloring(Method method, Location location, const std::string &name) {
        coloring_method_ = method;
        property_location_ = location;
        property_name_ = name;
    }

}
