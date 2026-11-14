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

#include <easy3d/algo/surface_mesh_ambient_occlusion.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <easy3d/util/logging.h>

#include <3rd_party/opcode/Opcode.h>

using namespace Opcode;
using namespace IceMaths;


namespace {

    float radical_inverse_base2(unsigned int bits) {
        bits = (bits << 16u) | (bits >> 16u);
        bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
        bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
        bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
        bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
        return float(bits) * 2.3283064365386963e-10f;
    }

    // Owns the Opcode mesh interface and the AABB tree built from it. The arrays
    // passed to the interface must outlive the model, so both are kept here.
    class AabbTree {
    public:
        explicit AabbTree(const easy3d::SurfaceMesh *mesh) {
            const auto &points = mesh->points();
            vertices_.resize(points.size());
            for (std::size_t i = 0; i < points.size(); ++i)
                vertices_[i].Set(points[i]);

            // Degenerate triangles cannot be ray cast and only add noise to the
            // tree, so they are not inserted.
            std::vector<std::array<udword, 3> > triangles;
            triangles.reserve(mesh->n_faces());
            for (auto f : mesh->faces()) {
                udword ids[3] = {0, 0, 0};
                unsigned int count = 0;
                for (auto v : mesh->vertices(f)) {
                    if (count < 3)
                        ids[count] = static_cast<udword>(v.idx());
                    ++count;
                }
                if (count != 3 || ids[0] == ids[1] || ids[1] == ids[2] || ids[2] == ids[0])
                    continue;
                triangles.push_back({ids[0], ids[1], ids[2]});
            }
            if (triangles.empty())
                throw std::runtime_error("ambient occlusion: the mesh has no valid triangles");

            index_.resize(triangles.size());
            for (std::size_t i = 0; i < triangles.size(); ++i)
                index_[i] = IndexedTriangle(triangles[i][0], triangles[i][1], triangles[i][2]);

            interface_.reset(new MeshInterface());
            interface_->SetNbTriangles(static_cast<udword>(triangles.size()));
            interface_->SetNbVertices(static_cast<udword>(points.size()));
            interface_->SetPointers(index_.data(), vertices_.data());

            BuildSettings settings;
            settings.mLimit = 1;
            settings.mRules = SPLIT_SPLATTER_POINTS | SPLIT_GEOM_CENTER;

            OPCODECREATE data;
            data.mIMesh = interface_.get();
            data.mCanRemap = false;
            data.mKeepOriginal = false;
            data.mNoLeaf = true;
            data.mQuantized = true;
            data.mSettings = settings;

            model_.reset(new Opcode::Model());
            if (!model_->Build(data)) {
                model_.reset();
                throw std::runtime_error("ambient occlusion: failed building the AABB tree");
            }
        }

        ~AabbTree() = default;

        const Opcode::Model &model() const { return *model_; }

        AabbTree(const AabbTree &) = delete;
        AabbTree &operator=(const AabbTree &) = delete;

    private:
        std::vector<Point> vertices_;
        std::vector<IndexedTriangle> index_;
        std::unique_ptr<MeshInterface> interface_;
        std::unique_ptr<Opcode::Model> model_;
    };

    // Casts the hemisphere rays of the vertices in [begin, end). Each thread owns
    // its collider, and the ray sequence of a vertex depends only on its index, so
    // the result does not depend on how the vertices are distributed.
    void bake_range(const easy3d::SurfaceMesh &mesh,
                    const Opcode::Model &model,
                    const std::vector<easy3d::SurfaceMesh::Vertex> &vertices,
                    easy3d::SurfaceMesh::VertexProperty<float> &ao,
                    const std::vector<easy3d::vec3> &normals,
                    int sample_count, float radius, float bias,
                    std::size_t begin, std::size_t end) {
        RayCollider collider;
        collider.SetClosestHit(true);
        collider.SetCulling(false); // two-sided surfaces still occlude
        collider.SetMaxDist(radius);

        Point origin, direction;
        const float two_pi = 6.28318530718f;
        for (std::size_t i = begin; i < end; ++i) {
            const auto v = vertices[i];
            const easy3d::vec3 p = mesh.points()[v.idx()];
            const easy3d::vec3 n = normals[v.idx()];
            if (easy3d::dot(n, n) < 1e-12f) {
                ao[v] = 1.0f;
                continue;
            }

            easy3d::vec3 tangent = easy3d::cross(n, std::abs(n.y) < 0.9f ? easy3d::vec3(0, 1, 0)
                                                                          : easy3d::vec3(1, 0, 0));
            if (easy3d::dot(tangent, tangent) < 1e-12f)
                tangent = easy3d::cross(n, easy3d::vec3(0, 0, 1));
            tangent = easy3d::normalize(tangent);
            const easy3d::vec3 bitangent = easy3d::cross(n, tangent);

            origin.Set(p + n * bias);
            // A per-vertex rotation hides the aligned structure of the sample set.
            const float rotation = two_pi * radical_inverse_base2(v.idx() * 2654435761u);

            int visible = 0;
            for (int s = 0; s < sample_count; ++s) {
                const float u1 = (float(s) + 0.5f) / float(sample_count);
                const float u2 = radical_inverse_base2(static_cast<unsigned int>(s));
                const float phi = two_pi * u2 + rotation;
                const float r = std::sqrt(u1);
                const easy3d::vec3 dir = tangent * (r * std::cos(phi))
                                       + bitangent * (r * std::sin(phi))
                                       + n * std::sqrt(std::max(0.0f, 1.0f - u1));
                direction.Set(dir);
                // Collide() reports whether the query ran, not whether the ray hit a
                // triangle; GetNbIntersections() is the in/out test.
                collider.Collide(Ray(origin, direction), model);
                if (collider.GetNbIntersections() == 0)
                    ++visible;
            }
            ao[v] = float(visible) / float(sample_count);
        }
    }

} // namespace


