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

#ifndef EASY3D_RENDERER_DRAWABLE_H
#define EASY3D_RENDERER_DRAWABLE_H

#include <string>
#include <vector>
#include <memory>
#include <functional>

#include <easy3d/core/types.h>
#include <easy3d/renderer/state.h>

namespace easy3d {

    class Model;
    class Camera;
    class Manipulator;
    class VertexArrayObject;

    /**
     * @brief The base class for drawable objects. A drawable represent a set of points, line segments, or triangles.
     * \class Drawable easy3d/renderer/drawable.h
     * @details A Drawable is an abstraction for "something that can be drawn", e.g., a point cloud, the surface
     *          of a mesh, the wireframe of a surface mesh, the vertices of a graph, the border of a polyhedral mesh.
     *          A drawable manages its rendering status and controls the upload of the data to the GPU.
     *          A drawable can live independently or be associated with a Model. A drawable is considered standalone
     *          if its associated \c Model is \c null.
     *          The rendering states of a drawable can be accessed/modified through the functions provided in its
     *          parent class State.
     * @sa State, Renderer, PointsDrawable, LinesDrawable, TrianglesDrawable, and LinesDrawable2D.
     */

    class Drawable : public State {
    public:
        /**
         * \brief The type of the drawable.
         */
        enum Type {
            DT_POINTS = 0x0000,     //!< Points drawable (GL_POINTS).
            DT_LINES = 0x0001,      //!< Lines drawable (GL_LINES).
            DT_TRIANGLES = 0x0004   //!< Triangles drawable (GL_TRIANGLES).
        };

    public:
        /**
         * \brief Constructor that initializes the drawable with a name and an optional model.
         * \param name The name of the drawable.
         * \param model The model to which the drawable is attached (can be nullptr).
         */
        explicit Drawable(const std::string &name = "unknown", Model *model = nullptr);

        /**
         * \brief Destructor.
         */
        ~Drawable() override;

        /**
         * \brief Returns the type of the drawable.
         * \return The type of the drawable.
         */
        virtual Type type() const = 0;

        /**
         * \brief Returns the name of the drawable.
         * \return The name of the drawable.
         */
        const std::string &name() const { return name_; }
        /**
         * \brief Sets the name of the drawable.
         * \param n The new name of the drawable.
         */
        void set_name(const std::string& n) { name_ = n; }

        /**
         * \brief Returns the model to which the drawable is attached.
         * \return The model to which the drawable is attached (can be nullptr).
         */
        Model *model() { return model_; }
        /**
         * \brief Returns the model to which the drawable is attached (const version).
         * \return The model to which the drawable is attached (can be nullptr).
         */
        const Model *model() const { return model_; }
        /**
         * \brief Sets the model to which the drawable is attached.
         * \param m The model to which the drawable is attached.
         */
        void set_model(Model *m) { model_ = m; }

        /**
         * \brief Returns the bounding box of the drawable.
         * \return The bounding box of the drawable.
         */
        const Box3 &bounding_box() const;

        /**
         * \brief Returns the state of the drawable.
         * \return The state of the drawable.
         */
        State& state() { return *this; }
        /**
         * \brief Returns the state of the drawable (const version).
         * \return The state of the drawable.
         */
        const State& state() const { return *this; }
        /**
         * \brief Sets the state of the drawable.
         * \param s The new state of the drawable.
         */
        void set_state(const State& s) { state() = s; }

        /**
         * \brief Prints statistics of the buffers to an output stream.
         * \param output The output stream (e.g., std::cout).
         */
        void buffer_stats(std::ostream &output) const;

        /// \name Buffer access and management
        /// @{
        /**
         * \brief Returns the vertex buffer ID.
         * \return The vertex buffer ID.
         */
        unsigned int vertex_buffer() const { return vertex_buffer_; }
        /**
         * \brief Returns the color buffer ID.
         * \return The color buffer ID.
         */
        unsigned int color_buffer() const { return color_buffer_; }
        /**
         * \brief Returns the normal buffer ID.
         * \return The normal buffer ID.
         */
        unsigned int normal_buffer() const { return normal_buffer_; }
        /**
         * \brief Returns the texture coordinate buffer ID.
         * \return The texture coordinate buffer ID.
         */
        unsigned int texcoord_buffer() const { return texcoord_buffer_; }
        /**
         * \brief Returns the element buffer ID.
         * \return The element buffer ID.
         */
        unsigned int element_buffer() const { return element_buffer_; }

        /**
         * \brief Creates/Updates the vertex buffer.
         * \param vertices The new vertex data.
         * \param dynamic Whether the buffer is dynamic.
         */
        void update_vertex_buffer(const std::vector<vec3> &vertices, bool dynamic = false);
        /**
         * \brief Creates/Updates the color buffer.
         * \param colors The new color data.
         * \param dynamic Whether the buffer is dynamic.
         */
        void update_color_buffer(const std::vector<vec3> &colors, bool dynamic = false);
        /**
         * \brief Updates the normal buffer.
         * \param normals The new normal data.
         * \param dynamic Whether the buffer is dynamic.
         */
        void update_normal_buffer(const std::vector<vec3> &normals, bool dynamic = false);
        /**
         * \brief Updates the texture coordinate buffer.
         * \param texcoords The new texture coordinate data.
         * \param dynamic Whether the buffer is dynamic.
         */
        void update_texcoord_buffer(const std::vector<vec2> &texcoords, bool dynamic = false);
        /**
         * \brief Updates the element buffer.
         * \details Primitives like lines and triangles can be drawn with or without the element buffer.
         *  - With an element buffer: this can reduce the GPU memory consumption.
         *  - Without an element buffer: easier data transfer, but uses more GPU memory. In this case, vertices need to
         *    be in a correct order, like f1_v1, f1_v2, f1_v3, f2_v1, f2_v2, f2_v3... This requires the shared vertices
         *    be duplicated in the vertex buffer.
         * \param elements The new element data.
         */
        void update_element_buffer(const std::vector<unsigned int> &elements);
        /**
         * \brief Updates the element buffer.
         * \details This is an overload of the above update_element_buffer() method.
         * \param elements The new element data.
         * \note Each entry must have 2 (for LinesDrawable) or 3 elements (for TrianglesDrawable).
         */
        void update_element_buffer(const std::vector< std::vector<unsigned int> > &elements);

