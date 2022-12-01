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

#ifndef EASY3D_TUTORIAL_REAL_CAMERA_H
#define EASY3D_TUTORIAL_REAL_CAMERA_H

#include <easy3d/viewer/viewer.h>

// This RealCamera class visualizes a model from the view point
// given the camera's intrinsic and extrinsic parameters. These
// camera parameters can (usually) be recovered using camera
// calibration or structure from motion techniques.

namespace easy3d {
    class Camera;
    class Texture;
    class LinesDrawable;
}

class RealCamera : public easy3d::Viewer
{
public:
    RealCamera(const std::string& title,
               const std::string& bundler_file,
               const std::string& cloud_file);

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
     *  \param img_x and img_y: the image point (the image origin corresponds to the top-left corner of the image)
     *  \param fx and fy: the focal length
     *  \param cx and cy: the principal point
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
    static easy3d::vec3 pixel_to_ray(int img_x, int img_y,
                              float fx, float fy, float skew, float cx, float cy,
                              const easy3d::mat3 &R, const easy3d::vec3 &t, bool convert = true);

    /** \brief Computes the 2D projection of a 3D point using the camera intrinsic and extrinsic parameters.
     *  \param p the 3D point
     *  \param fx and fy: the focal length
     *  \param cx and cy: the principal point
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
                                float fx, float fy, float skew, float cx, float cy,
                                const easy3d::mat3 &R, const easy3d::vec3 &t, bool convert = true);

protected:
    bool key_press_event(int key, int modifiers) override;
    bool mouse_free_move_event(int x, int y, int dx, int dy, int modifiers) override;

    easy3d::Rect calculate_image_rect() const;
    
private:
    struct CameraPara {
        int w, h;           // image size
        float fx, fy;       // focal length
        float cx, cy;       // principal point
        easy3d::mat3 R;     // rotation
        easy3d::vec3 t;     // the translation
    };
    std::vector<CameraPara> views_;
    int current_view_;
    
    bool read_bundler_file(const std::string& file_name);
    
    // K [R t] -> easy3d camera representation
    bool KRT_to_camera(int view_index, easy3d::Camera* c, bool ground_truth);
    
    void update_cameras_drawable(bool ground_truth);

    void load_image();

    void post_draw() override;

private:
    easy3d::Texture* texture_;
    easy3d::LinesDrawable* cameras_drawable_;
    easy3d::LinesDrawable* ray_drawable_;   // in 3D
    easy3d::LinesDrawable* cross_drawable_; // in image
};


#endif // EASY3D_TUTORIAL_REAL_CAMERA_H
