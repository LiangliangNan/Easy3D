/** \cond LICENSE
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
 \endcond */

#include "viewer.h"

#include <easy3d/core/point_cloud.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/algo/tessellator.h>
#include <algorithm>
#include <limits>

using namespace easy3d;

// \cond
TutorialPointSelection::TutorialPointSelection(const std::string &title) : Viewer(title) {
    // manual_ is the full help text (shown on demand); hint_ is the short HUD reminder.
    manual_ =
        "-------------- Point selection usage --------------\n"
        "Shift + left/right drag: select/deselect points\n"
        "L: lasso mode; R: rectangle mode; Escape: cancel\n"
        "---------------------------------------------------\n";
    hint_ =
        "Pressing Shift, drag the mouse to select (left) or deselect (right) points\n"
        "\tL: lasso mode (default)\n"
        "\tR: rectangle mode";
}

// Shift + left/right press starts a selection gesture — but only if the current model is a
// point cloud (region selection is defined on points here). Other presses go to the base
// Viewer so camera navigation still works.
bool TutorialPointSelection::mouse_press_event(int x, int y, int button, int modifiers) {
    if ((modifiers & MODIF_SHIFT) && (button == BUTTON_LEFT || button == BUTTON_RIGHT)) {
        cancel();
        for (const auto &model : models())
            if (model.get() == current_model() && dynamic_cast<easy3d::PointCloud *>(model.get())) {
                target_ = model;
                active_ = true;
                button_ = button; // remember: left = select, right = deselect
                // The region is stored in framebuffer pixels (top-left origin) because that
                // is what the GPU selection query expects. Mouse events arrive in logical
                // screen units, which differ by the DPI scaling factor on HiDPI displays.
                polygon_.push_back(easy3d::vec2(float(x) * framebuffer_width() / camera()->screen_width(),
                                                float(y) * framebuffer_height() / camera()->screen_height()));
                break;
            }
        return true;
    }
    return Viewer::mouse_press_event(x, y, button, modifiers);
}

// While the gesture is active, dragging extends the region instead of moving the camera.
bool TutorialPointSelection::mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) {
    if (active_) {
        extend(x, y);
        return true;
    }
    return Viewer::mouse_drag_event(x, y, dx, dy, button, modifiers);
}

// Releasing the starting button finishes the gesture: run the GPU region-selection query
// and apply the result as per-point colors.
bool TutorialPointSelection::mouse_release_event(int x, int y, int button, int modifiers) {
    if (!active_)
        return Viewer::mouse_release_event(x, y, button, modifiers);
    // Ignore releases of other buttons (e.g. a stray middle click during the gesture).
    if (button != button_)
        return true;
    extend(x, y);
    // The target is held weakly: verify the model still exists and is still in the scene.
    const auto model = target_.lock();
    if (model && std::find(models().begin(), models().end(), model) != models().end()) {
        auto *cloud = dynamic_cast<easy3d::PointCloud *>(model.get());
        auto drawable = cloud->renderer()->get_points_drawable("vertices");
        // Only select what the user can actually see rendered.
        if (drawable && cloud->renderer()->is_visible() && drawable->is_visible()) {
            // Assemble the GPU selection request.
            easy3d::graphics::PointSelectionRequest request;
            request.width = framebuffer_width();
            request.height = framebuffer_height();
            request.polygon = polygon_;
            // point_resource() is the immutable position snapshot shared by rendering and
            // selection, so the query tests exactly the points that are drawn.
            request.points = point_resource(drawable);
            // The result is a bitmask: one bit per point, 32 points per 32-bit word.
            request.words.resize((size_t(request.points->source_count()) + 31) / 32, 0);
            // Seed the input mask with the current selection so a deselect gesture clears
            // the right bits (and previously selected points outside the region survive).
            auto selected = cloud->vertex_property<bool>("v:select");
            for (auto v : cloud->vertices())
                if (selected[v])
                    request.words[v.idx() >> 5] |= 1u << (v.idx() & 31);
            // Points must be projected exactly as they are rendered: camera MVP combined
            // with the drawable's own manipulation transform.
            const auto transform = drawable->manipulated_matrix();
            request.model_view_projection = camera()->model_view_projection_matrix() * transform;
            // Clipping planes are given in world space; the query tests points in model
            // space, so convert each plane p by transpose(T): dot(p, T*q) == dot(T^T*p, q).
            for (const auto &plane : clipping_planes())
                request.local_clipping_planes.push_back(transpose(transform) * plane);
            request.deselect = button_ == BUTTON_RIGHT;
            // The GPU projects every point in parallel, tests it against the region, and
            // returns the updated bitmask. This is a through-selection: occluded points
            // inside the region are selected too.
            const auto words = select_points(request);
            // Unpack the result: selected points turn red, the others keep the drawable color.
            auto colors = cloud->vertex_property<easy3d::vec3>("v:color");
            for (auto v : cloud->vertices()) {
                selected[v] = (words[v.idx() >> 5] & (1u << (v.idx() & 31))) != 0;
                colors[v] = selected[v] ? easy3d::vec3(1, 0, 0) : drawable->color().xyz();
            }
            // Tell the drawable to source colors from the per-vertex property and upload it.
            drawable->set_coloring(easy3d::State::COLOR_PROPERTY, easy3d::State::VERTEX, "v:color");
            drawable->update_color_buffer(colors.vector());
        }
    }
    cancel();
    return true;
}

