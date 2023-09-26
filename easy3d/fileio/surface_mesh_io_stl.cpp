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

#include <easy3d/fileio/surface_mesh_io.h>

#include <cstdio>
#include <cstring>
#include <map>
#include <fstream>
#include <cfloat>

#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/surface_mesh_builder.h>
#include <easy3d/util/logging.h>


namespace easy3d {

    /// TODO: Translator not implemented

    // \cond
	namespace io {

		// helper function
		template <typename T> void read(FILE* in, T& t)
		{
			auto n_items = fread((char*)&t, 1, sizeof(t), in);
			assert(n_items > 0);
		}


		//-----------------------------------------------------------------------------


		// helper class for STL reader
		class CompareVec
		{
		public:

			explicit CompareVec(float _eps = FLT_MIN) : eps_(_eps) {}

			bool operator()(const vec3& v0, const vec3& v1) const
			{
				if (fabs(v0[0] - v1[0]) <= eps_)
				{
					if (fabs(v0[1] - v1[1]) <= eps_)
					{
						return (v0[2] < v1[2] - eps_);
					}
					else return (v0[1] < v1[1] - eps_);
				}
				else return (v0[0] < v1[0] - eps_);
			}

		private:
			float eps_;
		};


		//-----------------------------------------------------------------------------


		bool load_stl(const std::string& file_name, SurfaceMesh* mesh)
		{
			if (!mesh) {
                LOG(ERROR) << "null mesh pointer";
				return false;
			}

            char line[100], *c;
            unsigned int i, nT;
            vec3 p;
            SurfaceMesh::Vertex v;
            std::vector<SurfaceMesh::Vertex> vertices(3);

            CompareVec comp(FLT_MIN);
			std::map<vec3, SurfaceMesh::Vertex, CompareVec>            vMap(comp);
			std::map<vec3, SurfaceMesh::Vertex, CompareVec>::iterator  vMapIt;

			// clear mesh
			mesh->clear();

            SurfaceMeshBuilder builder(mesh);
            builder.begin_surface();

			// open file (in ASCII mode)
			FILE* in = fopen(file_name.c_str(), "r");
            if (!in) {
                LOG(ERROR) << "could not open file: " << file_name;
                return false;
            }

			// determine if the file is a binary STL file
			auto is_binary = [](const std::string& file) -> bool {
				// assume it's binary stl, then file size is known from #triangles.
				// if size matches, it's really binary
				std::ifstream input(file.c_str(), std::fstream::binary);
				if (input.fail()) {
					LOG(ERROR) << "could not open file: " << file;
					return false;
				}

				// skip header
				input.seekg(80, std::ifstream::beg);
				if (input.fail()) {
					LOG(ERROR) << "file size is less than 80 bytes";
					return false;
				}

				// read number of triangles
				std::int32_t nTriangles = 0;
				input.read((char*)&nTriangles, 4);

				// compute file size from nTriangles
				size_t needed_size = 84 + nTriangles * 50; // 50 bytes per triangle face (4*12+2 bytes)

				// get actual file size
				input.seekg(0, std::fstream::beg);
				size_t begin_pos = input.tellg();
				input.seekg(0, std::fstream::end);
				size_t end_pos = input.tellg();
				size_t file_size = end_pos - begin_pos;
				input.close();

				// if sizes match, it is indeed binary format
				if (needed_size == file_size)
					return true;

				// NOTE: many people may forget the last two bytes, so let's make it more tolerant
				if (needed_size == (file_size + 2)) {
					LOG(ERROR) << "number of triangles in STL file does not match file size. Bytes needed: " << needed_size
						<< ", available: " << file_size << ". Trying to open it as STL binary file...";
					return true;
				}

				return false;
			};


			// parse binary STL
			if (is_binary(file_name))
			{
				// re-open file in binary mode
				fclose(in);
				in = fopen(file_name.c_str(), "rb");
				if (!in) 
					return false;

				// skip dummy header
				auto n_items = fread(line, 1, 80, in);
				assert(n_items > 0);

				// read number of triangles
				read(in, nT);

				// read triangles
				while (nT)
				{
					// skip triangle normal
					n_items = fread(line, 1, 12, in);
					assert(n_items > 0);
					// triangle's vertices
					for (i = 0; i < 3; ++i)
					{
						read(in, p);

						// has vector been referenced before?
						if ((vMapIt = vMap.find(p)) == vMap.end())
						{
							// No : add vertex and remember idx/vector mapping
							v = builder.add_vertex(p);
							vertices[i] = v;
							vMap[p] = v;
						}
						else
						{
							// Yes : get index from map
							vertices[i] = vMapIt->second;
						}
					}

					// Add face only if it is not degenerated
					if ((vertices[0] != vertices[1]) &&
						(vertices[0] != vertices[2]) &&
						(vertices[1] != vertices[2]))
						builder.add_face(vertices);

					n_items = fread(line, 1, 2, in);
					assert(n_items > 0);
					--nT;
				}
			}


			// parse ASCII STL
			else
			{
				// parse line by line
				while (!feof(in) && fgets(line, 100, in))
				{
					// skip white-space
					for (c = line; isspace(*c) && *c != '\0'; ++c) {}

					// face begins
					if ((strncmp(c, "outer", 5) == 0) ||
						(strncmp(c, "OUTER", 5) == 0))
					{
						// read three vertices
						for (i = 0; i < 3; ++i)
						{
							// read line
							c = fgets(line, 100, in);
							assert(c != nullptr);

							// skip white-space
							for (c = line; isspace(*c) && *c != '\0'; ++c) {}

							// read x, y, z
							sscanf(c + 6, "%f %f %f", &p[0], &p[1], &p[2]);

							// has vector been referenced before?
							if ((vMapIt = vMap.find(p)) == vMap.end())
							{
								// No : add vertex and remember idx/vector mapping
								v = builder.add_vertex(p);
								vertices[i] = v;
								vMap[p] = v;
							}
							else
							{
								// Yes : get index from map
								vertices[i] = vMapIt->second;
							}
						}

						// Add face only if it is not degenerated
						if ((vertices[0] != vertices[1]) &&
							(vertices[0] != vertices[2]) &&
							(vertices[1] != vertices[2]))
							builder.add_face(vertices);
					}
				}
			}

			fclose(in);

            builder.end_surface();
			return mesh->n_faces() > 0;
		}


