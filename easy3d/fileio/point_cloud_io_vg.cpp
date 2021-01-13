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

#include <easy3d/fileio/point_cloud_io_vg.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/random.h>
#include <easy3d/util/logging.h>

#include <fstream>
#include <unordered_map>

/*
// file format definition
num_points: num
x  y  z
...

num_colors: num		// can be 0; if not, it must equal to num_points
r g b
...

num_normals: num	// can be 0; if not, it must equal to num_points
nx  ny  nz

num_groups: num		// can be 0

group_type: type (integer: 	PLANE = 0, CYLINDER = 1, SPHERE = 2, CONE = 3, TORUS = 4, GENERAL = 5)
num_group_parameters: NUM_GROUP_PARAMETERS   // number of floating point values (integer)
group_parameters: float[NUM_GROUP_PARAMETERS]
group_label: label  // the first group info
group_color: color (r, g, b)
group_num_points: num	// can be 0
idx ...

num_children: num		// can be 0

group_type: type (integer: 	PLANE = 0, CYLINDER = 1, SPHERE = 2, CONE = 3, TORUS = 4, GENERAL = 5)
num_group_parameters: NUM_GROUP_PARAMETERS   // number of floating point values (integer)
group_parameters: float[NUM_GROUP_PARAMETERS]
group_label: label  // 0th child of group 0
group_color: color (r, g, b)
group_num_points: num
idx ...

group_type: type (integer: 	PLANE = 0, CYLINDER = 1, SPHERE = 2, CONE = 3, TORUS = 4, GENERAL = 5)
num_group_parameters: NUM_GROUP_PARAMETERS   // number of floating point values (integer)
group_parameters: float[NUM_GROUP_PARAMETERS]
group_label: label  // 1st child of group 0
group_color: color (r, g, b)
group_num_points: num
idx ...
*/



namespace easy3d {

    namespace io {

        bool PointCloudIO_vg::save_vg(const std::string& file_name, const PointCloud* cloud) {
            // open file
            std::ofstream output(file_name.c_str());
            if (output.fail()) {
                LOG(ERROR) << "could not open file: " << file_name;
                return false;
            }
            output.precision(16);

            //////////////////////////////////////////////////////////////////////////

            const std::vector<vec3>& points = cloud->points();
            auto cls = cloud->get_vertex_property<vec3>("v:color");
            auto nms = cloud->get_vertex_property<vec3>("v:normal");


            std::vector<VertexGroup> groups;
            collect_groups(cloud, groups);
            output << "num_points: " << points.size() << std::endl;
            for (std::size_t i = 0; i < points.size(); ++i)
                output << points[i] << " ";
            output << std::endl;

            output << "num_colors: " << (cls ? points.size() : 0) << std::endl;
            if (cls) {
                const std::vector<vec3>& colors = cls.vector();
                for (std::size_t i = 0; i < colors.size(); ++i)
                    output << colors[i] << " ";
                output << std::endl;
            }

            output << "num_normals: " << (nms ? points.size() : 0) << std::endl;
            if (nms) {
                const std::vector<vec3>& normals = nms.vector();
                for (std::size_t i = 0; i < normals.size(); ++i)
                    output << normals[i] << " ";
                output << std::endl;
            }

            output << "num_groups: " << groups.size() << std::endl;
            for (std::size_t i = 0; i < groups.size(); ++i) {
                const VertexGroup& g = groups[i];
                write_ascii_group(output, g);

                // children
                const std::vector<VertexGroup>& children = g.children_;;
                output << "num_children: " << children.size() << std::endl;
                for (unsigned int j = 0; j < children.size(); ++j)
                    write_ascii_group(output, children[j]);
            }

            return true;
        }

