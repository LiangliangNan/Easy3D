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


#include "tools/tool_selection_point_cloud.h"
#include "tools/tool_manager.h"
#include "tools/canvas.h"

#include <easy3d/gui/picker_point_cloud.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/util/logging.h>


namespace easy3d {

    namespace tools {

        // -------------------- ToolPointCloudSelection ----------------------


        ToolPointCloudSelection::ToolPointCloudSelection(ToolManager *mgr)
                : Tool(mgr) {
        }

        // -------------------- Rect Select ----------------------

        ToolPointCloudSelectionRect::ToolPointCloudSelectionRect(ToolManager *mgr, PointCloudPicker *picker, SelectMode mode)
                : ToolPointCloudSelection(mgr), picker_(picker), select_mode_(mode) {
        }


        void ToolPointCloudSelectionRect::press(int x, int y) {
            start_ = vec2(x,y);
        }


        void ToolPointCloudSelectionRect::drag(int x, int y) {
        }


        void ToolPointCloudSelectionRect::release(int x, int y) {
            int num(0);
            for (auto model : tool_manager()->viewer()->models()) {
                auto cloud = dynamic_cast<PointCloud*>(model);
                num += picker_->pick_vertices(cloud, Rect(start_, vec2(x, y)), select_mode_ != SM_SELECT);
            }
            LOG(INFO) << num << " points " << (select_mode_ == SM_SELECT ? "selected." : "deselected.");
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
            start_ = vec2(x, y);
            end_ = vec2(x, y);
        }


        void MultitoolPointCloudSelectionRect::prepare_hint(ToolButton button, int x, int y) {
            if (button != NO_BUTTON && picker_) {
                end_ = vec2(x, y);
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

            int num = 0;
            for (auto model : tool_manager()->viewer()->models()) {
                auto cloud = dynamic_cast<PointCloud*>(model);
                num += picker_->pick_vertices(cloud, lasso_, select_mode_ != SM_SELECT);
            }
            LOG(INFO) << num << " points " << (select_mode_ == SM_SELECT ? "selected." : "deselected.");

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