        /**
         * \brief Disables the use of the element buffer.
         * \details This method should be used if existing vertex data is sufficient for rendering (may require
         *         duplicating vertex data). This method also releases the element buffer.
         */
        void disable_element_buffer();

        ///@}
        /**
         * \brief Returns the number of vertices.
         * \return The number of vertices.
         */
        std::size_t num_vertices() const { return num_vertices_; }

        /// \name Rendering
        ///@{

        /**
         * \brief Draws the drawable.
         * \param camera The camera used for rendering.
         */
        virtual void draw(const Camera *camera) const = 0;

        /**
         * \brief Draws the drawable using OpenGL.
         * \details This function should be called when your shader program is in use,
         *          i.e., between glUseProgram(id) and glUseProgram(0).
         */
        void gl_draw() const;

        /**
         * \brief Requests an update of the OpenGL buffers.
         * \details This function sets the status to trigger an update of the OpenGL buffers. The actual update does
         *      not occur immediately but is deferred to the rendering phase.
         * \note This method works for both standard drawables (no update function required) and non-standard
         *      drawable (update function required). Standard drawables include:
         *            - SurfaceMesh: "faces", "edges", "vertices", "borders", and "locks";
         *            - PointCloud: "vertices";
         *            - Graph: "edges", and "vertices",
         *            - PolyMesh: "faces:border" and "faces:interior".
         * \sa set_update_func(), Renderer::update()
         */
        void update();

        /**
         * \brief Sets the update function for the drawable.
         * \details The update function defines how a drawable updates its rendering buffers. It is required by only
         *      non-standard drawables for a special visualization purpose.
         *      Rendering buffers of standard drawables attached to a model can be automatically updated and do not
         *      require this function.
         *      For a subclassed drawable, you can also reimplement update_buffers_internal() for the same purpose.
         * \param func The update function.
         * \sa update(), Renderer::update().
         */
        void set_update_func(const std::function<void(Model*, Drawable*)>& func) { update_func_ = func; }

        ///@}

        /// \name Manipulation
        ///@{
        /**
         * \brief Returns the manipulator attached to this drawable.
         * \details If the drawable is part of a model, it returns the model's manipulator.
         *          Returns nullptr if the drawable cannot be manipulated.
         * \return The manipulator attached to this drawable.
         */
        Manipulator* manipulator();
        /**
         * \brief Returns the manipulator attached to this drawable (const version).
         * \details If the drawable is part of a model, it returns the model's manipulator.
         *          Returns nullptr if the drawable cannot be manipulated.
         * \return The manipulator attached to this drawable.
         */
        const Manipulator* manipulator() const;

        /**
         * \brief Attaches a manipulator to this drawable.
         * \details This is used to manipulate a drawable that is not part of a model.
         * \param manip The manipulator to attach.
         */
        void set_manipulator(std::shared_ptr<Manipulator> manip) { manipulator_ = manip; }

        /**
         * \brief Returns the manipulation matrix.
         * \return The manipulation matrix.
         */
        mat4 manipulated_matrix() const;
        ///@}

        /**
         * \brief Returns the vertex array object of this drawable.
         * \return The vertex array object.
         */
        VertexArrayObject *vao() { return vao_.get(); }
        /**
         * \brief Returns the vertex array object of this drawable (const version).
         * \return The vertex array object.
         */
        const VertexArrayObject *vao() const { return vao_.get(); }

    protected:
        // actual update of the rendering buffers are here
        virtual void update_buffers_internal();

        // clears all buffers
        void clear();

    protected:
        std::string name_;  //!< The name of the drawable.
        Model *model_;      //!< The model to which the drawable is attached.
        Box3 bbox_;         //!< The bounding box of the drawable.

        // vertex array object
        std::shared_ptr<VertexArrayObject> vao_;    //!< The vertex array object.

        std::size_t num_vertices_;  //!< The number of vertices.
        std::size_t num_indices_;   //!< The number of indices.

        bool update_needed_;        //!< Whether an update is needed.
        std::function<void(Model*, Drawable*)> update_func_;    //!< The update function.

        unsigned int vertex_buffer_;    //!< The vertex buffer ID.
        unsigned int color_buffer_;     //!< The color buffer ID.
        unsigned int normal_buffer_;    //!< The normal buffer ID.
        unsigned int texcoord_buffer_;  //!< The texture coordinate buffer ID.
        unsigned int element_buffer_;   //!< The element buffer ID.

        // drawables not attached to a model can also be manipulated
        std::shared_ptr<Manipulator> manipulator_;   //!< The manipulator for the drawable.
    };

}


#endif  // EASY3D_RENDERER_DRAWABLE_H
