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


/** ----------------------------------------------------------
 *
 * the code is adapted from Surface_mesh with modifications.
 *		- Surface_mesh (version 1.1)
 * The original code is available at
 * https://opensource.cit-ec.de/projects/surface_mesh
 *
 * Surface_mesh is a halfedge-based mesh data structure for
 * representing and processing 2-manifold polygonal surface
 * meshes. It is implemented in C++ and designed with an
 * emphasis on simplicity and efficiency.
 *
 *----------------------------------------------------------*/


#include <easy3d/fileio/surface_mesh_io.h>

#include <cstdio>
#include <cstring>

#include <easy3d/core/types.h>
#include <easy3d/core/surface_mesh.h>


namespace easy3d {


	namespace io {


		namespace details {

			// helper function
			template <typename T> size_t read(FILE* in, T& t)
			{
				size_t err = 0;
				err = fread(&t, 1, sizeof(t), in);
				return err;
			}

		} // namespace details

		//-----------------------------------------------------------------------------


		bool load_off_ascii(SurfaceMesh& mesh,
			FILE* in,
			const bool has_normals,
			const bool has_texcoords,
			const bool has_colors)
		{
			char                 line[200], *lp;
			int                  nc;
			unsigned int         i, j, items, idx;
			unsigned int         nV, nF, nE;
			vec3                p, n, c;
			vec2                t;
			SurfaceMesh::Vertex v;


			// properties
			SurfaceMesh::VertexProperty<vec3>              normals;
			SurfaceMesh::VertexProperty<vec2>  texcoords;
			SurfaceMesh::VertexProperty<vec3>               colors;
			if (has_normals)   normals = mesh.vertex_property<vec3>("v:normal");
			if (has_texcoords) texcoords = mesh.vertex_property<vec2>("v:texcoord");
			if (has_colors)    colors = mesh.vertex_property<vec3>("v:color");


			// #Vertice, #Faces, #Edges
			items = fscanf(in, "%d %d %d\n", (int*)&nV, (int*)&nF, (int*)&nE);
			mesh.clear();
			mesh.reserve(nV, std::max(3 * nV, nE), nF);


			// read vertices: pos [normal] [color] [texcoord]
			for (i = 0; i < nV && !feof(in); ++i)
			{
				// read line
				lp = fgets(line, 200, in);
				lp = line;

				// position
				items = sscanf(lp, "%f %f %f%n", &p[0], &p[1], &p[2], &nc);
				assert(items == 3);
				v = mesh.add_vertex(p);
				lp += nc;

				// normal
				if (has_normals)
				{
					if (sscanf(lp, "%f %f %f%n", &n[0], &n[1], &n[2], &nc) == 3)
					{
						normals[v] = n;
					}
					lp += nc;
				}

				// color
				if (has_colors)
				{
					if (sscanf(lp, "%f %f %f%n", &c[0], &c[1], &c[2], &nc) == 3)
					{
						if (c[0] > 1.0f || c[1] > 1.0f || c[2] > 1.0f) c *= (1.0 / 255.0);
						colors[v] = c;
					}
					lp += nc;
				}

				// tex coord
				if (has_texcoords)
				{
					items = sscanf(lp, "%f %f%n", &t[0], &t[1], &nc);
					assert(items == 2);
					texcoords[v][0] = t[0];
					texcoords[v][1] = t[1];
					lp += nc;
				}
			}



			// read faces: #N v[1] v[2] ... v[n-1]
			std::vector<SurfaceMesh::Vertex> vertices;
			for (i = 0; i < nF; ++i)
			{
				// read line
				lp = fgets(line, 200, in);
				lp = line;

				// #vertices
				items = sscanf(lp, "%d%n", (int*)&nV, &nc);
				assert(items == 1);
				vertices.resize(nV);
				lp += nc;

				// indices
				for (j = 0; j < nV; ++j)
				{
					items = sscanf(lp, "%d%n", (int*)&idx, &nc);
					assert(items == 1);
					vertices[j] = SurfaceMesh::Vertex(idx);
					lp += nc;
				}
				mesh.add_face(vertices);
			}


			return  mesh.n_faces() > 0;
		}


		//-----------------------------------------------------------------------------


		bool load_off_binary(SurfaceMesh& mesh,
			FILE* in,
			const bool has_normals,
			const bool has_texcoords,
			const bool has_colors)
		{
			unsigned int       i, j, idx;
			unsigned int       nV, nF, nE;
			vec3              p, n, c;
			vec2              t;
			SurfaceMesh::Vertex  v;


			// binary cannot (yet) read colors
			if (has_colors) return false;


			// properties
			SurfaceMesh::VertexProperty<vec3>              normals;
			SurfaceMesh::VertexProperty<vec2>  texcoords;
			if (has_normals)   normals = mesh.vertex_property<vec3>("v:normal");
			if (has_texcoords) texcoords = mesh.vertex_property<vec2>("v:texcoord");


			// #Vertice, #Faces, #Edges
			details::read(in, nV);
			details::read(in, nF);
			details::read(in, nE);
			mesh.clear();
			mesh.reserve(nV, std::max(3 * nV, nE), nF);


			// read vertices: pos [normal] [color] [texcoord]
			for (i = 0; i < nV && !feof(in); ++i)
			{
				// position
				details::read(in, p);
				v = mesh.add_vertex(p);

				// normal
				if (has_normals)
				{
					details::read(in, n);
					normals[v] = n;
				}

				// tex coord
				if (has_texcoords)
				{
					details::read(in, t);
					texcoords[v][0] = t[0];
					texcoords[v][1] = t[1];
				}
			}


			// read faces: #N v[1] v[2] ... v[n-1]
			std::vector<SurfaceMesh::Vertex> vertices;
			for (i = 0; i < nF; ++i)
			{
				details::read(in, nV);
				vertices.resize(nV);
				for (j = 0; j < nV; ++j)
				{
					details::read(in, idx);
					vertices[j] = SurfaceMesh::Vertex(idx);
				}
				mesh.add_face(vertices);
			}


			return  mesh.n_faces() > 0;
		}