        /*
        group_type: type (integer: 	PLANE = 0, CYLINDER = 1, SPHERE = 2, CONE = 3, TORUS = 4, GENERAL = 5)
        num_group_parameters: NUM_GROUP_PARAMETERS   // number of floating point values (integer)
        group_parameters: float[NUM_GROUP_PARAMETERS]
        group_label: label  // the first group info
        group_color: color (r, g, b)
        group_num_points: num
        idx ...
        */
        void PointCloudIO_vg::write_ascii_group(std::ostream& output, const VertexGroup& g) {
            int type = g.primitive_type_;
            output << "group_type: " << type << std::endl;

            const std::vector<float>& para = get_group_parameters(g);
            output << "num_group_parameters: " << para.size() << std::endl;
            output << "group_parameters: ";
            for (std::size_t i = 0; i < para.size(); ++i)
                output << para[i] << " ";
            output << std::endl;

            std::string label = g.label_;
            output << "group_label: " << label << std::endl;

            const vec3& c = g.color_;
            output << "group_color: " << c << std::endl;

            std::size_t num_point = g.size();
            output << "group_num_point: " << num_point << std::endl;

            for (std::size_t i = 0; i < num_point; ++i)
                output << g[i] << " ";
            output << std::endl;
        }


        bool PointCloudIO_vg::load_vg(const std::string& file_name, PointCloud* cloud) {
            std::ifstream input(file_name.c_str());
            if (input.fail()) {
                LOG(ERROR) << "could not open file: " << file_name;
                return false;
            }

            std::string dummy;
            std::size_t num;
            input >> dummy >> num;
            if (input.fail()) {
                LOG(ERROR) << "failed reading point number";
                return false;
            }
            if (num <= 0) {
                LOG(ERROR) << "invalid point number (must be positive): " << num;
                return false;
            }

            cloud->resize(num);
            std::vector<vec3>& points = cloud->points();
            for (std::size_t i = 0; i < num; ++i) {
                input >> points[i];
                if (input.fail()) {
                    LOG(ERROR) << "failed reading the " << i << "_th point";
                    return false;
                }
            }

            input >> dummy >> num;
            if (input.fail()) {
                LOG(ERROR) << "failed reading color number";
                return false;
            }
            if (num == points.size()) {
                auto cls = cloud->add_vertex_property<vec3>("v:color");
                std::vector<vec3>& colors = cls.vector();
                for (std::size_t i = 0; i < num; ++i) {
                    input >> colors[i];
                    if (input.fail()) {
                        LOG(ERROR) << "failed reading the color of the " << i << "_th point";
                        return false;
                    }
                }
            }

            input >> dummy >> num;
            if (input.fail()) {
                LOG(ERROR) << "failed reading normal number";
                return false;
            }
            if (num == points.size()) {
                auto nms = cloud->add_vertex_property<vec3>("v:normal");
                std::vector<vec3>& normals = nms.vector();
                for (std::size_t i = 0; i < num; ++i) {
                    input >> normals[i];
                    if (input.fail()) {
                        LOG(ERROR) << "failed reading the normal of the " << i << "_th point";
                        return false;
                    }
                }
                // check if the normals are normalized
                const float len = length(normals[0]);
                LOG_IF(std::abs(1.0 - len) > epsilon<float>(), WARNING)
                                << "normals not normalized (length of the first normal vector is " << len << ")";
            }

            //////////////////////////////////////////////////////////////////////////

            std::size_t num_groups = 0;
            input >> dummy >> num_groups;
            if (input.fail()) {
                LOG(ERROR) << "failed reading vertex group number";
                return false;
            }

            for (std::size_t i = 0; i<num_groups; ++i) {
                VertexGroup g;
                read_ascii_group(input, g);
                g.primitive_index_ = i;

                if (!g.empty()) {
                    auto prim_type = cloud->vertex_property<int>("v:primitive_type", VertexGroup::UNKNOWN);
                    auto prim_index = cloud->vertex_property<int>("v:primitive_index", -1);
                    for (auto v : g) {
                        prim_type[PointCloud::Vertex(v)] = g.primitive_type_;
                        prim_index[PointCloud::Vertex(v)] = g.primitive_index_;
                    }
                }

                std::size_t num_children = 0;
                input >> dummy >> num_children;
                if (input.fail()) {
                    LOG(ERROR) << "failed reading children number";
                    return false;
                }
                for (std::size_t j = 0; j<num_children; ++j) {
                    VertexGroup child;
                    read_ascii_group(input, child);
                    if (!child.empty()) {
                        g.children_.push_back(child);
                    }
                }
            }

            return true;
        }


