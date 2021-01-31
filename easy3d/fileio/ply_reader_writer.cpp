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

#include <easy3d/fileio/ply_reader_writer.h>
#include <easy3d/util/logging.h>

#include <cstring>
#include <unordered_map>


// This PLY reader/writer is based on rply. Here is a simple benchmark comparing various libraries for ply file i/o
// https://github.com/mhalber/ply_io_benchmark
// TODO: switch to msh_ply, or tinyply?
#include <3rd_party/rply/rply.h>


namespace easy3d {


    // \cond
    namespace io {


#define VERTEX  "vertex"
#define FACE    "face"
#define EDGE    "edge"


        std::string Element::property_statistics() const {
            std::string str;
            if (!vec3_properties.empty()) {
                str += "\n    [type]: vec3_properties";
                for (const auto &prop : vec3_properties)
                    str += ("\n         - [property name]: " + prop.name);
            }
            if (!vec2_properties.empty()) {
                str += "\n    [type]: vec2_properties";
                for (const auto &prop : vec2_properties)
                    str += ("\n         - [property name]: " + prop.name);
            }
            if (!float_properties.empty()) {
                str += "\n    [type]: float_properties";
                for (const auto &prop : float_properties)
                    str += ("\n         - [property name]: " + prop.name);
            }
            if (!int_properties.empty()) {
                str += "\n    [type]: int_properties";
                for (const auto &prop : int_properties)
                    str += ("\n         - [property name]: " + prop.name);
            }
            if (!float_list_properties.empty()) {
                str += "\n    [type]: float_list_properties";
                for (const auto &prop : float_list_properties)
                    str += ("\n         - [property name]: " + prop.name);
            }
            if (!int_list_properties.empty()) {
                str += "\n    [type]: int_list_properties";
                for (const auto &prop : int_list_properties)
                    str += ("\n         - [property name]: " + prop.name);
            }
            return str;
        }


        bool PlyWriter::is_big_endian() {
            const int i = 1;
            const char *p = reinterpret_cast<const char *>(&i);
            if (p[0] == 1) // Lowest address contains the least significant byte
                return false; // LITTLE_ENDIAN
            else
                return true; // BIG_ENDIAN
        }


