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

#include "../mapple_viewer.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <limits>
#include <string>
#include <typeinfo>
#include <vector>

#include <easy3d/core/constant.h>
#include <easy3d/core/graph.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/renderer/drawable.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/resource.h>

#include <3rd_party/imgui/imgui.h>

using namespace easy3d;

namespace {

// Mesh bookkeeping (and the selection display) are properties too, but editing them
// breaks the model or the selection view. They are listed only on request.
const std::vector<std::string> &internal_properties() {
    static const std::vector<std::string> names = {
            "v:point", "v:connectivity", "v:deleted", "v:locked",
            "f:connectivity", "f:deleted", "f:triangle_range",
            "e:deleted", "h:connectivity",
            "v:select", "f:select"
    };
    return names;
}

bool is_internal(const std::string &name) {
    const std::vector<std::string> &names = internal_properties();
    return std::find(names.begin(), names.end(), name) != names.end();
}

// The types a property can have in this app; a missing property reports void.
std::string type_name(const std::type_info &info) {
    if (info == typeid(void)) return "void";
    if (info == typeid(float)) return "float";
    if (info == typeid(double)) return "double";
    if (info == typeid(int)) return "int";
    if (info == typeid(unsigned int)) return "unsigned int";
    if (info == typeid(std::size_t)) return "std::size_t";
    if (info == typeid(bool)) return "bool";
    if (info == typeid(char)) return "char";
    if (info == typeid(unsigned char)) return "unsigned char";
    if (info == typeid(vec2)) return "vec2";
    if (info == typeid(vec3)) return "vec3";
    return info.name();
}

const char *location_prefix(State::Location location) {
    switch (location) {
        case State::VERTEX: return "v:";
        case State::FACE: return "f:";
        case State::EDGE: return "e:";
        case State::HALFEDGE: return "h:";
    }
    return "v:";
}

const char *location_name(State::Location location) {
    switch (location) {
        case State::VERTEX: return "vertex";
        case State::FACE: return "face";
        case State::EDGE: return "edge";
        case State::HALFEDGE: return "halfedge";
    }
    return "vertex";
}

// The numeric types a property can be converted to. vec2/vec3 properties are colors
// or texture coordinates, not data, and are deliberately not convertible.
const std::vector<std::string> &conversion_types() {
    static const std::vector<std::string> names = {
            "float", "double", "int", "unsigned int", "std::size_t", "bool", "char", "unsigned char"
    };
    return names;
}

bool is_convertible(const std::string &type) {
    const std::vector<std::string> &names = conversion_types();
    return std::find(names.begin(), names.end(), type) != names.end();
}

// Segmentation values see one random color per segment; float scalar fields are
// mapped through the rainbow colormap.
bool is_colorable(State::Location location, const std::string &type) {
    if (location != State::VERTEX && location != State::FACE)
        return false;
    return type == "int" || type == "unsigned int" || type == "std::size_t" ||
           type == "bool" || type == "unsigned char" || type == "float";
}

std::string property_type(Model *model, State::Location location, const std::string &name) {
    if (auto *mesh = dynamic_cast<SurfaceMesh *>(model)) {
        switch (location) {
            case State::VERTEX: return type_name(mesh->get_vertex_property_type(name));
            case State::FACE: return type_name(mesh->get_face_property_type(name));
            case State::EDGE: return type_name(mesh->get_edge_property_type(name));
            case State::HALFEDGE: return type_name(mesh->get_halfedge_property_type(name));
        }
    } else if (auto *graph = dynamic_cast<Graph *>(model)) {
        if (location == State::VERTEX)
            return type_name(graph->get_vertex_property_type(name));
        if (location == State::EDGE)
            return type_name(graph->get_edge_property_type(name));
    } else if (auto *cloud = dynamic_cast<PointCloud *>(model)) {
        if (location == State::VERTEX)
            return type_name(cloud->get_vertex_property_type(name));
    }
    return "void";
}

std::vector<std::string> property_names(Model *model, State::Location location) {
    if (auto *mesh = dynamic_cast<SurfaceMesh *>(model)) {
        switch (location) {
            case State::VERTEX: return mesh->vertex_properties();
            case State::FACE: return mesh->face_properties();
            case State::EDGE: return mesh->edge_properties();
            case State::HALFEDGE: return mesh->halfedge_properties();
        }
    } else if (auto *graph = dynamic_cast<Graph *>(model)) {
        if (location == State::VERTEX) return graph->vertex_properties();
        if (location == State::EDGE) return graph->edge_properties();
    } else if (auto *cloud = dynamic_cast<PointCloud *>(model)) {
        if (location == State::VERTEX) return cloud->vertex_properties();
    }
    return {};
}

bool remove_property(Model *model, State::Location location, const std::string &name) {
    bool removed = false;
    if (auto *mesh = dynamic_cast<SurfaceMesh *>(model)) {
        switch (location) {
            case State::VERTEX: removed = mesh->remove_vertex_property(name); break;
            case State::FACE: removed = mesh->remove_face_property(name); break;
            case State::EDGE: removed = mesh->remove_edge_property(name); break;
            case State::HALFEDGE: removed = mesh->remove_halfedge_property(name); break;
        }
    } else if (auto *graph = dynamic_cast<Graph *>(model)) {
        if (location == State::VERTEX)
            removed = graph->remove_vertex_property(name);
        else if (location == State::EDGE)
            removed = graph->remove_edge_property(name);
    } else if (auto *cloud = dynamic_cast<PointCloud *>(model)) {
        if (location == State::VERTEX)
            removed = cloud->remove_vertex_property(name);
    }

    if (removed)
        LOG(INFO) << location_name(location) << " property '" << name << "' removed from '"
                  << file_system::simple_name(model->name()) << "'";
    else
        LOG(WARNING) << "failed removing " << location_name(location) << " property '" << name
                     << "' from '" << file_system::simple_name(model->name()) << "'";
    return removed;
}

bool rename_property(Model *model, State::Location location,
                     const std::string &old_name, const std::string &new_name) {
    bool renamed = false;
    if (auto *mesh = dynamic_cast<SurfaceMesh *>(model)) {
        switch (location) {
            case State::VERTEX: renamed = mesh->rename_vertex_property(old_name, new_name); break;
            case State::FACE: renamed = mesh->rename_face_property(old_name, new_name); break;
            case State::EDGE: renamed = mesh->rename_edge_property(old_name, new_name); break;
            case State::HALFEDGE: renamed = mesh->rename_halfedge_property(old_name, new_name); break;
        }
    } else if (auto *graph = dynamic_cast<Graph *>(model)) {
        if (location == State::VERTEX)
            renamed = graph->rename_vertex_property(old_name, new_name);
        else if (location == State::EDGE)
            renamed = graph->rename_edge_property(old_name, new_name);
    } else if (auto *cloud = dynamic_cast<PointCloud *>(model)) {
        if (location == State::VERTEX)
            renamed = cloud->rename_vertex_property(old_name, new_name);
    }

    if (renamed)
        LOG(INFO) << location_name(location) << " property '" << old_name << "' renamed to '"
                  << new_name << "'";
    else
        LOG(WARNING) << "failed renaming " << location_name(location) << " property '" << old_name
                     << "' to '" << new_name << "'";
    return renamed;
}

//---- Data type conversion -----------------------------------------------------
// The tag type selects the property family, so one implementation serves vertices,
// faces, edges and halfedges instead of a table of near-identical copies.

struct VertexTag {};
struct FaceTag {};
struct EdgeTag {};
struct HalfedgeTag {};

template <typename M, typename T>
auto get_property(M *model, VertexTag, const std::string &name)
        -> decltype(model->template get_vertex_property<T>(name)) {
    return model->template get_vertex_property<T>(name);
}

template <typename M, typename T>
auto get_property(M *model, FaceTag, const std::string &name)
        -> decltype(model->template get_face_property<T>(name)) {
    return model->template get_face_property<T>(name);
}

template <typename M, typename T>
auto get_property(M *model, EdgeTag, const std::string &name)
        -> decltype(model->template get_edge_property<T>(name)) {
    return model->template get_edge_property<T>(name);
}

template <typename M, typename T>
auto get_property(M *model, HalfedgeTag, const std::string &name)
        -> decltype(model->template get_halfedge_property<T>(name)) {
    return model->template get_halfedge_property<T>(name);
}

template <typename M, typename T>
auto add_property(M *model, VertexTag, const std::string &name)
        -> decltype(model->template add_vertex_property<T>(name)) {
    return model->template add_vertex_property<T>(name);
}

template <typename M, typename T>
auto add_property(M *model, FaceTag, const std::string &name)
        -> decltype(model->template add_face_property<T>(name)) {
    return model->template add_face_property<T>(name);
}

template <typename M, typename T>
auto add_property(M *model, EdgeTag, const std::string &name)
        -> decltype(model->template add_edge_property<T>(name)) {
    return model->template add_edge_property<T>(name);
}

template <typename M, typename T>
auto add_property(M *model, HalfedgeTag, const std::string &name)
        -> decltype(model->template add_halfedge_property<T>(name)) {
    return model->template add_halfedge_property<T>(name);
}

template <typename M>
bool remove_property(M *model, VertexTag, const std::string &name) {
    return model->remove_vertex_property(name);
}

template <typename M>
bool remove_property(M *model, FaceTag, const std::string &name) {
    return model->remove_face_property(name);
}

template <typename M>
bool remove_property(M *model, EdgeTag, const std::string &name) {
    return model->remove_edge_property(name);
}

template <typename M>
bool remove_property(M *model, HalfedgeTag, const std::string &name) {
    return model->remove_halfedge_property(name);
}

// The container refuses a name that is already in use, whatever its type, so the
// temporary name must be checked through the container's type query.
template <typename M>
bool property_exists(M *model, VertexTag, const std::string &name) {
    return model->get_vertex_property_type(name) != typeid(void);
}

template <typename M>
bool property_exists(M *model, FaceTag, const std::string &name) {
    return model->get_face_property_type(name) != typeid(void);
}

template <typename M>
bool property_exists(M *model, EdgeTag, const std::string &name) {
    return model->get_edge_property_type(name) != typeid(void);
}

template <typename M>
bool property_exists(M *model, HalfedgeTag, const std::string &name) {
    return model->get_halfedge_property_type(name) != typeid(void);
}

template <typename M, typename Tag, typename SourceType, typename TargetType>
bool convert_property_at(M *model, Tag tag, const std::string &name) {
    auto source = get_property<M, SourceType>(model, tag, name);
    if (!source)
        return false;

    // The property keeps its name: the values move to a temporary property, the old
    // one is removed, and the temporary one takes the name. The temporary name is
    // unique, so a pre-existing property is never resized or overwritten.
    const std::string base = name + "#convert";
    std::string temporary = base;
    for (int suffix = 1; property_exists(model, tag, temporary); ++suffix)
        temporary = base + "-" + std::to_string(suffix);

    auto target = add_property<M, TargetType>(model, tag, temporary);
    if (!target)
        return false;
    target.vector().assign(source.vector().begin(), source.vector().end());
    if (!remove_property(model, tag, name)) {
        LOG(WARNING) << "failed removing property '" << name << "' while converting its data type";
        return false;
    }
    target.set_name(name);
    return true;
}

template <typename M, typename Tag, typename SourceType>
bool convert_property_from(M *model, Tag tag, const std::string &name,
                           const std::string &target_type) {
    if (target_type == "float")
        return convert_property_at<M, Tag, SourceType, float>(model, tag, name);
    if (target_type == "double")
        return convert_property_at<M, Tag, SourceType, double>(model, tag, name);
    if (target_type == "int")
        return convert_property_at<M, Tag, SourceType, int>(model, tag, name);
    if (target_type == "unsigned int")
        return convert_property_at<M, Tag, SourceType, unsigned int>(model, tag, name);
    if (target_type == "std::size_t")
        return convert_property_at<M, Tag, SourceType, std::size_t>(model, tag, name);
    if (target_type == "bool")
        return convert_property_at<M, Tag, SourceType, bool>(model, tag, name);
    if (target_type == "char")
        return convert_property_at<M, Tag, SourceType, char>(model, tag, name);
    if (target_type == "unsigned char")
        return convert_property_at<M, Tag, SourceType, unsigned char>(model, tag, name);
    LOG(WARNING) << "unaccepted target data type: " << target_type;
    return false;
}

template <typename M, typename Tag>
bool convert_property_by_type(M *model, Tag tag, const std::string &name,
                              const std::string &source_type, const std::string &target_type) {
    if (source_type == target_type) {
        LOG(WARNING) << "source and target data types are identical (" << source_type << " == "
                     << target_type << ") and nothing to convert";
        return false;
    }
    if (source_type == "float")
        return convert_property_from<M, Tag, float>(model, tag, name, target_type);
    if (source_type == "double")
        return convert_property_from<M, Tag, double>(model, tag, name, target_type);
    if (source_type == "int")
        return convert_property_from<M, Tag, int>(model, tag, name, target_type);
    if (source_type == "unsigned int")
        return convert_property_from<M, Tag, unsigned int>(model, tag, name, target_type);
    if (source_type == "std::size_t")
        return convert_property_from<M, Tag, std::size_t>(model, tag, name, target_type);
    if (source_type == "bool")
        return convert_property_from<M, Tag, bool>(model, tag, name, target_type);
    if (source_type == "char")
        return convert_property_from<M, Tag, char>(model, tag, name, target_type);
    if (source_type == "unsigned char")
        return convert_property_from<M, Tag, unsigned char>(model, tag, name, target_type);
    LOG(WARNING) << "property of type '" << source_type << "' cannot be converted";
    return false;
}

// Only the locations the concrete model has are instantiated: a point cloud never
// compiles the face/halfedge branches.
bool convert_property_type(Model *model, State::Location location, const std::string &name,
                           const std::string &source_type, const std::string &target_type) {
    if (auto *mesh = dynamic_cast<SurfaceMesh *>(model)) {
        switch (location) {
            case State::VERTEX:
                return convert_property_by_type(mesh, VertexTag{}, name, source_type, target_type);
            case State::FACE:
                return convert_property_by_type(mesh, FaceTag{}, name, source_type, target_type);
            case State::EDGE:
                return convert_property_by_type(mesh, EdgeTag{}, name, source_type, target_type);
            case State::HALFEDGE:
                return convert_property_by_type(mesh, HalfedgeTag{}, name, source_type, target_type);
        }
    } else if (auto *graph = dynamic_cast<Graph *>(model)) {
        if (location == State::VERTEX)
            return convert_property_by_type(graph, VertexTag{}, name, source_type, target_type);
        if (location == State::EDGE)
            return convert_property_by_type(graph, EdgeTag{}, name, source_type, target_type);
    } else if (auto *cloud = dynamic_cast<PointCloud *>(model)) {
        if (location == State::VERTEX)
            return convert_property_by_type(cloud, VertexTag{}, name, source_type, target_type);
    }
    return false;
}

//---- Color property generation ------------------------------------------------

std::string color_name_for(State::Location location, const std::string &name) {
    const std::string prefix(location_prefix(location));
    std::string result = prefix + "color-";
    if (name.compare(0, prefix.size(), prefix) == 0)
        result += name.substr(prefix.size());
    else
        result += name;
    return result;
}

// An existing color property of the same name is reused (its values are
// regenerated); a same-named property of another type would make the typed
// allocation fail, so the colors get a suffixed name instead.
std::string color_property_name(Model *model, State::Location location, const std::string &base) {
    std::string name = base;
    for (int suffix = 1; ; ++suffix) {
        const std::string type = property_type(model, location, name);
        if (type == "void" || type == "vec3")
            return name;
        name = base + "-" + std::to_string(suffix);
    }
}

bool load_rainbow(std::vector<unsigned char> &pixels, int &width, int &height, int &comp) {
    const std::string file = resource::directory() + "/colormaps/rainbow.png";
    if (!file_system::is_file(file)) {
        LOG(ERROR) << "colormap image file not found: " << file;
        return false;
    }
    // Flip vertically so the first row is the bottom of the image; we sample its
    // middle row, so either orientation works, but this matches the Qt dialog.
    if (!ImageIO::load(file, pixels, width, height, comp, 0, true) || pixels.empty()) {
        LOG(ERROR) << "failed reading colormap image file: " << file;
        return false;
    }
    if (width < 1 || height < 1 || comp < 3) {
        LOG(ERROR) << "colormap image is not a readable RGB image: " << file;
        return false;
    }
    return true;
}

template <typename Range, typename Values>
bool scalar_range(const Range &range, Values values, float &min_value, float &max_value) {
    min_value = std::numeric_limits<float>::max();
    max_value = -std::numeric_limits<float>::max();
    for (auto e : range) {
        const float value = static_cast<float>(values[e]);
        min_value = std::min(min_value, value);
        max_value = std::max(max_value, value);
    }
    // A constant field has no meaningful colormap position.
    return !epsilon_equal(min_value, max_value, epsilon<float>());
}

template <typename Range, typename Values, typename Colors>
void map_scalars_to_colors(const Range &range, Values values, Colors colors,
                           const std::vector<unsigned char> &pixels, int width, int height, int comp,
                           float min_value, float max_value) {
    const float span = max_value - min_value;
    const std::size_t row = static_cast<std::size_t>(height / 2);
    for (auto e : range) {
        const float value = static_cast<float>(values[e]);
        // The colormap runs horizontally; clamp so the maximum stays in the last
        // column instead of reading the first pixel of the next row.
        const int column = std::min(width - 1,
                                    static_cast<int>((value - min_value) / span * (width - 1)));
        const std::size_t pixel = (row * static_cast<std::size_t>(width) + column) * comp;
        colors[e] = vec3(pixels[pixel] / 255.0f, pixels[pixel + 1] / 255.0f, pixels[pixel + 2] / 255.0f);
    }
}

template <typename M, typename FT>
bool color_vertices_by_segments(M *model, const std::string &name, std::string &created) {
    auto segments = model->template get_vertex_property<FT>(name);
    if (!segments)
        return false;
    created = color_property_name(model, State::VERTEX, color_name_for(State::VERTEX, name));
    auto colors = model->template vertex_property<vec3>(created, vec3(0, 0, 0));
    Renderer::color_from_segmentation(model, segments, colors);
    return true;
}

template <typename FT>
bool color_faces_by_segments(SurfaceMesh *mesh, const std::string &name, std::string &created) {
    auto segments = mesh->get_face_property<FT>(name);
    if (!segments)
        return false;
    created = color_property_name(mesh, State::FACE, color_name_for(State::FACE, name));
    auto colors = mesh->face_property<vec3>(created, vec3(0, 0, 0));
    Renderer::color_from_segmentation(mesh, segments, colors);
    return true;
}

template <typename M>
bool color_vertices_by_scalars(M *model, const std::string &name, std::string &created) {
    auto values = model->template get_vertex_property<float>(name);
    if (!values)
        return false;

    float min_value = 0.0f, max_value = 0.0f;
    if (!scalar_range(model->vertices(), values, min_value, max_value)) {
        LOG(ERROR) << "scalar field '" << name << "' has an invalid range (all values are "
                      "identical)";
        return false;
    }

    std::vector<unsigned char> pixels;
    int width = 0, height = 0, comp = 0;
    if (!load_rainbow(pixels, width, height, comp))
        return false;

    created = color_property_name(model, State::VERTEX, color_name_for(State::VERTEX, name));
    auto colors = model->template vertex_property<vec3>(created, vec3(0, 0, 0));
    map_scalars_to_colors(model->vertices(), values, colors, pixels, width, height, comp,
                          min_value, max_value);
    return true;
}

bool color_faces_by_scalars(SurfaceMesh *mesh, const std::string &name, std::string &created) {
    auto values = mesh->get_face_property<float>(name);
    if (!values)
        return false;

    float min_value = 0.0f, max_value = 0.0f;
    if (!scalar_range(mesh->faces(), values, min_value, max_value)) {
        LOG(ERROR) << "scalar field '" << name << "' has an invalid range (all values are "
                      "identical)";
        return false;
    }

    std::vector<unsigned char> pixels;
    int width = 0, height = 0, comp = 0;
    if (!load_rainbow(pixels, width, height, comp))
        return false;

    created = color_property_name(mesh, State::FACE, color_name_for(State::FACE, name));
    auto colors = mesh->face_property<vec3>(created, vec3(0, 0, 0));
    map_scalars_to_colors(mesh->faces(), values, colors, pixels, width, height, comp,
                          min_value, max_value);
    return true;
}

template <typename M>
bool generate_vertex_color(M *model, const std::string &type, const std::string &name,
                           std::string &created) {
    if (type == "int") return color_vertices_by_segments<M, int>(model, name, created);
    if (type == "unsigned int")
        return color_vertices_by_segments<M, unsigned int>(model, name, created);
    if (type == "std::size_t")
        return color_vertices_by_segments<M, std::size_t>(model, name, created);
    if (type == "bool") return color_vertices_by_segments<M, bool>(model, name, created);
    if (type == "unsigned char")
        return color_vertices_by_segments<M, unsigned char>(model, name, created);
    if (type == "float") return color_vertices_by_scalars(model, name, created);
    return false;
}

bool generate_face_color(SurfaceMesh *mesh, const std::string &type, const std::string &name,
                         std::string &created) {
    if (type == "int") return color_faces_by_segments<int>(mesh, name, created);
    if (type == "unsigned int") return color_faces_by_segments<unsigned int>(mesh, name, created);
    if (type == "std::size_t") return color_faces_by_segments<std::size_t>(mesh, name, created);
    if (type == "bool") return color_faces_by_segments<bool>(mesh, name, created);
    if (type == "unsigned char") return color_faces_by_segments<unsigned char>(mesh, name, created);
    if (type == "float") return color_faces_by_scalars(mesh, name, created);
    return false;
}

const char *colorable_types_hint() {
    return "the property must have a type of int, unsigned int, std::size_t, bool, "
           "unsigned char, or float";
}

// Returns the created color property, or an empty string when nothing was made.
std::string generate_color_property(Model *model, State::Location location, const std::string &name) {
    const std::string type = property_type(model, location, name);
    if (type == "void") {
        LOG(WARNING) << "property '" << name << "' does not exist";
        return {};
    }

    std::string created;
    bool success = false;
    if (auto *mesh = dynamic_cast<SurfaceMesh *>(model)) {
        if (location == State::FACE)
            success = generate_face_color(mesh, type, name, created);
        else if (location == State::VERTEX)
            success = generate_vertex_color(mesh, type, name, created);
        else
            LOG(WARNING) << "color properties can only be generated from a vertex or face property";
    } else if (auto *cloud = dynamic_cast<PointCloud *>(model)) {
        if (location == State::VERTEX)
            success = generate_vertex_color(cloud, type, name, created);
        else
            LOG(WARNING) << "color properties can only be generated from a vertex property";
    } else {
        LOG(WARNING) << "color properties can only be generated for point clouds and surface meshes";
        return {};
    }

    if (!success) {
        if (is_colorable(location, type))
            LOG(ERROR) << "generating colors from '" << name << "' failed";
        else
            LOG(WARNING) << "cannot generate colors from a property of type '" << type
                         << "': " << colorable_types_hint();
        return {};
    }

    LOG(INFO) << location_name(location) << " property '" << created << "' generated from '"
              << name << "'";
    return created;
}

std::string trim(const std::string &text) {
    std::size_t begin = 0;
    while (begin < text.size() && std::isspace(static_cast<unsigned char>(text[begin])))
        ++begin;
    std::size_t end = text.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1])))
        --end;
    return text.substr(begin, end - begin);
}