		//-----------------------------------------------------------------------------

        // currently we save in ASCII format
		bool save_stl(const std::string& file_name, const SurfaceMesh* mesh)
		{
			if (!mesh) {
				LOG(ERROR) << "null mesh pointer";
				return false;
			}

			if (!mesh->is_triangle_mesh())
			{
                LOG(ERROR) << "write_stl: not a triangle mesh!";
				return false;
			}

			auto fnormals = mesh->get_face_property<vec3>("f:normal");
			if (!fnormals)
			{
				fnormals = const_cast<SurfaceMesh*>(mesh)->add_face_property<vec3>("f:normal");
				for (auto f : mesh->faces())
					fnormals[f] = mesh->compute_face_normal(f);
			}

			std::ofstream ofs(file_name.c_str());
            if (ofs.fail()) {
                LOG(ERROR) << "could not open file: " << file_name;
                return false;
            }

			auto points = mesh->get_vertex_property<vec3>("v:point");

			ofs << "solid stl" << std::endl;
			vec3 n;
			vec3 p;

			for (auto f : mesh->faces())
			{
				n = fnormals[f];
				ofs << "  facet normal ";
				ofs << n[0] << " " << n[1] << " " << n[2] << std::endl;
				ofs << "    outer loop" << std::endl;
				for (auto v : mesh->vertices(f))
				{
					p = points[v];
					ofs << "      vertex ";
					ofs << p[0] << " " << p[1] << " " << p[2] << std::endl;
				}
				ofs << "    endloop" << std::endl;
				ofs << "  endfacet" << std::endl;
			}
			ofs << "endsolid" << std::endl;
			ofs.close();
			return true;
		}

	} // namespace io
    // \endcond

} // namespace easy3d

