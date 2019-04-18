/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work, 
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan. 
*           Easy3D: a lightweight, easy-to-use, and efficient C++ 
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
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


#include <easy3d/fileio/surface_mesh_io.h>

#include <clocale>

#include <easy3d/core/surface_mesh.h>
#include <easy3d/util/file.h>
#include <easy3d/util/stop_watch.h>


namespace easy3d {


	SurfaceMesh* SurfaceMeshIO::load(const std::string& file_name)
	{
		std::setlocale(LC_NUMERIC, "C");

        const std::string& ext = file::extension(file_name, true);

		SurfaceMesh* mesh = new SurfaceMesh;
		mesh->set_name(file_name);

		StopWatch w;

		bool success = false;
        if (ext == "ply")
            success = io::load_ply(file_name, mesh);
        else if (ext == "poly")
            success = io::load_poly(file_name, mesh);
        else if (ext == "obj")
			success = io::load_obj(file_name, mesh);
		else if (ext == "off")
			success = io::load_off(file_name, mesh);
		else if (ext == "stl")
			success = io::load_stl(file_name, mesh);

		//	else if (ext == "plg")
		//        serializer = new MeshSerializer_plg();
		//	else if (ext == "geojson")
		//        serializer = new MeshSerializer_json();
		else {
			std::cerr << "unknown file format: " << ext << std::endl;
			success = false;
		}

		if (!success || mesh->n_faces() == 0) {
			delete mesh;
			return nullptr;
		}


#ifndef NDEBUG
		std::cout << "vertex properties on mesh " << file::base_name(file_name) << std::endl;
		const auto& vnames = mesh->vertex_properties();
		for (const auto& n : vnames)
			std::cout << "\t" << n << std::endl;

		std::cout << "face properties on mesh " << file::base_name(file_name) << std::endl;
		const auto& fnames = mesh->face_properties();
		for (const auto& n : fnames)
			std::cout << "\t" << n << std::endl;

		std::cout << "edge properties on mesh " << file::base_name(file_name) << std::endl;
		const auto& enames = mesh->edge_properties();
		for (const auto& n : enames)
			std::cout << "\t" << n << std::endl;
#endif

		std::cout << "load model done. time: " << w.time_string() << std::endl;

		return mesh;
	}


	bool SurfaceMeshIO::save(const std::string& file_name, const SurfaceMesh* mesh)
	{
		if (!mesh) {
			std::cerr << "surface mesh is null" << std::endl;
			return false;
		}

		std::string ext = file::extension(file_name, true);

		StopWatch w;

		bool success = false;
		if (ext == "ply")
			success = io::save_ply(file_name, mesh, true);
        else if (ext == "poly")
            success = io::save_poly(file_name, mesh);
        else if (ext == "obj")
            success = io::save_obj(file_name, mesh);
        else if (ext == "off")
			success = io::save_off(file_name, mesh);
		else if (ext == "stl")
			success = io::save_stl(file_name, mesh);

		//	else if (ext == "plg")
		//        serializer = new MeshSerializer_plg();
		//	else if (ext == "geojson")
		//        serializer = new MeshSerializer_json();
		else {
			std::cerr << "unknown file format: " << ext << std::endl;
			success = false;
		}

		std::cout << "save model done. time: " << w.time_string() << std::endl;

		return success;
	}


} // namespace easy3d