// A typed name gets the location's prefix; a prefix typed by hand is replaced, so
// "f:foo" renamed on vertices becomes "v:foo" instead of "v:f:foo".
std::string prefixed_name(const std::string &typed, State::Location location) {
    std::string result = trim(typed);
    const char *prefixes[] = {"v:", "f:", "e:", "h:"};
    for (const char *prefix : prefixes) {
        const std::string p(prefix);
        if (result.compare(0, p.size(), p) == 0) {
            result = result.substr(p.size());
            break;
        }
    }
    return location_prefix(location) + result;
}

} // namespace

//------------------------------------------------------------------------
// Window
//------------------------------------------------------------------------

void MappleViewer::open_properties_window() {
    show_properties_window_ = true;
    focus_properties_window_ = true;
    properties_model_ = current_model();
    rename_error_.clear();
}

void MappleViewer::sync_property_references(Model *model, State::Location location,
                                            const std::string &old_name, const std::string &new_name) {
    if (!model)
        return;

    // The saved appearance of the selection display restores the property too.
    auto fix_state = [&](State &state) -> bool {
        if (state.property_location() != location || state.property_name() != old_name)
            return false;
        if (new_name.empty()) {
            state.set_uniform_coloring(state.color());
            return true;
        }
        switch (state.coloring_method()) {
            case State::COLOR_PROPERTY:
                state.set_property_coloring(location, new_name);
                break;
            case State::TEXTURED:
                state.set_texture_coloring(location, new_name, state.texture(),
                                           state.texture_repeat(), state.texture_fractional_repeat());
                break;
            case State::SCALAR_FIELD: {
                const bool clamp = state.clamp_range();
                state.set_scalar_coloring(location, new_name, state.texture(),
                                          state.clamp_lower(), state.clamp_upper());
                state.set_clamp_range(clamp);
                break;
            }
            default:
                break;
        }
        return true;
    };

    if (Renderer *renderer = model->renderer()) {
        std::vector<Drawable *> drawables;
        for (const auto &drawable : renderer->points_drawables())
            drawables.push_back(drawable.get());
        for (const auto &drawable : renderer->lines_drawables())
            drawables.push_back(drawable.get());
        for (const auto &drawable : renderer->triangles_drawables())
            drawables.push_back(drawable.get());
        for (Drawable *drawable : drawables) {
            if (fix_state(*drawable))
                drawable->update();
        }
    }

    auto entry = selection_styles_.find(model);
    if (entry != selection_styles_.end())
        fix_state(entry->second.state);
}

