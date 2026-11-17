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


#include <tools/tool_selection_point_cloud.h>
#include <tools/tool_manager.h>
#include <tools/canvas.h>

#include <easy3d/gui/picker_model.h>
#include <easy3d/gui/picker_point_cloud.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/opengl.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/buffer.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/setting.h>


namespace easy3d {

    namespace tools {

        // -------------------- ToolPointCloudSelection ----------------------


        ToolPointCloudSelection::ToolPointCloudSelection(ToolManager *mgr)
                : Tool(mgr) {
        }


        void ToolPointCloudSelection::update_render_buffer(PointCloud* cloud) const {
            auto d = cloud->renderer()->get_points_drawable("vertices");
            if (!cloud->get_vertex_property<bool>("v:select"))
                cloud->add_vertex_property<bool>("v:select", false);
            // Selection tints the drawable's own coloring instead of replacing it.
            d->set_highlight_property(State::VERTEX, "v:select");
            buffer::update_highlight_mask(cloud, d);
        }


        // -------------------- Click Select ----------------------

        ToolPointCloudSelectionClick::ToolPointCloudSelectionClick(ToolManager *mgr, PointCloudPicker *picker, SelectMode mode)
                : ToolPointCloudSelection(mgr)
                , picker_(picker)
                , select_mode_(mode)
        {
            model_picker_ = new ModelPicker(mgr->viewer()->camera());
        }


        ToolPointCloudSelectionClick::~ToolPointCloudSelectionClick() {
            if (model_picker_)
                delete model_picker_;
        }


        void ToolPointCloudSelectionClick::press(int x, int y) {
            PointCloud::Vertex picked_vertex = PointCloud::Vertex();

            PointCloud *cloud = multiple_pick(picked_vertex, x, y);
            if (cloud && picked_vertex.is_valid()) {
                auto selected = cloud->vertex_property<bool>("v:select");
                // finer check to avoid unnecessary buffer update
                if (selected[picked_vertex] != (select_mode_ != SM_DESELECT)) {
                    selected[picked_vertex] = (select_mode_ != SM_DESELECT);
                    update_render_buffer(cloud);
                }
            }
        }


        PointCloud *ToolPointCloudSelectionClick::multiple_pick(PointCloud::Vertex& vertex, int x, int y) {
            vertex = PointCloud::Vertex();

            Model *model = model_picker_->pick(tool_manager()->viewer()->models(), x, y);
            PointCloud *picked_cloud = dynamic_cast<PointCloud *>(model);
            if (!picked_cloud)
                return nullptr;

            vertex = picker_->pick_vertex(picked_cloud, x, y);
            return picked_cloud;
        }

        // -------------------- Rect Select ----------------------

        ToolPointCloudSelectionRect::ToolPointCloudSelectionRect(ToolManager *mgr, PointCloudPicker *picker, SelectMode mode)
                : ToolPointCloudSelection(mgr), picker_(picker), select_mode_(mode) {
        }


        void ToolPointCloudSelectionRect::press(int x, int y) {
            start_ = vec2(static_cast<float>(x), static_cast<float>(y));
        }


        void ToolPointCloudSelectionRect::drag(int x, int y) {
        }


        void ToolPointCloudSelectionRect::release(int x, int y) {
            for (auto model : tool_manager()->viewer()->models()) {
                auto cloud = dynamic_cast<PointCloud*>(model.get());
                if (cloud && cloud->renderer()->is_visible()) {
                    picker_->pick_vertices(cloud, Rect(start_, vec2(static_cast<float>(x), static_cast<float>(y))), select_mode_ != SM_SELECT);
                    update_render_buffer(cloud);
                }
            }
        }



        MultitoolPointCloudSelectionClick::MultitoolPointCloudSelectionClick(ToolManager *mgr)
                : MultiTool(mgr) {
            picker_ = new PointCloudPicker(mgr->viewer()->camera());
            set_tool(LEFT_BUTTON, new ToolPointCloudSelectionClick(mgr, picker_, SM_SELECT));
            set_tool(RIGHT_BUTTON, new ToolPointCloudSelectionClick(mgr, picker_, SM_DESELECT));
        }


        MultitoolPointCloudSelectionClick::~MultitoolPointCloudSelectionClick() {
            delete picker_;
        }


        void MultitoolPointCloudSelectionClick::prepare_hint(ToolButton button, int x, int y) {
            clear_hint();
            if (button == NO_BUTTON && picker_) {
                PointCloud::Vertex vertex;
                auto cloud = dynamic_cast<ToolPointCloudSelectionClick *>(get_tool(LEFT_BUTTON))->multiple_pick(vertex, x, y);
                if (cloud && vertex.is_valid()) {
                    hover_cloud_ = cloud;
                    hover_vertex_ = vertex;
                }
            }
        }

        void MultitoolPointCloudSelectionClick::clear_hint() {
            hover_cloud_ = nullptr;
            hover_vertex_ = PointCloud::Vertex();
        }

