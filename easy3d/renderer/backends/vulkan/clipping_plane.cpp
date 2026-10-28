// SPDX-License-Identifier: GPL-3.0-or-later
#include "clipping_plane.h"
#include "shape.h"
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/manipulated_frame.h>

namespace easy3d { namespace vulkan {
std::vector<vec4> ClippingPlaneRenderer::planes(const ClippingPlane& plane) {
    if (!plane.is_enabled()) return {};
    const vec3 n = plane.normal(), c = plane.center();
    // Match ClippingPlane::set_program(): keep the negative-normal halfspace,
    // or a slab ending at the plane, rather than the opposite public Plane3.
    std::vector<vec4> result{vec4(-n, dot(n,c))};
    if (plane.cross_section())
        result.push_back(vec4(n, -dot(n,c) + plane.cross_section_width()*plane.scene_radius()));
    return graphics::normalize_clipping_planes(result);
}
std::vector<graphics::DrawCommand> ClippingPlaneRenderer::draw(const ClippingPlane& plane, const Camera& camera) {
    if (!plane.is_enabled() || !plane.is_visible()) return {};
    const float r = plane.scene_radius();
    const bool geometry_changed = !face_ || radius_ != r;
    const std::vector<vec3> corners{vec3(-r,-r,0),vec3(r,-r,0),vec3(r,r,0),vec3(-r,r,0)};
    if (geometry_changed) {
        std::vector<graphics::Vertex> vertices;
        for (unsigned i : {0u,1u,2u,0u,2u,3u}) {
            graphics::Vertex vertex; vertex.position = corners[i]; vertices.push_back(vertex);
        }
        face_ = std::make_shared<graphics::Geometry>(std::move(vertices)); radius_ = r;
    }
    const mat4 mvp = camera.modelViewProjectionMatrix()*plane.manipulator()->frame()->matrix();
    bool changed = geometry_changed || width_ != camera.screenWidth() || height_ != camera.screenHeight();
    for (unsigned i=0; i<16; ++i) changed = changed || wire_mvp_[i] != mvp[i];
    if (changed) {
        const std::vector<vec3> edges{corners[0],corners[1],corners[1],corners[2],
                                      corners[2],corners[3],corners[3],corners[0]};
        wire_ = shape::draw_lines_3d(edges,std::vector<vec3>(8,vec3(1)),mvp,
                                    camera.screenWidth(),camera.screenHeight());
        wire_mvp_ = mvp; width_ = camera.screenWidth(); height_ = camera.screenHeight();
    }
    graphics::DrawCommand fill; fill.geometry = face_; fill.model_view_projection = mvp;
    fill.color = plane.color(); fill.overlay = fill.overlay_depth_test = true;
    auto wire = wire_; wire.color = plane.color();
    return {fill,wire};
}
} }
