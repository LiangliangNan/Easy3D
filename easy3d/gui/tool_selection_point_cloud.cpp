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


#include <easy3d/gui/tool_selection_point_cloud.h>
#include <easy3d/gui/tool_manager.h>
#include <easy3d/gui/canvas.h>
#include <easy3d/gui/picker_model.h>
#include <easy3d/gui/picker_point_cloud.h>
#include <easy3d/viewer/shader_program.h>
#include <easy3d/viewer/shader_manager.h>
#include <easy3d/viewer/primitives.h>
#include <easy3d/viewer/opengl.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/util/logging.h>


namespace easy3d {

    namespace tools {

        static const float hint_line_width = 1.0f;
        static const vec4 hint_line_color(0.0f, 0.9f, 0.9f, 0.6f);
        static const vec4 hint_area_color(0.0f, 0.0f, 0.4f, 0.3f);

        // -------------------- PointCloudSelectTool ----------------------


        PointCloudSelectTool::PointCloudSelectTool(ToolManager *mgr)
                : TaskTool(mgr) {
        }

        // -------------------- Rect Select ----------------------

        PointCloudRectSelectTool::PointCloudRectSelectTool(ToolManager *mgr, PointCloudPicker *picker, SelectMode mode)
                : PointCloudSelectTool(mgr), picker_(picker), select_mode_(mode) {
        }


        void PointCloudRectSelectTool::press(int x, int y) {
            start_ = vec2(x,y);
        }


        void PointCloudRectSelectTool::drag(int x, int y) {
        }


        void PointCloudRectSelectTool::release(int x, int y) {
            int num(0);
            for (auto model : tool_manager()->viewer()->models()) {
                auto cloud = dynamic_cast<PointCloud*>(model);
                num += picker_->pick_vertices(cloud, Rect(start_, vec2(x, y)), select_mode_ != SM_SELECT);
            }
            LOG(INFO) << num << " points " << (select_mode_ == SM_SELECT ? "selected." : "deselected.");
        }


        PointCloudRectSelect::PointCloudRectSelect(ToolManager *mgr)
                : MultiTool(mgr) {
            picker_ = new PointCloudPicker(mgr->viewer()->camera());

            set_tool(LEFT_BUTTON, new PointCloudRectSelectTool(mgr, picker_, SM_SELECT));
            set_tool(RIGHT_BUTTON, new PointCloudRectSelectTool(mgr, picker_, SM_DESELECT));

            clear_hint();
        }


        PointCloudRectSelect::~PointCloudRectSelect() {
            delete picker_;
        }


        void PointCloudRectSelect::press(ToolButton button, int x, int y) {
            MultiTool::press(button, x, y);
            start_ = vec2(x, y);
            end_ = vec2(x, y);
        }


        void PointCloudRectSelect::prepare_hint(ToolButton button, int x, int y) {
            if (button != NO_BUTTON && picker_) {
                end_ = vec2(x, y);
            }
        }

        void PointCloudRectSelect::clear_hint() {
            start_ = vec2(0, 0);
            end_ = vec2(0, 0);
        }

        void PointCloudRectSelect::draw_hint() const {
            auto program = ShaderManager::get_program("screen_space/lines_color");
            if (!program) {
                std::vector<ShaderProgram::Attribute> attributes = {
                        ShaderProgram::Attribute(ShaderProgram::POSITION, "vertexMC"),
                        ShaderProgram::Attribute(ShaderProgram::COLOR, "vertexColor")
                };
                program = ShaderManager::create_program_from_files("screen_space/lines_color", attributes);
            }
            if (!program)
                return;

            Rect rect(start_, end_);
            int w = rect.width();
            int h = rect.height();
            if (w <= 0 || h <=0)
                return;

            int width = tool_manager()->viewer()->camera()->screenWidth();
            int height = tool_manager()->viewer()->camera()->screenHeight();

            // draw_quad_wire() requires the lower left corner
            float x0 = rect.x_min();
            float y0 = rect.y_max();

            program->bind();
            program->set_uniform("per_vertex_color", false);
            program->set_uniform("default_color", vec3(0.0f, 0.0f, 0.0f));
            opengl::draw_quad_wire(ShaderProgram::POSITION, x0, height - y0 - 1, w, h, width, height, -1.0f);
            program->release();



//            tool_manager()->canvas()->startScreenCoordinatesSystem();
//
//            glDisable(GL_LIGHTING);
//
//            // draw the frame of the rectangle
//            glLineWidth(hint_line_width);
//            glColor4fv(hint_line_color);
//            glBegin(GL_LINE_LOOP);
//            glVertex2i(start_x_, start_y_);
//            glVertex2i(start_x_, end_y_);
//            glVertex2i(end_x_, end_y_);
//            glVertex2i(end_x_, start_y_);
//            glEnd();
//
//            // draw the transparent inner region of the rectangle
//            glDepthMask(GL_FALSE);
//            glEnable(GL_BLEND);
//            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  //GL_ONE_MINUS_DST_ALPHA
//
//            glColor4fv(hint_area_color);
//            glBegin(GL_QUADS);
//            glVertex2i(start_x_, start_y_);
//            glVertex2i(start_x_, end_y_);
//            glVertex2i(end_x_, end_y_);
//            glVertex2i(end_x_, start_y_);
//            glEnd();
//
//            glDepthMask(GL_TRUE);
//            glDisable(GL_BLEND);
//
//            glEnable(GL_LIGHTING);
//            tool_manager()->canvas()->stopScreenCoordinatesSystem();
        }


