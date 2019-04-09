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

#include <fstream>
#include <sstream>

#include <easy3d/point_cloud.h>
#include <easy3d/drawable.h>
#include <easy3d/camera.h>
#include <easy3d/manipulated_camera_frame.h>

#include <3rd_party/glfw/include/GLFW/glfw3.h>	// for the KEYs


using namespace easy3d;


#define USE_BUNDLER_DATA

RealCamera::RealCamera(const std::string& title) : Viewer(title) {
    set_background_color(vec3(0.8f, 0.8f, 0.8f));


#ifdef USE_BUNDLER_DATA

	// Read the point cloud from a known file.
	const std::string cloud_file = "../../../Easy3D/data/fountain/pointcloud.bin";
	if (open(cloud_file)) {
		auto drawable = current_model()->points_drawable("points");
		drawable->set_per_vertex_color(true);
		drawable->set_point_size(5.0f);
	}
	else
		std::cerr << "Error: failed load point cloud. Please make sure the file exists and format is correct." << std::endl;

	// Read the camera parameters from the bundler file.
	const std::string bundler_file = "../../../Easy3D/data/fountain/bundle.out";
	if (!read_bundler_file(bundler_file)) {
		std::cerr << "Error: failed load bundler file." << std::endl;
	}

#else
	// Read the point cloud from a known file.
	const std::string cloud_file = "../../../Easy3D-Mapple/data/real_camera/pointcloud.bin";
	if (open(cloud_file)) {
		auto drawable = current_model()->points_drawable("vertices");
		drawable->set_point_size(5.0f);
	}
	else
		std::cerr << "Error: failed load point cloud. Please make sure the file exists and format is correct." << std::endl;

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
    cam.tz = 2.4276577107f; views_.push_back(cam);
    
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
    cam.tz = 2.3937444743f; views_.push_back(cam);
    
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
    cam.tz = 4.7867648132f;  views_.push_back(cam);
#endif
    
    current_view_ = 0;
	create_cameras_drawable();

    std::cout << "------------ Real Camera ----------" << std::endl
              << "Press keys 1, 2, 3 to switch views" << std::endl;
}


bool RealCamera::key_press_event(int key, int modifiers) {
    if (key == GLFW_KEY_1) {
        change_view(current_view_);
        return true;
    }
    else if (key == GLFW_KEY_2) {
        change_view(current_view_);
        return true;
    }
    else if (key == GLFW_KEY_3) {
        change_view(current_view_);
        return true;
    }
    
    else if (key == GLFW_KEY_SPACE) {
        current_view_ = (current_view_ + 1) % views_.size();
        change_view(current_view_);
        return true;
    }
   
    else
        return Viewer::key_press_event(key, modifiers);
}


#include <easy3d/transform.h>
bool RealCamera::KRT_to_camera(int view_index, Camera* c) {
	if (view_index < 0 || view_index >= views_.size()) {
		std::cerr << "Error: invalid view index (" << view_index << ")" << std::endl;
		return false;
	}

    std::cout << "----- view " << view_index << " ------" << std::endl;
	const CameraPara& cam = views_[view_index];

    const float w = cam.w;
    const float h = cam.h;
    const float fx = cam.fx;
    const float fy = cam.fy;
    const float cx = cam.u0;
    const float cy = cam.v0;
    const float skew = 0.0;
	const float n = camera()->zNear();
	const float f = camera()->zFar();

#ifdef USE_BUNDLER_DATA
        const mat3 K(
            fx, skew, cx,
            0,  fy,   cy,
            0,  0,    1
		);

        const mat4 R = mat4::rotation(vec3(cam.rx, cam.ry, cam.rz));
        const mat4 T = mat4::translation(cam.tx, cam.ty, cam.tz);

        mat34 M(1.0);
        M(1, 1) = -1;// invert the y axis
        M(2, 2) = -1;// invert the z axis

        const mat34& P = K * M * T * R;
        c->setFromProjectionMatrix(P);
#else
        // https://stackoverflow.com/questions/12933284/rodrigues-into-eulerangles-and-vice-versa/36506782
        const vec3 rvec(cam.rx, cam.ry, cam.rz);
        const float len = rvec.length();
        const quat q(rvec / len, len);
        c->setOrientation(quat(q[3], q[2], q[1], q[0]));
		c->setPosition(vec3(cam.tx, cam.ty, cam.tz));
        //float fov = 2.0f * atan(1.0f / (fy / cam.v0));
        float fov = 2.0f * atan(1.0f / (2.0 * fx / h));
		c->setFieldOfView(fov);
#endif
   
	return true;
}