        void PointCloudIO_vg::read_ascii_group(std::istream& input, VertexGroup& group) {
            group.clear();

            std::string dummy;
            int type;
            input >> dummy >> type;
            if (input.fail()) {
                LOG_FIRST_N(1, ERROR) << "failed reading vertex group type. " << COUNTER;
            }

            std::size_t num;
            input >> dummy >> num;
            LOG_IF(num != num_group_parameters(type), ERROR) << "sizes don't match";
            std::vector<float> para(num);
            input >> dummy;
            double v;
            for (std::size_t i = 0; i < num; ++i) {
                input >> v;
                if (input.fail()) {
                    LOG_FIRST_N(1, ERROR) << "failed reading vertex group parameters. " << COUNTER;
                } else
                    para[i] = v;
            }

            std::string label;
            input >> dummy >> label;
            if (input.fail()) {
                LOG_FIRST_N(1, ERROR) << "failed reading vertex group label. " << COUNTER;
            }

            vec3 color;
            input >> dummy >> color;

            std::size_t num_points;
            input >> dummy >> num_points;

            group.primitive_type_ = type;
            assign_group_parameters(group, para);

            group.resize(num_points);
            for (std::size_t i = 0; i < num_points; ++i) {
                input >> group[i];
            }

            group.label_ = label;
            group.color_ = color;
        }


        bool PointCloudIO_vg::load_bvg(const std::string& file_name, PointCloud* cloud) {
            std::ifstream input(file_name.c_str(), std::fstream::binary);
            if (input.fail()) {
                LOG(ERROR) << "could not open file: " << file_name;
                return false;
            }

            int num;
            input.read((char*)(&num), sizeof(int));
            if (num <= 0) {
                LOG(ERROR) << "no point exists in file'" << file_name << "'";
                return false;
            }

            cloud->resize(num);

            // read the points block
            auto points = cloud->get_vertex_property<vec3>("v:point");
            input.read((char*)points.data(), num * sizeof(vec3));

            // read the colors block if exists
            input.read((char*)(&num), sizeof(int));
            if (num == cloud->n_vertices()) {
                auto colors = cloud->add_vertex_property<vec3>("v:color");
                input.read((char*)colors.data(), num * sizeof(vec3));
            }

            // read the normals block if exists
            input.read((char*)(&num), sizeof(int));
            if (num == cloud->n_vertices()) {
                auto normals = cloud->add_vertex_property<vec3>("v:normal");
                input.read((char*)normals.data(), num * sizeof(vec3));
            }

            //////////////////////////////////////////////////////////////////////////

            int num_groups = 0;
            input.read((char*)&num_groups, sizeof(int));
            for (int i = 0; i<num_groups; ++i) {
                VertexGroup g;
                read_binary_group(input, g);
                g.primitive_index_ = i;

                if (!g.empty()) {
                    auto prim_type = cloud->vertex_property<int>("v:primitive_type", VertexGroup::UNKNOWN);
                    auto prim_index = cloud->vertex_property<int>("v:primitive_index", -1);
                    for (auto v : g) {
                        prim_type[PointCloud::Vertex(v)] = g.primitive_type_;
                        prim_index[PointCloud::Vertex(v)] = g.primitive_index_;
                    }
                }

                int num_children = 0;
                input.read((char*)&num_children, sizeof(int));
                for (int j = 0; j<num_children; ++j) {
                    VertexGroup child;
                    read_binary_group(input, child);
                    if (!child.empty()) {
                        g.children_.push_back(child);
                    }
                }
            }

            return true;
        }