        bool PlyWriter::write(
                const std::string &file_name,
                const std::vector<Element> &elements,
                const std::string &comment /* = "" */,
                bool binary /* = false */) const {
            e_ply_storage_mode mode = PLY_DEFAULT;
            if (binary) {
                if (is_big_endian())
                    mode = PLY_BIG_ENDIAN;
                else
                    mode = PLY_LITTLE_ENDIAN;
            } else
                mode = PLY_ASCII;

            p_ply ply = ply_create(file_name.c_str(), mode, nullptr, 0, nullptr);
            if (!ply) {
                LOG(ERROR) << "failed to create ply file: " << file_name;
                return false;
            }

            if (!ply_add_comment(ply, "Saved by Easy3D (Contact: liangliang.nan@gmail.com)")) {
                LOG(ERROR) << "failed to add comment";
                ply_close(ply);
                return false;
            }
            if (!comment.empty() && !ply_add_comment(ply, comment.c_str())) {
                LOG(ERROR) << "failed to add comment: " << comment;
                ply_close(ply);
                return false;
            }

            // For most scenarios (e.g., num of vertices in a face), PLY_UCHAR is enough: [0, 255].
            // But I want to store segmentation information (e.g., the vertex indices), which
            // requires a large integers, so I choose unsigned int.
            e_ply_type length_type = PLY_UINT32;

            for (std::size_t i = 0; i < elements.size(); ++i) {
                const std::string &element_name = elements[i].name;
                const std::size_t num = elements[i].num_instances;

                // add elements
                if (!ply_add_element(ply, element_name.data(), static_cast<long>(num))) {
                    LOG(ERROR) << "failed to add element: " << element_name;
                    ply_close(ply);
                    return false;
                }

                //-------------------------------------------
                // add properties

                // int list properties
                const std::vector<IntListProperty> &int_list_properties = elements[i].int_list_properties;
                for (std::size_t j = 0; j < int_list_properties.size(); ++j) {
                    const std::string &name = int_list_properties[j].name;
                    if (!ply_add_property(ply, name.data(), PLY_LIST, length_type, PLY_INT)) {
                        LOG(ERROR) << "failed to add int_list property '" << name << "' for element '" << element_name
                                   << "'";
                        ply_close(ply);
                        return false;
                    }
                }

                // float list properties
                const std::vector<FloatListProperty> &float_list_properties = elements[i].float_list_properties;
                for (std::size_t j = 0; j < float_list_properties.size(); ++j) {
                    const std::string &name = float_list_properties[j].name;
                    if (!ply_add_property(ply, name.data(), PLY_LIST, length_type, PLY_FLOAT)) {
                        LOG(ERROR) << "failed to add float_list property '" << name << "' for element '" << element_name
                                   << "'";
                        ply_close(ply);
                        return false;
                    }
                }

                // vector properties: vec3
                const std::vector<Vec3Property> &vec3_properties = elements[i].vec3_properties;
                for (std::size_t j = 0; j < vec3_properties.size(); ++j) {
                    const std::string &name = vec3_properties[j].name;

                    std::string names[3];
                    if (name == "point") {
                        names[0] = "x";
                        names[1] = "y";
                        names[2] = "z";
                    } else if (name == "normal") {
                        names[0] = "nx";
                        names[1] = "ny";
                        names[2] = "nz";
                    } else if (name == "color") {
                        names[0] = "r";
                        names[1] = "g";
                        names[2] = "b";
                    } else {
                        names[0] = name + "_x";
                        names[1] = name + "_y";
                        names[2] = name + "_z";
                    }

                    for (std::size_t k = 0; k < 3; ++k) {
                        if (!ply_add_property(ply, names[k].data(), PLY_FLOAT, length_type, PLY_FLOAT)) {
                            LOG(ERROR) << "failed to add float property '" << names[k] << "' for element '"
                                       << element_name << "'";
                            ply_close(ply);
                            return false;
                        }
                    }
                }

                // vector properties: vec2
                const std::vector<Vec2Property> &vec2_properties = elements[i].vec2_properties;
                for (std::size_t j = 0; j < vec2_properties.size(); ++j) {
                    const std::string &name = vec2_properties[j].name;

                    std::string names[2];
                    if (name == "texcoord") {
                        names[0] = "texcoord_x";
                        names[1] = "texcoord_y";
                    } else {
                        names[0] = name + "_x";
                        names[1] = name + "_y";
                    }

                    for (std::size_t k = 0; k < 2; ++k) {
                        if (!ply_add_property(ply, names[k].data(), PLY_FLOAT, length_type, PLY_FLOAT)) {
                            LOG(ERROR) << "failed to add float property '" << names[k] << "' for element '"
                                       << element_name << "'";
                            ply_close(ply);
                            return false;
                        }
                    }
                }

                const std::vector<FloatProperty> &float_properties = elements[i].float_properties;
                for (std::size_t j = 0; j < float_properties.size(); ++j) {
                    const std::string &name = float_properties[j].name;
                    if (!ply_add_property(ply, name.data(), PLY_FLOAT, length_type, PLY_FLOAT)) {
                        LOG(ERROR) << "failed to add float property '" << name << "' for element '" << element_name
                                   << "'";
                        ply_close(ply);
                        return false;
                    }
                }

                const std::vector<IntProperty> &int_properties = elements[i].int_properties;
                for (std::size_t j = 0; j < int_properties.size(); ++j) {
                    const std::string &name = int_properties[j].name;
                    if (!ply_add_property(ply, name.data(), PLY_INT, length_type, PLY_INT)) {
                        LOG(ERROR) << "failed to add int property '" << name << "' for element '" << element_name
                                   << "'";
                        ply_close(ply);
                        return false;
                    }
                }
            }

            // write output header
            if (!ply_write_header(ply))
                return false;

            for (std::size_t i = 0; i < elements.size(); ++i) {
                const std::size_t num = elements[i].num_instances;

                for (std::size_t j = 0; j < num; ++j) {
                    const std::vector<IntListProperty> &int_list_properties = elements[i].int_list_properties;
                    for (std::size_t k = 0; k < int_list_properties.size(); ++k) {
                        const std::vector<int> &values = int_list_properties[k][j];
                        ply_write(ply, static_cast<double>(values.size()));
                        for (std::size_t m = 0; m < values.size(); ++m)
                            ply_write(ply, values[m]);
                    }

                    const std::vector<FloatListProperty> &float_list_properties = elements[i].float_list_properties;
                    for (std::size_t k = 0; k < float_list_properties.size(); ++k) {
                        const std::vector<float> &values = float_list_properties[k][j];
                        ply_write(ply, static_cast<double>(values.size()));
                        for (std::size_t m = 0; m < values.size(); ++m)
                            ply_write(ply, static_cast<double>(values[m]));
                    }

                    const std::vector<Vec3Property> &vec3_properties = elements[i].vec3_properties;
                    for (std::size_t k = 0; k < vec3_properties.size(); ++k) {
                        const std::vector<vec3> &values = vec3_properties[k];
                        const vec3 &v = values[j];
                        ply_write(ply, static_cast<double>(v.x));
                        ply_write(ply, static_cast<double>(v.y));
                        ply_write(ply, static_cast<double>(v.z));
                    }

                    const std::vector<Vec2Property> &vec2_properties = elements[i].vec2_properties;
                    for (std::size_t k = 0; k < vec2_properties.size(); ++k) {
                        const std::vector<vec2> &values = vec2_properties[k];
                        const vec2 &v = values[j];
                        ply_write(ply, static_cast<double>(v.x));
                        ply_write(ply, static_cast<double>(v.y));
                    }

                    const std::vector<FloatProperty> &float_properties = elements[i].float_properties;
                    for (std::size_t k = 0; k < float_properties.size(); ++k) {
                        const std::vector<float> &values = float_properties[k];
                        ply_write(ply, static_cast<double>(values[j]));
                    }

                    const std::vector<IntProperty> &int_properties = elements[i].int_properties;
                    for (std::size_t k = 0; k < int_properties.size(); ++k) {
                        const std::vector<int> &values = int_properties[k];
                        ply_write(ply, static_cast<double>(values[j]));
                    }
                }
            }

            // close up, we are done
            if (!ply_close(ply)) {
                LOG(ERROR) << "failed to close the ply file: " << file_name;
                return false;
            }

            return true;
        }


