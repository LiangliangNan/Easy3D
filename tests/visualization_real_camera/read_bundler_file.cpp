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

#include "viewer.h"

#include <fstream>
#include <sstream>
#include <cstring>  // for memcpy()

using namespace easy3d;


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
        std::cerr << "could not open file \'" << file_name << "\'" << std::endl;
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
        cam.fx = cam.fy = static_cast<float>(c.f);
        cam.cx = 1520.69f;
        cam.cy = 1006.81f;
        cam.w = 3072;
        cam.h = 2048;

		mat3 R;
		for (int i = 0; i < 9; ++i)
            R[i] = static_cast<float>(c.R[i]);
        R = transpose(R); // r is column-major

        cam.R = R;
        cam.t = vec3(c.t);
	
		views_.push_back(cam);
	}

	return cameras.size() > 0;
}
