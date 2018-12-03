
#include "model.h"
#include "drawable.h"


namespace easy3d {

	Model::Model(const std::string& name /* = "no_name" */)
		: name_(name)
		, visible_(true)
		, bbox_known_(false)
	{
	}


	Model::~Model() {
		for (auto d : point_drawables_)	delete d;
		for (auto d : line_drawables_)		delete d;
		for (auto d : face_drawables_)		delete d;
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
				std::cerr << "point drawable \'" << name << "\' already exists" << std::endl;
				return d;
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