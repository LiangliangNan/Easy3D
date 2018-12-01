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

#include <GL/glew.h>
#include <easy3d/core/opengl_error.h>
#include <easy3d/core/camera.h>
#include <easy3d/core/drawable.h>
#include <easy3d/core/shader_program.h>


using namespace easy3d;

Mapple::Mapple(
	int num_samples /* = 4 */,
	int gl_major /* = 3 */,
	int gl_minor /* = 2 */ 
) : Viewer("Mapple", num_samples, gl_major, gl_minor)
{
}


void Mapple::init() {
    Viewer::init();		mpl_debug_gl_error;

	const std::vector<vec3> points = {
		vec3(-1.0f, -1.0f, 1.0f),	// 0
		vec3(1.0f, -1.0f, 1.0f),	// 1
		vec3(-1.0f, 1.0f, 1.0f),	// 2
		vec3(1.0f, 1.0f, 1.0f),		// 3
		vec3(-1.0f, -1.0f, -1.0f),	// 4
		vec3(1.0f, -1.0f, -1.0f),	// 5
		vec3(-1.0f, 1.0f, -1.0f),	// 6
		vec3(1.0f, 1.0f, -1.0f)		// 7
	};

	const std::vector<unsigned int> indices_surface = {
		0, 1, 2, 2, 1, 3,   // f
		4, 6, 5, 5, 6, 7,   // b
		4, 0, 6, 6, 0, 2,   // l
		1, 5, 3, 3, 5, 7,   // r
		2, 3, 6, 6, 3, 7,   // t
		4, 5, 0, 0, 5, 1,   // b
	};

	const std::vector<unsigned int> indices_wireframe = {
		0,1,2,3,4,5,6,7,
		0,2,4,6,1,3,5,7,
		0,4,2,6,1,5,3,7
	};


	surface_ = new FacesDrawable;
	surface_->update_vertex_buffer(points);
	surface_->update_index_buffer(indices_surface);

	wireframe_ = new LinesDrawable;
	wireframe_->update_vertex_buffer(points);
	wireframe_->update_index_buffer(indices_wireframe);

	vertices_ = new PointsDrawable;
	vertices_->update_vertex_buffer(points);

	program_ = new ShaderProgram;
	program_->load_shader_from_code(ShaderProgram::VERTEX,
		/* Vertex shader */
		"#version 330\n"
		"uniform mat4 mvp;\n"
		"in vec3 position;\n"
		"void main() {\n"
		"    gl_Position = mvp * vec4(position, 1.0);\n"
		"}"
	);

	program_->load_shader_from_code(ShaderProgram::FRAGMENT,
		/* Fragment shader */
		"#version 330\n"
		"uniform vec3 inColor;\n"
		"out vec4 color;\n"
		"void main() {\n"
		"    color = vec4(inColor, 1.0);\n"
		"}"
	);

	program_->set_attrib_name(ShaderProgram::POSITION, "position");
	program_->link_program();

	camera_->setSceneBoundingBox(vec3(-1, -1, -1), vec3(1, 1, 1));
	camera_->showEntireScene();
}



void Mapple::cleanup() {
	delete program_;
	delete surface_;
	delete wireframe_;
	delete vertices_;
    Viewer::cleanup();
}


void Mapple::draw() const {
	//glEnable(GL_VERTEX_PROGRAM_POINT_SIZE); // starting from GL3.2, using GL_PROGRAM_POINT_SIZE
    glPointSize(20);						mpl_debug_gl_error;

	program_->bind();						mpl_debug_gl_error;
	const mat4& mvp = camera_->modelViewProjectionMatrix();
	program_->set_uniform("mvp", mvp);		mpl_debug_gl_error;

	program_->set_uniform("inColor", vec3(0.4f, 0.8f, 0.8f));		mpl_debug_gl_error;
	surface_->draw(false);					mpl_debug_gl_error;

	program_->set_uniform("inColor", vec3(0.0f, 0.0f, 1.0f));
	wireframe_->draw(false);					mpl_debug_gl_error;

	program_->set_uniform("inColor", vec3(1.0f, 0.0f, 0.0f));
	vertices_->draw(false);					mpl_debug_gl_error;

	program_->unbind();						mpl_debug_gl_error;

	Viewer::draw();

}
