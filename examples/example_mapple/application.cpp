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


#include "application.h"


#include <cmath>

#include <GL/glew.h>
#include "opengl_error.h"
#include "camera.h"


Application::Application(
	const std::string& title, 
	int num_samples /* = 4 */,
	int gl_major /* = 3 */,
	int gl_minor /* = 2 */ 
) : Viewer(title, num_samples, gl_major, gl_minor)
{
}


Application::~Application()
{

}


void Application::init() {
	Viewer::init();

	const std::vector<vec3> points = { 
		vec3(-1.0f, -1.0f, 1.0f),
		vec3(1.0f, -1.0f, 1.0f),
		vec3(-1.0f, 1.0f, 1.0f),
		vec3(1.0f, 1.0f, 1.0f),
		vec3(-1.0f, -1.0f, -1.0f),
		vec3(1.0f, -1.0f, -1.0f),
		vec3(-1.0f, 1.0f, -1.0f),
		vec3(1.0f, 1.0f, -1.0f)
	};

	const std::vector<unsigned int> indices = {
		0, 1, 2, 2, 1, 3,   // f
		4, 6, 5, 5, 6, 7,   // b
		4, 0, 6, 6, 0, 2,   // l
		1, 5, 3, 3, 5, 7,   // r
		2, 3, 6, 6, 3, 7,   // t
		4, 5, 0, 0, 5, 1,   // b
	};

	surface_.update_vertex_buffer(points);
	surface_.update_index_buffer(indices);


	program_.load_shader_from_code(ShaderProgram::VERTEX,
		/* Vertex shader */
		"#version 330\n"
		"uniform mat4 mvp;\n"
		"in vec3 position;\n"
		"void main() {\n"
		"    gl_Position = mvp * vec4(position, 1.0);\n"
		"}"
	);

	program_.load_shader_from_code(ShaderProgram::FRAGMENT,
		/* Fragment shader */
		"#version 330\n"
		"out vec4 color;\n"
		"void main() {\n"
		"    color = vec4(0.4,0.8, 0.8, 1.0);\n"
		"}"
	);

	program_.set_attrib_name(ShaderProgram::POSITION, "position");
	program_.set_attrib_name(ShaderProgram::COLOR, "color");

	program_.link_program();

	camera_->setSceneBoundingBox(vec3(-1, -1, -1), vec3(1, 1, 1));
	camera_->showEntireScene();
}


void Application::draw() {
	
	program_.bind();						mpl_debug_gl_error;
	const mat4& mvp = camera_->modelViewProjectionMatrix();
	program_.set_uniform("mvp", mvp);		mpl_debug_gl_error;
	surface_.draw(false);					mpl_debug_gl_error;
	program_.unbind();						mpl_debug_gl_error;

}