void RealCamera::change_view(int view_index) {
	if (KRT_to_camera(view_index, camera())) {
		const CameraPara& c = views_[view_index];
        resize(c.w * 0.5, c.h * 0.5);
		update();
	}
}


bool RealCamera::read_bundler_file(const std::string& file_name) {
	typedef struct {
		int image;
		int key;
		double x;
		double y;
	} view_t;

	typedef struct {
		double pos[3];
		double color[3];
		std::vector<view_t> views;
	} point_t;

#define NUM_CAMERA_PARAMS 9
#define POLY_INVERSE_DEGREE 6

	typedef struct {
		double R[9];     /* Rotation */
		double t[3];     /* Translation */
		double f;        /* Focal length */
		double k[2];     /* Undistortion parameters */
		double k_inv[POLY_INVERSE_DEGREE]; /* Inverse undistortion parameters */
		char constrained[NUM_CAMERA_PARAMS];
		double constraints[NUM_CAMERA_PARAMS];  /* Constraints (if used) */
		double weights[NUM_CAMERA_PARAMS];      /* Weights on the constraints */
		double K_known[9];  /* Intrinsics (if known) */
		double k_known[5];  /* Distortion params (if known) */

		char known_intrinsics;   /* Are the intrinsics known? */

		double f_scale, k_scale; /* Scale on focal length, distortion params */
	} camera_params_t;

	std::vector<point_t> points;
	std::vector<camera_params_t> cameras;

	std::ifstream input(file_name.c_str());
	if (input.fail()) {
		std::cerr << "could not open file: \'" << file_name << "\'" << std::endl;
		return false;
	}

	int num_images, num_points;

	std::string first_line;
	getline(input, first_line);
	std::istringstream line_stream(first_line);
	double bundle_version;
	if (first_line[0] == '#') {
		std::string dummy;
		char v;
		line_stream >> dummy >> dummy >> dummy >> v >> bundle_version;
		input >> num_images >> num_points;
	}
	else if (first_line[0] == 'v') {
		char v;
		line_stream >> v >> bundle_version;
		input >> num_images >> num_points;
	}
	else {
		bundle_version = 0.1;
		input >> num_images >> num_points;
	}

	std::cout << "[ReadBundleFile] Reading " << num_images << " images and " << num_points << " points" << std::endl;

	/* Read cameras */
	for (int i = 0; i < num_images; i++) {
		double focal_length, k0, k1;
		double R[9];
		double t[3];

		/* Focal lengths*/
		input >> focal_length >> k0 >> k1;
		/* Rotation */
		input >> R[0] >> R[1] >> R[2] >> R[3] >> R[4] >> R[5] >> R[6] >> R[7] >> R[8];
		/* Translation */
		input >> t[0] >> t[1] >> t[2];

		camera_params_t cam;
		cam.f = focal_length;
		cam.k[0] = k0;
		cam.k[1] = k1;
		memcpy(cam.R, R, sizeof(double) * 9);
		memcpy(cam.t, t, sizeof(double) * 3);

		/* Flip the scene if needed */
		if (bundle_version < 0.3) {
			R[2] = -R[2];
			R[5] = -R[5];
			R[6] = -R[6];
			R[7] = -R[7];
			t[2] = -t[2];
		}

		cameras.push_back(cam);
	}

	/* Read points */
	int total_num_visible = 0;
	for (int i = 0; i < num_points; i++) {
		point_t pt;

		/* Position */
		input >> pt.pos[0] >> pt.pos[1] >> pt.pos[2];
		/* Color */
		input >> pt.color[0] >> pt.color[1] >> pt.color[2];

		int num_visible;
		input >> num_visible;
		total_num_visible += num_visible;

		for (int j = 0; j < num_visible; j++) {
			int image, key;
			input >> image >> key;

			double x = 0, y = 0;
			if (bundle_version >= 0.3)
				input >> x >> y;

			view_t view;
			view.image = image;
			view.key = key;
			view.x = x;
			view.y = y;

			pt.views.push_back(view);
		}

		if (bundle_version < 0.3)
			pt.pos[2] = -pt.pos[2];

		if (num_visible > 0) {
			points.push_back(pt);
		}
	}

	for (const auto& c : cameras) {
		CameraPara cam;
		cam.fx = 3686.4;
		cam.fy = 3686.4;
		cam.u0 = 1520.69;
		cam.v0 = 1006.81;

		cam.w = 3072;
		cam.h = 2048;

		mat3 r;
		for (int i = 0; i < 9; ++i)
			r[i] = c.R[i];
		r = transpose(r); // r is column-major

		quat q;
		q.set_from_rotation_matrix(r);

		vec3 axis;
		float angle;
		q.get_axis_angle(axis, angle);
		axis *= angle;
		cam.rx = axis.x;
		cam.ry = axis.y;
		cam.rz = axis.z;

		cam.tx = c.t[0];
		cam.ty = c.t[1];
		cam.tz = c.t[2];
	
		views_.push_back(cam);
	}

	return cameras.size() > 0;
}