        bool PointCloudIO_vg::save_bvg(const std::string& file_name, const PointCloud* cloud) {
            // open file
            std::ofstream output(file_name.c_str(), std::fstream::binary);
            if (output.fail()) {
                LOG(ERROR) << "could not open file: " << file_name;
                return false;
            }

            const std::vector<vec3>& points = cloud->points();
            auto cls = cloud->get_vertex_property<vec3>("v:color");
            auto nms = cloud->get_vertex_property<vec3>("v:normal");

            // write the points block
            std::size_t num = points.size();
            output.write((char*)&num, sizeof(int));
            output.write((char*)points.data(), num * sizeof(vec3));

            if (cls) {
                const std::vector<vec3>& colors = cls.vector();
                output.write((char*)&num, sizeof(int));
                output.write((char*)colors.data(), num * sizeof(vec3));
            }
            else {
                int dummy = 0;
                output.write((char*)&dummy, sizeof(int));
            }

            if (nms) {
                const std::vector<vec3>& normals = nms.vector();
                output.write((char*)&num, sizeof(int));
                output.write((char*)normals.data(), num * sizeof(vec3));
            }
            else {
                int dummy = 0;
                output.write((char*)&dummy, sizeof(int));
            }

            //////////////////////////////////////////////////////////////////////////

            std::vector<VertexGroup> groups;
            collect_groups(cloud, groups);
            std::size_t num_groups = groups.size();
            output.write((char*)&num_groups, sizeof(int));

            for (std::size_t i = 0; i < num_groups; ++i) {
                const VertexGroup& g = groups[i];
                write_binary_group(output, g);

                // children
                const std::vector<VertexGroup>& children = g.children_;
                std::size_t chld_num = children.size();
                output.write((char*)&chld_num, sizeof(int));
                for (std::size_t j = 0; j < chld_num; ++j)
                    write_binary_group(output, children[j]);
            }

            return true;
        }


        // for binary file format, no string stuff except labels. we add size info before each label
        void PointCloudIO_vg::read_binary_group(std::istream& input, VertexGroup& group) {
            group.clear();

            int type;
            input.read((char*)&type, sizeof(int));

            int num;
            input.read((char*)&num, sizeof(int));
            LOG_IF(num != num_group_parameters(type), ERROR) << "sizes don't match";

            std::vector<float> para(num);
            input.read((char*)para.data(), num * sizeof(float));

            group.primitive_type_ = type;
            assign_group_parameters(group, para);

            //////////////////////////////////////////////////////////////////////////
            int label_size = 0;
            input.read((char*)&label_size, sizeof(int));
            std::vector<char> label(label_size);
            input.read(label.data(), label_size);
            group.label_ = std::string(label.begin(), label.end());
            //////////////////////////////////////////////////////////////////////////

            vec3 color;
            input.read((char*)color.data(), 3 * sizeof(float));
            group.color_ = color;

            int num_points = 0;
            input.read((char*)&num_points, sizeof(int));
            group.resize(num_points);
            input.read((char*)group.data(), num_points * sizeof(int));
        }


        // for binary file format, no string stuff except labels. we add size info before each label
        void PointCloudIO_vg::write_binary_group(std::ostream& output, const VertexGroup& g) {
            int type = g.primitive_type_;
            output.write((char*)&type, sizeof(int));

            int num = num_group_parameters(type);
            output.write((char*)&num, sizeof(int));

            const std::vector<float>& para = get_group_parameters(g);
            output.write((char*)para.data(), sizeof(float) * num);

            //////////////////////////////////////////////////////////////////////////
            std::string label = g.label_;
            int label_size = static_cast<int>(label.size());
            output.write((char*)&label_size, sizeof(int));
            output.write(label.data(), label_size);
            //////////////////////////////////////////////////////////////////////////

            const vec3& c = g.color_;
            output.write((char*)c.data(), sizeof(float) * 3);

            std::size_t num_point = g.size();
            output.write((char*)&num_point, sizeof(int));
            output.write((char*)g.data(), num_point * sizeof(int));
        }


		int PointCloudIO_vg::num_group_parameters(int type) {
            switch (type)
            {
            case VertexGroup::PLANE:
                return 4;
            case VertexGroup::CYLINDER:
                LOG(WARNING) << "not implemented for CYLINDER";
                return 0;
            case VertexGroup::SPHERE:
                LOG(WARNING)  << "not implemented for SPHERE";
                return 0;
            case VertexGroup::CONE:
                LOG(WARNING)  << "not implemented for CONE";
                return 0;
            case VertexGroup::TORUS:
                LOG(WARNING)  << "not implemented for TORUS";
                return 0;
            case VertexGroup::GENERAL:
                LOG(WARNING)  << "not implemented for GENERAL";
                return 0;
            }

            return 0;
        }


