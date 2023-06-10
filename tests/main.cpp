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

#include <random>
#include <iostream>

#include <easy3d/util/logging.h>
#include <easy3d/util/resource.h>

int test_timer();
int test_signal();
int test_console_style();

int test_linear_solvers();
int test_spline();

int test_point_cloud();
int test_surface_mesh();
int test_polyhedral_mesh();
int test_graph();
int test_kdtree();

int test_point_cloud_algorithms();
int test_surface_mesh_algorithms();

int test_viewer_imgui(int duration);
int test_composite_view(int duration);
int test_real_camera();
int test_drawables(int duration);
int test_imposters(int duration);
int test_scalar_field(int duration);
int test_vector_field(int duration);
int test_texture(int duration);
int test_image(int duration);
int test_tessellator(int duration);
int test_convex_partition(int duration);
int test_texture_mesh(int duration);
int test_text_rendering(int duration);
int test_text_mesher(int duration);
int test_animation(int duration);
int test_multithread();
int test_cross_section(int duration);
int test_ambient_occlusion(int duration);
int test_hard_shadow(int duration);
int test_soft_shadow(int duration);
int test_transparency(int duration);
int test_eye_dome_lighting(int duration);
int test_depth_maps(int duration);

int test_camera_interpolation();
int test_model_picker();
int test_face_picker();
int test_point_selection();


using namespace easy3d;

int main(int argc, char* argv[]) {
    logging::initialize(false, false, true);
    resource::initialize();

    // Initialize random number generator.
    srand(0);

    int result = 0;

    result += test_console_style();
    result += test_timer();
    result += test_signal();

    result += test_linear_solvers();
    result += test_spline();

    result += test_point_cloud();
    result += test_surface_mesh();
    result += test_polyhedral_mesh();
    result += test_graph();
    result += test_kdtree();

    result += test_point_cloud_algorithms();
    result += test_surface_mesh_algorithms();

    const int duration = 1500; // in millisecond
    result += test_viewer_imgui(duration);
    result += test_composite_view(duration);
    result += test_real_camera();
    result += test_drawables(duration);
    result += test_imposters(duration);
    result += test_scalar_field(duration);
    result += test_vector_field(duration);
    result += test_texture(duration);
    result += test_image(duration);
    result += test_tessellator(duration);
    result += test_convex_partition(duration);
    result += test_texture_mesh(duration);
    result += test_text_rendering(duration);
    result += test_text_mesher(duration);
    result += test_animation(duration);
    result += test_multithread();
    result += test_cross_section(duration);
    result += test_ambient_occlusion(duration);
    result += test_hard_shadow(duration);
    result += test_soft_shadow(duration);
    result += test_transparency(duration);
    result += test_eye_dome_lighting(duration);
    result += test_depth_maps(duration);

    std::cout << "-------------------------------------------------------------------------\n"
                 "The following tests require user interaction. Please follow the usage of each application.\n"
                 "Close the application when you finish a test.\n"
                 "-------------------------------------------------------------------------\n";
    result += test_camera_interpolation();
    result += test_model_picker();
    result += test_face_picker();
    result += test_point_selection();

    std::cout << "\n-------------------------------------------------------------------------\n";
    std::cout << "If you didn't see any error message and all visualization were correct, then all tests have passed." << std::endl;
    return result;
}