// Mode switching and cancellation. Unhandled keys go to the base Viewer.
bool TutorialPointSelection::key_press_event(int key, int modifiers) {
    if (key == KEY_ESCAPE && active_) {
        cancel();
        return true;
    }
    if (modifiers == 0 && (key == KEY_R || key == KEY_L)) {
        cancel();
        rectangle_ = key == KEY_R;
        return true;
    }
    return Viewer::key_press_event(key, modifiers);
}

// If the window loses focus mid-gesture, the button release may arrive elsewhere; abort so
// the gesture does not get stuck. Returning false leaves focus handling to the base class.
bool TutorialPointSelection::focus_event(bool focused) {
    if (!focused)
        cancel();
    return false;
}

// The polygon is stored in framebuffer pixels, which change meaning after a resize.
void TutorialPointSelection::post_resize(int, int) {
    cancel();
}

// Draws the region being sketched as a screen-space overlay: a translucent red fill plus an
// opaque red outline. The geometry is cached and only rebuilt when the region changes.
void TutorialPointSelection::pre_draw() {
    Viewer::pre_draw();
    // No overlay when idle or when the region is still degenerate (fewer than 3 corners).
    if (!active_ || polygon_.size() < 3) {
        set_overlays({});
        return;
    }
    if (dirty_) {
        // Triangulate the (possibly concave) lasso interior. A simple triangle fan would
        // fail for concave polygons, so we use the Tessellator, which handles arbitrary
        // simple polygons. The normal (0,0,1) fixes the contour orientation in screen space.
        easy3d::Tessellator tess;
        tess.begin_polygon(easy3d::vec3(0, 0, 1));
        tess.begin_contour();
        for (const auto &p : polygon_)
            tess.add_vertex(easy3d::vec3(p, 0));
        tess.end_contour();
        tess.end_polygon();
        // Flatten the tessellated triangles into a plain vertex list for the overlay.
        std::vector<easy3d::graphics::Vertex> fill, edge;
        for (const auto &triangle : tess.elements())
            for (auto index : triangle) {
                const auto *p = tess.vertices()[index]->data();
                easy3d::graphics::Vertex v;
                v.position = easy3d::vec3(float(p[0]), float(p[1]), 0);
                fill.push_back(v);
            }
        // Build the outline as thin quads (two triangles per segment), 1.5 pixels wide.
        // Screen-space lines are not portable across backends, so quads are used instead.
        for (size_t i = 0, j = polygon_.size() - 1; i < polygon_.size(); j = i++) {
            const auto a = polygon_[j], b = polygon_[i], d = b - a;
            if (d.length2() < 1e-6f)
                continue; // skip zero-length segments (duplicate cursor samples)
            // Perpendicular offset of half the line width (0.75 px on each side).
            const auto n = easy3d::vec2(-d.y, d.x) * (.75f / d.length());
            const easy3d::vec2 corners[] = {a - n, b - n, b + n, a + n};
            for (unsigned k : {0u, 1u, 2u, 0u, 2u, 3u}) {
                easy3d::graphics::Vertex v;
                v.position = easy3d::vec3(corners[k], 0);
                edge.push_back(v);
            }
        }
        fill_ = fill.empty() ? nullptr : std::make_shared<easy3d::graphics::Geometry>(std::move(fill));
        edge_ = edge.empty() ? nullptr : std::make_shared<easy3d::graphics::Geometry>(std::move(edge));
        dirty_ = false;
    }
    // Submit the overlay draw commands. screen_space_projection() maps framebuffer-pixel
    // coordinates directly to the screen, so the geometry stays glued to the cursor region.
    std::vector<easy3d::graphics::DrawCommand> overlays;
    for (unsigned i = 0; i < 2; ++i) {
        easy3d::graphics::DrawCommand draw;
        draw.geometry = i ? edge_ : fill_;
        if (!draw.geometry)
            continue;
        draw.model_view_projection =
            easy3d::graphics::screen_space_projection(framebuffer_width(), framebuffer_height());
        // Translucent fill (alpha 0.2) so the points underneath stay visible; solid outline.
        draw.color = easy3d::vec4(1, 0, 0, i ? 1 : .2f);
        overlays.push_back(draw);
    }
    set_overlays(std::move(overlays));
}

// Adds the current cursor position to the region. In rectangle mode the region is always the
// axis-aligned box spanned by the first click and the current position; in lasso mode every
// sufficiently distant sample becomes a new polygon corner.
void TutorialPointSelection::extend(int x, int y) {
    // Convert logical screen units to framebuffer pixels (HiDPI scaling), as in mouse_press.
    const easy3d::vec2 p{float(x) * framebuffer_width() / camera()->screen_width(),
                         float(y) * framebuffer_height() / camera()->screen_height()};
    if (rectangle_) {
        // Four corners: start, current, and the two mixed-coordinate corners.
        const auto first = polygon_.front();
        polygon_ = {first, {first.x, p.y}, p, {p.x, first.y}};
    } else if ((p - polygon_.back()).length2() >= 1)
        // Lasso: append only if the cursor moved at least ~1 pixel, to limit polygon size.
        polygon_.push_back(p);
    dirty_ = true;
    update();
}

// Aborts the gesture and drops all cached overlay geometry.
void TutorialPointSelection::cancel() {
    active_ = false;
    polygon_.clear();
    target_.reset();
    fill_.reset();
    edge_.reset();
    dirty_ = true;
    update();
}

// \endcond