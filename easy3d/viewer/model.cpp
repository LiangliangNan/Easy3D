/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */

#include <easy3d/viewer/model.h>
#include <easy3d/viewer/drawable_points.h>
#include <easy3d/viewer/drawable_lines.h>
#include <easy3d/viewer/drawable_triangles.h>


namespace easy3d {

	Model::Model(const std::string& name /* = "unknown" */)
		: name_(name)
		, visible_(true)
		, bbox_known_(false)
	{
	}


	Model::~Model() {
		for (auto d : points_drawables_)	delete d;
		for (auto d : lines_drawables_)	delete d;
        for (auto d : triangles_drawables_)	delete d;
	}


	const Box3& Model::bounding_box() const {
        if (!bbox_known_) {
            Box3& box = const_cast<Model*>(this)->bbox_;
            box.clear();
            for (const auto& p : points())
                box.add_point(p);
        }

		return bbox_;
	}


	PointsDrawable* Model::points_drawable(const std::string& name) const {
		for (auto d : points_drawables_) {
			if (d->name() == name)
				return d;
		}
		return nullptr;
	}


	LinesDrawable* Model::lines_drawable(const std::string& name) const {
		for (auto d : lines_drawables_) {
			if (d->name() == name)
				return d;
		}
		return nullptr;
	}


    TrianglesDrawable* Model::triangles_drawable(const std::string& name) const {
        for (auto d : triangles_drawables_) {
			if (d->name() == name)
				return d;
		}
		return nullptr;
	}


	PointsDrawable* Model::add_points_drawable(const std::string& name) {
		for (auto d : points_drawables_) {
			if (d->name() == name) {
				LOG(ERROR) << "drawable already exists: " << name;
				return d;
			}
		}
		PointsDrawable* d = new PointsDrawable(name);
		d->set_model(this);
		points_drawables_.push_back(d);
		return d;
	}


	LinesDrawable* Model::add_lines_drawable(const std::string& name) {
		for (auto d : lines_drawables_) {
			if (d->name() == name) {
                LOG(ERROR) << "drawable already exists: " << name;
				return d;
			}
		}
		LinesDrawable* d = new LinesDrawable(name);
		d->set_model(this);
		lines_drawables_.push_back(d);
		return d;
	}


    TrianglesDrawable* Model::add_triangles_drawable(const std::string& name) {
        for (auto d : triangles_drawables_) {
			if (d->name() == name) {
                LOG(ERROR) << "drawable already exists: " << name;
				return d;
			}
		}
		TrianglesDrawable* d = new TrianglesDrawable(name);
		d->set_model(this);
        triangles_drawables_.push_back(d);
		return d;
	}

}
