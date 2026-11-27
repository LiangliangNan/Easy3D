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

#ifndef EASY3D_RENDERER_RENDER_SETTINGS_H
#define EASY3D_RENDERER_RENDER_SETTINGS_H

#include <easy3d/renderer/render_device.h>

namespace easy3d {

    /// Portable shadow-map settings.
    struct ShadowSettings {
        /// Enable shadow mapping.
        bool enabled = false;
        /// Shadow-map side length in pixels; must be a positive power of two.
        unsigned map_size = 1024;
        /// Light-frustum distance as a multiple of the scene radius; at least 1.1.
        float light_distance = 50.0f;
        /// Shadow darkening in [0,1], where one is black.
        float darkness = 0.6f;
        /// Normalized blocker-search/filter radius in [0,1]; zero selects hard shadows.
        float softness = 0.0f;
        /// Render a shadow-receiving background plane without a picking ID.
        bool virtual_background = false;
        /// Linear RGBA color of the virtual background plane.
        vec4 virtual_background_color = vec4(0.8f);
    };

    /// Portable screen-space ambient-occlusion settings.
    struct AmbientOcclusionSettings {
        /// Enable ambient occlusion.
        bool enabled = false;
        /// Sample radius as a fraction of the scene bounding-sphere radius; positive.
        float radius_ratio = 0.045f;
        /// Depth bias in view-space units; nonnegative.
        float bias = 0.005f;
        /// Occlusion darkening strength in [0,1].
        float intensity = 1.0f;
    };

    /// Portable transparent-fragment compositing settings.
    struct TransparencySettings {
        /// Enable a separate transparency pass over visible scene triangles.
        /// \details This explicit switch is required because triangle drawables
        ///      default to opacity 0.6 while ordinary opaque rendering has always
        ///      ignored it. Enabling by method alone would therefore change every
        ///      default scene as soon as any unrelated effect was configured.
        bool enabled = false;
        /// Compositing method; sorted is the portable default.
        graphics::TransparencyMethod method = graphics::TransparencyMethod::sorted;
        /// Maximum depth-peeling layers or linked-list fragments per pixel; positive.
        unsigned layers = 4;
    };

    /// Declarative effect settings shared by all rendering hosts.
    struct RenderSettings {
        /// Shadow-map settings.
        ShadowSettings shadow;
        /// Screen-space ambient-occlusion settings.
        AmbientOcclusionSettings ambient_occlusion;
        /// Enable eye-dome lighting.
        bool eye_dome_lighting = false;
        /// Transparent-fragment compositing settings.
        TransparencySettings transparency;
        /// Up to two world-space halfspaces, each retaining dot(plane, vec4(position,1)) >= 0.
        std::vector<vec4> clipping_planes;
    };

    /// Validate all settings, including values belonging to disabled effects.
    /// \throw std::invalid_argument if a value violates its documented range or representation.
    void validate_render_settings(const RenderSettings &settings);

    /// Validate settings and reject explicitly enabled effects or methods unsupported by a device.
    /// \throw std::invalid_argument if the settings are invalid or unsupported.
    void validate_render_settings(const RenderSettings &settings,
                                  const graphics::DeviceCapabilities &capabilities);

} // namespace easy3d

#endif // EASY3D_RENDERER_RENDER_SETTINGS_H