        PlyReader::~PlyReader() {
            for (auto prop : list_properties_)
                delete prop;
            for (auto prop : value_properties_)
                delete prop;
        }


        bool PlyReader::read(const std::string &file_name, std::vector<Element> &elements) {
            p_ply ply = ply_open(file_name.c_str(), nullptr, 0, nullptr);
            if (!ply) {
                LOG(ERROR) << "failed to open ply file: " << file_name;
                return false;
            }

            if (!ply_read_header(ply)) {
                LOG(ERROR) << "failed to read ply header";
                ply_close(ply);
                return false;
            }

            // setup callbacks
            auto callback_value_property = [](p_ply_argument argument) -> int {
                long instance_index = 0; // the index of the current element instance
                ply_get_argument_element(argument, nullptr, &instance_index);

                ValueProperty *prop_ptr = nullptr;
                ply_get_argument_user_data(argument, (void **) (&prop_ptr), nullptr);

                auto &prop = *prop_ptr;
                prop[instance_index] = ply_get_argument_value(argument);

                return 1;
            };

            auto callback_list_property = [](p_ply_argument argument) -> int {
                long instance_index = 0; // the index of the current element instance
                ply_get_argument_element(argument, nullptr, &instance_index);

                long length = 0, value_index = 0;
                ply_get_argument_property(argument, nullptr, &length, &value_index);
                if (value_index < 0 || value_index >= length) {
                    return 1;
                }

                ListProperty *prop_ptr = nullptr;
                ply_get_argument_user_data(argument, (void **) (&prop_ptr), nullptr);

                auto &prop = *prop_ptr;
                auto &entry = prop[instance_index];
                if (entry.empty())
                    entry.resize(length);
                entry[value_index] = ply_get_argument_value(argument);

                return 1;
            };

            p_ply_element element = nullptr;
            /* iterate over all elements in input file */
            while ((element = ply_get_next_element(ply, element))) {
                long num_instances = 0;
                const char *element_name = nullptr;
                ply_get_element_info(element, &element_name, &num_instances);
                if (num_instances <= 0)
                    continue;

//                if (strcmp(element_name, VERTEX) && strcmp(element_name, FACE) && strcmp(element_name, EDGE)) {
//                    LOG(ERROR) << "unknown element: " << element_name << " (ignored)";
//                    continue;
//                }

                p_ply_property property = nullptr;
                /* iterate over all properties of current element */
                while ((property = ply_get_next_property(element, property))) {
                    const char *property_name = nullptr;
                    e_ply_type type, length_type, value_type;
                    ply_get_property_info(property, &property_name, &type, &length_type, &value_type);

//                    if (strcmp(element_name, VERTEX) && strcmp(element_name, FACE) && strcmp(element_name, EDGE)) {
//                        LOG(ERROR) << "property '" << property_name << "' on unknown element '" << element_name
//                                   << "' (ignored)";
//                        continue;
//                    }

                    // It is possible to save all properties as PLY_LIST of value type double. This allows me to use
                    // the same callback function to handle all the properties. But the performance is low. So I handle
                    // list properties and value properties separately.
                    if (type == PLY_LIST) {
                        ListProperty *prop = new ListProperty(element_name, property_name);
                        prop->orig_value_type = value_type;
                        prop->resize(num_instances);
                        list_properties_.push_back(prop);
                        if (!ply_set_read_cb(ply, element_name, property_name, callback_list_property, prop, 0)) {
                            LOG(ERROR) << "failed to set callback for list property '" << property_name
                                       << "' for element '" << element_name << "'";
                            return false;
                        }
                    } else {
                        ValueProperty *prop = new ValueProperty(element_name, property_name);
                        prop->orig_value_type = type;
                        prop->resize(num_instances);
                        value_properties_.push_back(prop);
                        if (!ply_set_read_cb(ply, element_name, property_name, callback_value_property, prop, 0)) {
                            LOG(ERROR) << "failed to set callback for property '" << property_name << "' for element '"
                                       << element_name << "'";
                            return false;
                        }
                    }

                }
            }

            if (!ply_read(ply)) {
                LOG(ERROR) << "error occurred while parsing ply file";
                ply_close(ply);
                return false;
            }

            ply_close(ply);

            // create the elements
            collect_elements(elements);

            // cleaning
            for (auto prop : list_properties_) delete prop;
            list_properties_.clear();
            for (auto prop : value_properties_) delete prop;
            value_properties_.clear();

            return (elements.size() > 0 && elements[0].num_instances > 0);
        }


