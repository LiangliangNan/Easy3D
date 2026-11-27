/** \cond LICENSE
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering of 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
 * Easy3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Easy3D. If not, see <http://www.gnu.org/licenses/>.
 * \endcond
 */

#include <easy3d/renderer/render_settings.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>

namespace easy3d {
namespace {

    void require_finite(float value, const char *name) {
        if (!std::isfinite(value))
            throw std::invalid_argument(std::string(name) + " must be finite");
    }

    void require_unit_interval(float value, const char *name) {
        require_finite(value, name);
        if (value < 0.0f || value > 1.0f)
            throw std::invalid_argument(std::string(name) + " must be in [0,1]");
    }

    void require_transparency_method(graphics::TransparencyMethod method) {
        switch (method) {
            case graphics::TransparencyMethod::sorted:
            case graphics::TransparencyMethod::average:
            case graphics::TransparencyMethod::peeling:
            case graphics::TransparencyMethod::linked_lists:
                return;
        }
        throw std::invalid_argument("unknown transparency method");
    }

} // namespace

    void validate_render_settings(const RenderSettings &settings) {
        const auto &shadow = settings.shadow;
        if (shadow.map_size == 0 || (shadow.map_size & (shadow.map_size - 1)) != 0)
            throw std::invalid_argument("shadow map_size must be a positive power of two");
        if (shadow.map_size > unsigned(std::numeric_limits<int>::max()))
            throw std::invalid_argument("shadow map_size exceeds the host effect range");
        require_finite(shadow.light_distance, "shadow light_distance");
        if (shadow.light_distance < 1.1f)
            throw std::invalid_argument("shadow light_distance must be at least 1.1");
        require_unit_interval(shadow.darkness, "shadow darkness");
        require_unit_interval(shadow.softness, "shadow softness");
        for (unsigned i = 0; i < 4; ++i)
            require_finite(shadow.virtual_background_color[i], "shadow virtual_background_color");

        const auto &ao = settings.ambient_occlusion;
        require_finite(ao.radius_ratio, "ambient_occlusion radius_ratio");
        if (ao.radius_ratio <= 0.0f)
            throw std::invalid_argument("ambient_occlusion radius_ratio must be positive");
        require_finite(ao.bias, "ambient_occlusion bias");
        if (ao.bias < 0.0f)
            throw std::invalid_argument("ambient_occlusion bias must be nonnegative");
        require_unit_interval(ao.intensity, "ambient_occlusion intensity");

        require_transparency_method(settings.transparency.method);
        if (settings.transparency.layers == 0)
            throw std::invalid_argument("transparency layers must be positive");

        graphics::normalize_clipping_planes(settings.clipping_planes);
    }

    void validate_render_settings(const RenderSettings &settings,
                                  const graphics::DeviceCapabilities &capabilities) {
        validate_render_settings(settings);
        if (settings.shadow.enabled && !capabilities.shadow)
            throw std::invalid_argument("rendering device does not support shadows");
        if (settings.ambient_occlusion.enabled && !capabilities.ambient_occlusion)
            throw std::invalid_argument("rendering device does not support ambient occlusion");
        if (settings.eye_dome_lighting && !capabilities.eye_dome_lighting)
            throw std::invalid_argument("rendering device does not support eye-dome lighting");
        if (settings.transparency.enabled) {
            const auto method = settings.transparency.method;
            if (std::find(capabilities.transparency_methods.begin(),
                          capabilities.transparency_methods.end(), method) ==
                capabilities.transparency_methods.end())
                throw std::invalid_argument("rendering device does not support the transparency method");
        }
    }

} // namespace easy3d
