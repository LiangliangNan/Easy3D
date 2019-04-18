/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D: software for processing and rendering
*   meshes and point clouds.
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#include <easy3d/fileio/point_cloud_io_vg.h>
#include <easy3d/core/point_cloud.h>

#include <cassert>
#include <fstream>


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
void PointSetSerializer_vg::save_vg(const std::string& file_name, const PointSet* pset) {
	// open file
	std::ofstream output(file_name.c_str());
	if (output.fail()) {
        std::cerr << "could not open file\'" << file_name << "\'" << std::endl;
		return;
	}
	output.precision(16);

	//////////////////////////////////////////////////////////////////////////

	const std::vector<vec3>& points = pset->points();
	PointSet::Vertex_property<vec3> cls = pset->get_vertex_property<vec3>("v:color");
	PointSet::Vertex_property<vec3> nms = pset->get_vertex_property<vec3>("v:normal");
	int num_elements = points.size();
	if (nms)
		num_elements += points.size();
	if (cls)
		num_elements += points.size();

	const std::vector<VertexGroup::Ptr>& groups = pset->groups();
	ProgressLogger progress(num_elements + groups.size());

	output << "num_points: " << points.size() << std::endl;
	for (std::size_t i = 0; i < points.size(); ++i) {
		output << points[i] << " ";
		progress.next();
	}
	output << std::endl;

	output << "num_colors: " << (cls ? points.size() : 0) << std::endl;
	if (cls) {
		const std::vector<vec3>& colors = cls.vector();
		for (std::size_t i = 0; i < colors.size(); ++i) {
			output << colors[i] << " ";
			progress.next();
		}
		output << std::endl;
	}

	output << "num_normals: " << (nms ? points.size() : 0) << std::endl;
	if (nms) {
		const std::vector<vec3>& normals = nms.vector();
			for (std::size_t i = 0; i < normals.size(); ++i) {
				output << normals[i] << " ";
				progress.next();
			}
		output << std::endl;
	}

	output << "num_groups: " << groups.size() << std::endl;
	for (std::size_t i = 0; i < groups.size(); ++i) {
		VertexGroup* g = groups[i];
		write_ascii_group(output, g);

		// children
		const std::vector<VertexGroup::Ptr>& children = g->children();
		output << "num_children: " << children.size() << std::endl;
		for (unsigned int j = 0; j < children.size(); ++j) {
			VertexGroup* chld = children[j];
			write_ascii_group(output, chld);
		}
		progress.next();
	}
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
void PointSetSerializer_vg::write_ascii_group(std::ostream& output, VertexGroup* g) {
	int type = g->type();
	output << "group_type: " << type << std::endl;

	const std::vector<float>& para = get_group_parameters(g);
	output << "num_group_parameters: " << para.size() << std::endl;
	output << "group_parameters: ";
	for (std::size_t i = 0; i < para.size(); ++i)
		output << para[i] << " ";
	output << std::endl;

	std::string label = g->label();
	output << "group_label: " << label << std::endl;

	const vec3& c = g->color();
	output << "group_color: " << c << std::endl;

	std::size_t num_point = g->size();
	output << "group_num_point: " << num_point << std::endl;

	for (std::size_t i = 0; i < g->size(); ++i) {
		output << g->at(i) << " ";
	}
	output << std::endl;
}


void PointSetSerializer_vg::load_vg(const std::string& file_name, PointSet* pset) {
	std::ifstream input(file_name.c_str());
	if (input.fail()) {
        std::cerr << "could not open file\'" << file_name << "\'" << std::endl;
		return;
	}

	// get length of file
	input.seekg(0, input.end);
	std::streamoff length = input.tellg();
	input.seekg(0, input.beg);
	ProgressLogger progress(length);

	std::string dumy;
	std::size_t num;

	input >> dumy >> num;

	pset->resize(num);
	std::vector<vec3>& points = pset->points();
	for (int i = 0; i < num; ++i) {
		input >> points[i];

		std::streamoff pos = input.tellg();
		progress.notify(pos);
	}

	input >> dumy >> num;
	if (num == points.size()) {
		PointSet::Vertex_property<vec3> cls = pset->add_vertex_property<vec3>("v:color");
		std::vector<vec3>& colors = cls.vector();
		for (int i = 0; i < num; ++i) {
			input >> colors[i];

			std::streamoff pos = input.tellg();
			progress.notify(pos);
		}
	}

	input >> dumy >> num;
	if (num == points.size()) {
		PointSet::Vertex_property<vec3> nms = pset->add_vertex_property<vec3>("v:normal");
		std::vector<vec3>& normals = nms.vector();
		for (int i = 0; i < num; ++i) {
			input >> normals[i];

			std::streamoff pos = input.tellg();
			progress.notify(pos);
		}
	}

	//////////////////////////////////////////////////////////////////////////

	std::size_t num_groups = 0;
	input >> dumy >> num_groups;
	for (int i = 0; i<num_groups; ++i) {
		VertexGroup::Ptr g = read_ascii_group(input);

		if (!g->empty()) {
			g->set_point_set(pset);
			pset->groups().push_back(g);
		}

		int num_children = 0;
		input >> dumy >> num_children;
		for (int j = 0; j<num_children; ++j) {
			VertexGroup::Ptr chld = read_ascii_group(input);
			if (!chld->empty()) {
				chld->set_point_set(pset);
				g->add_child(chld);
			}
		}

		std::streamoff pos = input.tellg();
		progress.notify(pos);
	}
}


VertexGroup* PointSetSerializer_vg::read_ascii_group(std::istream& input) {
	std::string dumy;
	int type;
	input >> dumy >> type;

	int num;
	input >> dumy >> num;
	assert(num == num_group_parameters(static_cast<VertexGroupType>(type)));
	std::vector<float> para(num);
	input >> dumy;
	for (int i = 0; i < num; ++i)
		input >> para[i];

	std::string label;
	input >> dumy >> label;

	vec3 color;
	input >> dumy >> color;

	int num_points;
	input >> dumy >> num_points;

	VertexGroup* grp = nil;
	switch (static_cast<VertexGroupType>(type))
	{
	case VG_PLANE:		grp = new VertexGroupPlane;		break;
	case VG_CYLINDER:	grp = new VertexGroupCylinder;	break;
	case VG_SPHERE:		grp = new VertexGroupSphere;	break;
	case VG_CONE:		grp = new VertexGroupCone;		break;
	case VG_TORUS:		grp = new VertexGroupTorus;		break;
	case VG_GENERAL:	grp = new VertexGroupGeneral;	break;
	default:	break;
	}
	assign_group_parameters(grp, para);

	for (int i = 0; i < num_points; ++i) {
		int idx;
		input >> idx;
		grp->push_back(idx);
	}

	grp->set_label(label);
	grp->set_color(color);

	return grp;
}


void PointSetSerializer_vg::load_bvg(const std::string& file_name, PointSet* pset) {
	std::ifstream input(file_name.c_str(), std::fstream::binary);
	if (input.fail()) {
        std::cerr << "could not open file\'" << file_name << "\'" << std::endl;
		return;
	}

	int num;
	input.read((char*)(&num), sizeof(int));
	if (num <= 0) {
        std::cerr << "no point exists in file\'" << file_name << "\'" << std::endl;
		return;
	}

	pset->resize(num);

	// read the points block
	PointSet::Vertex_property<vec3> points = pset->get_vertex_property<vec3>("v:point");
	input.read((char*)points.data(), num * sizeof(vec3));

	// read the colors block if exists
	input.read((char*)(&num), sizeof(int));
	if (num == pset->n_vertices()) {
		PointSet::Vertex_property<vec3> colors = pset->add_vertex_property<vec3>("v:color");
		input.read((char*)colors.data(), num * sizeof(vec3));
	}

	// read the normals block if exists
	input.read((char*)(&num), sizeof(int));
	if (num == pset->n_vertices()) {
		PointSet::Vertex_property<vec3> normals = pset->add_vertex_property<vec3>("v:normal");
		input.read((char*)normals.data(), num * sizeof(vec3));
	}

	//////////////////////////////////////////////////////////////////////////

	int num_groups = 0;
	input.read((char*)&num_groups, sizeof(int));
	for (int i = 0; i<num_groups; ++i) {
		VertexGroup::Ptr g = read_binary_group(input);

		if (!g->empty()) {
			g->set_point_set(pset);
			pset->groups().push_back(g);
		}

		int num_children = 0;
		input.read((char*)&num_children, sizeof(int));
		for (int j = 0; j<num_children; ++j) {
			VertexGroup::Ptr chld = read_binary_group(input);
			if (!chld->empty()) {
				chld->set_point_set(pset);
				g->add_child(chld);
			}
			else
				delete chld;
		}
	}
}


void PointSetSerializer_vg::save_bvg(const std::string& file_name, const PointSet* pset) {
	// open file
	std::ofstream output(file_name.c_str(), std::fstream::binary);
	if (output.fail()) {
        std::cerr << "could not open file\'" << file_name << "\'" << std::endl;
		return;
	}

	const std::vector<vec3>& points = pset->points();
	PointSet::Vertex_property<vec3> cls = pset->get_vertex_property<vec3>("v:color");
	PointSet::Vertex_property<vec3> nms = pset->get_vertex_property<vec3>("v:normal");

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

	const std::vector<VertexGroup::Ptr>& groups = pset->groups();
	std::size_t num_groups = groups.size();
	output.write((char*)&num_groups, sizeof(int));

	for (std::size_t i = 0; i < num_groups; ++i) {
		VertexGroup* g = groups[i];
		write_binary_group(output, g);

		// children
		const std::vector<VertexGroup::Ptr>& children = g->children();
		std::size_t chld_num = children.size();
		output.write((char*)&chld_num, sizeof(int));
		for (std::size_t j = 0; j < chld_num; ++j) {
			VertexGroup* chld = children[j];
			write_binary_group(output, chld);
		}
	}
}


// for binary file format, no string stuff except labels. we add size info before each label
VertexGroup* PointSetSerializer_vg::read_binary_group(std::istream& input) {
	int type;
	input.read((char*)&type, sizeof(int));

	int num;
	input.read((char*)&num, sizeof(int));
	assert(num == num_group_parameters(static_cast<VertexGroupType>(type)));

	std::vector<float> para(num);
	input.read((char*)para.data(), num * sizeof(float));

	VertexGroup* grp = nil;
	switch (static_cast<VertexGroupType>(type))
	{
	case VG_PLANE:		grp = new VertexGroupPlane;		break;
	case VG_CYLINDER:	grp = new VertexGroupCylinder;	break;
	case VG_SPHERE:		grp = new VertexGroupSphere;	break;
	case VG_CONE:		grp = new VertexGroupCone;		break;
	case VG_TORUS:		grp = new VertexGroupTorus;		break;
	case VG_GENERAL:	grp = new VertexGroupGeneral;	break;
	default:	break;
	}
	assign_group_parameters(grp, para);

	////////////////////////////////////////////////////////////////////////// 
	int label_size = 0;
	input.read((char*)&label_size, sizeof(int));
	std::vector<char> label(label_size);
	input.read(label.data(), label_size);
	grp->set_label(std::string(label.begin(), label.end()));
	//////////////////////////////////////////////////////////////////////////

	vec3 color;
	input.read((char*)color.data(), 3 * sizeof(float));
	grp->set_color(color);

	int num_points = 0;
	input.read((char*)&num_points, sizeof(int));
	grp->resize(num_points);
	input.read((char*)grp->data(), num_points * sizeof(int));

	return grp;
}


// for binary file format, no string stuff except labels. we add size info before each label
void PointSetSerializer_vg::write_binary_group(std::ostream& output, VertexGroup* g) {
	int type = g->type();
	output.write((char*)&type, sizeof(int));

	int num = num_group_parameters(static_cast<VertexGroupType>(type));
	output.write((char*)&num, sizeof(int));

	const std::vector<float>& para = get_group_parameters(g);
	output.write((char*)para.data(), sizeof(float) * num);

	//////////////////////////////////////////////////////////////////////////
 	std::string label = g->label();
	int label_size = label.size();
 	output.write((char*)&label_size, sizeof(int));
 	output.write(label.data(), label_size);
	//////////////////////////////////////////////////////////////////////////

	const vec3& c = g->color();
	output.write((char*)c.data(), sizeof(float) * 3);

	std::size_t num_point = g->size();
	output.write((char*)&num_point, sizeof(int));
	output.write((char*)g->data(), num_point * sizeof(int));
}


int PointSetSerializer_vg::num_group_parameters(VertexGroupType type) {
	switch (type)
	{
	case VG_PLANE:
		return 4;
		break;
	case VG_CYLINDER:	
		return 4;
		Logger::warn(title()) << "not implemented for VG_CYLINDER" << std::endl;
		break;
	case VG_SPHERE:
		return 4;
		Logger::warn(title()) << "not implemented for VG_SPHERE" << std::endl;
		break;
	case VG_CONE:
		return 4;
		Logger::warn(title()) << "not implemented for VG_CONE" << std::endl;
		break;
	case VG_TORUS:
		return 4;
		Logger::warn(title()) << "not implemented for VG_TORUS" << std::endl;
		break;
	case VG_GENERAL:
		return 4;
		Logger::warn(title()) << "not implemented for VG_GENERAL" << std::endl;
		break;
	default:
		return 4;
        std::cerr << "unknown vertex group type" << std::endl;
		break;
	}
	return 4;
}


std::vector<float> PointSetSerializer_vg::get_group_parameters(VertexGroup* group) {
	int num = num_group_parameters(static_cast<VertexGroupType>(group->type()));
	std::vector<float> para(num);

	switch (group->type())
	{
	case VG_PLANE: {
		VertexGroupPlane* g = dynamic_cast<VertexGroupPlane*>(group);
		para[0] = static_cast<float>(g->plane().a());
		para[1] = static_cast<float>(g->plane().b());
		para[2] = static_cast<float>(g->plane().c());
		para[3] = static_cast<float>(g->plane().d());
		break;
	}
	case VG_CYLINDER:
		Logger::warn(title()) << "not implemented for VG_CYLINDER" << std::endl;
		break;
	case VG_SPHERE:
		Logger::warn(title()) << "not implemented for VG_SPHERE" << std::endl;
		break;
	case VG_CONE:
		Logger::warn(title()) << "not implemented for VG_CONE" << std::endl;
		break;
	case VG_TORUS:
		Logger::warn(title()) << "not implemented for VG_TORUS" << std::endl;
		break;
	case VG_GENERAL:
		Logger::warn(title()) << "not implemented for VG_GENERAL" << std::endl;
		break;
	default:
        std::cerr << "unknown vertex group type" << std::endl;
		break;
	}

	return para;
}


void PointSetSerializer_vg::assign_group_parameters(VertexGroup* group, std::vector<float>& para) {
	int num = num_group_parameters(static_cast<VertexGroupType>(group->type()));
	assert(para.size() == num);

	switch (group->type())
	{
	case VG_PLANE: {   
		VertexGroupPlane* g = dynamic_cast<VertexGroupPlane*>(group);
		g->set_plane(Plane3(para[0], para[1], para[2], para[3]));
		break;
	}
	case VG_CYLINDER:
		Logger::warn(title()) << "not implemented for VG_CYLINDER" << std::endl;
		break;
	case VG_SPHERE:
		Logger::warn(title()) << "not implemented for VG_SPHERE" << std::endl;
		break;
	case VG_CONE:
		Logger::warn(title()) << "not implemented for VG_CONE" << std::endl;
		break;
	case VG_TORUS:
		Logger::warn(title()) << "not implemented for VG_TORUS" << std::endl;
		break;
	case VG_GENERAL:
		Logger::warn(title()) << "not implemented for VG_GENERAL" << std::endl;
		break;
	default:
        std::cerr << "unknown vertex group type" << std::endl;
		break;
	}
}