        std::size_t PlyReader::num_instances(const std::string &file_name, const std::string &name) {
            p_ply ply = ply_open(file_name.c_str(), nullptr, 0, nullptr);
            if (!ply) {
                LOG(ERROR) << "failed to open ply file: " << file_name;
                return 0;
            }

            if (!ply_read_header(ply)) {
                LOG(ERROR) << "failed to read ply header";
                ply_close(ply);
                return 0;
            }

            p_ply_element element = nullptr;
            /* iterate over all elements in input file */
            while ((element = ply_get_next_element(ply, element))) {
                long num_element = 0;
                const char *element_name = nullptr;
                ply_get_element_info(element, &element_name, &num_element);

                if (!strcmp(element_name, name.c_str())) {
                    if (num_element > 0) {
                        ply_close(ply);
                        return static_cast<std::size_t>(num_element);
                    }
                }
            }
            ply_close(ply);
            return 0;
        }


        namespace details {

            template<typename VT_Input, typename VT_Output>
            inline void convert(const GenericProperty<VT_Input> &input, GenericProperty<VT_Output> &output) {
                output.resize(input.size());
                output.name = input.name;
                for (std::size_t i = 0; i < input.size(); ++i) {
                    output[i] = static_cast<VT_Output>(input[i]);
                }
            }

