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



#include <easy3d/model.h>
#include <easy3d/drawable.h>


namespace easy3d {

	Model::Model(const std::string& name /* = "no_name" */)
		: name_(name)
		, visible_(true)
		, bbox_known_(false)
	{
	}


	Model::~Model() {
		for (auto d : point_drawables_)	delete d;
		for (auto d : line_drawables_)	delete d;
		for (auto d : face_drawables_)	delete d;
	}


	const Box3& Model::bounding_box() const {
		if (!bbox_known_)
			std::cerr << "bounding box not set" << std::endl;
		return bbox_;
	}


	void Model::set_bounding_box(const Box3& box) { 
		bbox_ = box; 
		bbox_known_ = true;
	}


	PointsDrawable* Model::point_drawable(const std::string& name) const {
		for (auto d : point_drawables_) {
			if (d->name() == name)
				return d;
		}
		return nullptr;
	}


	LinesDrawable* Model::line_drawable(const std::string& name) const {
		for (auto d : line_drawables_) {
			if (d->name() == name)
				return d;
		}
		return nullptr;
	}


	FacesDrawable* Model::face_drawable(const std::string& name) const {
		for (auto d : face_drawables_) {
			if (d->name() == name)
				return d;
		}
		return nullptr;
	}


	PointsDrawable* Model::add_point_drawable(const std::string& name) {
		for (auto d : point_drawables_) {
			if (d->name() == name) {
				std::cerr << "Error: point drawable \'" << name << "\' already exists" << std::endl;
				return nullptr;
			}
		}
		PointsDrawable* d = new PointsDrawable(name);
		point_drawables_.push_back(d);
		return d;
	}


	LinesDrawable* Model::add_line_drawable(const std::string& name) {
		for (auto d : line_drawables_) {
			if (d->name() == name) {
				std::cerr << "line drawable \'" << name << "\' already exists" << std::endl;
				return d;
			}
		}
		LinesDrawable* d = new LinesDrawable(name);
		line_drawables_.push_back(d);
		return d;
	}


	FacesDrawable* Model::add_face_drawable(const std::string& name) {
		for (auto d : face_drawables_) {
			if (d->name() == name) {
				std::cerr << "face drawable \'" << name << "\' already exists" << std::endl;
				return d;
			}
		}
		FacesDrawable* d = new FacesDrawable(name);
		face_drawables_.push_back(d);
		return d;
	}

}
