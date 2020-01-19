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

#ifndef EASY_TRANSFORM_DECOMPOSE_H
#define EASY_TRANSFORM_DECOMPOSE_H


#include <easy3d/core/types.h>


// Defines functions that decompose a transformation matrix into its original components.

// If you want to work with real cameras, see "Extrinsic/Intrinsic Decomposition"
// The Perspective Camera - An Interactive Tour:
//			http://ksimek.github.io/2012/08/13/introduction/
// Dissecting the Camera Matrix, Part 1: Extrinsic/Intrinsic Decomposition:
//			http://ksimek.github.io/2012/08/14/decompose/
// Dissecting the Camera Matrix, Part 2: The Extrinsic Matrix: 
//			http://ksimek.github.io/2012/08/22/extrinsic/
//		Calibrated Cameras in OpenGL without glFrustum:	
//			http://ksimek.github.io/2013/06/03/calibrated_cameras_in_opengl/
//		Calibrated Cameras and gluPerspective:			
//			http://ksimek.github.io/2013/06/18/calibrated-cameras-and-gluperspective/
// Dissecting the Camera Matrix, Part 3: The Intrinsic Matrix:					
// 			http://ksimek.github.io/2013/08/13/intrinsic/  
//
// Some related articles:
// https://sightations.wordpress.com/2010/08/03/simulating-calibrated-cameras-in-opengl/

namespace easy3d {

    namespace transform {

        /* @brief Decompose a transformation matrix into its original components
        *  @param M is the input transformation matrix
        *  @param scaling receives the output scaling for the x, y, z axes
        *  @param rotation receives the output rotation
        *  @param translation receives the output translation for the x, y, z axes
        * NOTE: this function can not handle skew and perspective transformation. See the overloaded function below
        * TODO: add functions that extract single components, i.e.,
        *      -) Quat extract_rotation(const mat4& M);
        *      -) vec3 extract_scale(const mat4& M);
        *      -) vec3 extract_translation(const mat4& M);
        */
        void decompose(const mat4& M, vec3& scaling, mat3& rotation, vec3& translation);
        void decompose(const mat4& M, vec3& scaling, quat& rotation, vec3& translation);

        /* @brief Decompose a transformation matrix with no scaling into its original components
        *  @param rotation receives the output rotation
        *  @param translation receives the output translation for the x, y, z axes
        * NOTE: this function can not handle skew and perspective transformation. See the overloaded function below
        * TODO: add functions that extract single components, i.e.,
        *      -) Quat  extract_rotation(const mat4& M);
        *      -) vec3  extract_translation(const mat4& M);
        */
        void decompose_no_scaling(const mat4& M, mat3& rotation, vec3& translation);
        void decompose_no_scaling(const mat4& M, quat& rotation, vec3& translation);

        // Decomposes a transformation matrix into to its original components(i.e., scaling, rotation, translation, skew and perspective)
        bool decompose(const mat4& M, vec3& scaling, quat& rotation, vec3& translation, vec3& skew, vec4& persp);

    }

}



#endif  // EASY_TRANSFORM_DECOMPOSE_H
