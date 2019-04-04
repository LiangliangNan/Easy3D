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

#include "real_camera.h"
#include <easy3d/point_cloud.h>
#include <easy3d/drawable.h>
#include <easy3d/camera.h>

#include <3rd_party/glfw/include/GLFW/glfw3.h>	// for the KEYs


using namespace easy3d;

RealCamera::RealCamera(const std::string& title) : Viewer(title) {
    set_background_color(vec3(1.0f, 1.0f, 1.0f));

    // Read the point cloud from a known file.
    const std::string file = "../../../Easy3D/data/real_camera/point_cloud.bin";
    if (!open(file)) {
        std::cerr << "Error: failed load point cloud. Please make sure the file exists and format is correct." << std::endl;
        abort();
    }

    auto drawable = current_model()->points_drawable("points");
    drawable->set_point_size(5.0f);

    std::cout << "------------ Real Camera ----------" << std::endl
              << "Press keys 1, 2, 3 to switch views" << std::endl;
}


bool RealCamera::key_press_event(int key, int modifiers) {
    if (key == GLFW_KEY_1) {
        std::cout << "view 1 " << std::endl;
        CameraPara cam;
        cam.w = 1920;
        cam.h = 1080;
        cam.fx = 1842.2077240f;
        cam.fy = 1838.9055970f;
        cam.u0 = 975.13697300f;
        cam.v0 = 546.13760400f;
        cam.rx = 0.1707014584f;
        cam.ry = 0.5250153953f;
        cam.rz = -0.009298784f;
        cam.tx = 0.1360685179f;
        cam.ty = -0.278497233f;
        cam.tz = 2.4276577107f;
        change_view(cam);
        return true;
    }
    else if (key == GLFW_KEY_2) {
        std::cout << "view 2 " << std::endl;
        CameraPara cam;
        cam.w = 1920;
        cam.h = 1080;
        cam.fx = 1955.9917480f;
        cam.fy = 1952.7915110f;
        cam.u0 = 975.07971700f;
        cam.v0 = 539.05811400f;
        cam.rx = 0.0070340829f;
        cam.ry = -0.148287175f;
        cam.rz = 0.0360668739f;
        cam.tx = -0.007660835f;
        cam.ty = 0.1060953556f;
        cam.tz = 2.3937444743f;
        change_view(cam);
        return true;
    }
    else if (key == GLFW_KEY_3) {
        std::cout << "view 3 " << std::endl;
        CameraPara cam;
        cam.w = 1920;
        cam.h = 1080;
        cam.fx = 1834.4864850f;
        cam.fy = 1839.7799250f;
        cam.u0 = 953.54416900f;
        cam.v0 = 535.75893000f;
        cam.rx = 0.0611976305f;
        cam.ry = -1.695009600f;
        cam.rz = 0.0292804834f;
        cam.tx = 0.7386870949f;
        cam.ty = -0.113801253f;
        cam.tz = 4.7867648132f;
        change_view(cam);
        return true;
    }
    else
        return Viewer::key_press_event(key, modifiers);
}


void RealCamera::change_view(const CameraPara &cam) {
    // https://stackoverflow.com/questions/12933284/rodrigues-into-eulerangles-and-vice-versa/36506782
    const vec3 rvec(cam.rx, cam.ry, cam.rz);
    float angle = rvec.length();
    quat q(rvec / angle, angle);
    camera()->setOrientation(quat(q[3], q[2], q[1], q[0]));
    camera()->setPosition(vec3(cam.tx, cam.ty, cam.tz));
    float fov = 2.0 * atan(1.0 / (cam.fy / cam.v0));
    camera()->setFieldOfView(fov);
    resize(cam.w, cam.h);
    update();
}
