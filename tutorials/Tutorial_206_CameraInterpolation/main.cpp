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

#include "viewer.h"
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_206_CameraInterpolation/main.cpp
 *
 * \brief Recording camera keyframes and playing them back as a smooth fly-through animation.
 *
 * <b>Overview</b>
 * This tutorial turns the interactive Viewer into a simple camera-path animator: you navigate
 * to a viewpoint, press a key to record it as a keyframe, repeat for several viewpoints, then
 * play the path so the camera glides smoothly through them. It demonstrates Easy3D's built-in
 * KeyFrameInterpolator, which the Viewer already owns. The custom subclass
 * (TutorialCameraInterpolation, shown after this file) only adds convenient key bindings.
 *
 * <b>The technique</b>
 * A camera pose is a position plus an orientation (a quaternion), together a Frame. Animating
 * between poses means interpolating both: positions are blended along a smooth curve and
 * orientations are interpolated on the unit quaternion sphere (spherical linear interpolation,
 * or a cubic spline through several keys) so the camera turns continuously without flipping.
 * Each keyframe also carries a time; the interpolator samples the path at a fixed frame rate
 * and updates the camera's Frame on a timer, emitting a signal that triggers a redraw. The
 * result is a real-time fly-through whose speed follows the keyframe spacing. See
 * KeyFrameInterpolator for the interpolation and playback machinery.
 *
 * <b>Editing the path</b>
 * The keyframe cameras drawn in the scene are also the editing handles. Click one of them: it becomes
 * selected and a gizmo appears around it. The easiest way to reposition it is to drag the camera itself:
 * it slides in the plane of the screen and follows the cursor exactly, so two drags from different
 * viewing directions place it anywhere in 3D. For a precise move along a single world axis, drag one of
 * the three arrows (red/green/blue = X/Y/Z); drag a ring to rotate the keyframe around that world axis.
 * The rings enclose the drawn camera glyph, so the gizmo reads as a frame around the keyframe and keeps
 * its apparent size while you navigate. The pose is written back with
 * KeyFrameInterpolator::set_keyframe_position()/set_keyframe_orientation() on every mouse event, so the
 * interpolated path (the yellow curve) changes while you drag. The handle under the cursor is
 * highlighted; a ring seen edge-on (or a shaft seen end-on) cannot be followed and is ignored until you
 * turn the view toward it. Click empty space or press Esc to stop editing; the keyframe keeps its new pose.
 *
 * <b>Easy3D API</b>
 * - Viewer: already owns a KeyFrameInterpolator (kfi_) bound to the camera's Frame, plus the
 *   show_camera_path_ flag that draws the recorded path and keyframe cameras.
 * - KeyFrameInterpolator: add_keyframe() records a pose; delete_path() clears it; playback is
 *   started/stopped through the Viewer's Ctrl+K shortcut; set_keyframe_position() and
 *   set_keyframe_orientation() edit a keyframe and invalidate the interpolated path.
 * - Camera / Frame: camera()->frame() is the pose captured as a keyframe; scene_radius() is
 *   grown so the whole path stays in view; view_direction() defines the screen plane of a free
 *   drag; world_pixel_ratio() sizes the gizmo; and convert_click_to_line()/projected_coordinates_of()
 *   turn mouse positions into gizmo drags.
 * - LinesDrawable: the gizmo is a standalone drawable with per-vertex colors, attached with
 *   add_drawable().
 *
 * <b>Running it</b>
 * Navigate with the mouse, then press 'k' to add the current view as a keyframe (the path is
 * drawn in the scene). Click a green keyframe camera, then drag the camera itself to move it, an
 * arrow to move it along one axis, or a ring to rotate it. Press 'space' to start/stop the animation,
 * and 'd' to delete the path. Esc ends editing.
 *
 * The header file of the class:
 * \include{lineno} Tutorial_206_CameraInterpolation/viewer.h
 * The source file of the class:
 * \include{lineno} Tutorial_206_CameraInterpolation/viewer.cpp
 *
 * \see KeyFrameInterpolator, Camera, Frame, Viewer, Tutorial_205_RealCamera
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // A simple model to fly around.
    const std::string &file_name = resource::directory() + "/data/sphere.obj";

    // Create the viewer subclass that adds keyframe/animation shortcuts.
    TutorialCameraInterpolation viewer(EXAMPLE_TITLE);

    // Load and add the model; the second argument (true) centers and scales it to fit.
    if (!viewer.add_model(file_name, true)) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    // Enter the interactive render loop; returns only when the window is closed.
    return viewer.run();
}
