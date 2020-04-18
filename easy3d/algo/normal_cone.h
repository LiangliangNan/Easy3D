//=============================================================================
// Copyright (C) 2011-2019 The pmp-library developers
//
// This file is part of the Polygon Mesh Processing Library.
// Distributed under a MIT-style license, see LICENSE.txt for details.
//
// SPDX-License-Identifier: MIT-with-employer-disclaimer
//=============================================================================
#pragma once
//=============================================================================

#include <easy3d/core/types.h>

#include <cmath>

//=============================================================================

namespace easy3d {

//=============================================================================

//! \addtogroup algorithms algorithms
//!@{

//! A class implementing a normal cone.
class NormalCone
{
public:
    //! default constructor (not initialized)
    NormalCone() {}

    //! Initialize cone with center (unit vector) and angle (radius in radians)
    NormalCone(const vec3& normal, float angle = 0.0)
        : center_normal_(normal), angle_(angle)
    {
    }

    //! returns center normal
    const vec3& center_normal() const { return center_normal_; }

    //! returns size of cone (radius in radians)
    float angle() const { return angle_; }

    //! merge *this with n.
    NormalCone& merge(const vec3& n) { return merge(NormalCone(n)); }

    //! merge *this with nc. *this will then enclose both cones.
    NormalCone& merge(const NormalCone& nc)
    {
        const float dp = dot(center_normal_, nc.center_normal_);

        // axes point in same direction
        if (dp > 0.99999)
        {
            angle_ = std::max(angle_, nc.angle_);
        }

        // axes point in opposite directions
        else if (dp < -0.99999)
        {
            angle_ = 2 * M_PI;
        }

        else
        {
            // new angle
            float center_angle = acos(dp);
            float min_angle = std::min(-angle_, center_angle - nc.angle_);
            float max_angle = std::max(angle_, center_angle + nc.angle_);
            angle_ = 0.5 * (max_angle - min_angle);

            // axis by SLERP
            float axis_angle = 0.5 * (min_angle + max_angle);
            center_normal_ = ((center_normal_ * sin(center_angle - axis_angle) +
                               nc.center_normal_ * sin(axis_angle)) /
                              sin(center_angle));
        }

        return *this;
    }

private:
    vec3 center_normal_;
    float angle_;
};

//=============================================================================
//!@}
//=============================================================================
} // namespace easy3d
//=============================================================================
