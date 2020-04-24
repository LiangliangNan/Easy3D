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


#include "polycube.h"
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/principal_axes.h>
#include <easy3d/util/stop_watch.h>
#include <easy3d/util/logging.h>
#include <easy3d/viewer/drawable_lines.h>

#include "../liblbfgs/include/lbfgs.h"


namespace easy3d {

    Polygonizer::Polygonizer(SurfaceMesh *mesh)
            : mesh_(mesh) {
        vertex_planarity_ = mesh_->vertex_property<float>("v:planarity");
    }


    Polygonizer::~Polygonizer(void) {
    }


    void Polygonizer::compute_planarity(int k_ring) {
        auto drawable = mesh_->get_lines_drawable("vector - f:normal");
        if (!drawable) {
            drawable = mesh_->add_lines_drawable("vector - f:normal");
            drawable->set_update_func([this](Model *m, Drawable *d) -> void {
                auto surface = dynamic_cast<SurfaceMesh *>(m);
                auto points = surface->get_vertex_property<vec3>("v:point");
                auto fprop = surface->get_face_property<vec3>("f:normal");

                // use a limited number of edge to compute the length of the vectors.
                float avg_edge_length = 0.0f;
                const int num = std::min(static_cast<unsigned int>(500), surface->n_edges());
                for (unsigned int i = 0; i < num; ++i) {
                    SurfaceMesh::Edge edge(i);
                    auto vs = surface->vertex(edge, 0);
                    auto vt = surface->vertex(edge, 1);
                    avg_edge_length += distance(points[vs], points[vt]);
                }
                avg_edge_length /= num;

                std::vector<vec3> vertices(surface->n_faces() * 2, vec3(0.0f, 0.0f, 0.0f));
                int idx = 0;
                float scale = 3.0f;
                for (auto f: surface->faces()) {
                    int size = 0;
                    for (auto v: surface->vertices(f)) {
                        vertices[idx] += points[v];
                        ++size;
                    }
                    vertices[idx] /= size;
                    vertices[idx + 1] = vertices[idx] + fprop[f] * avg_edge_length * scale;
                    idx += 2;
                }
                d->update_vertex_buffer(vertices);
            });
        }
    }


    namespace details {

        namespace normals {

            void compute_expected_normals(SurfaceMesh *mesh) {
                std::cout << "this should be replaced by the vertex normal of the seeds" << std::endl;

                auto chart = mesh->get_face_property<int>("f:chart");
                if (!chart)
                    return;

                // segment id -> vertex ids of this segments
                std::unordered_map<int, std::set<SurfaceMesh::Vertex> > vertices;
                for (auto f : mesh->faces()) {
                    const int id = chart[f];
                    for (auto v : mesh->vertices(f))
                        vertices[id].insert(v);
                }

                auto points = mesh->get_vertex_property<vec3>("v:point");
                std::unordered_map<int, vec3> normals;
                PrincipalAxes<3, float> pca;
                for (const auto &vts : vertices) {
                    pca.begin();
                    for (const auto &v : vts.second)
                        pca.add_point(points[v]);
                    pca.end();
                    const vec3 &n = pca.axis(2);// the eigen vector corresponding to the smallest eigen value
                    normals[vts.first] = normalize(n);
                }

                auto expected_normals = mesh->face_property<vec3>("f:expected_normals");
                auto fnormals = mesh->face_property<vec3>("f:normal");
                for (auto f : mesh->faces()) {
                    const int id = chart[f];
                    if (dot(fnormals[f], normals[id]) >= 0)
                        expected_normals[f] = normals[id];
                    else
                        expected_normals[f] = -normals[id];
                }
            }


            struct NormalsData {
                NormalsData(SurfaceMesh *mesh) : lambda(1.0) {
                    std::size_t n_faces = mesh->n_faces();
                    auto normals = mesh->get_face_property<vec3>("f:normal");
                    // initialize x0
                    x0.resize(n_faces * 3);
                    for (auto f : mesh->faces()) {
                        const int id = f.idx();
                        const vec3 &n = normals[f];
                        x0[id * 3] = n.x;
                        x0[id * 3 + 1] = n.y;
                        x0[id * 3 + 2] = n.z;;
                    }

                    std::size_t n_edges = mesh->n_edges();
                    // initialize nbs
                    nbs.reserve(n_edges);
                    for (auto e : mesh->edges()) {
                        if (mesh->is_boundary(e))
                            continue;

                        SurfaceMesh::Face fi = mesh->face(e, 0);
                        SurfaceMesh::Face fj = mesh->face(e, 1);
                        nbs.push_back(std::make_pair(fi.idx(), fj.idx()));
                    }
                }