        void MultitoolPointCloudSelectionClick::draw_hint() const {
            if (!hover_cloud_ || !hover_vertex_.is_valid())
                return;

            bool exists = false;
            for (auto model : tool_manager()->viewer()->models()) {
                if (model.get() == hover_cloud_) {
                    exists = true;
                    break;
                }
            }
            if (!exists) // the hovered model has been removed
                return;

            auto camera = tool_manager()->viewer()->camera();
            const mat4 mvp = camera->model_view_projection_matrix();
            const vec3 p = hover_cloud_->manipulator()->matrix() * hover_cloud_->position(hover_vertex_);
            if ((mvp * vec4(p, 1.0f)).w <= 0.0f) // the vertex is at/behind the eye
                return;
            const vec3 proj = camera->projected_coordinates_of(p);

            auto drawable = hover_cloud_->renderer()->get_points_drawable("vertices");
            const float half = 0.5f * drawable->point_size() + 1.5f;
            const Rect marker(vec2(proj.x - half, proj.y - half), vec2(proj.x + half, proj.y + half));

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            // The cue is drawn every frame: it must not write depth, or it would hide later 3D hints
            // (the manipulator frame) drawn in front of the vertex.
            glDepthMask(GL_FALSE);
            shape::draw_quad_filled(marker, vec4(setting::highlight_color.xyz(), 0.35f),
                                    camera->screen_width(), camera->screen_height(), -0.9f);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }


        MultitoolPointCloudSelectionRect::MultitoolPointCloudSelectionRect(ToolManager *mgr)
                : MultiTool(mgr) {
            picker_ = new PointCloudPicker(mgr->viewer()->camera());

            set_tool(LEFT_BUTTON, new ToolPointCloudSelectionRect(mgr, picker_, SM_SELECT));
            set_tool(RIGHT_BUTTON, new ToolPointCloudSelectionRect(mgr, picker_, SM_DESELECT));

            clear_hint();
        }


        MultitoolPointCloudSelectionRect::~MultitoolPointCloudSelectionRect() {
            delete picker_;
        }


        void MultitoolPointCloudSelectionRect::press(ToolButton button, int x, int y) {
            MultiTool::press(button, x, y);
            start_ = vec2(static_cast<float>(x), static_cast<float>(y));
            end_ = vec2(static_cast<float>(x), static_cast<float>(y));
        }


        void MultitoolPointCloudSelectionRect::prepare_hint(ToolButton button, int x, int y) {
            if (button != NO_BUTTON && picker_) {
                end_ = vec2(static_cast<float>(x), static_cast<float>(y));
            }
        }

        void MultitoolPointCloudSelectionRect::clear_hint() {
            start_ = vec2(-1, -1);
            end_ = vec2(-1, -1);
        }

        void MultitoolPointCloudSelectionRect::draw_hint() const {
            draw_rect(Rect(start_, end_));
        }

        // ------------------ Lasso Select -----------------------

        ToolPointCloudSelectionLasso::ToolPointCloudSelectionLasso(ToolManager *mgr, PointCloudPicker *picker, SelectMode mode)
                : ToolPointCloudSelection(mgr), picker_(picker), select_mode_(mode) {
        }

        void ToolPointCloudSelectionLasso::press(int x, int y) {
            lasso_.clear();
            lasso_.push_back(vec2(float(x), float(y)));
        }


        void ToolPointCloudSelectionLasso::drag(int x, int y) {
            lasso_.push_back(vec2(float(x), float(y)));
        }


        void ToolPointCloudSelectionLasso::release(int x, int y) {
            if (lasso_.size() < 3)
                return;

            for (auto model : tool_manager()->viewer()->models()) {
                auto cloud = dynamic_cast<PointCloud*>(model.get());
                if (cloud && cloud->renderer()->is_visible()) {
                    picker_->pick_vertices(cloud, lasso_, select_mode_ != SM_SELECT);
                    update_render_buffer(cloud);
                }
            }
            lasso_.clear();
        }


        MultitoolPointCloudSelectionLasso::MultitoolPointCloudSelectionLasso(ToolManager *mgr)
                : MultiTool(mgr) {
            picker_ = new PointCloudPicker(mgr->viewer()->camera());
            set_tool(LEFT_BUTTON, new ToolPointCloudSelectionLasso(mgr, picker_, SM_SELECT));
            set_tool(RIGHT_BUTTON, new ToolPointCloudSelectionLasso(mgr, picker_, SM_DESELECT));

            clear_hint();
        }


        MultitoolPointCloudSelectionLasso::~MultitoolPointCloudSelectionLasso() {
            delete picker_;
        }


        void MultitoolPointCloudSelectionLasso::press(ToolButton button, int x, int y) {
            MultiTool::press(button, x, y);
            lasso_.clear();
            lasso_.push_back(vec2(float(x), float(y)));
        }


        void MultitoolPointCloudSelectionLasso::prepare_hint(ToolButton button, int x, int y) {
            if (button != NO_BUTTON)
                lasso_.push_back(vec2(float(x), float(y)));
        }

        void MultitoolPointCloudSelectionLasso::clear_hint() {
            lasso_.clear();
        }

        void MultitoolPointCloudSelectionLasso::draw_hint() const {
            draw_lasso(lasso_);
        }

    }

}
