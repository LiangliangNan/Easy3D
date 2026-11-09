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

#ifndef EASY3D_TUTORIAL_REAL_CAMERA_H
#define EASY3D_TUTORIAL_REAL_CAMERA_H

#include <easy3d/viewer/viewer.h>

// This RealCamera class visualizes a model from the view point
// given the camera's intrinsic and extrinsic parameters. These
// camera parameters can (usually) be recovered using camera
// calibration or structure from motion techniques.

namespace easy3d {
    class Camera;
    class LinesDrawable;
} // namespace easy3d

/**
 * \brief A Viewer that reproduces a real calibrated camera and links image pixels to 3D rays.
 *
 * We subclass Viewer because the tutorial needs custom data (a set of recovered camera poses),
 * custom drawing (the camera frusta, a back-projected ray, and the source photo as a 2D
 * overlay), and custom interaction (cycling views and projecting under the cursor). The static
 * helpers camera_pos(), pixel_to_ray(), and point_to_pixel() implement the pinhole-camera math
 * and are reusable independently of the viewer. The private members cache the overlay geometry
 * so it is only rebuilt when the image, window size, or cursor marker actually changes.
 */
class TutorialRealCamera : public easy3d::Viewer {
public:
    // Loads the point cloud and the Bundler camera parameters, then sets up the scene/overlays.
    TutorialRealCamera(const std::string &title, const std::string &bundler_file, const std::string &cloud_file);

    /// two useful functions

    /** \brief Computes the camera position in the world coordinate system using the camera extrinsic parameters.
     *  \param R: the camera rotation. It denotes the coordinate system transformation from 3D world coordinates
     *      to 3D camera coordinates.
     *  \param t: the camera translation. It is the position of the origin of the world coordinate system expressed
     *      in the camera coordinate system. \note t is often mistakenly considered the position of the camera. The
     *      position C of the camera expressed in world coordinates is C = -inverse(rot) * t = -transpose(rot) * t.
     *  \return the camera position in the world coordinate system.
     */
    static easy3d::vec3 camera_pos(const easy3d::mat3 &R, const easy3d::vec3 &t);

    /** \brief Computes the ray in the world coordinate system from an image point using the camera
     *      intrinsic and extrinsic parameters.
     *  \param img_x the x coordinate of the image point (the image origin corresponds to the top-left corner)
     *  \param img_y the y coordinate of the image point (the image origin corresponds to the top-left corner)
     *  \param fx the focal length along the x axis (in pixels)
     *  \param fy the focal length along the y axis (in pixels)
     *  \param cx the x coordinate of the principal point (in pixels)
     *  \param cy the y coordinate of the principal point (in pixels)
     *  \param skew: distortion
     *  \param R: the camera rotation. It denotes the coordinate system transformation from 3D world coordinates
     *      to 3D camera coordinates.
     *  \param t: the camera translation. It is the position of the origin of the world coordinate system expressed
     *      in the camera coordinate system. \note t is often mistakenly considered the position of the camera. The
     *      position C of the camera expressed in world coordinates is C = -inverse(rot) * t = -transpose(rot) * t.
     *  \param convert: \c true to convert from vision convention to OpenGL convention (i.e., invert Y and Z axes).
     *                  This is because the camera coordinates in computer vision goes X right, Y down, Z forward,
     *                  while the camera coordinates in OpenGL goes X right, Y up, Z inward.
     *  \return the ray in the world coordinate system.
     *  \attention This function assumes the camera parameters were obtained by standard camera calibration, in
     *      which image coordinates are denoted in pixels, with the origin point (0, 0) corresponding to the
     *      top-left corner of the image. The X axis starts at the left edge of an image and goes towards the right
     *      edge. The Y axis starts at the top of the image towards image bottom. All image pixels have non-negative
     *      coordinates.
     */
    static easy3d::vec3 pixel_to_ray(int img_x,
                                     int img_y,
                                     float fx,
                                     float fy,
                                     float skew,
                                     float cx,
                                     float cy,
                                     const easy3d::mat3 &R,
                                     const easy3d::vec3 &t,
                                     bool convert = true);

