/** \cond LICENSE
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
 \endcond */

/** ----------------------------------------------------------
 *
 * the code is adapted from libQGLViewer with modifications.
 *		- libQGLViewer (Version 2.7.1, Nov 17th, 2017)
 * The original code is available at
 * http://libqglviewer.com/
 *
 * libQGLViewer is a C++ library based on Qt that eases the
 * creation of OpenGL 3D viewers.
 *
 *----------------------------------------------------------*/

#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/manipulated_camera_frame.h>

// \cond
namespace easy3d {

	/**
	 * Default constructor.
	 *
	 * The translation is set to (0,0,0), with an identity rotation (0,0,0,1) (see
	 * Frame constructor for details).
	 *
	 * The different sensitivities are set to their default values (see
	 * rotation_sensitivity(), translation_sensitivity(), spinningSensitivity() and
	 * wheel_sensitivity()). */
	ManipulatedFrame::ManipulatedFrame()
	{
		set_rotation_sensitivity(1.0);
		set_translation_sensitivity(1.0);
		set_wheel_sensitivity(1.0);
		set_zoom_sensitivity(1.0);

        previousConstraint_ = nullptr;
	}

	/** Equal operator. Calls Frame::operator=() and then copy attributes. */
	ManipulatedFrame &ManipulatedFrame::operator=(const ManipulatedFrame &mf) {
		Frame::operator=(mf);

		set_rotation_sensitivity(mf.rotation_sensitivity());
		set_translation_sensitivity(mf.translation_sensitivity());
		set_wheel_sensitivity(mf.wheel_sensitivity());
		set_zoom_sensitivity(mf.zoom_sensitivity());

		return *this;
	}

	/** Copy constructor. Performs a deep copy of all attributes using operator=(). */
	ManipulatedFrame::ManipulatedFrame(const ManipulatedFrame &mf)
		: Frame(mf)
	{
		(*this) = mf;
	}

	////////////////////////////////////////////////////////////////////////////////
	//                 M o u s e    h a n d l i n g                               //
	////////////////////////////////////////////////////////////////////////////////

	float ManipulatedFrame::wheel_delta(int wheel_dy) const {
		static constexpr float WHEEL_SENSITIVITY_COEFF = 0.1f;
		return static_cast<float>(wheel_dy) * wheel_sensitivity() * WHEEL_SENSITIVITY_COEFF;
	}


	void ManipulatedFrame::action_start() {
	}


	void ManipulatedFrame::action_end() {
		if (previousConstraint_)
			set_constraint(previousConstraint_);
	}


	void ManipulatedFrame::action_rotate(int mouse_x, int mouse_y, int mouse_dx, int mouse_dy, Camera *const camera, ScreenAxis axis)
	{
        if (mouse_dx == 0 && mouse_dy == 0)
            return;

        //todo: not fully tested
        //todo: use the Constraint class (see action_rotate() in ManipulatedCameraFrame)
		vec3 trans = camera->projected_coordinates_of(position());
		DLOG_IF(has_nan(trans), ERROR)
						<< "projected_coordinates_of(position()): " << trans
						<< "\n\tposition(): " << position()
						<< "\n\tcamera position: " << camera->position()
						<< "\n\tcamera orientation: " << camera->orientation();
		if (has_nan(trans))
			return;

		const int w = camera->screen_width();
		const int h = camera->screen_height();

		quat rot;
		if (axis == NONE) {	// free rotation
            const int pre_x = mouse_x - mouse_dx;
            const int pre_y = mouse_y - mouse_dy;
			// The incremental rotation defined in the ManipulatedFrame coordinate system.
			rot = deformed_ball_quaternion(mouse_x, mouse_y, pre_x, pre_y, trans[0], trans[1], w, h);
		}
		else {
            if (axis == ORTHOGONAL) {
                const int pre_x = mouse_x - mouse_dx;
                const int pre_y = mouse_y - mouse_dy;
                const float prev_angle = std::atan2(static_cast<float>(pre_y) - trans[1], static_cast<float>(pre_x) - trans[0]);
                const float angle = std::atan2(static_cast<float>(mouse_y) - trans[1], static_cast<float>(mouse_x) - trans[0]);
                // The incremental rotation defined in the ManipulatedCameraFrame's coordinate system.
                rot = quat(vec3(0.0, 0.0, 1.0), angle - prev_angle);
            } else if (axis == VERTICAL) {
                const int pre_x = mouse_x - mouse_dx;
                const int pre_y = mouse_y;    // restricts the movement to be horizontal (so purl vertical rotation)
                rot = deformed_ball_quaternion(mouse_x, mouse_y, pre_x, pre_y, trans.x, trans.y, w, h);
            } else if (axis == HORIZONTAL) {
                const int pre_x = mouse_x;    // restricts the movement to be vertical (so purl horizontal rotation)
                const int pre_y = mouse_y - mouse_dy;
                rot = deformed_ball_quaternion(mouse_x, mouse_y, pre_x, pre_y, trans.x, trans.y, w, h);
            }
		}

        trans = vec3(-rot[0], -rot[1], -rot[2]);
        trans = camera->frame()->orientation().rotate(trans);
        trans = transform_of(trans);
        rot[0] = trans[0];
        rot[1] = trans[1];
        rot[2] = trans[2];
        // Rotates the ManipulatedFrame around its origin.
        rotate(rot);

        modified.send();
	}

