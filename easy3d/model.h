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

#ifndef EASY3D_MODEL_H
#define EASY3D_MODEL_H


#include <string>
#include <vector>

#include <easy3d/types.h>

/********************************************************************************************
*
* A Model is the base class for 3D models, e.g., SurfaceMesh, PointCloud. 
* A Model can have multiple drawables, e.g., faces, vertices, wireframe. 
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
		PointsDrawable* points_drawable(const std::string& name) const;
		LinesDrawable*  lines_drawable(const std::string& name) const;
		FacesDrawable*  faces_drawable(const std::string& name) const;

		// Create a drawable named 'name'.
		// Ignore the creation if a drawable named 'name' already exist.
		PointsDrawable* add_points_drawable(const std::string& name);
		LinesDrawable*  add_lines_drawable(const std::string& name);
		FacesDrawable*  add_faces_drawable(const std::string& name);

		const std::vector<PointsDrawable*>  points_drawables() { return points_drawables_; }
		const std::vector<LinesDrawable*>   lines_drawables() { return lines_drawables_; }
		const std::vector<FacesDrawable*>   faces_drawables() { return faces_drawables_; }

	protected:
		std::string	name_;
		bool		visible_;
		Box3		bbox_;
		bool		bbox_known_;

		std::vector<PointsDrawable*>  points_drawables_;
		std::vector<LinesDrawable*>   lines_drawables_;
		std::vector<FacesDrawable*>   faces_drawables_;
	};
}

#endif  // EASY3D_MODEL_H