		//-----------------------------------------------------------------------------


		bool load_off(const std::string& file_name, SurfaceMesh* mesh)
		{
			if (!mesh) {
				std::cerr << "null mesh pointer" << std::endl;
				return false;
			}

			char  line[200];
			bool  has_texcoords = false;
			bool  has_normals = false;
			bool  has_colors = false;
			bool  has_hcoords = false;
			bool  has_dim = false;
			bool  is_binary = false;

			// open file (in ASCII mode)
			FILE* in = fopen(file_name.c_str(), "r");
            if (!in) {
                std::cerr << "could not open file \'" << file_name << "\'" << std::endl;
                return false;
            }

			// read header: [ST][C][N][4][n]OFF BINARY
			char *c = fgets(line, 200, in);
			assert(c != nullptr);
			c = line;
			if (c[0] == 'S' && c[1] == 'T') { has_texcoords = true; c += 2; }
			if (c[0] == 'C') { has_colors = true; ++c; }
			if (c[0] == 'N') { has_normals = true; ++c; }
			if (c[0] == '4') { has_hcoords = true; ++c; }
			if (c[0] == 'n') { has_dim = true; ++c; }
			if (strncmp(c, "OFF", 3) != 0) { fclose(in); return false; } // no OFF
			if (strncmp(c + 4, "BINARY", 6) == 0) is_binary = true;


			// homogeneous coords, and vertex dimension != 3 are not supported
			if (has_hcoords || has_dim)
			{
				fclose(in);
				return false;
			}


			// if binary: reopen file in binary mode
			if (is_binary)
			{
				fclose(in);
				in = fopen(file_name.c_str(), "rb");
				c = fgets(line, 200, in);
				assert(c != nullptr);
			}


			// read as ASCII or binary
			bool ok = (is_binary ?
				load_off_binary(*mesh, in, has_normals, has_texcoords, has_colors) :
				load_off_ascii(*mesh, in, has_normals, has_texcoords, has_colors));


			fclose(in);
			return ok && mesh->n_faces() > 0;
		}


		//-----------------------------------------------------------------------------


		bool save_off(const std::string& file_name, const SurfaceMesh* mesh)
		{
			if (!mesh) {
				std::cerr << "null mesh pointer" << std::endl;
				return false;
			}

			FILE* out = fopen(file_name.c_str(), "w");
            if (!out) {
                std::cerr << "could not open file \'" << file_name << "\'" << std::endl;
				return false;
            }

			bool  has_normals = false;
			bool  has_texcoords = false;
			bool  has_colors = false;
			SurfaceMesh::VertexProperty<vec3> normals = mesh->get_vertex_property<vec3>("v:normal");
			SurfaceMesh::VertexProperty<vec2>  texcoords = mesh->get_vertex_property<vec2>("v:texcoord");
			SurfaceMesh::VertexProperty<vec3> colors = mesh->get_vertex_property<vec3>("v:color");
			if (normals)   has_normals = true;
			if (texcoords) has_texcoords = true;
			if (colors) has_colors = true;

			// header
			if (has_texcoords)
				fprintf(out, "ST");
			if (has_colors)
				fprintf(out, "C");
			if (has_normals)
				fprintf(out, "N");
			fprintf(out, "OFF\n%d %d 0\n", mesh->n_vertices(), mesh->n_faces());

			// vertices, and optionally normals and texture coordinates
			SurfaceMesh::VertexProperty<vec3> points = mesh->get_vertex_property<vec3>("v:point");
			for (SurfaceMesh::VertexIterator vit = mesh->vertices_begin(); vit != mesh->vertices_end(); ++vit)
			{
				const vec3& p = points[*vit];
				fprintf(out, "%.10f %.10f %.10f", p[0], p[1], p[2]);

				if (has_normals)
				{
					const vec3& n = normals[*vit];
					fprintf(out, " %.10f %.10f %.10f", n[0], n[1], n[2]);
				}

				if (has_colors)
				{
					const vec3& c = colors[*vit];
					fprintf(out, " %.10f %.10f %.10f", c[0], c[1], c[2]);
				}

				if (has_texcoords)
				{
					const vec2& t = texcoords[*vit];
					fprintf(out, " %.10f %.10f", t[0], t[1]);
				}

				fprintf(out, "\n");
			}


			// faces
			for (SurfaceMesh::FaceIterator fit = mesh->faces_begin(); fit != mesh->faces_end(); ++fit)
			{
				int nV = mesh->valence(*fit);
				fprintf(out, "%d", nV);
				SurfaceMesh::VertexAroundFaceCirculator fvit = mesh->vertices(*fit), fvend = fvit;
				do
				{
					fprintf(out, " %d", (*fvit).idx());
				} while (++fvit != fvend);
				fprintf(out, "\n");
			}

			fclose(out);
			return true;
		}

	} // namespace io

} // namespace easy3d