void MappleViewer::apply_color_property(Model *model, State::Location location,
                                        const std::string &name) {
    if (!model || !model->renderer())
        return;

    Drawable *drawable = nullptr;
    if (dynamic_cast<SurfaceMesh *>(model)) {
        drawable = model->renderer()->get_triangles_drawable("faces", false);
        if (!drawable && !model->renderer()->triangles_drawables().empty())
            drawable = model->renderer()->triangles_drawables().front().get();
    } else if (dynamic_cast<PointCloud *>(model)) {
        drawable = model->renderer()->get_points_drawable("vertices", false);
        if (!drawable && !model->renderer()->points_drawables().empty())
            drawable = model->renderer()->points_drawables().front().get();
    }
    if (!drawable) {
        LOG(INFO) << "generated '" << name << "'; select it in the Rendering panel to show it";
        return;
    }

    auto entry = selection_styles_.find(model);
    if (model == current_model() && entry != selection_styles_.end() && selection_count() > 0) {
        // The selection keeps the screen; the colors become the appearance restored
        // when the selection ends.
        entry->second.state.set_property_coloring(location, name);
        refresh_selection_display(model);
    } else {
        drawable->set_property_coloring(location, name);
        drawable->update();
        update();
    }
    LOG(INFO) << "'" << name << "' shown on the '" << drawable->name() << "' drawable";
}

