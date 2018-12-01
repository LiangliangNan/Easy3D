//=============================================================================
// Copyright (C) 2001-2005 by Computer Graphics Group, RWTH Aachen
// Copyright (C) 2011-2016 by Graphics & Geometry Group, Bielefeld University
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public License
// as published by the Free Software Foundation, version 2.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================


//== INCLUDES =================================================================

#include <easy3d/model/IO.h>

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
		Point_cloud::Vertex_property<Point> points = cloud.vertex_property<Point>("v:point");
		input.read((char*)points.data(), num * sizeof(Point));

		// read the colors block if exists
		input.read((char*)(&num), sizeof(int));
		if (num > 0) {
			Point_cloud::Vertex_property<Color> colors = cloud.vertex_property<Color>("v:color");
			input.read((char*)colors.data(), num * sizeof(Color));
		}

		// read the normals block if exists
		input.read((char*)(&num), sizeof(int));
		if (num > 0) {
			Point_cloud::Vertex_property<Normal> normals = cloud.vertex_property<Normal>("v:normal");
			input.read((char*)normals.data(), num * sizeof(Normal));
		}

		return true;
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
		Point_cloud::Vertex_property<Point> points = cloud.get_vertex_property<Point>("v:point");
		int num = (int)points.vector().size();
		output.write((char*)&num, sizeof(int));
		output.write((char*)points.data(), num * sizeof(Point));

		Point_cloud::Vertex_property<Color>  colors = cloud.get_vertex_property<Color>("v:color");
		num = (int)colors.vector().size();
		output.write((char*)&num, sizeof(int));
		if (num > 0)
			output.write((char*)colors.data(), num * sizeof(Color));

		Point_cloud::Vertex_property<Normal> normals = cloud.get_vertex_property<Normal>("v:normal");
		num = (int)normals.vector().size();
		output.write((char*)&num, sizeof(int));
		if (num > 0)
			output.write((char*)normals.data(), num * sizeof(Normal));

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
