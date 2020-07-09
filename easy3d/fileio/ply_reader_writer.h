/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */

#ifndef EASY3D_FILEIO_PLY_READER_WRITER_H
#define EASY3D_FILEIO_PLY_READER_WRITER_H


#include <string>
#include <vector>

#include <easy3d/core/types.h>


namespace easy3d {

	namespace io {

		// VT: the value type, e.g., int, float, string, vec3, std::vector
		template <typename VT>
        class GenericProperty : public std::vector<VT> {
        public:
            GenericProperty(const std::string &prop_name = "", const std::vector<VT> &values = std::vector<VT>())
                    : std::vector<VT>(values), name(prop_name) {}
            std::string name;
        };

		typedef GenericProperty<vec3>                   Vec3Property;
        typedef GenericProperty<vec2>                   Vec2Property;
		typedef GenericProperty<float>                  FloatProperty;
		typedef GenericProperty<int>                    IntProperty;
		typedef GenericProperty< std::vector<float> >	FloatListProperty;
		typedef GenericProperty< std::vector<int> >     IntListProperty;

		// model element (e.g., faces, vertices, edges) with optional properties
		struct Element {
            Element(const std::string &elem_name, std::size_t n_instances = 0) : name(elem_name),
                                                                                 num_instances(n_instances) {}

            std::string name;           // e.g., "vertex", "face", "edge"
            std::size_t num_instances;  // number of instances

			std::vector<Vec3Property>       vec3_properties;    // for "point", "normal", "color", and vector fields
            std::vector<Vec2Property>       vec2_properties;    // for "texcoord"
			std::vector<FloatProperty>      float_properties;	// for scalar fields of float values
			std::vector<IntProperty>        int_properties;     // for scalar fields of integer values
            std::vector<FloatListProperty>  float_list_properties;	// for properties of a list of float values
            std::vector<IntListProperty>	int_list_properties;    // for properties of a list of integer values

            std::string property_statistics() const;
		};


		// A general purpose PLY file reader
		// This class is internally used by PointCloudIO, SurfaceMeshIO, and GraphIO.
		// Client code should use PointCloudIO, SurfaceMeshIO, and GraphIO.
		class PlyReader
		{
		public:
            PlyReader() {}
            ~PlyReader();

			bool read(const std::string& file_name, std::vector<Element>& elements);

            /**
             * \brief A quick check of the number of instances of a type of element. The typical use is to determine if
             * 		  a PLY file stores a point cloud, a graph, or a surface mesh. Internally it reads the ply file
             * 		  header only (without parsing the entire file).
             * \param element_name A string denoting the type of the element to be checked. Typical elements are
             * 		  "vertex", "face", and "edge".
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

			struct ValueProperty : PlyProperty, GenericProperty< double > {
                ValueProperty(const std::string &elem_name, const std::string &prop_name)
                        : GenericProperty<double>(prop_name), element_name(elem_name) {}
                std::string element_name;
			};

			std::vector< ListProperty* >	list_properties_;
			std::vector< ValueProperty* >	value_properties_;
		};


		// A general purpose PLY file writer.
		// This class is internally used by PointCloudIO, SurfaceMeshIO, and GraphIO.
		// Client code should use PointCloudIO, SurfaceMeshIO, and GraphIO.
		class PlyWriter {
		public:
			bool write(
					const std::string& file_name,
					const std::vector<Element>& elements,
					const std::string& comment = "",
					bool binary = false
			) const;

			static bool is_big_endian();
		};

	} // namespace io

} // namespace easy3d

#endif  // EASY3D_FILEIO_PLY_READER_WRITER_H