                std::vector<double> x0;
                std::vector<std::pair<int, int> > nbs; // adjacent faces sharing an edge
                double lambda;
            };


            static lbfgsfloatval_t normals_evaluate(
                    void *data,
                    const lbfgsfloatval_t *x,
                    lbfgsfloatval_t *g,
                    const int N,
                    const lbfgsfloatval_t step
            ) {
                NormalsData *d = (NormalsData *) data;
                const auto &x0 = d->x0;
                const auto &nbs = d->nbs;
                const double lambda = d->lambda;

                // initialize
                lbfgsfloatval_t f = 0;
                for (int i = 0; i < N; ++i) {
                    g[i] = 0;
                }

                // deviation term
                for (int i = 0; i < N; ++i) {
                    double tmp = (x[i] - x0[i]);
                    f += tmp * tmp;
                    g[i] += 2 * tmp;
                }

                // adjacency term for function value
                double coeff = 2.0 * lambda;
                for (unsigned int k = 0; k < nbs.size(); ++k) {
                    const unsigned int i = nbs[k].first;
                    const unsigned int j = nbs[k].second;
                    const double dx = (x[i * 3] - x[j * 3]);
                    const double dy = (x[i * 3 + 1] - x[j * 3 + 1]);
                    const double dz = (x[i * 3 + 2] - x[j * 3 + 2]);
                    f += lambda * (dx * dx + dy * dy + dz * dz);

                    g[i * 3] += (coeff * dx);
                    g[j * 3] -= (coeff * dx);
                    g[i * 3 + 1] += (coeff * dy);
                    g[j * 3 + 1] -= (coeff * dy);
                    g[i * 3 + 2] += (coeff * dz);
                    g[j * 3 + 2] -= (coeff * dz);
                }

                return f;
            }

        }


#define ENCODE_EDGE_LENGTH
//#define ERROR_RELATIVE_TO_ORIGINAL_VERTICES

        namespace vertices {

            struct EdgeVector {
                SurfaceMesh::Vertex s; // the source of an edge
                SurfaceMesh::Vertex t; // the target of an edge
                // for the normal constraints: the normal of the face, so n _|_ (va - vb);
                // for the edge length constraints: the vector from s -> t
                vec3 vec;
            };


            struct VerticesData {
                VerticesData(SurfaceMesh *mesh)
                        : lambda_data(0.01), lambda_smoothness(10), lambda_edge_length(5) {
                    std::size_t num = mesh->n_vertices();

                    // initialize x0
                    x0.resize(num * 3);

#ifdef ERROR_RELATIVE_TO_ORIGINAL_VERTICES

                    if (!MeshVertexAttribute<vec3>::is_defined(mesh, "input"))
            std::cout << "Error: original vertex positions not exist" << std::endl;

        MeshVertexAttribute<vec3> vts(mesh, "input");
        FOR_EACH_VERTEX(SurfaceMesh* mesh, it) {
            int id = vid[it];
            const vec3& v = vts[it];
            x0[id * 3] = v.x;
            x0[id * 3 + 1] = v.y;
            x0[id * 3 + 2] = v.z;;
        }

#else  // error relative to the previous iteration

                    // initialize the variable to the initial vertex coordinates
                    auto points = mesh->get_vertex_property<vec3>("v:point");
                    for (auto v : mesh->vertices()) {
                        const int id = v.idx();
                        const vec3 &p = points[v];
                        x0[id * 3] = p.x;
                        x0[id * 3 + 1] = p.y;
                        x0[id * 3 + 2] = p.z;;
                    }

#endif // ERROR_RELATIVE_TO_ORIGINAL_VERTICES


                    auto normals = mesh->get_face_property<vec3>("f:normal");
                    // initialize constraints
                    normal_constrs.reserve(mesh->n_faces() * 3);
                    for (auto f : mesh->faces()) {
                        EdgeVector tri;
                        tri.vec = normals[f];
                        for (auto e : mesh->halfedges(f)) {
                            tri.s = mesh->to_vertex(e);
                            tri.t = mesh->from_vertex(e);
                            normal_constrs.push_back(tri);
                        }
                    }

#ifdef ENCODE_EDGE_LENGTH
                    for (auto e :mesh->edges()) {
                        EdgeVector esl;
                        esl.s = mesh->vertex(e, 0);
                        esl.t = mesh->vertex(e, 1);
                        esl.vec = (points[esl.s] - points[esl.t]);
                        edge_len_constrs.push_back(esl);
                    }
#endif
                }

