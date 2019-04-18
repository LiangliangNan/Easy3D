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


#ifndef EASY3D_FILEIO_POINT_CLOUD_IO_VG_H
#define EASY3D_FILEIO_POINT_CLOUD_IO_VG_H

#include <string>
#include <vector>

class PointSet;
class VertexGroup;


class PointSetSerializer_vg
{
public:
	static std::string title() { return "PointSetSerializer_vg"; }

	// labeled vertex groups
	static void load_vg(const std::string& file_name, PointSet* pset);
	static void save_vg(const std::string& file_name, const PointSet* pset);

	static void load_bvg(const std::string& file_name, PointSet* pset);
	static void save_bvg(const std::string& file_name, const PointSet* pset);

private:
	static VertexGroup* read_ascii_group(std::istream& input);
	static void write_ascii_group(std::ostream& output, VertexGroup* g);

	static VertexGroup* read_binary_group(std::istream& input);
	static void write_binary_group(std::ostream& output, VertexGroup* g);

	static int num_group_parameters(VertexGroupType type);

	static std::vector<float> get_group_parameters(VertexGroup* g);
	static void assign_group_parameters(VertexGroup* g, std::vector<float>& para);
};

#endif  // EASY3D_FILEIO_POINT_CLOUD_IO_VG_H

