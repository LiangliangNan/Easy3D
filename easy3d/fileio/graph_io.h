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

#ifndef EASY3D_FILEIO_GRAPH_IO_H
#define EASY3D_FILEIO_GRAPH_IO_H


#include <string>


namespace easy3d {

    class Graph;

    /// \brief Load and save graphs in PLY format.
    /// \class GraphIO easy3d/fileio/graph_io.h
    class GraphIO
	{
	public:
        /**
         * \brief Reads a graph from file \p file_name.
         * \param file_name Input PLY path.
         * \return Newly allocated graph owned by the caller, or nullptr if loading fails.
         * \details The PLY extension is matched case-insensitively.
         */
        static Graph* load(const std::string& file_name);

        /**
         * \brief Saves \p graph to file \p file_name.
         * \details File extension determines file format (currently only PLY format is supported).
         * \param file_name Output path.
         * \param graph Graph to write.
         * \return True on success; false for an unsupported extension or I/O/format failure.
         */
        static bool	save(const std::string& file_name, const Graph* graph);
	};

    namespace io {

        /**
         * \brief Load a graph from PLY.
         * \param file_name Input path.
         * \param graph Destination graph.
         * \return True on success; false on an I/O or parse error.
         */
        bool load_ply(const std::string& file_name, Graph* graph);
        /**
         * \brief Save a graph to PLY.
         * \param file_name Output path.
         * \param graph Graph to write.
         * \param binary True to write binary PLY; false to write ASCII PLY.
         * \return True on success; false on an I/O or format error.
         */
        bool save_ply(const std::string& file_name, const Graph* graph, bool binary = true);

    } // namespace io


} // namespace easy3d

#endif // EASY3D_FILEIO_GRAPH_IO_H