    /** \brief Computes the 2D projection of a 3D point using the camera intrinsic and extrinsic parameters.
     *  \param p the 3D point
     *  \param fx the focal length along the x axis (in pixels)
     *  \param fy the focal length along the y axis (in pixels)
     *  \param cx the x coordinate of the principal point (in pixels)
     *  \param cy the y coordinate of the principal point (in pixels)
     *  \param skew: distortion
     *  \param R: the camera rotation. It denotes the coordinate system transformation from 3D world coordinates
     *      to 3D camera coordinates.
     *  \param t: the camera translation. It is the position of the origin of the world coordinate system expressed
     *      in the camera coordinate system. \note t is often mistakenly considered the position of the camera. The
     *      position C of the camera expressed in world coordinates is C = -inverse(rot) * t = -transpose(rot) * t.
     *  \param convert: \c true to convert from vision convention to OpenGL convention (i.e., invert Y and Z axes).
     *                  This is because the camera coordinates in computer vision goes X right, Y down, Z forward,
     *                  while the camera coordinates in OpenGL goes X right, Y up, Z inward.
     *  \return the image point of the projected 3D point (the image origin corresponds to the top-left corner).
     *  \attention This function assumes the camera parameters were obtained by standard camera calibration, in
     *      which image coordinates are denoted in pixels, with the origin point (0, 0) corresponding to the
     *      top-left corner of the image. The X axis starts at the left edge of an image and goes towards the right
     *      edge. The Y axis starts at the top of the image towards image bottom. All image pixels have non-negative
     *      coordinates.
     */
    static easy3d::vec2 point_to_pixel(const easy3d::vec3 &p,
                                       float fx,
                                       float fy,
                                       float skew,
                                       float cx,
                                       float cy,
                                       const easy3d::mat3 &R,
                                       const easy3d::vec3 &t,
                                       bool convert = true);

protected:
    // Cycles views ('space'), toggles calibration/ground-truth ('1'/'2'), and hides cameras ('h').
    bool key_press_event(int key, int modifiers) override;
    // Projects between the cursor position and 3D: pixel->ray over the image, point->pixel over the scene.
    bool mouse_free_move_event(int x, int y, int dx, int dy, int modifiers) override;

    // Computes where the source photo is drawn on screen, preserving its aspect ratio.
    easy3d::Rect calculate_image_rect() const;

private:
    // One recovered camera: image size, intrinsics (fx, fy, cx, cy), and extrinsics (R, t).
    struct CameraPara {
        int w, h;       // image size
        float fx, fy;   // focal length
        float cx, cy;   // principal point
        easy3d::mat3 R; // rotation
        easy3d::vec3 t; // the translation
    };
    std::vector<CameraPara> views_; //!< All cameras read from the Bundler file.
    int current_view_;              //!< Index of the view currently shown.

    // Parses the Bundler file into views_ (implemented in read_bundler_file.cpp).
    bool read_bundler_file(const std::string &file_name);

    // K [R t] -> easy3d camera representation
    bool KRT_to_camera(int view_index, easy3d::Camera *c, bool ground_truth);

    // Rebuilds the line drawable that draws every camera frustum in the scene.
    void update_cameras_drawable(bool ground_truth);

    // Loads the photo corresponding to current_view_ for the 2D overlay.
    void load_image();

    void pre_draw() override;            // builds/updates the image and crosshair overlays each frame
    void post_resize(int, int) override; // invalidates the cursor marker on resize
    // Cached overlay state: rebuilt only when something relevant changes (see pre_draw()).
    std::shared_ptr<const easy3d::graphics::Image> image_;      //!< The current source photo (GPU texture).
    int image_view_ = -1;                                       //!< Which view image_ belongs to (-1 = none).
    easy3d::Rect overlay_rect_{0, 0, 0, 0};                     //!< On-screen rectangle of the photo overlay.
    const easy3d::graphics::Image *overlay_image_ = nullptr;    //!< Image used to build the cached overlay.
    bool cross_geometry_dirty_ = true;                          //!< Whether the crosshair geometry must be rebuilt.
    std::vector<easy3d::vec3> cross_points_;                    //!< Endpoints of the 2D crosshair marker.
    bool cross_overlay_visible_ = false;                        //!< Whether the crosshair is currently shown.
    int overlay_width_ = 0, overlay_height_ = 0;                //!< Framebuffer size the overlay was built for.
    std::vector<easy3d::graphics::DrawCommand> image_commands_; //!< Cached photo + border draw commands.
    easy3d::graphics::DrawCommand cross_command_;               //!< Cached crosshair draw command.

private:
    easy3d::LinesDrawable *cameras_drawable_; //!< Draws all camera frusta in the 3D scene.
    easy3d::LinesDrawable *ray_drawable_;     // in 3D
    bool cross_visible_ = false;              // 2D annotation, never a scene drawable
};

#endif // EASY3D_TUTORIAL_REAL_CAMERA_H
