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
     *          A drawable can live independently or be associated with a Model.
     *          The rendering states of a drawable can be accessed/modified through the functions provided in its
     *          parent class State.
     * @sa State, Renderer
     */

    class Drawable : public State {
    public:
        // three types of drawables
        enum Type {
            DT_POINTS = 0x0000,     // == GL_POINTS
            DT_LINES = 0x0001,      // == GL_LINES
            DT_TRIANGLES = 0x0004   // == GL_TRIANGLES
        };

    public:
        // a drawable can be stand-alone or attached to a model
        explicit Drawable(const std::string &name = "unknown", Model *model = nullptr);
        ~Drawable() override;

        /// Returns the type of the drawable.
        virtual Type type() const = 0;

        const std::string &name() const { return name_; }
        void set_name(const std::string& n) { name_ = n; }

        /// the model to which the drawable is attached to (can be NULL).
        Model *model() { return model_; }
        const Model *model() const { return model_; }
        void set_model(Model *m) { model_ = m; }

        const Box3 &bounding_box() const;

        State& state() { return *this; };
        const State& state() const { return *this; };
        void set_state(const State& s) { state() = s; };

        /// print statistics (e.g., num vertices, memory usage) of the buffers to an output stream (e.g., std::cout).
        void buffer_stats(std::ostream &output) const;

        /// \name Buffer access and management
        /// @{
        unsigned int vertex_buffer() const { return vertex_buffer_; }
        unsigned int color_buffer() const { return color_buffer_; }
        unsigned int normal_buffer() const { return normal_buffer_; }
        unsigned int texcoord_buffer() const { return texcoord_buffer_; }
        unsigned int element_buffer() const { return element_buffer_; }

        /**
         * \brief Creates/Updates a single buffer.
         * \details Primitives like lines and triangles can be drawn with or without the element buffer.
         *  - With an element buffer: this can reduce the GPU memory consumption.
         *  - Without an element buffer: easier data transfer, but uses more GPU memory. In this case, vertices need to
         *    be in a correct order, like f1_v1, f1_v2, f1_v3, f2_v1, f2_v2, f2_v3... This requires the shared vertices
         *    be duplicated in the vertex buffer.
         */
        void update_vertex_buffer(const std::vector<vec3> &vertices, bool dynamic = false);
        void update_color_buffer(const std::vector<vec3> &colors, bool dynamic = false);
        void update_normal_buffer(const std::vector<vec3> &normals, bool dynamic = false);
        void update_texcoord_buffer(const std::vector<vec2> &texcoords, bool dynamic = false);
        void update_element_buffer(const std::vector<unsigned int> &elements);
        /**
         * \brief Updates the element buffer.
         * \details This is an overload of the above update_element_buffer() method.
         * \note Each entry must have 2 (for LinesDrawable) or 3 elements (for TrianglesDrawable).
         */
        void update_element_buffer(const std::vector< std::vector<unsigned int> > &elements);

        /// \brief Disables the use of the element buffer.
        /// \details This method should be used if existing vertex data is sufficient for rendering (may require
        ///         duplicating vertex data).
        /// \note This method also releases the element buffer.
        void disable_element_buffer();

        ///@}

        std::size_t num_vertices() const { return num_vertices_; }

        /// \name Rendering
        ///@{

        /// The draw method.
        virtual void draw(const Camera *camera) const = 0;

        /// The internal draw method of this drawable.
        /// NOTE: this functions should be called when your shader program is in use,
        ///		 i.e., between glUseProgram(id) and glUseProgram(0);
        void gl_draw() const;

        /**
         * @brief Requests an update of the OpenGL buffers.
         * @details This function sets the status to trigger an update of the OpenGL buffers. The actual update does
         *      not occur immediately but is deferred to the rendering phase.
         * @note This method works for both standard drawables (no update function required) and non-standard
         *      drawable (update function required). Standard drawables include:
         *            - SurfaceMesh: "faces", "edges", "vertices", "borders", and "locks";
         *            - PointCloud: "vertices";
         *            - Graph: "edges", and "vertices",
         *            - PolyMesh: "faces:border" and "faces:interior".
         * \sa set_update_func(), Renderer::update()
         */
        void update();

        /**
         * @brief Setups how a drawable updates its rendering buffers.
         * @details This function is required by only non-standard drawables for a special visualization purpose.
         *      Rendering buffers of standard drawables attached to a model can be automatically updated and do not
         *      require this function.
         *      The drawable is considered standalone if \c Model is \c null.
         * @sa update(), Renderer::update().
         * @note For a subclassed drawable, you can also reimplement update_buffers_internal() for the same purpose.
         */
        void set_update_func(const std::function<void(Model*, Drawable*)>& func) { update_func_ = func; }

        ///@}

        /// \name Manipulation
        ///@{
        /**
         * \brief Gets the manipulator attached to this drawable.
         * \details If the drawable is part of a model, it returns the model's manipulator.
         *      Returns \c nullptr if the drawable cannot be manipulated.
         */
        Manipulator* manipulator();
        /**
         * \brief Gets the manipulator attached to this drawable.
         * \details If the drawable is part of a model, it returns the model's manipulator.
         *      Returns \c nullptr if the drawable cannot be manipulated.
         */
        const Manipulator* manipulator() const;

        /**
         * \brief Attaches a manipulator to this model.
         * \details This is used to manipulate a drawable that is not part of a model.
         */
        void set_manipulator(Manipulator* manip) { manipulator_ = manip; }

        /// \brief Returns the manipulation matrix.
        mat4 manipulated_matrix() const;
        ///@}

        /// \brief Returns the vertex array object of this drawable.
        VertexArrayObject *vao() { return vao_; }
        /// \brief Returns the vertex array object of this drawable.
        const VertexArrayObject *vao() const { return vao_; }

    protected:
        // actual update of the rendering buffers are here
        virtual void update_buffers_internal();

        void clear();

    protected:
        std::string name_;
        Model *model_;
        Box3 bbox_;

        // vertex array object
        VertexArrayObject *vao_;

        std::size_t num_vertices_;
        std::size_t num_indices_;

        bool update_needed_;
        std::function<void(Model*, Drawable*)> update_func_;

        unsigned int vertex_buffer_;
        unsigned int color_buffer_;
        unsigned int normal_buffer_;
        unsigned int texcoord_buffer_;
        unsigned int element_buffer_;

        // drawables not attached to a model can also be manipulated
        Manipulator* manipulator_;   // for manipulation
    };

}


#endif  // EASY3D_RENDERER_DRAWABLE_H
