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


#include <tools/tool_selection_surface_mesh.h>
#include <tools/tool_manager.h>
#include <tools/canvas.h>

#include <easy3d/gui/picker_model.h>
#include <easy3d/gui/picker_surface_mesh.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/buffer.h>
#include <easy3d/util/logging.h>


namespace easy3d {

    namespace tools {

        // -------------------- ToolSurfaceMeshFaceSelection ----------------------

        ToolSurfaceMeshFaceSelection::ToolSurfaceMeshFaceSelection(ToolManager *mgr)
                : Tool(mgr) {
        }

        void ToolSurfaceMeshFaceSelection::update_render_buffer(SurfaceMesh* mesh) const {
            auto d = mesh->renderer()->get_triangles_drawable("faces");
            if ((d->coloring_method() != easy3d::State::SCALAR_FIELD) || (d->property_location() != State::FACE) || (d->property_name() != "f:select")) {
                if (!mesh->get_face_property<bool>("f:select"))
                    mesh->add_face_property<bool>("f:select", false);
                d->set_coloring(State::SCALAR_FIELD, State::FACE, "f:select");
                d->set_distinct_back_color(false); // the backside also highlighted
                buffer::update(mesh, d);
            }
            else {
                auto select = mesh->face_property<bool>("f:select", false);
                auto triangle_range = mesh->face_property<std::pair<int, int> >("f:triangle_range");

                // update the drawable's texcoord buffer
                std::vector<vec2> texcoords(d->num_vertices());
                for (auto f : mesh->faces()) {
                    int start = triangle_range[f].first;
                    int end = triangle_range[f].second;
                    for (int idx = start; idx <= end; ++idx)
                        texcoords[idx * 3] = texcoords[idx * 3 + 1] = texcoords[idx * 3 + 2] = vec2(select[f], 0.5f);
                }
                d->update_texcoord_buffer(texcoords);
                d->set_coloring(State::SCALAR_FIELD, State::FACE, "f:select");
                d->set_distinct_back_color(false); // the backside also highlighted
            }
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
                    auto triangle_range = mesh->face_property<std::pair<int, int> >("f:triangle_range");
                    auto drawable = mesh->renderer()->get_triangles_drawable("faces");
                    drawable->set_highlight(true);
                    drawable->set_highlight_range(triangle_range[face]);
                }
            }
        }

        void MultitoolSurfaceMeshFaceSelectionClick::clear_hint() {
            for (auto model : tool_manager()->viewer()->models()) {
                auto mesh = dynamic_cast<SurfaceMesh *>(model);
                if (mesh) {
                    auto drawable = mesh->renderer()->get_triangles_drawable("faces");
                    drawable->set_highlight(false);
                    drawable->set_highlight_range({-1, -1});
                }
            }
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
                auto mesh = dynamic_cast<SurfaceMesh*>(model);
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
                auto mesh = dynamic_cast<SurfaceMesh*>(model);
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