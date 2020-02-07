/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */


#include <easy3d/gui/tool_selection_surface_mesh.h>
#include <easy3d/gui/tool_manager.h>
#include <easy3d/gui/canvas.h>
#include <easy3d/gui/picker_model.h>
#include <easy3d/gui/picker_surface_mesh.h>
#include <easy3d/viewer/drawable_triangles.h>
#include <easy3d/util/logging.h>


namespace easy3d {

    namespace tools {

        static const std::string select_attr_name = "select";

        static const vec4 hint_facet_color(0.3f, 1.0f, 1.0f, 1.0f);

        static const float hint_line_width = 1.0f;
        static const vec4 hint_line_color(0.0f, 0.9f, 0.9f, 0.6f);
        static const vec4 hint_area_color(0.0f, 0.0f, 0.4f, 0.3f);



        // -------------------- MeshFacetSelectTool ----------------------


        MeshFacetSelectTool::MeshFacetSelectTool(ToolManager *mgr)
                : TaskTool(mgr) {
        }


        // -------------------- Click Select ----------------------

        MeshFacetClickSelectTool::MeshFacetClickSelectTool(ToolManager *mgr, SurfaceMeshPicker *picker, SelectMode mode)
                : MeshFacetSelectTool(mgr)
                , picker_(picker)
                , select_mode_(mode)
        {
            model_picker_ = new ModelPicker(mgr->viewer()->camera());
        }


        MeshFacetClickSelectTool::~MeshFacetClickSelectTool() {
            if (model_picker_)
                delete model_picker_;
        }


        void MeshFacetClickSelectTool::press(int x, int y) {
            SurfaceMesh::Face picked_face = SurfaceMesh::Face();

            SurfaceMesh *mesh = multiple_pick(picked_face, x, y);
            if (mesh && picked_face.is_valid()) {
                std::cout << "face selected: " << picked_face << std::endl;
//                MeshFacetAttribute<bool> status(mesh, select_attr_name);
//                status[picked_facet] = (select_mode_ == SM_SELECT);
//
//                if (select_mode_ == SM_SELECT)
//                    Logger::out(title()) << "1 facet selected" << std::endl;
//                else
//                    Logger::out(title()) << "1 facet deselected" << std::endl;
//
//                int id = picker_->picked_facet_id();
//                dynamic_cast<FacesDrawable *>(mesh->drawable(DT_FACES, "surface"))->set_selected(id,
//                                                                                                 select_mode_ ==
//                                                                                                 SM_SELECT);
//
//                mesh->notify_selection_change();
            }
        }


        SurfaceMesh *MeshFacetClickSelectTool::multiple_pick(SurfaceMesh::Face& face, int x, int y) {
            face = SurfaceMesh::Face();

            Model *model = model_picker_->pick(tool_manager()->viewer()->models(), x, y);
            SurfaceMesh *picked_mesh = dynamic_cast<SurfaceMesh *>(model);
            if (!picked_mesh)
                return nullptr;

            face = picker_->pick_face(picked_mesh, x, y);
            return picked_mesh;
        }


        MeshFacetClickSelect::MeshFacetClickSelect(ToolManager *mgr)
                : MultiTool(mgr)
        //, hint_facet_(nil)
        {
            picker_ = new SurfaceMeshPicker(mgr->viewer()->camera());
            set_tool(LEFT_BUTTON, new MeshFacetClickSelectTool(mgr, picker_, SM_SELECT));
            set_tool(RIGHT_BUTTON, new MeshFacetClickSelectTool(mgr, picker_, SM_DESELECT));
        }


        MeshFacetClickSelect::~MeshFacetClickSelect() {
            delete picker_;
        }


        void MeshFacetClickSelect::prepare_hint(ToolButton button, int x, int y) {
            clear_hint();
            if (button == NO_BUTTON && picker_) {
                SurfaceMesh::Face face;
                auto *mesh = dynamic_cast<MeshFacetClickSelectTool *>(get_tool(LEFT_BUTTON))->multiple_pick(face, x, y);
                if (mesh && face.is_valid()) {
                    auto triangle_range = mesh->face_property<std::pair<int, int> >("f:triangle_range");
                    auto drawable = mesh->triangles_drawable("faces");
                    drawable->set_highlight(true);
                    drawable->set_highlight_range(triangle_range[face]);
                }
            }
        }

        void MeshFacetClickSelect::clear_hint() {
            for (auto model : tool_manager()->viewer()->models()) {
                SurfaceMesh *mesh = dynamic_cast<SurfaceMesh *>(model);
                if (mesh) {
                    auto drawable = mesh->triangles_drawable("faces");
                    drawable->set_highlight(false);
                }
            }
        }


        // -------------------- Rect Select ----------------------

        MeshFacetRectSelectTool::MeshFacetRectSelectTool(ToolManager *mgr, SurfaceMeshPicker *picker, SelectMode mode)
                : MeshFacetSelectTool(mgr), picker_(picker), select_mode_(mode) {
        }


        void MeshFacetRectSelectTool::press(int x, int y) {
            start_ = vec2(x, y);
        }


        void MeshFacetRectSelectTool::drag(int x, int y) {
        }


