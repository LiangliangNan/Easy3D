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


#include <tools/tool_selection_surface_mesh.h>
#include <tools/tool_manager.h>
#include <tools/canvas.h>

#include <easy3d/gui/picker_model.h>
#include <easy3d/gui/picker_surface_mesh.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/opengl.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/buffer.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/setting.h>


namespace easy3d {

    namespace tools {

        // -------------------- ToolSurfaceMeshFaceSelection ----------------------

        ToolSurfaceMeshFaceSelection::ToolSurfaceMeshFaceSelection(ToolManager *mgr)
                : Tool(mgr) {
        }

        void ToolSurfaceMeshFaceSelection::update_render_buffer(SurfaceMesh* mesh) const {
            auto d = mesh->renderer()->get_triangles_drawable("faces");
            if (!mesh->get_face_property<bool>("f:select"))
                mesh->add_face_property<bool>("f:select", false);
            // Selection tints the drawable's own coloring instead of replacing it.
            d->set_highlight_property(State::FACE, "f:select");
            buffer::update_highlight_mask(mesh, d);
        }

        // -------------------- Click Select ----------------------

        ToolSurfaceMeshFaceSelectionClick::ToolSurfaceMeshFaceSelectionClick(ToolManager *mgr, SurfaceMeshPicker *picker, SelectMode mode)
                : ToolSurfaceMeshFaceSelection(mgr)
                , picker_(picker)
                , select_mode_(mode)
        {
            model_picker_ = new ModelPicker(mgr->viewer()->camera());
        }


        ToolSurfaceMeshFaceSelectionClick::~ToolSurfaceMeshFaceSelectionClick() {
            delete model_picker_;
        }


        void ToolSurfaceMeshFaceSelectionClick::press(int x, int y) {
            SurfaceMesh::Face picked_face = SurfaceMesh::Face();

            SurfaceMesh *mesh = multiple_pick(picked_face, x, y);
            if (mesh && picked_face.is_valid()) {
                auto selected = mesh->face_property<bool>("f:select");
                // finer check to avoid unnecessary buffer update
                if (selected[picked_face] != (select_mode_ != SM_DESELECT)) {
                    selected[picked_face] = (select_mode_ != SM_DESELECT);
                    update_render_buffer(mesh);
                }
            }
        }


        SurfaceMesh *ToolSurfaceMeshFaceSelectionClick::multiple_pick(SurfaceMesh::Face& face, int x, int y) {
            face = SurfaceMesh::Face();

            Model *model = model_picker_->pick(tool_manager()->viewer()->models(), x, y);
            auto picked_mesh = dynamic_cast<SurfaceMesh *>(model);
            if (!picked_mesh)
                return nullptr;

            face = picker_->pick_face(picked_mesh, x, y);
            return picked_mesh;
        }


        MultitoolSurfaceMeshFaceSelectionClick::MultitoolSurfaceMeshFaceSelectionClick(ToolManager *mgr)
                : MultiTool(mgr)
        {
            picker_ = new SurfaceMeshPicker(mgr->viewer()->camera());
            set_tool(LEFT_BUTTON, new ToolSurfaceMeshFaceSelectionClick(mgr, picker_, SM_SELECT));
            set_tool(RIGHT_BUTTON, new ToolSurfaceMeshFaceSelectionClick(mgr, picker_, SM_DESELECT));
        }


        MultitoolSurfaceMeshFaceSelectionClick::~MultitoolSurfaceMeshFaceSelectionClick() {
            delete picker_;
        }


        void MultitoolSurfaceMeshFaceSelectionClick::prepare_hint(ToolButton button, int x, int y) {
            clear_hint();
            if (button == NO_BUTTON && picker_) {
                SurfaceMesh::Face face;
                auto *mesh = dynamic_cast<ToolSurfaceMeshFaceSelectionClick *>(get_tool(LEFT_BUTTON))->multiple_pick(face, x, y);
                if (mesh && face.is_valid()) {
                    hover_mesh_ = mesh;
                    hover_face_ = face;
                }
            }
        }

