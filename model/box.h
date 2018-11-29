
#ifndef _MATH_BOX_H_
#define _MATH_BOX_H_

#include "vec.h"
#include <cassert>

template <class FT>
class GenericBox2 {
public:
	GenericBox2()
		: initialized_(false)
		, x_min_(1e30f), y_min_(1e30f)
		, x_max_(-1e30f), y_max_(-1e30f)
	{
	}

	bool initialized() const { return initialized_; }
	void clear() { initialized_ = false; }

	FT x_min() const { if (initialized_) return x_min_; else return 0; }
	FT y_min() const { if (initialized_) return y_min_; else return 0; }
	FT x_max() const { if (initialized_) return x_max_; else return 0; } 
	FT y_max() const { if (initialized_) return y_max_; else return 0; }

	FT min(unsigned axis) const { if (initialized_) return (axis == 0) ? x_min_ : y_min_; else return 0; }
	FT max(unsigned axis) const { if (initialized_) return (axis == 0) ? x_max_ : y_max_; else return 0; }

	FT x_range() const { if (initialized_) return x_max() - x_min(); else return 0; }
	FT y_range() const { if (initialized_) return y_max() - y_min(); else return 0; }

    FT area() const { if (initialized_) return x_range() * y_range(); else return 0; }

	Vec<2, FT> min() const { if (initialized_) return Vec<2, FT>(x_min_, y_min_); else return Vec<2, FT>(FT(0), FT(0)); }
	Vec<2, FT> max() const { if (initialized_) return Vec<2, FT>(x_max_, y_max_); else return Vec<2, FT>(FT(0), FT(0)); }
	
	Vec<2, FT> center() const { 
		if(initialized_)
			return Vec<2, FT>(FT(0.5) * (x_max() + x_min()), FT(0.5) * (y_max() + y_min())); 
		else 
			return Vec<2, FT>(FT(0), FT(0));
	}
	
	FT diagonal() const {
		if (initialized_)
			return ::sqrt(mpl_sqr(x_max() - x_min()) + mpl_sqr(y_max() - y_min()));
		else
			return FT(0);
	}

	void add_point(const Vec<2, FT>& p) {
		if (!initialized_) {
			x_min_ = p.x;
			y_min_ = p.y;
			x_max_ = p.x;
			y_max_ = p.y;
			initialized_ = true;
		}
		else {
			x_min_ = mpl_min(x_min_, p.x);
			y_min_ = mpl_min(y_min_, p.y);
			x_max_ = mpl_max(x_max_, p.x);
			y_max_ = mpl_max(y_max_, p.y);
		}
	}

	void add_box(const GenericBox2<FT>& b) {
		add_point(b.min());
		add_point(b.min());
	}

	GenericBox2<FT> operator+(const GenericBox2<FT>& b) const {
		GenericBox2<FT> result = *this;
		result += b;
		return result;
	}

	GenericBox2<FT>& operator+=(const GenericBox2<FT>& b) {
		add_point(b.min());
		add_point(b.max());
		return *this;
	}

private:
	bool initialized_;
	FT x_min_;
	FT y_min_;
	FT x_max_;
	FT y_max_;
};

//_________________________________________________________________________

template <class FT>
class GenericBox3 {
public:
	GenericBox3()
		: initialized_(false)
		, x_min_(1e30f), y_min_(1e30f), z_min_(1e30f)
		, x_max_(-1e30f), y_max_(-1e30f), z_max_(-1e30f)
	{
	}

	bool initialized() const { return initialized_; }
	void clear() { initialized_ = false; }

	FT x_min() const { if (initialized_) return x_min_; else return 0; }
	FT y_min() const { if (initialized_) return y_min_; else return 0; }
	FT z_min() const { if (initialized_) return z_min_; else return 0; }
	FT x_max() const { if (initialized_) return x_max_; else return 0; }
	FT y_max() const { if (initialized_) return y_max_; else return 0; }
	FT z_max() const { if (initialized_) return z_max_; else return 0; }

	FT min(unsigned axis) const { if (initialized_) return (axis == 0) ? x_min_ : ((axis == 1) ? y_min_ : z_min_); else return 0; }
	FT max(unsigned axis) const { if (initialized_) return (axis == 0) ? x_max_ : ((axis == 1) ? y_max_ : z_max_); else return 0; }

	FT x_range() const { if (initialized_) return x_max() - x_min(); else return 0; }
	FT y_range() const { if (initialized_) return y_max() - y_min(); else return 0; }
	FT z_range() const { if (initialized_) return z_max() - z_min(); else return 0; }

    FT area() const { if (initialized_) return FT(2.0) * (x_range() * y_range() + y_range() * z_range() + z_range() * x_range()); else return 0; }

	Vec<3, FT> min() const { if (initialized_)	return Vec<3, FT>(x_min_, y_min_, z_min_); else return Vec<3, FT>(FT(0), FT(0), FT(0)); }
	Vec<3, FT> max() const { if (initialized_)	return Vec<3, FT>(x_max_, y_max_, z_max_); else return Vec<3, FT>(FT(0), FT(0), FT(0)); }

	Vec<3, FT> center() const {
		if (initialized_)
			return Vec<3, FT>(
				FT(0.5) * (x_max() + x_min()),
				FT(0.5) * (y_max() + y_min()),
				FT(0.5) * (z_max() + z_min())
				);
		else 
			return Vec<3, FT>(FT(0), FT(0), FT(0));
	}

	FT diagonal() const {
		if (initialized_)
			return ::sqrt(mpl_sqr(x_max() - x_min()) + mpl_sqr(y_max() - y_min()) + mpl_sqr(z_max() - z_min()));
		else
			return FT(0);
	}

	void add_point(const Vec<3, FT>& p) {
		if (!initialized_) {
			x_min_ = p.x;
			y_min_ = p.y;
			z_min_ = p.z;
			x_max_ = p.x;
			y_max_ = p.y;
			z_max_ = p.z;
			initialized_ = true;
		}
		else {
			x_min_ = mpl_min(x_min_, p.x);
			y_min_ = mpl_min(y_min_, p.y);
			z_min_ = mpl_min(z_min_, p.z);
			x_max_ = mpl_max(x_max_, p.x);
			y_max_ = mpl_max(y_max_, p.y);
			z_max_ = mpl_max(z_max_, p.z);
		}
	}

	void add_box(const GenericBox3<FT>& b) {
		add_point(b.min());
		add_point(b.max());
	}

	GenericBox3<FT> operator+(const GenericBox3<FT>& b) const {
		GenericBox3<FT> result = *this;
		result += b;
		return result;
	}

	GenericBox3<FT>& operator+=(const GenericBox3<FT>& b) {
		add_point(b.min());
		add_point(b.max());
		return *this;
	}

private:
	bool initialized_;
	FT x_min_;
	FT y_min_;
	FT z_min_;
	FT x_max_;
	FT y_max_;
	FT z_max_;
};



#endif
