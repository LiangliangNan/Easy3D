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


#ifndef EASY3D_GENERIC_BOX_H
#define EASY3D_GENERIC_BOX_H

#include <cassert>
#include <algorithm>

#include <easy3d/vec.h>


namespace easy3d {


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
                x_min_ = std::min(x_min_, p.x);
                y_min_ = std::min(y_min_, p.y);
                x_max_ = std::max(x_max_, p.x);
                y_max_ = std::max(y_max_, p.y);
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

        // defined by the diagonal corners
        GenericBox3(const Vec<3, FT>& pmin, const Vec<3, FT>& pmax)
            : initialized_(true)
        {
            x_min_ = pmin.x;  x_max_ = pmax.x;
            y_min_ = pmin.y;  y_max_ = pmax.y;
            z_min_ = pmin.z;  z_max_ = pmax.z;
        }


        // defined by center and radius
        GenericBox3(const Vec<3, FT>& c, FT r)
            : initialized_(true)
        {
            Vec<3, FT> dir(1, 1, 1);
            dir.normalize();
            const Vec<3, FT>& pmin = c - dir * r;
            const Vec<3, FT>& pmax = c + dir * r;
            x_min_ = pmin.x;  x_max_ = pmax.x;
            y_min_ = pmin.y;  y_max_ = pmax.y;
            z_min_ = pmin.z;  z_max_ = pmax.z;
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
            if (initialized_) {
                FT dx = x_max() - x_min();
                FT dy = y_max() - y_min();
                FT dz = z_max() - z_min();
                return ::sqrt(dx * dx + dy * dy + dz * dz);
            }
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
                x_min_ = std::min(x_min_, p.x);
                y_min_ = std::min(y_min_, p.y);
                z_min_ = std::min(z_min_, p.z);
                x_max_ = std::max(x_max_, p.x);
                y_max_ = std::max(y_max_, p.y);
                z_max_ = std::max(z_max_, p.z);
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

} // namespace easy3d


#endif  // EASY3D_GENERIC_BOX_H
