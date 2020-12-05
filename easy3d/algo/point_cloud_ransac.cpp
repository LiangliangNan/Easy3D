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

#include <easy3d/algo/point_cloud_ransac.h>

#include <set>
#include <list>

#include <easy3d/core/point_cloud.h>

#include <3rd_party/RANSAC-1.1/RansacShapeDetector.h>
#include <3rd_party/RANSAC-1.1/PlanePrimitiveShapeConstructor.h>
#include <3rd_party/RANSAC-1.1/CylinderPrimitiveShapeConstructor.h>
#include <3rd_party/RANSAC-1.1/SpherePrimitiveShapeConstructor.h>
#include <3rd_party/RANSAC-1.1/ConePrimitiveShapeConstructor.h>
#include <3rd_party/RANSAC-1.1/TorusPrimitiveShapeConstructor.h>
#include <3rd_party/RANSAC-1.1/PlanePrimitiveShape.h>
#include <3rd_party/RANSAC-1.1/SpherePrimitiveShape.h>
#include <3rd_party/RANSAC-1.1/CylinderPrimitiveShape.h>
#include <3rd_party/RANSAC-1.1/ConePrimitiveShape.h>
#include <3rd_party/RANSAC-1.1/TorusPrimitiveShape.h>

//OMG, there is class with exactly the same name in RANSAC!!!
typedef ::PointCloud PointCloud_Ransac;

namespace easy3d {