void MappleViewer::draw_properties_window() {
    if (!show_properties_window_)
        return;

    if (focus_properties_window_) {
        ImGui::SetNextWindowFocus();
        focus_properties_window_ = false;
    }
    // The height follows the content on first use: a model with few properties gets a
    // short window, a rich one grows until the display clamps it.
    ImGui::SetNextWindowSize(ImVec2(420.0f, 0.0f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Properties", &show_properties_window_)) {
        ImGui::End();
        return;
    }

    std::vector<Model *> available;
    for (const auto &model : models())
        available.push_back(model.get());
    if (std::find(available.begin(), available.end(), properties_model_) == available.end())
        properties_model_ = current_model();

    if (available.empty()) {
        ImGui::TextDisabled("No models.");
        ImGui::End();
        return;
    }

    const std::string preview = properties_model_ ? file_system::simple_name(properties_model_->name())
                                                  : "No model";
    // Leave room for the label on the right; a negative width would push it out.
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Model").x -
                            ImGui::GetStyle().ItemInnerSpacing.x);
    if (ImGui::BeginCombo("Model", preview.c_str())) {
        for (Model *candidate : available) {
            const bool selected = candidate == properties_model_;
            const std::string label = file_system::simple_name(candidate->name());
            if (ImGui::Selectable(label.c_str(), selected))
                properties_model_ = candidate;
            if (selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    if (!properties_model_) {
        ImGui::End();
        return;
    }
    Model *model = properties_model_;

    ImGui::TextDisabled("Right-click a property to remove, rename, convert or color by it.");
    ImGui::Checkbox("Show internal properties", &show_internal_properties_);
    ImGui::Separator();

    if (open_rename_popup_) {
        ImGui::OpenPopup("Rename Property");
        open_rename_popup_ = false;
    }

    auto context_menu = [this, model](State::Location location, const std::string &name) {
        if (!ImGui::BeginPopupContextItem("##property-actions"))
            return;

        const std::string type = property_type(model, location, name);
        const bool selection_property = name == "v:select" || name == "f:select";
        ImGui::TextDisabled("%s (%s)", name.c_str(), type.c_str());
        ImGui::Separator();

        if (ImGui::MenuItem("Remove")) {
            if (remove_property(model, location, name)) {
                // Restore the appearance saved before the selection display if the
                // property that displayed the selection is gone.
                if (selection_property && selection_styles_.count(model))
                    restore_selection_style(model);
                sync_property_references(model, location, name, std::string());
                invalidate_selection_count();
                update();
            }
        }

        if (ImGui::MenuItem("Rename...")) {
            rename_property_ = name;
            rename_location_ = location;
            rename_error_.clear();
            std::snprintf(rename_buffer_, sizeof(rename_buffer_), "%s", name.c_str());
            open_rename_popup_ = true;
        }

        if (is_convertible(type) && !selection_property) {
            if (ImGui::BeginMenu("Convert to")) {
                for (const std::string &target : conversion_types()) {
                    if (target == type)
                        continue;
                    if (ImGui::MenuItem(target.c_str())) {
                        if (convert_property_type(model, location, name, type, target))
                            LOG(INFO) << location_name(location) << " property '" << name
                                      << "' converted from '" << type << "' to '" << target << "'";
                        // The name is unchanged; refresh the drawables so buffers are
                        // rebuilt from the new data type.
                        sync_property_references(model, location, name, name);
                        invalidate_selection_count();
                        update();
                    }
                }
                ImGui::EndMenu();
            }
        } else {
            ImGui::MenuItem("Convert to", nullptr, false, false);
        }

        if (ImGui::MenuItem("Generate color property", nullptr, false, is_colorable(location, type))) {
            const std::string created = generate_color_property(model, location, name);
            if (!created.empty())
                apply_color_property(model, location, created);
        }

        ImGui::EndPopup();
    };

    auto section = [&](const char *title, State::Location location) {
        const std::vector<std::string> names = property_names(model, location);
        std::size_t visible = 0;
        for (const std::string &name : names)
            if (show_internal_properties_ || !is_internal(name))
                ++visible;

        const std::string header = std::string(title) + " (" + std::to_string(visible) + ")";
        if (!ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
            return;
        if (visible == 0) {
            ImGui::TextDisabled("No properties.");
            return;
        }

        const std::string table_id = std::string("##properties-") + title;
        if (ImGui::BeginTable(table_id.c_str(), 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
            for (const std::string &name : names) {
                if (!show_internal_properties_ && is_internal(name))
                    continue;
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::PushID(name.c_str());
                ImGui::Selectable(name.c_str());
                context_menu(location, name);
                ImGui::TableNextColumn();
                ImGui::TextDisabled("%s", property_type(model, location, name).c_str());
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
    };

    if (dynamic_cast<SurfaceMesh *>(model)) {
        section("Vertex", State::VERTEX);
        section("Face", State::FACE);
        section("Edge", State::EDGE);
        section("Halfedge", State::HALFEDGE);
    } else if (dynamic_cast<Graph *>(model)) {
        section("Vertex", State::VERTEX);
        section("Edge", State::EDGE);
    } else if (dynamic_cast<PointCloud *>(model)) {
        section("Vertex", State::VERTEX);
    }

    if (ImGui::BeginPopupModal("Rename Property", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (!properties_model_) {
            ImGui::CloseCurrentPopup();
        } else {
            ImGui::TextDisabled("%s", rename_property_.c_str());
            ImGui::SetNextItemWidth(320.0f);
            const bool entered = ImGui::InputText("##new-name", rename_buffer_, sizeof(rename_buffer_),
                                                  ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::TextDisabled("The '%s' prefix is added automatically.", location_prefix(rename_location_));
            if (!rename_error_.empty())
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", rename_error_.c_str());

            bool rename = ImGui::Button("Rename", ImVec2(100, 0)) || entered;
            ImGui::SameLine();
            const bool cancel = ImGui::Button("Cancel", ImVec2(100, 0)) ||
                                ImGui::IsKeyPressed(ImGuiKey_Escape);
            if (cancel)
                ImGui::CloseCurrentPopup();

            if (rename) {
                const std::string new_name = prefixed_name(rename_buffer_, rename_location_);
                const std::string bare = new_name.substr(2); // without the location prefix
                if (new_name == rename_property_) {
                    ImGui::CloseCurrentPopup();
                } else if (bare.empty()) {
                    rename_error_ = "The new name is empty.";
                } else if (property_type(properties_model_, rename_location_, new_name) != "void") {
                    rename_error_ = "A property with this name already exists.";
                } else if (rename_property(properties_model_, rename_location_, rename_property_, new_name)) {
                    sync_property_references(properties_model_, rename_location_, rename_property_, new_name);
                    invalidate_selection_count();
                    update();
                    ImGui::CloseCurrentPopup();
                } else {
                    rename_error_ = "Renaming failed; see the Log panel.";
                }
            }
        }
        ImGui::EndPopup();
    }

    ImGui::End();
}

