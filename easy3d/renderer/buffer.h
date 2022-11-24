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

#ifndef EASY3D_RENDERER_BUFFER_H
#define EASY3D_RENDERER_BUFFER_H


#include <easy3d/renderer/state.h>

#include <string>

namespace easy3d {

    class Model;
    class Graph;
    class PointCloud;
    class SurfaceMesh;
    class PolyMesh;
    class Drawable;
    class PointsDrawable;
    class LinesDrawable;
    class TrianglesDrawable;

    /// \brief Functions for updating render buffers.
    /// \namespace easy3d::buffer
    namespace buffer {

        /// \name The generic API for render buffer update
        //@{
        // -------------------------------------------------------------------------------------------------------------

        /**
         * @brief Update render buffers of a drawable. Coloring determined by the drawable's coloring scheme.
         * @param model     The model.
         * @param drawable  The drawable.
         */
        void update(Model* model, Drawable* drawable);
        //@}

        /// \name Render buffer update for PointCloud
        //@{
        // PointCloud -------------------------------------------------------------------------------------------------

        /**
         * @brief Update render buffers for the default "vertices" drawable of a point cloud.
         * Coloring determined by the drawable's coloring scheme.
         * @param model     The model.
         * @param drawable  The drawable.
         */
        void update(PointCloud* model, PointsDrawable* drawable);

        /**
         * @brief Update render buffers for a vector field defined on a point cloud.
         * @param model     The model.
         * @param drawable  The drawable.
         * @param field The name of the vector field.
         * @param scale The length scale of the vectors is w.r.t. (0.01 * radius) of the model's bounding sphere.
         */
        void update(PointCloud *model, LinesDrawable *drawable, const std::string& field, float scale);
        //@}


        /// \name Render buffer update for SurfaceMesh
        //@{
        // SurfaceMesh ------------------------------------------------------------------------------------------------

        /**
         * @brief Update render buffers for the default "vertices" drawable of a surface mesh.
         * Coloring determined by the drawable's coloring scheme.
         * @param model     The model.
         * @param drawable  The drawable.
         */
         void update(SurfaceMesh* model, PointsDrawable* drawable);

        /**
         * @brief Update render buffers for the default "edges" drawable of a surface mesh.
         * Coloring determined by the drawable's coloring scheme.
         * @param model     The model.
         * @param drawable  The drawable.
         */
        void update(SurfaceMesh* model, LinesDrawable* drawable);

        /**
         * @brief Update render buffers for the default "faces" drawable of a surface mesh.
         * Coloring determined by the drawable's coloring scheme.
         * @param model     The model.
         * @param drawable  The drawable.
         */
        void update(SurfaceMesh* model, TrianglesDrawable* drawable);

        /**
         * @brief Update render buffers for a vector field defined on a surface mesh.
         * @param model     The model.
         * @param drawable  The The drawable for visualizing the vector field.
         * @param field The name of the vector field.
         * @param location The location where the vector field is defined.
         * @param scale The scale of the vector length w.r.t. half of the average edge length of the surface mesh.
         */
        void update(SurfaceMesh *model, LinesDrawable *drawable, const std::string& field, State::Location location, float scale);
        //@}


        /// \name Render buffer update for Graph
        //@{
        // Graph ------------------------------------------------------------------------------------------------------

        /**
         * @brief Update render buffers for the default "vertices" drawable of a graph.
         * Coloring determined by the drawable's coloring scheme.
         * @param model     The model.
         * @param drawable  The drawable.
         */
        void update(Graph* model, PointsDrawable* drawable);


        /**
         * @brief Update render buffers for the default "edges" drawable of a graph.
         * Coloring determined by the drawable's coloring scheme.
         * @param model     The model.
         * @param drawable  The drawable.
         */
        void update(Graph* model, LinesDrawable* drawable);
        //@}



        /// \name Render buffer update for PolyMesh
        //@{
        // PolyMesh ------------------------------------------------------------------------------------------------

        /**
         * @brief Update render buffers for the default "vertices" drawable of a polyhedral mesh.
         * Coloring determined by the drawable's coloring scheme.
         * @param model     The model.
         * @param drawable  The drawable.
         */
        void update(PolyMesh* model, PointsDrawable* drawable);

        /**
         * @brief Update render buffers for the default "edges" drawable of a polyhedral mesh.
         * Coloring determined by the drawable's coloring scheme.
         * @param model     The model.
         * @param drawable  The drawable.
         */
        void update(PolyMesh* model, LinesDrawable* drawable);

        /**
         * @brief Update render buffers for the default "faces" drawables of a polyhedral mesh.
         * @note Interior and boundary faces are rendered using two drawables. Thus, this function has an extra
         *      parameter to specify for which drawable the renderer buffers are be updated.
         * @param model     The model.
         * @param drawable  The drawable.
         * @param border  \c true for the boundary drawable and \c false for the interior drawable.
         */
        void update(PolyMesh* model, TrianglesDrawable* drawable, bool border);

        /**
         * @brief Update render buffers for a vector field defined on a polyhedral mesh.
         * @param model     The model.
         * @param drawable  The drawable for visualizing the vector field.
         * @param field The name of the vector field.
         * @param location The location where the vector field is defined.
         * @param scale The scale of the vector length w.r.t. half of the average edge length of the polyhedral mesh.
         * \note Only the vector fields on border faces, border vertices, and border edges are supported.
         */
        void update(PolyMesh *model, LinesDrawable *drawable, const std::string& field, State::Location location, float scale);
        //@}

    }   // namespaces buffer

}   // namespaces easy3d


#endif  // EASY3D_RENDERER_BUFFER_H