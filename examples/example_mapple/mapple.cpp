/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of EasyGUI: software for processing and rendering
*   meshes and point clouds.
*
*	EasyGUI is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	EasyGUI is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#include "mapple.h"

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
	vertices_ = nullptr;
	segments_ = nullptr;
}


#include <fstream>
void Mapple::init() {
	MainWindow::init();		mpl_debug_gl_error;

	const std::string dir = "C:/Users/liang/Dropbox/Students/Nikos/weekly progress/20181120/";
	const std::string file = dir + "2ndCrossSectionProblem_1.lines";
	std::ifstream input(file.c_str());
	if (input.fail()) {
		std::cerr << "could not open file \'" << file << "\'" << std::endl;
		return;
	}


	std::string dummy;
	int num_lines = 0;
	input >> dummy >> num_lines;
	std::vector<vec3> points;
	Box3 box;
	for (int i=0; i< num_lines; ++i) {
		vec3 s, t;
		input >> s >> t;
		points.push_back(s);
		points.push_back(t);
		box.add_point(s);
		box.add_point(t);
	}

	segments_ = new LinesDrawable("cross_section");
	segments_->update_vertex_buffer(points);

	vertices_ = new PointsDrawable("vertices");
	vertices_->update_vertex_buffer(points);

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

	camera_->setSceneBoundingBox(box.min(), box.max());
	camera_->showEntireScene();

	glEnable(GL_LINE_SMOOTH);
}



void Mapple::cleanup() {
	if (segments_) delete segments_;
	if (vertices_) delete vertices_;
	if (program_)  delete program_;
	MainWindow::cleanup();
}


void Mapple::draw() {
	MainWindow::draw();

	glPointSize(point_size_);		

	program_->bind();					
	const mat4& mvp = camera_->modelViewProjectionMatrix();
	program_->set_uniform("mvp", mvp);	

	program_->set_uniform("inColor", vec3(0.4f, 0.8f, 0.8f));	
	segments_->draw(false);				

	program_->set_uniform("inColor", vec3(1.0f, 0.0f, 0.0f));
	vertices_->draw(false);					mpl_debug_gl_error;

	program_->unbind();						mpl_debug_gl_error;

}