        // ------------------ Lasso Select -----------------------

        PointCloudLassoSelectTool::PointCloudLassoSelectTool(ToolManager *mgr, PointCloudPicker *picker, SelectMode mode)
                : PointCloudSelectTool(mgr), picker_(picker), select_mode_(mode) {
        }

        void PointCloudLassoSelectTool::press(int x, int y) {
            lasso_.clear();
            lasso_.push_back(vec2(float(x), float(y)));
        }


        void PointCloudLassoSelectTool::drag(int x, int y) {
            lasso_.push_back(vec2(float(x), float(y)));
        }


        void PointCloudLassoSelectTool::release(int x, int y) {
            if (lasso_.size() < 3)
                return;

            int num = 0;
            for (auto model : tool_manager()->viewer()->models()) {
                auto cloud = dynamic_cast<PointCloud*>(model);
                num += picker_->pick_vertices(cloud, lasso_, select_mode_ != SM_SELECT);
            }
            LOG(INFO) << num << " points " << (select_mode_ == SM_SELECT ? "selected." : "deselected.");

            lasso_.clear();
        }


        PointCloudLassoSelect::PointCloudLassoSelect(ToolManager *mgr)
                : MultiTool(mgr) {
            picker_ = new PointCloudPicker(mgr->viewer()->camera());
            set_tool(LEFT_BUTTON, new PointCloudLassoSelectTool(mgr, picker_, SM_SELECT));
            set_tool(RIGHT_BUTTON, new PointCloudLassoSelectTool(mgr, picker_, SM_DESELECT));

            clear_hint();
        }


        PointCloudLassoSelect::~PointCloudLassoSelect() {
            delete picker_;
        }


        void PointCloudLassoSelect::press(ToolButton button, int x, int y) {
            MultiTool::press(button, x, y);
            lasso_.clear();
            lasso_.push_back(vec2(float(x), float(y)));
        }


        void PointCloudLassoSelect::prepare_hint(ToolButton button, int x, int y) {
            if (button != NO_BUTTON)
                lasso_.push_back(vec2(float(x), float(y)));
        }

        void PointCloudLassoSelect::clear_hint() {
            lasso_.clear();
        }

        void PointCloudLassoSelect::draw_hint() const {
//            tool_manager()->canvas()->startScreenCoordinatesSystem();
//            glDisable(GL_LIGHTING);
//
//            glLineWidth(hint_line_width);
//            glColor4fv(hint_line_color);
//
//            // draw the strokes that have been finished.
//            if (lasso_.size() > 1) {
//                glBegin(GL_LINE_STRIP);
//                for (std::size_t i = 0; i < lasso_.size(); ++i) {
//                    const vec2 &p = lasso_[i];
//                    glVertex2fv(p);
//                }
//                glEnd();
//            }
//
//            // make the polygon closed
//            if (lasso_.size() >= 3) {
//                const vec2 &p = lasso_.front();
//                const vec2 &q = lasso_.back();
//                glBegin(GL_LINES);
//                glVertex2fv(p);
//                glVertex2fv(q);
//                glEnd();
//            }
//
//            //////////////////////////////////////////////////////////////////////////
//
//            // draw the transparent lasso region.
//            glDepthMask(GL_FALSE);
//            glEnable(GL_BLEND);
//            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  //GL_ONE_MINUS_DST_ALPHA
//
//            glColor4fv(hint_area_color);
//
//            Tessellator tess;
//            tess.begin_polygon();
//            tess.begin_contour();
//            // Liangliang: the region could be non-convex, so I use my tessellator.
//            for (std::size_t i = 0; i < lasso_.size(); ++i) {
//                const vec2 &p = lasso_[i];
//                tess.add_vertex(vec3(p.x, p.y, 0));
//            }
//            tess.end_contour();
//            tess.end_polygon();
//
//            glDepthMask(GL_TRUE);
//            glDisable(GL_BLEND);
//
//            glEnable(GL_LIGHTING);
//            tool_manager()->canvas()->stopScreenCoordinatesSystem();
        }

    }

}