        void MultitoolSurfaceMeshFaceSelectionClick::clear_hint() {
            hover_mesh_ = nullptr;
            hover_face_ = SurfaceMesh::Face();
        }

        void MultitoolSurfaceMeshFaceSelectionClick::draw_hint() const {
            if (!hover_mesh_ || !hover_face_.is_valid())
                return;

            bool exists = false;
            for (auto model : tool_manager()->viewer()->models()) {
                if (model.get() == hover_mesh_) {
                    exists = true;
                    break;
                }
            }
            if (!exists) // the hovered model has been removed
                return;

            auto camera = tool_manager()->viewer()->camera();
            const mat4 mvp = camera->model_view_projection_matrix();
            const mat4 manip = hover_mesh_->manipulator()->matrix();

            Polygon2 polygon;
            for (auto v : hover_mesh_->vertices(hover_face_)) {
                const vec3 p = manip * hover_mesh_->position(v);
                if ((mvp * vec4(p, 1.0f)).w <= 0.0f) // the face crosses the eye plane: no fill this frame
                    return;
                const vec3 proj = camera->projected_coordinates_of(p);
                polygon.push_back(vec2(proj.x, proj.y));
            }
            if (polygon.size() < 3)
                return;

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            // The cue is drawn every frame: it must not write depth, or it would hide later 3D hints
            // (the manipulator frame) drawn in front of the face.
            glDepthMask(GL_FALSE);
            shape::draw_polygon_filled(polygon, vec4(setting::highlight_color.xyz(), 0.35f),
                                       camera->screen_width(), camera->screen_height(), -0.9f);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }


        // -------------------- Rect Select ----------------------

        ToolSurfaceMeshFaceSelectionRect::ToolSurfaceMeshFaceSelectionRect(ToolManager *mgr, SurfaceMeshPicker *picker, SelectMode mode)
                : ToolSurfaceMeshFaceSelection(mgr), picker_(picker), select_mode_(mode) {
        }


        void ToolSurfaceMeshFaceSelectionRect::press(int x, int y) {
            start_ = vec2(static_cast<float>(x), static_cast<float>(y));
        }


        void ToolSurfaceMeshFaceSelectionRect::drag(int x, int y) {
        }


        void ToolSurfaceMeshFaceSelectionRect::release(int x, int y) {
            for (auto model : tool_manager()->viewer()->models()) {
                auto mesh = dynamic_cast<SurfaceMesh*>(model.get());
                if (mesh && mesh->renderer()->is_visible()) {
                    const auto& faces = picker_->pick_faces(mesh, Rect(start_, vec2(static_cast<float>(x), static_cast<float>(y))));
                    auto select = mesh->face_property<bool>("f:select");
                    if (!faces.empty()) {
                        for (auto f : faces)
                            select[f] = (select_mode_ != SM_DESELECT);
                        update_render_buffer(mesh);
                    }
                }
            }
        }


        MultitoolSurfaceMeshFaceSelectionRect::MultitoolSurfaceMeshFaceSelectionRect(ToolManager *mgr)
                : MultiTool(mgr) {
            picker_ = new SurfaceMeshPicker(mgr->viewer()->camera());
            set_tool(LEFT_BUTTON, new ToolSurfaceMeshFaceSelectionRect(mgr, picker_, SM_SELECT));
            set_tool(RIGHT_BUTTON, new ToolSurfaceMeshFaceSelectionRect(mgr, picker_, SM_DESELECT));

            start_ = vec2(0, 0);
            end_ = vec2(0, 0);
        }


        MultitoolSurfaceMeshFaceSelectionRect::~MultitoolSurfaceMeshFaceSelectionRect() {
            delete picker_;
        }

