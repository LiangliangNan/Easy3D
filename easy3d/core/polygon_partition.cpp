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

#include <easy3d/core/polygon_partition.h>

#include <3rd_party/polypartition/polypartition.h>


namespace easy3d {


    bool PolygonPartition::apply(const std::vector<vec2> &input_polygon, std::vector<Polygon> &parts, Method method) const {
        TPPLPoly poly;
        poly.Init(input_polygon.size());
        for (std::size_t i=0; i<input_polygon.size(); ++i) {
            const auto& p = input_polygon[i];
            poly[i] = {p.x, p.y, i};
        }

        TPPLPartition partition;
        std::list<TPPLPoly> outputs;

        switch (method) {
            case Hertel_Mehlhorn:
                if (partition.ConvexPartition_HM(&poly, &outputs) == 0) {
                    LOG(WARNING) << "convex partition with failed (method: Hertel_Mehlhorn)";
                    return false;
                }
                break;
            case Optimal:
                if (partition.ConvexPartition_OPT(&poly, &outputs) == 0) {
                    LOG(WARNING) << "convex partition with failed (method: Optimal)";
                    return false;
                }
                break;
        }

        for (auto& plg : outputs) {
            const long size = plg.GetNumPoints();
            Polygon polygon;
            for (long i=0; i<size; ++i)
                polygon.push_back(plg[i].index);
            parts.push_back(polygon);
        }
        return true;
    }

}