    // returns the number of detected primitives
    int do_detect(
            PointCloud *cloud,
            PointCloud_Ransac &pc,
            const std::set<PrimitivesRansac::PrimType> &types,
            unsigned int min_support,
            float dist_thresh,
            float bitmap_reso,
            float normal_thresh,
            float overlook_prob
    ) {
        const Box3 &box = cloud->bounding_box();
        pc.setBBox(
                Vec3f(static_cast<float>(box.min(0)), static_cast<float>(box.min(1)), static_cast<float>(box.min(2))),
                Vec3f(static_cast<float>(box.max(0)), static_cast<float>(box.max(1)), static_cast<float>(box.max(2)))
        );

        //////////////////////////////////////////////////////////////////////////

        LOG(INFO) << "detecting primitives...";
//        StopWatch w;

        RansacShapeDetector::Options ransacOptions;
        ransacOptions.m_minSupport = min_support;
        ransacOptions.m_epsilon = dist_thresh * pc.getScale();
        ransacOptions.m_bitmapEpsilon = bitmap_reso * pc.getScale();
        ransacOptions.m_normalThresh = normal_thresh;
        ransacOptions.m_probability = overlook_prob;

        RansacShapeDetector detector(ransacOptions); // the detector object

        // set which primitives are to be detected by adding the respective constructors
        std::set<PrimitivesRansac::PrimType>::const_iterator it = types.begin();
        for (; it != types.end(); ++it) {
            switch (*it) {
                case PrimitivesRansac::PLANE:
                    detector.Add(new PlanePrimitiveShapeConstructor());
                    break;
                case PrimitivesRansac::CYLINDER:
                    detector.Add(new CylinderPrimitiveShapeConstructor());
                    break;
                case PrimitivesRansac::SPHERE:
                    detector.Add(new SpherePrimitiveShapeConstructor());
                    break;
                case PrimitivesRansac::CONE:
                    detector.Add(new ConePrimitiveShapeConstructor());
                    break;
                case PrimitivesRansac::TORUS:
                    detector.Add(new TorusPrimitiveShapeConstructor());
                    break;
                case PrimitivesRansac::UNKNOWN:
                    break;
            }
        }

        MiscLib::Vector<std::pair<MiscLib::RefCountPtr<PrimitiveShape>, size_t> > shapes; // stores the detected shapes
        // returns number of unassigned points
        // the array shapes is filled with pointers to the detected shapes
        // the second element per shapes gives the number of points assigned to that primitive (the support)
        // the points belonging to the first shape (shapes[0]) have been sorted to the end of pc,
        // i.e. into the range [ pc.size() - shapes[0].second, pc.size() )
        // the points of shape i are found in the range
        // [ pc.size() - \sum_{j=0..i} shapes[j].second, pc.size() - \sum_{j=0..i-1} shapes[j].second )
        std::size_t remaining = detector.Detect(pc, 0, pc.size(), &shapes); // run detection

        PointCloud_Ransac::reverse_iterator start = pc.rbegin();
        MiscLib::Vector<std::pair<MiscLib::RefCountPtr<PrimitiveShape>, std::size_t> >::const_iterator shape_itr = shapes.begin();

        auto primitive_types = cloud->vertex_property<int>("v:primitive_type", PrimitivesRansac::UNKNOWN);
        auto primitive_indices = cloud->vertex_property<int>("v:primitive_index", -1);
        primitive_types.vector().assign(cloud->n_vertices(), PrimitivesRansac::UNKNOWN);
        primitive_indices.vector().assign(cloud->n_vertices(), -1);

        int index = 0;
        for (; shape_itr != shapes.end(); ++shape_itr) {
            const PrimitiveShape *primitive = shape_itr->first;
            std::size_t num = shape_itr->second;

            std::list<int> vts;
            PointCloud_Ransac::reverse_iterator point_itr = start;
            for (std::size_t count = 0; count < num; ++count) {
                int v = int(point_itr->index);
                vts.push_back(v);
                ++point_itr;
            }
            start = point_itr;

            // Liangliang: I have to check Ruwen Schnabel's source code of RANSAC. Can the
            //			   returned primitive have a support point number less than min_support?
            //			   If so, just skip it.
            if (num < min_support)
                continue;

            // extract parameters for this primitive
            switch (primitive->Identifier()) {
                case PrimitivesRansac::PLANE: {
                    // parameters are discarded
//                const Plane& pl = dynamic_cast<const PlanePrimitiveShape*>(primitive)->Internal();
//                const Vec3f& p = pl.getPosition();
//                const Vec3f& n = pl.getNormal();
//                const Plane3 plane(vec3(p.getValue()), vec3(n.getValue()));
                    for (auto id : vts) {
                        const PointCloud::Vertex v(id);
                        primitive_types[v] = PrimitivesRansac::PLANE;
                        primitive_indices[v] = index;
                    }
                    break;
                }
                case PrimitivesRansac::CYLINDER: {
                    // parameters are discarded
//                const Cylinder& cylinder = dynamic_cast<const CylinderPrimitiveShape*>(primitive)->Internal();
//                double radius = cylinder.Radius();
//                const Vec3f& pos = cylinder.AxisPosition();
//                const Vec3f& nor = cylinder.AxisDirection();
//                const vec3  position(pos[0], pos[1], pos[2]);
//                vec3  dir(nor[0], nor[1], nor[2]); dir = normalize(dir);
                    for (auto id : vts) {
                        const PointCloud::Vertex v(id);
                        primitive_types[v] = PrimitivesRansac::CYLINDER;
                        primitive_indices[v] = index;
                    }
                    break;
                }
                case PrimitivesRansac::SPHERE: {
                    // parameters are discarded
//                const Sphere& sphere = dynamic_cast<const SpherePrimitiveShape*>(primitive)->Internal();
//                double radius = sphere.Radius();
//                const Vec3f& center = sphere.Center();
                    for (auto id : vts) {
                        const PointCloud::Vertex v(id);
                        primitive_types[v] = PrimitivesRansac::SPHERE;
                        primitive_indices[v] = index;
                    }
                    break;
                }

                case PrimitivesRansac::CONE: {
                    // parameters are discarded
/*                const Cone& cone = dynamic_cast<const ConePrimitiveShape*>(primitive)->Internal();
                const Vec3f& dir = cone.AxisDirection();
                double angle = cone.Angle();
                double radius = cone.RadiusAtLength(1.0f); */ // NOTE:: the center is the apex of the cone
                    for (auto id : vts) {
                        const PointCloud::Vertex v(id);
                        primitive_types[v] = PrimitivesRansac::CONE;
                        primitive_indices[v] = index;
                    }
                    break;
                }
                case PrimitivesRansac::TORUS: {
                    // parameters are discarded
//                const Torus& torus = dynamic_cast<const TorusPrimitiveShape*>(primitive)->Internal();
//                const Vec3f& center = torus.Center();
//                const Vec3f& dir = torus.AxisDirection();
//                double min_radius = torus.MinorRadius();
//                double max_radius = torus.MajorRadius();
                    for (auto id : vts) {
                        const PointCloud::Vertex v(id);
                        primitive_types[v] = PrimitivesRansac::TORUS;
                        primitive_indices[v] = index;
                    }
                    break;
                }
            }
            ++index;
        }

        LOG(INFO) << index << " primitives extracted. " << remaining << " points remained";
        return index;
    }


