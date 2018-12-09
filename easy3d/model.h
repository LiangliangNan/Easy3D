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

#ifndef _EASY3D_OPENGL_MODEL_H_
#define _EASY3D_OPENGL_MODEL_H_


#include <string>
#include <vector>

#include <easy3d/math_types.h>

/********************************************************************************************
*
* A Model is the base class for Mesh, PointSet, etc. 
* A Model can have multiple drawable object, e.g., surface, vertices, wire frame, etc. 
* A Model can update the display if needed.
*
*	Version:	 1.1
*	created:	 Dec. 03, 2018
*	author:		 Liangliang Nan
*	contact:     liangliang.nan@gmail.com
*
*********************************************************************************************/

namespace easy3d {

	class PointsDrawable;
	class LinesDrawable;
	class FacesDrawable;

	class Model
	{
	public:
		Model(const std::string& name = "no_name");
		virtual ~Model();

		const std::string& name() const { return name_; }
		void set_name(const std::string& n) { name_ = n; }

		const Box3& bounding_box() const;
		void set_bounding_box(const Box3& box);

		/// returns number of vertices in the model
		virtual unsigned int n_vertices() const = 0;

		//-------------------- rendering  -----------------------

		bool is_visible() const { return visible_; }
		void set_visible(bool b) { visible_ = b; }

		// Get the drawable named 'name'.
		// Return NULL if not exist.
		PointsDrawable* point_drawable(const std::string& name) const;
		LinesDrawable*  line_drawable(const std::string& name) const;
		FacesDrawable*  face_drawable(const std::string& name) const;

		// Create a drawable named 'name'.
		// Ignore the creation if a drawable named 'name' already exist.
		PointsDrawable* add_point_drawable(const std::string& name);
		LinesDrawable*  add_line_drawable(const std::string& name);
		FacesDrawable*  add_face_drawable(const std::string& name);

		const std::vector<PointsDrawable*>  point_drawables() { return point_drawables_; }
		const std::vector<LinesDrawable*>   line_drawables() { return line_drawables_; }
		const std::vector<FacesDrawable*>   face_drawables() { return face_drawables_; }

	protected:
		std::string	name_;
		bool		visible_;
		Box3		bbox_;
		bool		bbox_known_;

		std::vector<PointsDrawable*>  point_drawables_;
		std::vector<LinesDrawable*>   line_drawables_;
		std::vector<FacesDrawable*>   face_drawables_;
	};
}

#endif