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


#include <easy3d/gui/picker_point_cloud.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/util/logging.h>


namespace easy3d {


    PointCloudPicker::PointCloudPicker(const Camera *cam)
            : Picker(cam) {
        use_gpu_if_supported_ = false;
    }


    void PointCloudPicker::pick_vertices(PointCloud *model, const Rect& rect, bool deselect) {
        if (!model)
            return;
        int win_width = camera()->screenWidth();
        int win_height = camera()->screenHeight();

        float xmin = rect.left() / (win_width - 1.0f);
        float ymin = 1.0f - rect.top() / (win_height - 1.0f);
        float xmax = rect.right() / (win_width - 1);
        float ymax = 1.0f - rect.bottom() / (win_height - 1.0f);
        if (xmin > xmax) std::swap(xmin, xmax);
        if (ymin > ymax) std::swap(ymin, ymax);

        const auto &points = model->get_vertex_property<vec3>("v:point").vector();
        int num = static_cast<int>(points.size());
        const mat4 &m = camera()->modelViewProjectionMatrix() * model->manipulator()->matrix();

        auto &select = model->vertex_property<bool>("v:select").vector();

#pragma omp parallel for
        for (int i = 0; i < num; ++i) {
            const vec3 &p = points[i];
            float x = m[0] * p.x + m[4] * p.y + m[8] * p.z + m[12];
            float y = m[1] * p.x + m[5] * p.y + m[9] * p.z + m[13];
            //float z = m[2] * p.x + m[6] * p.y + m[10] * p.z + m[14]; // I don't need z
            float w = m[3] * p.x + m[7] * p.y + m[11] * p.z + m[15];
            x /= w;
            y /= w;
            x = 0.5f * x + 0.5f;
            y = 0.5f * y + 0.5f;

            if (x >= xmin && x <= xmax && y >= ymin && y <= ymax)
                select[i] = !deselect;
        }

        auto count = std::count(select.begin(), select.end(), 1);
        LOG(INFO) << "current selection: " << count << " points";
    }


    void PointCloudPicker::pick_vertices(PointCloud *model, const Polygon2 &plg, bool deselect) {
        if (!model)
            return;

        int win_width = camera()->screenWidth();
        int win_height = camera()->screenHeight();

        std::vector<vec2> region; // the transformed selection region
        for (std::size_t i = 0; i < plg.size(); ++i) {
            const vec2 &p = plg[i];
            float x = p.x / float(win_width - 1);
            float y = 1.0f - p.y / float(win_height - 1);
            region.push_back(vec2(x, y));
        }

        const Box2& box = plg.bbox();
        float xmin = box.min_point().x / (win_width - 1.0f);
        float ymin = 1.0f - box.min_point().y / (win_height - 1.0f);
        float xmax = box.max_point().x / (win_width - 1);
        float ymax = 1.0f - box.max_point().y / (win_height - 1.0f);
        if (xmin > xmax) std::swap(xmin, xmax);
        if (ymin > ymax) std::swap(ymin, ymax);

        const auto &points = model->get_vertex_property<vec3>("v:point").vector();
        int num = static_cast<int>(points.size());
        const mat4 &m = camera()->modelViewProjectionMatrix() * model->manipulator()->matrix();

        auto& select = model->vertex_property<bool>("v:select").vector();

#pragma omp parallel for
        for (int i = 0; i < num; ++i) {
            const vec3 &p = points[i];
            float x = m[0] * p.x + m[4] * p.y + m[8] * p.z + m[12];
            float y = m[1] * p.x + m[5] * p.y + m[9] * p.z + m[13];
            //float z = m[2] * p.x + m[6] * p.y + m[10] * p.z + m[14]; // I don't need z
            float w = m[3] * p.x + m[7] * p.y + m[11] * p.z + m[15];
            x /= w;
            y /= w;
            x = 0.5f * x + 0.5f;
            y = 0.5f * y + 0.5f;

            if (x >= xmin && x <= xmax && y >= ymin && y <= ymax) {
                if (geom::point_in_polygon(vec2(x, y), region))
                    select[i] = !deselect;
            }
        }

        auto count = std::count(select.begin(), select.end(), 1);
        LOG(INFO) << "current selection: " << count << " points";
    }

}