        void MultitoolSurfaceMeshFaceSelectionRect::press(ToolButton button, int x, int y) {
            MultiTool::press(button, x, y);
            start_ = vec2(static_cast<float>(x), static_cast<float>(y));
            end_ = vec2(static_cast<float>(x), static_cast<float>(y));
        }

        void MultitoolSurfaceMeshFaceSelectionRect::release(ToolButton button, int x, int y) {
            MultiTool::release(button, x, y);
            clear_hint();
        }

        void MultitoolSurfaceMeshFaceSelectionRect::prepare_hint(ToolButton button, int x, int y) {
            if (button != NO_BUTTON && picker_)
                end_ = vec2(static_cast<float>(x), static_cast<float>(y));
            else
                clear_hint();
        }

        void MultitoolSurfaceMeshFaceSelectionRect::clear_hint() {
            start_ = vec2(0, 0);
            end_ = vec2(0, 0);
        }

        void MultitoolSurfaceMeshFaceSelectionRect::draw_hint() const {
            draw_rect(Rect(start_, end_));
        }

        // ------------------ Lasso Select -----------------------

        ToolSurfaceMeshFaceSelectionLasso::ToolSurfaceMeshFaceSelectionLasso(ToolManager *mgr, SurfaceMeshPicker *picker,
                                                           SelectMode mode)
                : ToolSurfaceMeshFaceSelection(mgr), picker_(picker), select_mode_(mode) {
        }

        void ToolSurfaceMeshFaceSelectionLasso::press(int x, int y) {
            lasso_.clear();
            lasso_.push_back(vec2(float(x), float(y)));
        }


        void ToolSurfaceMeshFaceSelectionLasso::drag(int x, int y) {
            lasso_.push_back(vec2(float(x), float(y)));
        }


        void ToolSurfaceMeshFaceSelectionLasso::release(int x, int y) {
            if (lasso_.size() < 3)
                return;

            for (auto model : tool_manager()->viewer()->models()) {
                auto mesh = dynamic_cast<SurfaceMesh*>(model.get());
                if (mesh && mesh->renderer()->is_visible()) {
                    const auto& faces = picker_->pick_faces(mesh, lasso_);
                    auto select = mesh->face_property<bool>("f:select");
                    if (!faces.empty()) {
                        for (auto f : faces)
                            select[f] = (select_mode_ != SM_DESELECT);
                        update_render_buffer(mesh);
                    }
                }
            }

            lasso_.clear();
        }


        MultitoolSurfaceMeshFaceSelectionLasso::MultitoolSurfaceMeshFaceSelectionLasso(ToolManager *mgr)
                : MultiTool(mgr) {
            picker_ = new SurfaceMeshPicker(mgr->viewer()->camera());
            set_tool(LEFT_BUTTON, new ToolSurfaceMeshFaceSelectionLasso(mgr, picker_, SM_SELECT));
            set_tool(RIGHT_BUTTON, new ToolSurfaceMeshFaceSelectionLasso(mgr, picker_, SM_DESELECT));

            lasso_.clear();
        }


        MultitoolSurfaceMeshFaceSelectionLasso::~MultitoolSurfaceMeshFaceSelectionLasso() {
            delete picker_;
        }


        void MultitoolSurfaceMeshFaceSelectionLasso::press(ToolButton button, int x, int y) {
            MultiTool::press(button, x, y);
            lasso_.clear();
            lasso_.push_back(vec2(float(x), float(y)));
        }


        void MultitoolSurfaceMeshFaceSelectionLasso::prepare_hint(ToolButton button, int x, int y) {
            if (button != NO_BUTTON)
                lasso_.push_back(vec2(float(x), float(y)));
        }

        void MultitoolSurfaceMeshFaceSelectionLasso::clear_hint() {
            lasso_.clear();
        }

        void MultitoolSurfaceMeshFaceSelectionLasso::draw_hint() const {
            draw_lasso(lasso_);
        }

    }

}