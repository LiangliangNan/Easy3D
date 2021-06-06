/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
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
 ********************************************************************/


#include <random>
#include <easy3d/util/logging.h>

int test_linear_solvers();
int test_spline();

int test_point_cloud();
int test_surface_mesh();
int test_polyhedral_mesh();
int test_graph();

int test_point_cloud_algorithms();
int test_surface_mesh_algorithms();

int test_viewer_imgui();
int test_composite_view();
int test_camera_interpolation();
int test_real_camera();
int test_drawables();
int test_imposters();
int test_scalar_field();
int test_vector_field();
int test_texture();
int test_image();
int test_tessellator();
int test_texture_mesh();
int test_text_rendering();
int test_text_mesher();
int test_animation();
int test_model_picker();
int test_face_picker();
int test_point_selection();
int test_ambient_occlusion();
int test_hard_shadow();
int test_soft_shadow();
int test_transparency();
int test_eye_dome_lighting();
int test_depth_maps();
        

int main(int argc, char* argv[]) {
    easy3d::logging::initialize();

    // Initialize random number generator.
    srand(0);

    int result = 0;

    result += test_linear_solvers();
    result += test_spline();

    result += test_point_cloud();
    result += test_surface_mesh();
    result += test_polyhedral_mesh();
    result += test_graph();

    result += test_point_cloud_algorithms();
    result += test_surface_mesh_algorithms();

    result += test_viewer_imgui();
    result += test_composite_view();
    result += test_real_camera();
    result += test_drawables();
    result += test_imposters();
    result += test_scalar_field();
    result += test_vector_field();
    result += test_texture();
    result += test_image();
    result += test_tessellator();
    result += test_texture_mesh();
    result += test_text_rendering();
    result += test_text_mesher();
    result += test_animation();
    result += test_ambient_occlusion();
    result += test_hard_shadow();
    result += test_soft_shadow();
    result += test_transparency();
    result += test_eye_dome_lighting();
    result += test_depth_maps();

    std::cout << "-------------------------------------------------------------------------\n"
                 "The following tests require user interaction. Please follow the usage of each application.\n"
                 "Close the application when you finish a test.\n"
                 "-------------------------------------------------------------------------\n";
    result += test_camera_interpolation();
    result += test_model_picker();
    result += test_face_picker();
    result += test_point_selection();

    return result;
}