            template<typename VT_Input, typename VT_Output>
            inline void convert(const GenericProperty<std::vector<VT_Input> > &input,
                                GenericProperty<std::vector<VT_Output> > &output) {
                output.resize(input.size());
                output.name = input.name;
                for (std::size_t i = 0; i < input.size(); ++i) {
                    const auto &v_in = input[i];
                    auto &v_out = output[i];
                    v_out.resize(v_in.size());
                    for (std::size_t j = 0; j < v_in.size(); ++j)
                        v_out[j] = static_cast<VT_Output>(v_in[j]);
                }
            }


            template<typename PropertyT>
            inline bool
            extract_named_property(std::vector<PropertyT> &properties, PropertyT &wanted, const std::string &name) {
                typename std::vector<PropertyT>::iterator it = properties.begin();
                for (; it != properties.end(); ++it) {
                    const PropertyT &property = *it;
                    if (property.name == name) {
                        wanted = property;
                        properties.erase(it);
                        return true;
                    }
                }
                return false;
            }

            template<typename PropertyT>
            inline bool extract_vector_property(std::vector<PropertyT> &properties,
                                                const std::string &x_name, const std::string &y_name,
                                                const std::string &z_name,
                                                Vec3Property &prop) {
                PropertyT x_coords, y_coords, z_coords;
                if (details::extract_named_property(properties, x_coords, x_name) &&
                    details::extract_named_property(properties, y_coords, y_name) &&
                    details::extract_named_property(properties, z_coords, z_name)) {
                    std::size_t num = x_coords.size();
                    prop.resize(num);
                    for (std::size_t j = 0; j < num; ++j)
                        prop[j] = vec3(
                                static_cast<float>(x_coords[j]),
                                static_cast<float>(y_coords[j]),
                                static_cast<float>(z_coords[j])
                        );
                    return true;
                } else
                    return false;
            }

            template<typename PropertyT>
            inline bool extract_vector_property(std::vector<PropertyT> &properties,
                                                const std::string &x_name, const std::string &y_name,
                                                Vec2Property &prop) {
                PropertyT x_coords, y_coords;
                if (details::extract_named_property(properties, x_coords, x_name) &&
                    details::extract_named_property(properties, y_coords, y_name) ) {
                    std::size_t num = x_coords.size();
                    prop.resize(num);
                    for (std::size_t j = 0; j < num; ++j)
                        prop[j] = vec2(
                                static_cast<float>(x_coords[j]),
                                static_cast<float>(y_coords[j])
                        );
                    return true;
                } else
                    return false;
            }

        } // namespace details