        std::vector<float> PointCloudIO_vg::get_group_parameters(const VertexGroup& group) {
            int num = num_group_parameters(group.primitive_type_);
            std::vector<float> para(num);

            switch (group.primitive_type_)
            {
            case VertexGroup::PLANE: {
//                VertexGroupPlane* g = dynamic_cast<VertexGroupPlane*>(group);
//                para[0] = static_cast<float>(g->plane().a());
//                para[1] = static_cast<float>(g->plane().b());
//                para[2] = static_cast<float>(g->plane().c());
//                para[3] = static_cast<float>(g->plane().d());
                return para;
            }
            case VertexGroup::CYLINDER:
                LOG(WARNING) << "not implemented for CYLINDER";
                return para;
            case VertexGroup::SPHERE:
                LOG(WARNING)  << "not implemented for SPHERE";
                return para;
            case VertexGroup::CONE:
                LOG(WARNING)  << "not implemented for CONE";
                return para;
            case VertexGroup::TORUS:
                LOG(WARNING)  << "not implemented for TORUS";
                return para;
            case VertexGroup::GENERAL:
                LOG(WARNING)  << "not implemented for GENERAL";
                return para;
            }

            return para;
        }


        void PointCloudIO_vg::assign_group_parameters(VertexGroup& group, std::vector<float>& para) {
            int num = num_group_parameters(group.primitive_type_);
            assert(para.size() == num);

            switch (group.primitive_type_)
            {
            case VertexGroup::PLANE: {
//                VertexGroupPlane* g = dynamic_cast<VertexGroupPlane*>(group);
//                g->set_plane(Plane3(para[0], para[1], para[2], para[3]));
                return;
            }
            case VertexGroup::CYLINDER:
                LOG(WARNING) << "not implemented for CYLINDER";
                return;
            case VertexGroup::SPHERE:
                LOG(WARNING)  << "not implemented for SPHERE";
                return;
            case VertexGroup::CONE:
                LOG(WARNING)  << "not implemented for CONE";
                return;
            case VertexGroup::TORUS:
                LOG(WARNING)  << "not implemented for TORUS";
                return;
            case VertexGroup::GENERAL:
                LOG(WARNING)  << "not implemented for GENERAL";
                return;
            }
        }


        void PointCloudIO_vg::collect_groups(const PointCloud* cloud, std::vector<VertexGroup>& groups) {
            auto primitve_type = cloud->get_vertex_property<int>("v:primitive_type");
            auto primitve_index = cloud->get_vertex_property<int>("v:primitive_index");

            // each type has a number of groups; primitive type may not be continuous, e.g., 1, 2, 5, 6
            std::unordered_map<int, std::unordered_map<int, VertexGroup> > temp_groups;    // groups[primitive type][primitive index]
            for (auto v : cloud->vertices()) {
                int type = primitve_type[v];
                int index = primitve_index[v];
                if (index >= 0)
                    temp_groups[type][index].push_back(v.idx());
            }

            groups.clear();

            int index = 0;
            for (const auto& type_groups : temp_groups) {
                int type = type_groups.first;
                for (const auto& index_group : type_groups.second) {
                    groups.push_back(index_group.second);
                    VertexGroup& g = groups.back();
                    g.primitive_type_ = type;
                    g.primitive_index_ = index;
                    ++index;
                }
            }

            // assign each vertex group a unique color
            for (std::size_t i = 0; i<groups.size(); ++i) {
                VertexGroup& g = groups[i];
                g.color_ = random_color();
                g.label_ = "group_" + std::to_string(i);
             }

            // sort the vertex groups according to the number of points (not necessary but useful)
            struct VertexGroupCmpDecreasing {
                bool operator()(const VertexGroup& g0, const VertexGroup& g1) const {
                    return g0.size() > g1.size();
                }
            };
            std::sort(groups.begin(), groups.end(), VertexGroupCmpDecreasing());
        }

    } // namespace io

} // namespace easy3d