void RealCamera::create_cameras_drawable(float scale)
{
	const double halfHeight = scale * 0.07;
	const double halfWidth = halfHeight * 1.3;
	const double dist = halfHeight / tan(double(M_PI) / 8.0);

	const double arrowHeight = 1.5 * halfHeight;
	const double baseHeight = 1.2 * halfHeight;
	const double arrowHalfWidth = 0.5 * halfWidth;
	const double baseHalfWidth = 0.3 * halfWidth;

	std::vector<vec3> points;

	//--------------
	// Frustum outline
	//--------------

	// LINE_STRIP
	const vec3 p0(-halfWidth, halfHeight, -dist);
	const vec3 p1(-halfWidth, -halfHeight, -dist);	points.push_back(p0); points.push_back(p1);
	const vec3 p2(0.0, 0.0, 0.0);					points.push_back(p1); points.push_back(p2);
	const vec3 p3(halfWidth, -halfHeight, -dist);	points.push_back(p2); points.push_back(p3);
	const vec3 p4(-halfWidth, -halfHeight, -dist);	points.push_back(p3); points.push_back(p4);

	// LINE_STRIP
	const vec3 q0(halfWidth, -halfHeight, -dist);
	const vec3 q1(halfWidth, halfHeight, -dist);
	const vec3 q2(0.0, 0.0, 0.0);
	const vec3 q3(-halfWidth, halfHeight, -dist);
	const vec3 q4(halfWidth, halfHeight, -dist);
	points.push_back(q0); points.push_back(q1);
	points.push_back(q1); points.push_back(q2);
	points.push_back(q2); points.push_back(q3);
	points.push_back(q3); points.push_back(q4);

	//------------------
	// Up arrow
	//------------------

	// Base - QUAD
	const vec3 r0(-baseHalfWidth, halfHeight, -dist);
	const vec3 r1(baseHalfWidth, halfHeight, -dist);
	const vec3 r2(baseHalfWidth, baseHeight, -dist);
	const vec3 r3(-baseHalfWidth, baseHeight, -dist);
	points.push_back(r0); points.push_back(r1);
	points.push_back(r1); points.push_back(r2);
	points.push_back(r2); points.push_back(r3);
	points.push_back(r3); points.push_back(r0);

	// Arrow - TRIANGLE
	const vec3 a0(0.0, arrowHeight, -dist);
	const vec3 a1(-arrowHalfWidth, baseHeight, -dist);
	const vec3 a2(arrowHalfWidth, baseHeight, -dist);
	points.push_back(a0); points.push_back(a1);
	points.push_back(a1); points.push_back(a2);
	points.push_back(a2); points.push_back(a0);

	std::vector<vec3> vertices;
	for (int i = 0; i < views_.size(); ++i) {
		Camera c;
		KRT_to_camera(i, &c);
		const mat4& m = c.frame()->worldMatrix();
		for (auto& p : points) {
			vertices.push_back(m * p);
		}
	}
	LinesDrawable* d = current_model()->lines_drawable("camera");
	if (!d)
		d = current_model()->add_lines_drawable("camera");
	d->update_vertex_buffer(vertices);
	d->set_default_color(vec3(0, 0, 1));

}
