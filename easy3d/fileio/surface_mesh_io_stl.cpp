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

#include <cstdio>
#include <cstring>
#include <map>
#include <fstream>
#include <cfloat>

#include <easy3d/core/surface_mesh.h>


namespace easy3d {

	namespace io {


		// helper function
		template <typename T> void read(FILE* in, T& t)
		{
			size_t n_items(0);
			n_items = fread((char*)&t, 1, sizeof(t), in);
			assert(n_items > 0);
		}


		//-----------------------------------------------------------------------------


		// helper class for STL reader
		class CmpVec
		{
		public:

			CmpVec(float _eps = FLT_MIN) : eps_(_eps) {}

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
				std::cerr << "null mesh pointer" << std::endl;
				return false;
			}

			char                            line[100], *c;
			unsigned int                    i, nT;
			vec3                           p;
			SurfaceMesh::Vertex               v;
			std::vector<SurfaceMesh::Vertex>  vertices(3);
			size_t n_items(0);

			CmpVec comp(FLT_MIN);
			std::map<vec3, SurfaceMesh::Vertex, CmpVec>            vMap(comp);
			std::map<vec3, SurfaceMesh::Vertex, CmpVec>::iterator  vMapIt;


			// clear mesh
			mesh->clear();


			// open file (in ASCII mode)
			FILE* in = fopen(file_name.c_str(), "r");
            if (!in) {
                std::cerr << "could not open file \'" << file_name << "\'" << std::endl;
                return false;
            }


			// ASCII or binary STL?
			c = fgets(line, 6, in);
			assert(c != nullptr);
			const bool binary = ((strncmp(line, "SOLID", 5) != 0) &&
				(strncmp(line, "solid", 5) != 0));


			// parse binary STL
			if (binary)
			{
				// re-open file in binary mode
				fclose(in);
				in = fopen(file_name.c_str(), "rb");
				if (!in) return false;

				// skip dummy header
				n_items = fread(line, 1, 80, in);
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
							v = mesh->add_vertex(p);
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
						mesh->add_face(vertices);

					n_items = fread(line, 1, 2, in);
					assert(n_items > 0);
					--nT;
				}
			}


			// parse ASCII STL
			else
			{
				// parse line by line
				while (in && !feof(in) && fgets(line, 100, in))
				{
					// skip white-space
					for (c = line; isspace(*c) && *c != '\0'; ++c) {};

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
							for (c = line; isspace(*c) && *c != '\0'; ++c) {};

							// read x, y, z
							sscanf(c + 6, "%f %f %f", &p[0], &p[1], &p[2]);

							// has vector been referenced before?
							if ((vMapIt = vMap.find(p)) == vMap.end())
							{
								// No : add vertex and remember idx/vector mapping
								v = mesh->add_vertex(p);
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
							mesh->add_face(vertices);
					}
				}
			}


			fclose(in);
			return mesh->n_faces() > 0;
		}


		//-----------------------------------------------------------------------------

        // currently we save in ASCII format
		bool save_stl(const std::string& file_name, const SurfaceMesh* mesh)
		{
			if (!mesh) {
				std::cerr << "null mesh pointer" << std::endl;
				return false;
			}

			if (!mesh->is_triangle_mesh())
			{
				std::cerr << "write_stl: not a triangle mesh!" << std::endl;
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
                std::cerr << "could not open file \'" << file_name << "\'" << std::endl;
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

} // namespace easy3d

