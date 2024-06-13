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

#ifndef EASY3D_VIEWER_OFFSCREEN_H
#define EASY3D_VIEWER_OFFSCREEN_H

#include <easy3d/viewer/viewer.h>


namespace easy3d {

    /**
     * @brief Offscreen rendering.
     * @class OffScreen easy3d/viewer/offscreen.h
     * @details To use offscreen rendering, simply create an instance of OffScreen and call the render() method, e.g.,
     *      \code
     *          OffScreen os;
     *          if (!os.add_model("bunny.ply")) {
     *              LOG(ERROR) << "failed to load model";
     *              return EXIT_FAILURE;
     *          }
     *          ... when necessary, access the camera and modify the view here.
     *          bool success = os.render("image.png");
     *      \endcode
     */
    class OffScreen : protected Viewer
	{
	public:
        /**
         * @brief Constructor
         * @param width The width of the offscreen renderer, which can be changed by calling resize() after construction.
         * @param height The height of the offscreen renderer, which can be changed by calling resize() after construction.
         */
		explicit OffScreen(int width = 800, int height = 600);

        /// @name Camera manipulation.
        //@{
        /// @brief Returns the camera used by the offscreen renderer. See \c Camera.
        Camera* camera();
        /// @brief Returns the camera used by the offscreen renderer. See \c Camera.
        const Camera* camera() const;
        //@}

        /**
         * @brief  Render the current scene into an image file. Supported image format: png, jpg, bmp, and tga.
         * @details This function renders the scene into a framebuffer and takes a snapshot of the framebuffer.
         *      It allows the snapshot image to have a dimension different from the offscreen renderer, and it
         *      has no limit on the image size (if memory allows).
         * @param file_name The image file name.
         * @param scaling The scaling factor that determines the size of the image (default to 1.0, same size as
         *      the offscreen renderer), i.e.,
         *      image_width = width() * scaling;
         *      image_height = height() * scaling;
         * @param samples The required number of samples for antialiased rendering (which can be different from
         *      that of the default framebuffer). The default value is 4.
         * @param back_ground Determines the background color. 0: current color; 1: white; 2: transparent.
         * @param expand Expand the frustum to ensure the image aspect ratio.
         * @return true on success and false otherwise.
         */
        bool render(const std::string& file_name, float scaling = 1.0f, int samples = 4, int back_ground = 1, bool expand = true) const;


        /// @name Other properties
        //@{
        /**
         * @brief Set/Change the size of the offscreen renderer.
         * @param w/h The requested width/height (in pixels) of the offscreen renderer.
         */
        void resize(int w, int h);

        /// @brief Returns the width of the offscreen renderer.
        int width() const;
        /// @brief Returns the height of the offscreen renderer.
        int height() const;

        /**
         * @brief Set the background color of the offscreen renderer
         * @param color The background color.
         */
        void set_background_color(const easy3d::vec4& c);

        /**
         * @brief Query the background color of the offscreen renderer.
         * @return The background color of the offscreen renderer
         */
        const easy3d::vec4& background_color() const;
        //@}

        /// @name Model management
        //@{

        /**
         * @brief Add a model from a file to the offscreen renderer. On success, the offscreen renderer
         *        will be in charge of the memory management of the model. The loaded model can be
         *        accessed by the 'current_model()' method.
         * @details This method loads a model into the offscreen renderer. It allows the user to control if
         *          default drawables will be created. The default drawables are
         *            - for point clouds: "vertices".
         *            - for surface meshes: "faces", "vertices", "edges", "borders", and "locks".
         *            - for graphs: "vertices" and "edges".
         *            - polyhedral meshes: "faces:border"， "faces:interior"， "vertices"， and "edges".
         *          These drawables are usually sufficient for basic rendering of the model. In case
         *          the default drawables don't meet the particular visualization purpose, you can
         *          set 'create_default_drawables' to false and create your needed drawables by 
         *          calling model->renderer()->add_[type]_drawable(). Here the [type] must be one of 
         *          "points", "lines", and "triangles'.
         * @param file_name The string of the file name.
         * @param create_default_drawables If true, the default drawables will be created. 
         * @return The pointer to the model added to the offscreen renderer (nullptr if failed).
         * @related add_model(Model*, bool).
         */
        virtual Model* add_model(const std::string& file_name, bool create_default_drawables = true);

        /**
         * @brief Add an existing model to the offscreen renderer. On success, the offscreen renderer will be in
         *      charge of its memory management.
         * @details This method adds a model into the offscreen renderer. It allows the user to control if
         *          default drawables will be created. The default drawables are
         *            - for point clouds: "vertices".
         *            - for surface meshes: "faces", "vertices", "edges", "borders", and "locks".
         *            - for graphs: "vertices" and "edges".
         *            - polyhedral meshes: "faces:border"， "faces:interior"， "vertices"， and "edges".
         *          These drawables are usually sufficient for basic rendering of the model. In case
         *          the default drawables don't meet the particular visualization purpose, you can
         *          set 'create_default_drawables' to false and create your needed drawables by
         *          calling model->renderer()->add_[type]_drawable(). Here the [type] must be one of
         *          "points", "lines", and "triangles'.
         * @param model The pointer to the model.
         * @param create_default_drawables If true, the default drawables will be created.
         * @return The pointer to the model added to the offscreen renderer (nullptr if failed).
         * @related add_model(const std::string&, bool).
         */
        Model* add_model(Model* model, bool create_default_drawables = true);

        /**
         * @brief Delete a model. The memory of the model will be released and its existing drawables
         *        also be deleted.
         * @param model The pointer to the model.
         * @return True if the model has been deleted.
         */
        bool delete_model(Model* model);

        /**
         * @brief Query the models managed by this offscreen renderer.
         * @return The models managed by this offscreen renderer.
         */
        const std::vector<Model*>& models() const;

        /**
         * @brief Query the active model.
         * @details The offscreen renderer can manage multiple models, and only one model is actively being processed.
         *      This method is used to identify this active model.
         * @return The active model.
         */
        Model* current_model() const;
        //@}

        /// @name Drawable management
        //@{

        /**
         * @brief Add a drawable to the offscreen renderer. On success, the offscreen renderer will be in charge of
         *      its memory management.
         * @details The use of drawables for visualization is quite flexible. Drawables are
         *          typically created for rendering 3D models (e.g., point clouds, meshes, graphs)
         *          and a 3D model is usually loaded from a file or generated by an algorithm. This
         *          method allows the user to visualize drawables without defining a 3D model.
         * @param drawable The pointer to the drawable.
         * @return True if the drawable has been added.
         */
	    bool add_drawable(Drawable* drawable);

        /**
         * Delete the drawable from the offscreen renderer. The related drawables will also be deleted.
         * @param drawable The pointer to the drawable.
         * @return True if the drawable has been deleted.
         */
	    bool delete_drawable(Drawable* drawable);

        /**
         * @brief Query the drawables managed by this offscreen renderer.
         * @return The drawables managed by this offscreen renderer.
         */
        const std::vector<Drawable*>& drawables() const;

        /**
         * @brief Delete all visual contents of the offscreen renderer (all models and drawables).
         */
        void clear_scene();
        //@}
	};

}


#endif	// EASY3D_VIEWER_OFFSCREEN_H
