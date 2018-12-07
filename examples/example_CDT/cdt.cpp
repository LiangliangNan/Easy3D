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


#include "cdt.h"

#include <fstream>

#include <3rd_party/glew/include/GL/glew.h>
#include <easy3d/core/opengl_error.h>
#include <easy3d/core/camera.h>
#include <easy3d/core/drawable.h>
#include <easy3d/core/shader_program.h>


using namespace easy3d;

Mapple::Mapple(
	int num_samples /* = 4 */,
	int gl_major /* = 3 */,
	int gl_minor /* = 2 */ 
) : MainWindow("Mapple", num_samples, gl_major, gl_minor)
{
	program_ = nullptr;
	input_vertices_ = nullptr;
	input_segments_ = nullptr;
	CDT_segments_ = nullptr;
}



void Mapple::init() {
	MainWindow::init();		mpl_debug_gl_error;

	const std::string dir = "C:/Users/liangliang/Dropbox/Students/Nikos/weekly progress/20181120/";
	
	//const std::string file_name = dir + "2ndCrossSectionProblem_0.lines";
	const std::string file_name = dir + "2ndCrossSectionProblem_1.lines";
	//const std::string file_name = dir + "2ndCrossSectionProblem_2.lines";

	std::vector<vec3> points;
	load_crosssection(file_name, points);
	compute_CDT(points);
}


void Mapple::cleanup() {
	if (input_segments_) delete input_segments_;
	if (input_vertices_) delete input_vertices_;
	if (program_)  delete program_;
	MainWindow::cleanup();
}


void Mapple::load_crosssection(const std::string& file_name, std::vector<vec3>& points)
{
	std::ifstream input(file_name.c_str());
	if (input.fail()) {
		std::cerr << "could not open file \'" << file_name << "\'" << std::endl;
		return;
	}

	std::string dummy;
	int num_lines = 0;
	input >> dummy >> num_lines;
	Box3 box;
	for (int i = 0; i < num_lines; ++i) {
		vec3 s, t;
		input >> s >> t;
		points.push_back(s);
		points.push_back(t);
		box.add_point(s);
		box.add_point(t);
	}

	input_segments_ = new LinesDrawable("input_segments_");
	input_segments_->update_vertex_buffer(points);
	input_vertices_ = new PointsDrawable("input_vertices_");
	input_vertices_->update_vertex_buffer(points);

	camera_->setSceneBoundingBox(box.min(), box.max());
	camera_->showEntireScene();

	program_ = new ShaderProgram;
	program_->load_shader_from_code(ShaderProgram::VERTEX,
		"#version 330\n"
		"uniform mat4 mvp;\n"
		"in vec3 position;\n"
		"void main() {\n"
		"    gl_Position = mvp * vec4(position, 1.0);\n"
		"}"
	);

	program_->load_shader_from_code(ShaderProgram::FRAGMENT,
		"#version 330\n"
		"uniform vec3 inColor;\n"
		"out vec4 color;\n"
		"void main() {\n"
		"    color = vec4(inColor, 1.0);\n"
		"}"
	);

	program_->set_attrib_name(ShaderProgram::POSITION, "position");
	program_->link_program();

}


void Mapple::draw() {
	MainWindow::draw();

	if (!program_)
		return;

	glPointSize(point_size_);		

	program_->bind();					
	const mat4& mvp = camera_->modelViewProjectionMatrix();
	program_->set_uniform("mvp", mvp);	

	if (input_segments_) {
		program_->set_uniform("inColor", vec3(0.4f, 0.8f, 0.8f));
		input_segments_->draw(false);
	}

	if (input_vertices_) {
		program_->set_uniform("inColor", vec3(1.0f, 0.0f, 0.0f));
		input_vertices_->draw(false);
	}

	if (CDT_segments_) {
		program_->set_uniform("inColor", vec3(0.0f, 1.0f, 0.0f));
		CDT_segments_->draw(false);
	}

	program_->unbind();					

}


#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Exact_predicates_tag                               Itag;
typedef CGAL::Constrained_Delaunay_triangulation_2<K, CGAL::Default, Itag> CDT;
typedef CDT::Point          Point;


void Mapple::compute_CDT(const std::vector<vec3>& points) {
	if (points.empty())
		return;

	const float z = points[0].z;

	CDT cdt;
	for (std::size_t i=0;i< points.size(); i+=2) {
		const vec3& s = points[i];
		const vec3& t = points[i + 1];
		cdt.insert_constraint(Point(s.x, s.y), Point(t.x, t.y));
	}

	assert(cdt.is_valid());

	std::vector<vec3> cdt_points;
	CDT::Finite_edges_iterator eit = cdt.finite_edges_begin();
	for (; eit != cdt.finite_edges_end(); ++eit) {
		if (!cdt.is_constrained(*eit))
		{
			const Point& s = eit->first->vertex(eit->first->cw(eit->second))->point();
			const Point& t = eit->first->vertex(eit->first->ccw(eit->second))->point();
			vec3 ss(s.x(), s.y(), z);
			vec3 tt(t.x(), t.y(), z);
			cdt_points.push_back(ss);
			cdt_points.push_back(tt);
		}
	}

	//std::ofstream output("C:/tmp/2.cdt");
	//output << cdt;

	CDT_segments_ = new LinesDrawable("CDT_segments_");
	CDT_segments_->update_vertex_buffer(cdt_points);
}
