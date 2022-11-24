/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
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
 ********************************************************************/

#include <easy3d/core/model.h>


namespace easy3d {

    Model::Model(const std::string& name /* = "unknown" */)
            : name_(name)
            , bbox_known_(false)
            , renderer_(nullptr)
            , manipulator_(nullptr)
    {
    }


    const Box3& Model::bounding_box(bool recompute) const {
        if (!bbox_known_ || recompute) {
            Box3& box = const_cast<Model*>(this)->bbox_;
            box.clear();
            for (const auto& p : points())
                box.grow(p);

            if (box.is_valid())
                const_cast<Model*>(this)->bbox_known_ = true;
            else
                LOG(WARNING) << "model has no valid geometry";
        }
        return bbox_;
    }


    void Model::invalidate_bounding_box() {
        bbox_.clear();
        bbox_known_ = false;
    }

}