    void PrimitivesRansac::add_primitive_type(PrimType t) {
        types_.insert(t);
    }


    int PrimitivesRansac::detect(
            PointCloud *cloud,
            unsigned int min_support /* = 1000 */,
            float dist_thresh /* = 0.005 */,
            float bitmap_reso /* = 0.02 */,
            float normal_thresh /* = 0.8 */,
            float overlook_prob /* = 0.001 */ ) {
        if (!cloud) {
            LOG(ERROR) << "no data exists";
            return 0;
        }

        if (cloud->n_vertices() < 3) {
            LOG(ERROR) << "point set has less than 3 points";
            return 0;
        }

        if (types_.empty()) {
            LOG(ERROR) << "no primitive types specified";
            return 0;
        }

        PointCloud::VertexProperty<vec3> normals = cloud->get_vertex_property<vec3>("v:normal");
        if (!normals) {
            LOG(ERROR) << "RANSAC Detector requires point cloud normals";
            return 0;
        }

        // prepare the data
        PointCloud_Ransac pc;
        pc.resize(cloud->n_vertices());

        const std::vector<vec3> &nms = normals.vector();
        const std::vector<vec3> &pts = cloud->points();
#pragma omp parallel for
        for (int i = 0; i < pts.size(); ++i) {
            const vec3 &p = pts[i];
            const vec3 &n = nms[i];
            pc[i] = Point(
                    Vec3f(p.x, p.y, p.z),
                    Vec3f(n.x, n.y, n.z)
            );
            pc[i].index = i;
        }

        return do_detect(cloud, pc, types_, min_support, dist_thresh, bitmap_reso, normal_thresh, overlook_prob);
    }


    int PrimitivesRansac::detect(
            PointCloud *cloud,
            const std::vector<int> &vertitces,
            unsigned int min_support /* = 1000 */,
            float dist_thresh /* = 0.005 */,
            float bitmap_reso /* = 0.02 */,
            float normal_thresh /* = 0.8 */,
            float overlook_prob /* = 0.001 */ ) {
        if (!cloud) {
            LOG(ERROR) << "no data exists";
            return 0;
        }

        if (cloud->n_vertices() < 3) {
            LOG(ERROR) << "point set has less than 3 points";
            return 0;
        }

        if (types_.empty()) {
            LOG(ERROR) << "no primitive types specified";
            return 0;
        }

        PointCloud::VertexProperty<vec3> normals = cloud->get_vertex_property<vec3>("v:normal");
        if (!normals) {
            LOG(ERROR) << "RANSAC Detector requires point cloud normals";
            return 0;
        }

        // prepare the data
        PointCloud_Ransac pc;
        pc.resize(vertitces.size());

        const std::vector<vec3> &nms = normals.vector();
        const std::vector<vec3> &pts = cloud->points();
#pragma omp parallel for
        for (int index = 0; index < vertitces.size(); ++index) {
            std::size_t idx = vertitces[index];
            const vec3 &p = pts[idx];
            const vec3 &n = nms[idx];
            pc[index] = Point(
                    Vec3f(p.x, p.y, p.z),
                    Vec3f(n.x, n.y, n.z)
            );
            pc[index].index = idx;
        }

        return do_detect(cloud, pc, types_, min_support, dist_thresh, bitmap_reso, normal_thresh, overlook_prob);
    }

}