namespace easy3d {

    SurfaceMesh::VertexProperty<float>
    SurfaceMeshAmbientOcclusion::bake(SurfaceMesh *mesh) {
        return bake(mesh, Parameters());
    }


    SurfaceMesh::VertexProperty<float>
    SurfaceMeshAmbientOcclusion::bake(SurfaceMesh *mesh, const Parameters &parameters) {
        if (!mesh)
            throw std::invalid_argument("ambient occlusion: null mesh");
        if (!mesh->is_triangle_mesh())
            throw std::invalid_argument("ambient occlusion: the mesh is not a triangle mesh");
        if (mesh->n_vertices() == 0 || mesh->n_faces() == 0)
            throw std::invalid_argument("ambient occlusion: the mesh has no geometry");
        if (!(parameters.radius_ratio > 0.0f) || !std::isfinite(parameters.radius_ratio))
            throw std::invalid_argument("ambient occlusion: the radius ratio must be positive");
        if (parameters.sample_count < 1)
            throw std::invalid_argument("ambient occlusion: the sample count must be positive");
        if (!mesh->is_triangle_mesh())
            throw std::invalid_argument("ambient occlusion: input is not a triangle mesh");
            // LOG(WARNING) << "input is not a triangle mesh";

        const float radius = std::max(mesh->bounding_box().radius() * parameters.radius_ratio, 1e-6f);
        const float bias = std::max(parameters.bias_ratio, 0.0f) * radius;
        const int samples = std::min(parameters.sample_count, 4096);

        mesh->update_vertex_normals();
        auto normals = mesh->get_vertex_property<vec3>("v:normal");
        std::vector<vec3> normals_by_index(mesh->vertices_size());
        for (auto v : mesh->vertices())
            normals_by_index[v.idx()] = normals[v];

        std::vector<SurfaceMesh::Vertex> vertices;
        vertices.reserve(mesh->n_vertices());
        for (auto v : mesh->vertices())
            vertices.push_back(v);

        auto ao = mesh->vertex_property<float>("v:ao", 1.0f);

        try {
            AabbTree tree(mesh);
            // Validate the collider once: a configuration error must not be raised
            // from inside a worker thread.
            {
                RayCollider probe;
                probe.SetClosestHit(true);
                probe.SetCulling(false);
                probe.SetMaxDist(radius);
                const char *message = probe.ValidateSettings();
                if (message)
                    throw std::runtime_error(std::string("ambient occlusion: ") + message);
            }

            unsigned int threads = parameters.thread_count;
            if (threads == 0)
                threads = std::max(std::thread::hardware_concurrency(), 1u);
            threads = static_cast<unsigned int>(std::min<std::size_t>(threads, vertices.size()));

            if (threads <= 1)
                bake_range(*mesh, tree.model(), vertices, ao, normals_by_index,
                           samples, radius, bias, 0, vertices.size());
            else {
                std::vector<std::thread> workers;
                workers.reserve(threads - 1);
                const std::size_t chunk = (vertices.size() + threads - 1) / threads;
                for (unsigned int t = 1; t < threads; ++t) {
                    const std::size_t begin = std::min(chunk * t, vertices.size());
                    const std::size_t end = std::min(begin + chunk, vertices.size());
                    workers.emplace_back(bake_range, std::cref(*mesh), std::cref(tree.model()),
                                         std::cref(vertices), std::ref(ao), std::cref(normals_by_index),
                                         samples, radius, bias, begin, end);
                }
                bake_range(*mesh, tree.model(), vertices, ao, normals_by_index,
                           samples, radius, bias, 0, std::min(chunk, vertices.size()));
                for (auto &worker : workers)
                    worker.join();
            }
        }
        catch (const std::exception &e) {
            LOG(ERROR) << e.what();
            throw;
        }

        return ao;
    }


    void SurfaceMeshAmbientOcclusion::colorize(SurfaceMesh *mesh,
                                               const SurfaceMesh::VertexProperty<float> &ao,
                                               const vec3 &color) {
        if (!mesh)
            throw std::invalid_argument("ambient occlusion: null mesh");
        if (!ao)
            throw std::invalid_argument("ambient occlusion: invalid occlusion property");

        auto colors = mesh->vertex_property<vec3>("v:color");
        for (auto v : mesh->vertices()) {
            const float visibility = std::max(0.0f, std::min(ao[v], 1.0f));
            colors[v] = color * visibility;
        }
    }

} // namespace easy3d