        void MeshFacetRectSelectTool::release(int x, int y) {
            int count = 0;
            for (auto model : tool_manager()->viewer()->models()) {
                SurfaceMesh *mesh = dynamic_cast<SurfaceMesh*>(model);
                int num = picker_->pick_faces(mesh, Rect(start_, vec2(x, y)), select_mode_ == SM_DESELECT);
                std::cout << num << " faces marked" << std::endl;

                count += num;

//                MeshFacetAttribute<bool> status(mesh, select_attr_name);
//                for (std::size_t i = 0; i < facets.size(); ++i) {
//                    MeshTypes::Facet *f = facets[i];
//                    if (status[f] != (select_mode_ == SM_SELECT)) {
//                        status[f] = (select_mode_ == SM_SELECT);
//                        ++count;
//                    }
//                }
//
//                // mark in shader storage buffer
//                MeshFacetAttribute<int> faceID(mesh, "faceID");
//                //#pragma omp parallel for // this will NOT work due to the bitwise operations
//                for (int i = 0; i < (int) facets.size(); ++i) {
//                    unsigned int id = faceID[facets[i]];
//                    dynamic_cast<FacesDrawable *>(mesh->drawable(DT_FACES, "surface"))->set_selected(id, select_mode_ ==
//                                                                                                         SM_SELECT);
//                }
//                mesh->notify_selection_change();
            }

            if (select_mode_ == SM_SELECT)
                LOG(INFO) << count << " facet selected.";
            else
                LOG(INFO) << count << " facet deselected.";
        }


        MeshFacetRectSelect::MeshFacetRectSelect(ToolManager *mgr)
                : MultiTool(mgr) {
            picker_ = new SurfaceMeshPicker(mgr->viewer()->camera());
            set_tool(LEFT_BUTTON, new MeshFacetRectSelectTool(mgr, picker_, SM_SELECT));
            set_tool(RIGHT_BUTTON, new MeshFacetRectSelectTool(mgr, picker_, SM_DESELECT));

            clear_hint();
        }


        MeshFacetRectSelect::~MeshFacetRectSelect() {
            delete picker_;
        }


        void MeshFacetRectSelect::press(ToolButton button, int x, int y) {
            MultiTool::press(button, x, y);
            start_ = vec2(x, y);
            end_ = vec2(x, y);
        }


        void MeshFacetRectSelect::prepare_hint(ToolButton button, int x, int y) {
            if (button != NO_BUTTON && picker_) {
                end_ = vec2(x, y);
            }
        }

        void MeshFacetRectSelect::clear_hint() {
            start_ = vec2(0, 0);
            end_ = vec2(0, 0);
        }

        void MeshFacetRectSelect::draw_hint() const {
            draw_rect(Rect(start_, end_));
        }

        // ------------------ Lasso Select -----------------------

        MeshFacetLassoSelectTool::MeshFacetLassoSelectTool(ToolManager *mgr, SurfaceMeshPicker *picker,
                                                           SelectMode mode)
                : MeshFacetSelectTool(mgr), picker_(picker), select_mode_(mode) {
        }

        void MeshFacetLassoSelectTool::press(int x, int y) {
            lasso_.clear();
            lasso_.push_back(vec2(float(x), float(y)));
        }


        void MeshFacetLassoSelectTool::drag(int x, int y) {
            lasso_.push_back(vec2(float(x), float(y)));
        }


        void MeshFacetLassoSelectTool::release(int x, int y) {
            if (lasso_.size() < 3)
                return;

            int count = 0;
            for (auto model : tool_manager()->viewer()->models()) {
                SurfaceMesh *mesh = dynamic_cast<SurfaceMesh*>(model);
                int num = picker_->pick_faces(mesh, lasso_, select_mode_ == SM_DESELECT);
                std::cout << num << " faces marked" << std::endl;

                count += num;

//                MeshFacetAttribute<bool> status(mesh, select_attr_name);
//                for (std::size_t i = 0; i < facets.size(); ++i) {
//                    MeshTypes::Facet *f = facets[i];
//                    if (status[f] != (select_mode_ == SM_SELECT)) {
//                        status[f] = (select_mode_ == SM_SELECT);
//                        ++count;
//                    }
//                }
//
//                // mark in shader storage buffer
//                MeshFacetAttribute<int> faceID(mesh, "faceID");
//                //#pragma omp parallel for // this will NOT work due to the bitwise operations
//                for (int i = 0; i < (int) facets.size(); ++i) {
//                    unsigned int id = faceID[facets[i]];
//                    dynamic_cast<FacesDrawable *>(mesh->drawable(DT_FACES, "surface"))->set_selected(id, select_mode_ ==
//                                                                                                         SM_SELECT);
//                }
//                mesh->notify_selection_change();
            }

            if (select_mode_ == SM_SELECT)
                LOG(INFO) << count << " facet selected.";
            else
                LOG(INFO) << count << " facet deselected.";

            lasso_.clear();
        }


        MeshFacetLassoSelect::MeshFacetLassoSelect(ToolManager *mgr)
                : MultiTool(mgr) {
            picker_ = new SurfaceMeshPicker(mgr->viewer()->camera());
            set_tool(LEFT_BUTTON, new MeshFacetLassoSelectTool(mgr, picker_, SM_SELECT));
            set_tool(RIGHT_BUTTON, new MeshFacetLassoSelectTool(mgr, picker_, SM_DESELECT));

            clear_hint();
        }


        MeshFacetLassoSelect::~MeshFacetLassoSelect() {
            delete picker_;
        }


        void MeshFacetLassoSelect::press(ToolButton button, int x, int y) {
            MultiTool::press(button, x, y);
            lasso_.clear();
            lasso_.push_back(vec2(float(x), float(y)));
        }


        void MeshFacetLassoSelect::prepare_hint(ToolButton button, int x, int y) {
            if (button != NO_BUTTON)
                lasso_.push_back(vec2(float(x), float(y)));
        }

        void MeshFacetLassoSelect::clear_hint() {
            lasso_.clear();
        }

        void MeshFacetLassoSelect::draw_hint() const {
            draw_lasso(lasso_);
        }

    }

}