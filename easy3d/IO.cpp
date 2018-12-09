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


/** ----------------------------------------------------------
 *
 * the code is adapted from Surface_mesh with modifications.
 *		- Surface_mesh (version 1.1)
 * The orignal code is available at
 * https://opensource.cit-ec.de/projects/surface_mesh
 *
 * Surface_mesh is a halfedge-based mesh data structure for
 * representing and processing 2-manifold polygonal surface
 * meshes. It is implemented in C++ and designed with an
 * emphasis on simplicity and efficiency.
 *----------------------------------------------------------*/


//== INCLUDES =================================================================

#include <easy3d/IO.h>

#include <clocale>
#include <fstream>


//== NAMESPACE ================================================================


namespace easy3d {


	//== IMPLEMENTATION ===========================================================

	bool read_cloud(Point_cloud& cloud, const std::string& filename)
	{
		std::setlocale(LC_NUMERIC, "C");

		// clear cloud before reading from file
		cloud.clear();

		// extract file extension
		std::string::size_type dot(filename.rfind("."));
		if (dot == std::string::npos) return false;
		std::string ext = filename.substr(dot + 1, filename.length() - dot - 1);
		std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
		if (ext != "bin") {
			std::cerr << "currently only \'bin\' format is supported" << std::endl;
			return false;
		}

		std::ifstream input(filename.c_str(), std::fstream::binary);
		if (input.fail()) {
			std::cerr << "could not open file\'" << filename << "\'" << std::endl;
			return false;
		}

		int num = 0;
		input.read((char*)(&num), sizeof(int));
		if (num <= 0) {
			std::cerr << "no point exists in file\'" << filename << "\'" << std::endl;
			return false;
		}
		cloud.resize(num);

		// read the points block
		Point_cloud::Vertex_property<vec3> points = cloud.vertex_property<vec3>("v:point");
		input.read((char*)points.data(), num * sizeof(vec3));

		// read the colors block if exists
		input.read((char*)(&num), sizeof(int));
		if (num > 0) {
			Point_cloud::Vertex_property<vec3> colors = cloud.vertex_property<vec3>("v:color");
			input.read((char*)colors.data(), num * sizeof(vec3));
		}

		// read the normals block if exists
		input.read((char*)(&num), sizeof(int));
		if (num > 0) {
			Point_cloud::Vertex_property<vec3> normals = cloud.vertex_property<vec3>("v:normal");
			input.read((char*)normals.data(), num * sizeof(vec3));
		}

		return cloud.n_vertices() > 0;
	}


	//-----------------------------------------------------------------------------


	bool write_cloud(const Point_cloud& cloud, const std::string& filename)
	{
		// extract file extension
		std::string::size_type dot(filename.rfind("."));
		if (dot == std::string::npos) return false;
		std::string ext = filename.substr(dot + 1, filename.length() - dot - 1);
		std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
		if (ext != "bin") {
			std::cerr << "currently only \'bin\' format is supported" << std::endl;
			return false;
		}

		// open file
		std::ofstream output(filename.c_str(), std::fstream::binary);
		if (output.fail()) {
			std::cerr << "could not open file\'" << filename << "\'" << std::endl;
			return false;
		}

		// write the points block
		Point_cloud::Vertex_property<vec3> points = cloud.get_vertex_property<vec3>("v:point");
		int num = (int)points.vector().size();
		output.write((char*)&num, sizeof(int));
		output.write((char*)points.data(), num * sizeof(vec3));

		Point_cloud::Vertex_property<vec3>  colors = cloud.get_vertex_property<vec3>("v:color");
		num = (int)colors.vector().size();
		output.write((char*)&num, sizeof(int));
		if (num > 0)
			output.write((char*)colors.data(), num * sizeof(vec3));

		Point_cloud::Vertex_property<vec3> normals = cloud.get_vertex_property<vec3>("v:normal");
		num = (int)normals.vector().size();
		output.write((char*)&num, sizeof(int));
		if (num > 0)
			output.write((char*)normals.data(), num * sizeof(vec3));

		// we didn't find a writer module
		return false;
	}


//== IMPLEMENTATION ===========================================================


bool read_mesh(Surface_mesh& mesh, const std::string& filename)
{
    std::setlocale(LC_NUMERIC, "C");

    // clear mesh before reading from file
    mesh.clear();

    // extract file extension
    std::string::size_type dot(filename.rfind("."));
    if (dot == std::string::npos) return false;
    std::string ext = filename.substr(dot+1, filename.length()-dot-1);
    std::transform(ext.begin(), ext.end(), ext.begin(), tolower);

    // extension determines reader
    if (ext == "off")
    {
        return read_off(mesh, filename);
    }
    else if (ext == "obj")
    {
        return read_obj(mesh, filename);
    }
    else if (ext == "stl")
    {
        return read_stl(mesh, filename);
    }
    else if (ext == "poly")
    {
        return read_poly(mesh, filename);
    }

    // we didn't find a reader module
    return false;
}


//-----------------------------------------------------------------------------


bool write_mesh(const Surface_mesh& mesh, const std::string& filename)
{
    // extract file extension
    std::string::size_type dot(filename.rfind("."));
    if (dot == std::string::npos) return false;
    std::string ext = filename.substr(dot+1, filename.length()-dot-1);
    std::transform(ext.begin(), ext.end(), ext.begin(), tolower);


    // extension determines reader
    if (ext == "off")
    {
        return write_off(mesh, filename);
    }
    else if (ext=="obj")
    {
        return write_obj(mesh, filename);
    }
    else if (ext=="poly")
    {
        return write_poly(mesh, filename);
    }
    else if (ext=="stl")
    {
        return write_stl(mesh, filename);
    }

    // we didn't find a writer module
    return false;
}


//=============================================================================
} // namespace easy3d
//=============================================================================
