/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work,
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan.
*           Easy3D: a lightweight, easy-to-use, and efficient C++
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
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

#include "camera_interpolation.h"
#include <easy3d/viewer/frame.h>
#include <easy3d/viewer/camera.h>

#include <easy3d/viewer/manipulated_camera_frame.h>

#include <3rd_party/glfw/include/GLFW/glfw3.h>	// for the KEYs


using namespace easy3d;


CameraIntrepolation::CameraIntrepolation(const std::string& title)
    : Viewer(title, 4, 3, 2, false, false)
    , kfi_(camera())
{
    std::cout << "------------ Camera Intrepolation ----------" << std::endl
              << "Press 'K' to add key frames" << std::endl
              << "Press 'Space' to start/stop the animation" << std::endl;
    kfi_.setFrame(camera()->frame());
}


bool CameraIntrepolation::key_press_event(int key, int modifiers)
{
    if (key == GLFW_KEY_K) {
        easy3d::Frame* f = camera()->frame();
        easy3d::Frame* key_frame = new Frame(*f);
        kfi_.addKeyFrame(key_frame);
        return true;
    }
    else if (key == GLFW_KEY_SPACE) {
        if (kfi_.interpolationIsStarted())
            kfi_.stopInterpolation();
        else
            kfi_.startInterpolation();
        return true;
    }
    else if (key == GLFW_KEY_D) {
        kfi_.deletePath();
        std::cout << "path deleted"<< std::endl;
        return true;
    }
    else
        return Viewer::key_press_event(key, modifiers);
}




//void CameraIntrepolation::create_cameras_drawable(float scale)
//{
//    const double halfHeight = scale * 0.07;
//    const double halfWidth = halfHeight * 1.3;
//    const double dist = halfHeight / tan(double(M_PI) / 8.0);
    
//    const double arrowHeight = 1.5 * halfHeight;
//    const double baseHeight = 1.2 * halfHeight;
//    const double arrowHalfWidth = 0.5 * halfWidth;
//    const double baseHalfWidth = 0.3 * halfWidth;
    
//    std::vector<vec3> points;
    
//    //--------------
//    // Frustum outline
//    //--------------
    
//    // LINE_STRIP
//    const vec3 p0(-halfWidth, halfHeight, -dist);
//    const vec3 p1(-halfWidth, -halfHeight, -dist);    points.push_back(p0); points.push_back(p1);
//    const vec3 p2(0.0, 0.0, 0.0);                    points.push_back(p1); points.push_back(p2);
//    const vec3 p3(halfWidth, -halfHeight, -dist);    points.push_back(p2); points.push_back(p3);
//    const vec3 p4(-halfWidth, -halfHeight, -dist);    points.push_back(p3); points.push_back(p4);
    
//    // LINE_STRIP
//    const vec3 q0(halfWidth, -halfHeight, -dist);
//    const vec3 q1(halfWidth, halfHeight, -dist);
//    const vec3 q2(0.0, 0.0, 0.0);
//    const vec3 q3(-halfWidth, halfHeight, -dist);
//    const vec3 q4(halfWidth, halfHeight, -dist);
//    points.push_back(q0); points.push_back(q1);
//    points.push_back(q1); points.push_back(q2);
//    points.push_back(q2); points.push_back(q3);
//    points.push_back(q3); points.push_back(q4);
    
//    //------------------
//    // Up arrow
//    //------------------
    
//    // Base - QUAD
//    const vec3 r0(-baseHalfWidth, halfHeight, -dist);
//    const vec3 r1(baseHalfWidth, halfHeight, -dist);
//    const vec3 r2(baseHalfWidth, baseHeight, -dist);
//    const vec3 r3(-baseHalfWidth, baseHeight, -dist);
//    points.push_back(r0); points.push_back(r1);
//    points.push_back(r1); points.push_back(r2);
//    points.push_back(r2); points.push_back(r3);
//    points.push_back(r3); points.push_back(r0);
    
//    // Arrow - TRIANGLE
//    const vec3 a0(0.0, arrowHeight, -dist);
//    const vec3 a1(-arrowHalfWidth, baseHeight, -dist);
//    const vec3 a2(arrowHalfWidth, baseHeight, -dist);
//    points.push_back(a0); points.push_back(a1);
//    points.push_back(a1); points.push_back(a2);
//    points.push_back(a2); points.push_back(a0);
    
//    std::vector<vec3> vertices;
//    for (int i = 0; i < views_.size(); ++i) {
//        Camera c;
//        KRT_to_camera(i, 1, &c);
//        const mat4& m = c.frame()->worldMatrix();
//        for (auto& p : points) {
//            vertices.push_back(m * p);
//        }
//    }
//    LinesDrawable* d = current_model()->lines_drawable("cameras");
//    if (!d)
//        d = current_model()->add_lines_drawable("cameras");
//    d->update_vertex_buffer(vertices);
//    d->set_default_color(vec3(0, 0, 1));
//}


void CameraIntrepolation::draw() const {
    Viewer::draw();

    const_cast<KeyFrameInterpolator*>(&kfi_)->drawPath();

    // draw the path here
}