                std::vector<double> x0;
                std::vector<EdgeVector> normal_constrs;
                std::vector<EdgeVector> edge_len_constrs;

                double lambda_data;
                double lambda_smoothness;
                double lambda_edge_length;
            };


            static lbfgsfloatval_t vertices_evaluate(
                    void *data,
                    const lbfgsfloatval_t *x,
                    lbfgsfloatval_t *g,
                    const int N,
                    const lbfgsfloatval_t step
            ) {
                VerticesData *d = (VerticesData *) data;
                const auto &x0 = d->x0;
                const auto &normal_constrs = d->normal_constrs;
                const double lambda_data = d->lambda_data;
                const double lambda_smoothness = d->lambda_smoothness;
                const double lambda_edge_length = d->lambda_edge_length;

                //////////////////////////////////////////////////////////////////////////
                // initialize
                lbfgsfloatval_t f = 0;
                for (int i = 0; i < N; ++i) {
                    g[i] = 0;
                }

                //////////////////////////////////////////////////////////////////////////
                // deviation term
                for (int i = 0; i < N; ++i) {
                    double tmp = x[i] - x0[i];
                    f += (tmp * tmp) * lambda_data;
                    g[i] += 2.0 * tmp * lambda_data;
                }

                //////////////////////////////////////////////////////////////////////////
                // normal consistency term
                double coeff = 2.0 * lambda_smoothness;
                for (unsigned int k = 0; k < normal_constrs.size(); ++k) {
                    const EdgeVector &tri = normal_constrs[k];
                    const vec3 &n = tri.vec;
                    unsigned int i = tri.s.idx();
                    unsigned int j = tri.t.idx();
                    double dot_prod =
                            (x[i * 3] - x[j * 3]) * n.x +
                            (x[i * 3 + 1] - x[j * 3 + 1]) * n.y +
                            (x[i * 3 + 2] - x[j * 3 + 2]) * n.z;

                    f += lambda_smoothness * dot_prod * dot_prod;

                    g[i * 3] += (coeff * dot_prod * n.x);
                    g[j * 3] -= (coeff * dot_prod * n.x);
                    g[i * 3 + 1] += (coeff * dot_prod * n.y);
                    g[j * 3 + 1] -= (coeff * dot_prod * n.y);
                    g[i * 3 + 2] += (coeff * dot_prod * n.z);
                    g[j * 3 + 2] -= (coeff * dot_prod * n.z);
                }

                //////////////////////////////////////////////////////////////////////////

#ifdef ENCODE_EDGE_LENGTH
                const std::vector<EdgeVector> &edge_len_constrs = d->edge_len_constrs;

                for (int k = 0; k < edge_len_constrs.size(); ++k) {
                    const EdgeVector &el = edge_len_constrs[k];
                    unsigned int i = el.s.idx();
                    unsigned int j = el.t.idx();
                    double dx = x[i * 3] - x[j * 3] - el.vec.x;
                    double dy = x[i * 3 + 1] - x[j * 3 + 1] - el.vec.y;
                    double dz = x[i * 3 + 2] - x[j * 3 + 2] - el.vec.z;

                    f += lambda_edge_length * (dx * dx + dy * dy + dz * dz);

                    g[i * 3] += 2 * lambda_edge_length * dx;
                    g[i * 3 + 1] += 2 * lambda_edge_length * dy;
                    g[i * 3 + 2] += 2 * lambda_edge_length * dz;

                    g[j * 3] -= 2 * lambda_edge_length * dx;
                    g[j * 3 + 1] -= 2 * lambda_edge_length * dy;
                    g[j * 3 + 2] -= 2 * lambda_edge_length * dz;
                }
#endif
                return f;
            }

        }


        static int progress(
                void *data,
                const lbfgsfloatval_t *x,
                const lbfgsfloatval_t *g,
                const lbfgsfloatval_t fx,
                const lbfgsfloatval_t xnorm,
                const lbfgsfloatval_t gnorm,
                const lbfgsfloatval_t step,
                int n,
                int k,
                int ls
        ) {
            printf("Iteration %d\r", k);
//    printf("Iteration %d:  xnorm = %f, gnorm = %f, step = %f\n", k, xnorm, gnorm, step);
            return 0;
        }

    }