	void ManipulatedFrame::action_translate(int x, int y, int dx, int dy, Camera *const camera, ScreenAxis axis)
	{
        if (dx == 0 && dy == 0)
            return;

        //todo: not fully tested
        //todo: use the Constraint class
		if (axis == NONE) {    // free translation
			vec3 trans(static_cast<float>(dx), -static_cast<float>(dy), 0.0);
			// Scale to fit the screen mouse displacement
			switch (camera->type()) {
				case Camera::PERSPECTIVE:
					trans *= 2.0f * std::tan(camera->field_of_view() / 2.0f) *
							 std::fabs((camera->frame()->coordinates_of(position())).z) /
                            static_cast<float>(camera->screen_height());
					break;
				case Camera::ORTHOGRAPHIC: {
					float w, h;
					camera->get_ortho_width_height(w, h);
					trans[0] *= 2.0f * w / static_cast<float>(camera->screen_width());
					trans[1] *= 2.0f * h / static_cast<float>(camera->screen_height());
					break;
				}
			}
			// Transform to world coordinate system.
			trans = camera->frame()->orientation().rotate(translation_sensitivity() * trans);
			// And then down to frame
			if (reference_frame())
				trans = reference_frame()->transform_of(trans);
			translate(trans);
		}
		else {  // new implementation
			vec3 trans;
			if (axis == HORIZONTAL)
				trans = vec3(static_cast<float>(dx), 0.0f, 0.0f);
			else if (axis == VERTICAL)
				trans = vec3(0.0f, static_cast<float>(-dy), 0.0f);
			else
				return;

			switch (camera->type()) {
			case Camera::PERSPECTIVE:
				trans *= 2.0 * std::tan(camera->field_of_view() / 2.0f) *
					fabs((camera->frame()->coordinates_of(position())).z) /
					camera->screen_height();
				break;
			case Camera::ORTHOGRAPHIC: {
				float w, h;
				camera->get_ortho_width_height(w, h);
				trans[0] *= 2.0f * w / static_cast<float>(camera->screen_width());
				trans[1] *= 2.0f * h / static_cast<float>(camera->screen_height());
				break;
			    }
			}
			// Transform to world coordinate system.
			trans = camera->frame()->orientation().rotate(translation_sensitivity() * trans);
			// And then down to frame
			if (reference_frame())
				trans = reference_frame()->transform_of(trans);

			translate(trans);
		}

        modified.send();
	}


	void ManipulatedFrame::action_zoom(int wheel_dy, Camera *const camera)
	{
		const float delta = wheel_delta(wheel_dy);

		vec3 trans(0.0f, 0.0f, (camera->position() - position()).norm() * delta);

		trans = camera->frame()->orientation().rotate(trans);
		if (reference_frame())
			trans = reference_frame()->transform_of(trans);
		translate(trans);
        modified.send();

		// #CONNECTION# startAction should always be called before
		if (previousConstraint_)
			set_constraint(previousConstraint_);
	}

	////////////////////////////////////////////////////////////////////////////////

	/** Returns "pseudo-distance" from (x,y) to ball of radius size.
	 * \arg for a point inside the ball, it is proportional to the Euclidean distance
	 * to the ball \arg for a point outside the ball, it is proportional to the inverse
	 * of this distance (tends to zero) on the ball, the function is continuous. */
	static float projectOnBall(float x, float y) {
		// If you change the size value, change angle computation in
		// deformed_ball_quaternion().
		constexpr float size = 1.0f;
		constexpr float size2 = size * size;
		constexpr float size_limit = size2 * 0.5f;

		const float d = x * x + y * y;
		return d < size_limit ? std::sqrt(size2 - d) : size_limit / std::sqrt(d);
	}


	/** Returns a quaternion computed according to the mouse motion. Mouse positions
	 * are projected on a deformed ball, centered on (\p cx,\p cy). */
	quat ManipulatedFrame::deformed_ball_quaternion(int x, int y, int pre_x, int pre_y, float cx, float cy, int w, int h) const {
		// Points on the deformed ball
		const float px = rotation_sensitivity() * (static_cast<float>(pre_x) - cx) / static_cast<float>(w);
		const float py = rotation_sensitivity() * (cy - static_cast<float>(pre_y)) / static_cast<float>(h);
		const float dx = rotation_sensitivity() * (static_cast<float>(x) - cx) / static_cast<float>(w);
		const float dy = rotation_sensitivity() * (cy - static_cast<float>(y)) / static_cast<float>(h);

		const vec3 p1(px, py, projectOnBall(px, py));
		const vec3 p2(dx, dy, projectOnBall(dx, dy));
		// Approximation of rotation angle
		// Should be divided by the projectOnBall size, but it is 1.0
		const vec3 axis = cross(p2, p1);
		const float angle = 5.0f * std::asin(std::sqrt(axis.length2() / p1.length2() / p2.length2()));
		return {axis, angle};
	}

}
// \endcond
