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

#ifndef EASY3D_FILEIO_PLY_READER_WRITER_H
#define EASY3D_FILEIO_PLY_READER_WRITER_H


#include <string>
#include <vector>

#include <easy3d/core/types.h>

// Todo: check happly, a header-only implementation of the .ply file format.
//       https://github.com/nmwsharp/happly

namespace easy3d {

	/**
	 * \brief File input/output functionalities.
	 * \namespace easy3d::io
	 */
	namespace io {

		/**
		 * \brief Generic property.
		 * \tparam VT The value type, e.g., int, float, string, vec3, std::vector
		 * \attention Current implementation uses 'float' --> loss in accuracy when handling large coordinates
		 * \class GenericProperty easy3d/fileio/ply_reader_writer.h
		 */
		template <typename VT>
        class GenericProperty : public std::vector<VT> {
        public:
			/**
			 * \brief Constructor to initialize the property with a name and values.
			 * \param prop_name The name of the property.
			 * \param values The values of the property.
			 */
            explicit GenericProperty(const std::string &prop_name = "", const std::vector<VT> &values = std::vector<VT>())
                    : std::vector<VT>(values), name(prop_name) {}
            std::string name;	///< The name of the property.
        };

		typedef GenericProperty<vec3>                   Vec3Property;		///< Property for 3D vectors
        typedef GenericProperty<vec2>                   Vec2Property;		///< Property for 2D vectors
		typedef GenericProperty<float>                  FloatProperty;		///< Property for scalar fields of float values
		typedef GenericProperty<int>                    IntProperty;		///< Property for scalar fields of integer values
		typedef GenericProperty< std::vector<float> >	FloatListProperty;	///< Property for properties of a list of float values
		typedef GenericProperty< std::vector<int> >     IntListProperty;	///< Property for properties of a list of integer values

		/**
		 * \brief Model element (e.g., faces, vertices, edges) with optional properties.
		 * \class Element easy3d/fileio/ply_reader_writer.h
		 */
		struct Element {
			/**
			 * \brief Constructor to initialize the element with a name and number of instances.
			 * \param elem_name The name of the element.
			 * \param n_instances The number of instances of the element.
			 */
            explicit Element(const std::string &elem_name, std::size_t n_instances = 0) : name(elem_name),
                                                                                 num_instances(n_instances) {}

            std::string name;           ///< The name of the element, e.g., "vertex", "face", "edge".
            std::size_t num_instances;  ///< The number of instances of the element.

			std::vector<Vec3Property>       vec3_properties;		///< Properties for "point", "normal", "color", and vector fields
            std::vector<Vec2Property>       vec2_properties;		///< Properties for "texcoord".
			std::vector<FloatProperty>      float_properties;		///< Properties for scalar fields of float values.
			std::vector<IntProperty>        int_properties;			///< Properties for scalar fields of integer values.
            std::vector<FloatListProperty>  float_list_properties;	///< Properties for a list of float values.
            std::vector<IntListProperty>	int_list_properties;    ///< Properties for a list of integer values.

			/**
			 * \brief Returns a string representation of the property statistics.
			 * \return A string containing the property statistics.
			 */
            std::string property_statistics() const;
		};


		/**
		 * \brief A general purpose PLY file reader.
		 * \details This class is internally used by PointCloudIO, SurfaceMeshIO, and GraphIO.
		 * Client code should use PointCloudIO, SurfaceMeshIO, and GraphIO.
		 * \class PlyReader easy3d/fileio/ply_reader_writer.h
		 */
		class PlyReader
		{
		public:
            PlyReader() = default;
            ~PlyReader();

			/**
			 * \brief Reads a PLY file and stores the model as a set of elements.
			 * \param file_name The name of the PLY file to read.
			 * \param elements The vector to store the read elements.
			 * \return True if the file was successfully read, false otherwise.
			 */
			bool read(const std::string& file_name, std::vector<Element>& elements);
			/**
			 * \brief A quick check of the number of instances of a type of element.
			 * \details The typical use is to determine if a PLY file stores a point cloud, a graph, or a surface mesh.
			 *		Internally it reads the ply file header only (without parsing the entire file).
			 * \param file_name The input file.
			 * \param element_name A string denoting the type of the element to be checked. Typical elements are
			 *		"vertex", "face", and "edge".
			 * \return The number of instances of the element.
			 */
            static std::size_t num_instances(const std::string& file_name, const std::string& element_name);

		private:
			// Collect all elements stored as general properties (in list_properties_ and value_properties_).
			// Meanwhile, convert the "list" intermediate representation into the user requested format.
			void collect_elements(std::vector<Element>& elements) const;

		private:
            // For simpler code, it is possible to save all data as properties of type PLY_LIST with value type double.
            // This can allow us to use a single callback function to handle all the properties. However, the
            // performance is not optimal. Thus, I process list properties and value properties separately.
			struct PlyProperty { 
				int orig_value_type;	// e.g., PLY_INT, PLY_FLOAT
			};

			struct ListProperty : PlyProperty, GenericProperty< std::vector<double> > {
                ListProperty(const std::string &elem_name, const std::string &prop_name)
                        : GenericProperty<std::vector<double> >(prop_name), element_name(elem_name) {}
                std::string element_name;
			};

			struct ValueProperty : PlyProperty, GenericProperty<double> {
                ValueProperty(const std::string &elem_name, const std::string &prop_name)
                        : GenericProperty<double>(prop_name), element_name(elem_name) {}
                std::string element_name;
			};

			std::vector< ListProperty* >	list_properties_;
			std::vector< ValueProperty* >	value_properties_;
		};


		/**
		 * \brief A general purpose PLY file writer.
		 * \details This class is internally used by PointCloudIO, SurfaceMeshIO, and GraphIO.
		 * Client code should use PointCloudIO, SurfaceMeshIO, and GraphIO.
		 * \class PlyWriter easy3d/fileio/ply_reader_writer.h
		 */
		class PlyWriter {
		public:
			/**
			 * \brief Saves a model stored as a set of elements to a file.
			 * \param file_name The name of the output file.
			 * \param elements The model elements to save.
			 * \param comment The comment to be written to the file.
			 * \param binary True for binary format, otherwise ASCII format.
			 * \return True if the file was successfully written, false otherwise.
			 */
            static bool write(
                    const std::string &file_name,
                    const std::vector<Element> &elements,
                    const std::string &comment = "",
                    bool binary = false
            );

		};

		/**
		 * \brief Returns the endianness of the system.
		 * \return True if the system is big-endian, false otherwise.
		 */
        bool is_big_endian();

	} // namespace io

} // namespace easy3d

#endif  // EASY3D_FILEIO_PLY_READER_WRITER_H