        void PlyReader::collect_elements(std::vector<Element> &elements) const {
            elements.clear();

            // collect all element names and num of instances
            std::unordered_map<std::string, std::size_t> element_sizes;
            for (auto prop : list_properties_) {
                if (element_sizes.find(prop->element_name) == element_sizes.end())
                    element_sizes[prop->element_name] = prop->size();
            }
            for (auto prop : value_properties_) {
                if (element_sizes.find(prop->element_name) == element_sizes.end())
                    element_sizes[prop->element_name] = prop->size();
            }

            // create all the elements
            for (const auto& element : element_sizes)
                elements.emplace_back(Element(element.first, element.second));

            // a mapping from element name to its pointer
            std::unordered_map<std::string, Element*> name_to_element;
            for (auto& element : elements)
                name_to_element[element.name] = &element;

            for (auto prop : list_properties_) {
                Element *element = name_to_element[prop->element_name];

                e_ply_type type = e_ply_type(prop->orig_value_type);
                if (type == PLY_FLOAT || type == PLY_DOUBLE || type == PLY_FLOAT32 || type == PLY_FLOAT64) {
                    FloatListProperty values;
                    details::convert(*prop, values);
                    element->float_list_properties.push_back(values);
                } else { // must be one of the following integer types:
                    //       PLY_INT8, PLY_UINT8, PLY_INT16, PLY_UINT16, PLY_INT32, PLY_UINT32,
                    //       PLY_CHAR, PLY_UCHAR, PLY_SHORT, PLY_USHORT, PLY_INT, PLY_UINT
                    IntListProperty values;
                    details::convert(*prop, values);
                    element->int_list_properties.push_back(values);
                }
            }

            for (auto prop : value_properties_) {
                Element *element = name_to_element[prop->element_name];

                e_ply_type type = e_ply_type(prop->orig_value_type);
                if (type == PLY_FLOAT || type == PLY_DOUBLE || type == PLY_FLOAT32 || type == PLY_FLOAT64) {
                    FloatProperty values;
                    details::convert(*prop, values);
                    element->float_properties.push_back(values);
                } else { // must be one of the following integer types:
                    //       PLY_INT8, PLY_UINT8, PLY_INT16, PLY_UINT16, PLY_INT32, PLY_UINT32,
                    //       PLY_CHAR, PLY_UCHAR, PLY_SHORT, PLY_USHORT, PLY_INT, PLY_UINT
                    IntProperty values;
                    details::convert(*prop, values);
                    element->int_properties.push_back(values);
                }
            }

            // extract some standard vec3 properties, e.g., points, normals, colors, texture coords
            for (auto &element : elements) {
                Vec3Property prop_point("point");
                if (details::extract_vector_property(element.float_properties, "x", "y", "z", prop_point) ||
                    details::extract_vector_property(element.float_properties, "X", "Y", "Z", prop_point)) {
                    element.vec3_properties.push_back(prop_point);
                }

                Vec2Property prop_texcoord("texcoord");
                if (details::extract_vector_property(element.float_properties, "texcoord_x", "texcoord_y", prop_texcoord)) {
                    element.vec2_properties.push_back(prop_texcoord);
                }

                Vec3Property prop_normal("normal");
                if (details::extract_vector_property(element.float_properties, "nx", "ny", "nz", prop_normal)) {
                    element.vec3_properties.push_back(prop_normal);
                    // check if the normals are normalized
                    if (!prop_normal.empty()) {
                        const float len = length(prop_normal[0]);
                        LOG_IF(std::abs(1.0 - len) > epsilon<float>(), WARNING)
                                        << "normals (defined on element '" << element.name
                                        << "') not normalized (length of the first normal vector is " << len << ")";
                    }
                }

                Vec3Property prop_color("color");
                if (details::extract_vector_property(element.float_properties, "r", "g", "b", prop_color))
                    element.vec3_properties.push_back(prop_color);
                else if (details::extract_vector_property(element.int_properties, "red", "green", "blue", prop_color) ||
                         details::extract_vector_property(element.int_properties, "diffuse_red", "diffuse_green",
                                                          "diffuse_blue", prop_color)) {
                    for (std::size_t i = 0; i < prop_color.size(); ++i)
                        prop_color[i] /= 255.0f;
                    element.vec3_properties.push_back(prop_color);
                }

                // "alpha" property is stored separately (if exists)
                FloatProperty prop_alpha("alpha");
                if (details::extract_named_property(element.float_properties, prop_alpha, "a"))
                    element.float_properties.push_back(prop_alpha);

                else { // might be in Int format
                    IntProperty temp("alpha");
                    if (details::extract_named_property(element.int_properties, temp, "alpha")) {
                        prop_alpha.resize(temp.size());
                        for (std::size_t i = 0; i < prop_alpha.size(); ++i)
                            prop_alpha[i] = temp[i] / 255.0f;
                        element.float_properties.push_back(prop_alpha);
                    }
                }
            }
        }

    } // namespace io
    // \endcond

} // namespace easy3d
