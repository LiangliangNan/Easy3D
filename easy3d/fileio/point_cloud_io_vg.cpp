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

group_type: type (integer: 	VG_PLANE = 0, VG_CYLINDER = 1, VG_SPHERE = 2, VG_CONE = 3, VG_TORUS = 4, VG_GENERAL = 5)
num_group_parameters: NUM_GROUP_PARAMETERS   // number of floating point values (integer)
group_parameters: float[NUM_GROUP_PARAMETERS]
group_label: label  // the first group info
group_color: color (r, g, b)
group_num_points: num	// can be 0
idx ...

num_children: num		// can be 0

group_type: type (integer: 	VG_PLANE = 0, VG_CYLINDER = 1, VG_SPHERE = 2, VG_CONE = 3, VG_TORUS = 4, VG_GENERAL = 5)
num_group_parameters: NUM_GROUP_PARAMETERS   // number of floating point values (integer)
group_parameters: float[NUM_GROUP_PARAMETERS]
group_label: label  // 0th child of group 0
group_color: color (r, g, b)
group_num_points: num
idx ...

group_type: type (integer: 	VG_PLANE = 0, VG_CYLINDER = 1, VG_SPHERE = 2, VG_CONE = 3, VG_TORUS = 4, VG_GENERAL = 5)
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
                LOG(ERROR) << "Could not open file\'" << file_name << "\'";
                return false;
            }
            output.precision(16);

            //////////////////////////////////////////////////////////////////////////

            const std::vector<vec3>& points = cloud->points();
            auto cls = cloud->get_vertex_property<vec3>("v:color");
            auto nms = cloud->get_vertex_property<vec3>("v:normal");


            const std::vector<VertexGroup>& groups = collect_groups(cloud);
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
        group_type: type (integer: 	VG_PLANE = 0, VG_CYLINDER = 1, VG_SPHERE = 2, VG_CONE = 3, VG_TORUS = 4, VG_GENERAL = 5)
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
                LOG(ERROR) << "Could not open file\'" << file_name << "\'";
                return false;
            }

            std::string dumy;
            unsigned int num;
            input >> dumy >> num;
            if (input.fail()) {
                LOG(ERROR) << "failed reading point number";
                return false;
            }

            cloud->resize(num);
            std::vector<vec3>& points = cloud->points();
            for (unsigned int i = 0; i < num; ++i) {
                input >> points[i];
                if (input.fail()) {
                    LOG(ERROR) << "failed reading the " << i << "_th point";
                    return false;
                }
            }

            input >> dumy >> num;
            if (input.fail()) {
                LOG(ERROR) << "failed reading color number";
                return false;
            }
            if (num == points.size()) {
                auto cls = cloud->add_vertex_property<vec3>("v:color");
                std::vector<vec3>& colors = cls.vector();
                for (unsigned int i = 0; i < num; ++i) {
                    input >> colors[i];
                    if (input.fail()) {
                        LOG(ERROR) << "failed reading the color of the " << i << "_th point";
                        return false;
                    }
                }
            }

            input >> dumy >> num;
            if (input.fail()) {
                LOG(ERROR) << "failed reading normal number";
                return false;
            }
            if (num == points.size()) {
                auto nms = cloud->add_vertex_property<vec3>("v:normal");
                std::vector<vec3>& normals = nms.vector();
                for (unsigned int i = 0; i < num; ++i) {
                    input >> normals[i];
                    if (input.fail()) {
                        LOG(ERROR) << "failed reading the normal of the " << i << "_th point";
                        return false;
                    }
                }
            }

            //////////////////////////////////////////////////////////////////////////

            std::size_t num_groups = 0;
            input >> dumy >> num_groups;
            if (input.fail()) {
                LOG(ERROR) << "failed reading vertex group number";
                return false;
            }
            for (unsigned int i = 0; i<num_groups; ++i) {
                VertexGroup g = read_ascii_group(input);
                g.primitive_index_ = i;

                if (!g.empty()) {
                    auto prim_type = cloud->vertex_property<int>("v:primitive_type");
                    auto prim_index = cloud->vertex_property<int>("v:primitive_index");
                    for (auto v : g) {
                        prim_type[PointCloud::Vertex(v)] = g.primitive_type_;
                        prim_index[PointCloud::Vertex(v)] = g.primitive_index_;
                    }
                }

                int num_children = 0;
                input >> dumy >> num_children;
                if (input.fail()) {
                    LOG(ERROR) << "failed reading children number";
                    return false;
                }
                for (int j = 0; j<num_children; ++j) {
                    VertexGroup chld = read_ascii_group(input);
                    if (!chld.empty()) {
                        g.children_.push_back(chld);
                    }
                }
            }

            return true;
        }


        PointCloudIO_vg::VertexGroup PointCloudIO_vg::read_ascii_group(std::istream& input) {
            std::string dumy;
            int type;
            input >> dumy >> type;
            if (input.fail()) {
                LOG_FIRST_N(ERROR, 3) << "failed reading vertex group type";
            }

            std::size_t num;
            input >> dumy >> num;
            LOG_IF(ERROR, num != num_group_parameters(type)) << "sizes don't match";
            std::vector<float> para(num);
            input >> dumy;
            for (std::size_t i = 0; i < num; ++i)
                input >> para[i];

            std::string label;
            input >> dumy >> label;

            vec3 color;
            input >> dumy >> color;

            int num_points;
            input >> dumy >> num_points;

            VertexGroup grp(type);
            assign_group_parameters(grp, para);

            for (int i = 0; i < num_points; ++i) {
                int idx;
                input >> idx;
                grp.push_back(idx);
            }

            grp.label_ = label;
            grp.color_ = color;

            return grp;
        }


        bool PointCloudIO_vg::load_bvg(const std::string& file_name, PointCloud* cloud) {
            std::ifstream input(file_name.c_str(), std::fstream::binary);
            if (input.fail()) {
                LOG(ERROR) << "Could not open file\'" << file_name << "\'";
                return false;
            }

            int num;
            input.read((char*)(&num), sizeof(int));
            if (num <= 0) {
                LOG(ERROR) << "no point exists in file\'" << file_name << "\'";
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
                VertexGroup g = read_binary_group(input);
                g.primitive_index_ = i;

                if (!g.empty()) {
                    auto prim_type = cloud->vertex_property<int>("v:primitive_type");
                    auto prim_index = cloud->vertex_property<int>("v:primitive_index");
                    for (auto v : g) {
                        prim_type[PointCloud::Vertex(v)] = g.primitive_type_;
                        prim_index[PointCloud::Vertex(v)] = g.primitive_index_;
                    }
                }

                int num_children = 0;
                input.read((char*)&num_children, sizeof(int));
                for (int j = 0; j<num_children; ++j) {
                    VertexGroup chld = read_binary_group(input);
                    if (!chld.empty()) {
                        g.children_.push_back(chld);
                    }
                }
            }

            return true;
        }


        bool PointCloudIO_vg::save_bvg(const std::string& file_name, const PointCloud* cloud) {
            // open file
            std::ofstream output(file_name.c_str(), std::fstream::binary);
            if (output.fail()) {
                LOG(ERROR) << "Could not open file\'" << file_name << "\'";
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

            const std::vector<VertexGroup>& groups = collect_groups(cloud);
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
        PointCloudIO_vg::VertexGroup PointCloudIO_vg::read_binary_group(std::istream& input) {
            int type;
            input.read((char*)&type, sizeof(int));

            int num;
            input.read((char*)&num, sizeof(int));
            LOG_IF(ERROR, num != num_group_parameters(type)) << "sizes don't match";

            std::vector<float> para(num);
            input.read((char*)para.data(), num * sizeof(float));

            VertexGroup grp(type);
            assign_group_parameters(grp, para);

            //////////////////////////////////////////////////////////////////////////
            int label_size = 0;
            input.read((char*)&label_size, sizeof(int));
            std::vector<char> label(label_size);
            input.read(label.data(), label_size);
            grp.label_ = std::string(label.begin(), label.end());
            //////////////////////////////////////////////////////////////////////////

            vec3 color;
            input.read((char*)color.data(), 3 * sizeof(float));
            grp.color_ = color;

            int num_points = 0;
            input.read((char*)&num_points, sizeof(int));
            grp.resize(num_points);
            input.read((char*)grp.data(), num_points * sizeof(int));

            return grp;
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
            case VertexGroup::VG_PLANE:
                return 4;
            case VertexGroup::VG_CYLINDER:
                LOG(WARNING) << "not implemented for VG_CYLINDER";
                return 0;
            case VertexGroup::VG_SPHERE:
                LOG(WARNING)  << "not implemented for VG_SPHERE";
                return 0;
            case VertexGroup::VG_CONE:
                LOG(WARNING)  << "not implemented for VG_CONE";
                return 0;
            case VertexGroup::VG_TORUS:
                LOG(WARNING)  << "not implemented for VG_TORUS";
                return 0;
            case VertexGroup::VG_GENERAL:
                LOG(WARNING)  << "not implemented for VG_GENERAL";
                return 0;
            }

            return 0;
        }


        std::vector<float> PointCloudIO_vg::get_group_parameters(const VertexGroup& group) {
            int num = num_group_parameters(group.primitive_type_);
            std::vector<float> para(num);

            switch (group.primitive_type_)
            {
            case VertexGroup::VG_PLANE: {
//                VertexGroupPlane* g = dynamic_cast<VertexGroupPlane*>(group);
//                para[0] = static_cast<float>(g->plane().a());
//                para[1] = static_cast<float>(g->plane().b());
//                para[2] = static_cast<float>(g->plane().c());
//                para[3] = static_cast<float>(g->plane().d());
                return para;
            }
            case VertexGroup::VG_CYLINDER:
                LOG(WARNING) << "not implemented for VG_CYLINDER";
                return para;
            case VertexGroup::VG_SPHERE:
                LOG(WARNING)  << "not implemented for VG_SPHERE";
                return para;
            case VertexGroup::VG_CONE:
                LOG(WARNING)  << "not implemented for VG_CONE";
                return para;
            case VertexGroup::VG_TORUS:
                LOG(WARNING)  << "not implemented for VG_TORUS";
                return para;
            case VertexGroup::VG_GENERAL:
                LOG(WARNING)  << "not implemented for VG_GENERAL";
                return para;
            }

            return para;
        }


        void PointCloudIO_vg::assign_group_parameters(VertexGroup& group, std::vector<float>& para) {
            int num = num_group_parameters(group.primitive_type_);
            assert(para.size() == num);

            switch (group.primitive_type_)
            {
            case VertexGroup::VG_PLANE: {
//                VertexGroupPlane* g = dynamic_cast<VertexGroupPlane*>(group);
//                g->set_plane(Plane3(para[0], para[1], para[2], para[3]));
                return;
            }
            case VertexGroup::VG_CYLINDER:
                LOG(WARNING) << "not implemented for VG_CYLINDER";
                return;
            case VertexGroup::VG_SPHERE:
                LOG(WARNING)  << "not implemented for VG_SPHERE";
                return;
            case VertexGroup::VG_CONE:
                LOG(WARNING)  << "not implemented for VG_CONE";
                return;
            case VertexGroup::VG_TORUS:
                LOG(WARNING)  << "not implemented for VG_TORUS";
                return;
            case VertexGroup::VG_GENERAL:
                LOG(WARNING)  << "not implemented for VG_GENERAL";
                return;
            }
        }


        std::vector<PointCloudIO_vg::VertexGroup> PointCloudIO_vg::collect_groups(const PointCloud* model) {
            auto primitve_type = model->get_vertex_property<int>("v:primitive_type");
            auto primitve_index = model->get_vertex_property<int>("v:primitive_index");
            int num = 0;
            for (auto v : model->vertices())
                num = std::max(num, primitve_index[v]);
            ++num;
            // assign each plane a unique color
            std::vector<vec3> color_table(num);
            for (auto& c : color_table)
                c = random_color();

            std::vector<PointCloudIO_vg::VertexGroup> groups(num);

            for (auto v : model->vertices()) {
                int index = primitve_index[v];
                VertexGroup& g = groups[index];
                g.push_back(v.idx());
            }

            for (std::size_t i = 0; i<groups.size(); ++i) {
                VertexGroup& g = groups[i];
                auto v = PointCloud::Vertex(g[0]);
                g.primitive_type_ = primitve_type[v];
                g.primitive_index_ = primitve_index[v];
                g.color_ = color_table[i];
                g.label_ = "group_" + std::to_string(i);
             }

            return groups;
        }

    } // namespace io

} // namespace easy3d