    void Polygonizer::polygonize(int num_faces) {
#if 1
        // record the original geometry
        auto orig_points = mesh_->get_vertex_property<vec3>("v:original");
        if (!orig_points) {
            orig_points = mesh_->add_vertex_property<vec3>("v:original");
            for (auto v : mesh_->vertices())
                orig_points[v] = mesh_->position(v);
        }

        // use the current geometry for further processing (if the original model is being visualized). 
        auto cur_points = mesh_->get_vertex_property<vec3>("v:polygonizer");
        if (cur_points)
            mesh_->points() = cur_points.vector();

#endif

        LOG(INFO) << "optimizing normals...";
        StopWatch w;
        optimize_normals();
        LOG(INFO) << "done. time: " << w.elapsed_seconds() << " seconds.";

        LOG(INFO) << "optimizing vertices...";
        w.start();
        optimize_vertices();
        LOG(INFO) << "done. time: " << w.elapsed_seconds() << " seconds.";

        auto output = mesh_->vertex_property<vec3>("v:output");
        auto points = mesh_->vertex_property<vec3>("v:point");
        for (auto v :  mesh_->vertices())
            output[v] = points[v];

        mesh_->update();
    }


    void Polygonizer::optimize_normals() {
#if 1
        details::normals::compute_expected_normals(mesh_);
        auto expected_normals = mesh_->get_face_property<vec3>("f:expected_normals");
        auto normals = mesh_->get_face_property<vec3>("f:normal");
        for (auto f : mesh_->faces()) {
            normals[f] = expected_normals[f];
        }

        std::cout << "disable optimize_normals() to converge faster and have sharper edges" << std::endl;
        return;
#endif

        details::normals::NormalsData data(mesh_);
        data.lambda = 1.0;

        int n = mesh_->n_faces() * 3;

        lbfgsfloatval_t *x = lbfgs_malloc(n);
        if (!x) {
            LOG(ERROR) << "L-BFGS failed to allocate a memory block for variables.";
            return;
        }
        /* Initialize the variables. */
        for (int i = 0; i < n; ++i) {
            x[i] = data.x0[i];
        }

        lbfgs_parameter_t param;
        /* Initialize the parameters for the L-BFGS optimization. */
        lbfgs_parameter_init(&param);
        //----------------------------------------------------------------------------
        // 1.0 to have L1 regularization
        param.orthantwise_c = 0.0;
        param.linesearch = LBFGS_LINESEARCH_BACKTRACKING;
        //----------------------------------------------------------------------------

        // Start the L-BFGS optimization; this will invoke the callback functions
        // evaluate() and progress() when necessary.
        lbfgsfloatval_t fx;
        int ret = lbfgs(n, x, &fx, details::normals::normals_evaluate, details::progress, &data, &param);
        if (ret != 0) {
            printf("L-BFGS optimization terminated with status code = %d\n", ret);
            lbfgs_free(x);
            return;
        }

        /* Report the result. */
        printf("L-BFGS optimization terminated with status code = %d\n", ret);
        printf("  fx = %f\n", fx);

        // assign new normals
        auto fnormals = mesh_->face_property<vec3>("f:normal");
        for (auto f : mesh_->faces()) {
            const int id = f.idx();
            vec3 n(x[id * 3], x[id * 3 + 1], x[id * 3 + 2]);
            fnormals[f] = normalize(n);
        }
        lbfgs_free(x);
    }


    void Polygonizer::optimize_vertices() {
        details::vertices::VerticesData data(mesh_);
        data.lambda_data = 1;
        data.lambda_smoothness = 100;
        data.lambda_edge_length = 5;

        int n = mesh_->n_vertices() * 3;

        lbfgsfloatval_t *x = lbfgs_malloc(n);
        if (!x) {
            LOG(ERROR) << "L-BFGS failed to allocate a memory block for variables.";
            return;
        }
        /* Initialize the variables. */
        for (int i = 0; i < n; ++i) {
            x[i] = data.x0[i];
        }

        lbfgs_parameter_t param;
        /* Initialize the parameters for the L-BFGS optimization. */
        lbfgs_parameter_init(&param);

        // Start the L-BFGS optimization; this will invoke the callback functions
        // evaluate() and progress() when necessary.
        lbfgsfloatval_t fx;
        int ret = lbfgs(n, x, &fx, details::vertices::vertices_evaluate, details::progress, &data, &param);
        if (ret != 0) {
            printf("L-BFGS optimization terminated with status code = %d\n", ret);
            lbfgs_free(x);
            return;
        }

        /* Report the result. */
        printf("L-BFGS optimization terminated with status code = %d\n", ret);
        printf("  fx = %f\n", fx);

        // assign new vertices
        auto points = mesh_->get_vertex_property<vec3>("v:point");
        for (auto v : mesh_->vertices()) {
            const int id = v.idx();
            points[v] = vec3(x[id * 3], x[id * 3 + 1], x[id * 3 + 2]);
        }
        lbfgs_free(x);

        mesh_->update_face_normals();
    }


}