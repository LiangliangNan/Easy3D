/********************************************************************
 * Copyright (C) 2020-2021 by Liangliang Nan <liangliang.nan@gmail.com>
 * Copyright (C) 2011-2020 the Polygon Mesh Processing Library developers.
 *
 * The code in this file is adapted from the PMP (Polygon Mesh Processing
 * Library) with modifications.
 *      https://github.com/pmp-library/pmp-library
 * The original code was distributed under a MIT-style license, see
 *      https://github.com/pmp-library/pmp-library/blob/master/LICENSE.txt
 ********************************************************************/

#include <easy3d/algo/surface_mesh_simplification.h>

#include <cfloat>
#include <iterator> // for back_inserter on Windows


namespace easy3d {

    SurfaceMeshSimplification::SurfaceMeshSimplification(SurfaceMesh *mesh)
            : mesh_(mesh), initialized_(false), queue_(nullptr), has_selection_(false), has_features_(false) {
        aspect_ratio_ = 0;
        edge_length_ = 0;
        max_valence_ = 0;
        normal_deviation_ = 0;
        hausdorff_error_ = 0;

        // get properties
        vpoint_ = mesh_->get_vertex_property<vec3>("v:point");

        // compute face normals
        mesh_->update_face_normals();
        fnormal_ = mesh_->get_face_property<vec3>("f:normal");
    }

    //-----------------------------------------------------------------------------

    SurfaceMeshSimplification::~SurfaceMeshSimplification() = default;

    //-----------------------------------------------------------------------------

    void SurfaceMeshSimplification::initialize(float aspect_ratio, float edge_length,
                                               unsigned int max_valence,
                                               float normal_deviation,
                                               float hausdorff_error) {
        if (!mesh_->is_triangle_mesh())
            return;

        // store parameters
        aspect_ratio_ = aspect_ratio;
        max_valence_ = max_valence;
        edge_length_ = edge_length;
        normal_deviation_ = normal_deviation / static_cast<float>(180.0f * M_PI);
        hausdorff_error_ = hausdorff_error;

        // properties
        if (normal_deviation_ > 0.0)
            normal_cone_ = mesh_->face_property<NormalCone>("f:normalCone");
        else
            mesh_->remove_face_property(normal_cone_);
        if (hausdorff_error > 0.0)
            face_points_ = mesh_->face_property<Points>("f:points");
        else
            mesh_->remove_face_property(face_points_);

        // add quadric property
        vquadric_ = mesh_->add_vertex_property<Quadric>("v:quadric");

        // vertex selection
        has_selection_ = false;
        vselected_ = mesh_->get_vertex_property<bool>("v:selected");
        if (vselected_) {
            for (auto v : mesh_->vertices()) {
                if (vselected_[v]) {
                    has_selection_ = true;
                    break;
                }
            }
        }

        // feature vertices/edges
        has_features_ = false;
        vfeature_ = mesh_->get_vertex_property<bool>("v:feature");
        efeature_ = mesh_->get_edge_property<bool>("e:feature");
        if (vfeature_ && efeature_) {
            for (auto v : mesh_->vertices()) {
                if (vfeature_[v]) {
                    has_features_ = true;
                    break;
                }
            }
        }

        // initialize quadrics
        for (auto v : mesh_->vertices()) {
            vquadric_[v].clear();

            if (!mesh_->is_isolated(v)) {
                for (auto f : mesh_->faces(v)) {
                    vquadric_[v] += Quadric(fnormal_[f], vpoint_[v]);
                }
            }
        }

        // initialize normal cones
        if (std::abs(normal_deviation_) > std::numeric_limits<float>::min()) {
            for (auto f : mesh_->faces()) {
                normal_cone_[f] = NormalCone(fnormal_[f]);
            }
        }

        // initialize faces' point list
        if (std::abs(hausdorff_error_) > std::numeric_limits<float>::min()) {
            for (auto f : mesh_->faces()) {
                Points().swap(face_points_[f]); // free mem
            }
        }

        initialized_ = true;
    }

    //-----------------------------------------------------------------------------

    void SurfaceMeshSimplification::simplify(unsigned int n_vertices) {
        if (!mesh_->is_triangle_mesh()) {
            std::cerr << "Not a triangle mesh!" << std::endl;
            return;
        }

        // make sure the decimater is initialized
        if (!initialized_)
            initialize();

        // add properties for priority queue
        vpriority_ = mesh_->add_vertex_property<float>("v:prio");
        heap_pos_ = mesh_->add_vertex_property<int>("v:heap");
        vtarget_ = mesh_->add_vertex_property<SurfaceMesh::Halfedge>("v:target");

        // build priority queue
        HeapInterface hi(vpriority_, heap_pos_);
        queue_ = new PriorityQueue(hi);
        queue_->reserve(mesh_->n_vertices());
        for (auto v : mesh_->vertices()) {
            queue_->reset_heap_position(v);
            enqueue_vertex(v);
        }

        unsigned int nv(mesh_->n_vertices());
        std::vector<SurfaceMesh::Vertex> one_ring;
        std::vector<SurfaceMesh::Vertex>::iterator or_it, or_end;
        SurfaceMesh::Halfedge h;
        SurfaceMesh::Vertex v;
        while (nv > n_vertices && !queue_->empty()) {
            // get 1st element
            v = queue_->front();
            queue_->pop_front();
            h = vtarget_[v];
            CollapseData cd(mesh_, h);

            // check this (again)
            if (!mesh_->is_collapse_ok(h))
                continue;

            // store one-ring
            one_ring.clear();
            for (auto vv : mesh_->vertices(cd.v0)) {
                one_ring.push_back(vv);
            }

            // perform collapse
            mesh_->collapse(h);
            --nv;
            //if (nv % 1000 == 0) std::cerr << nv << "\r";

            // postprocessing, e.g., update quadrics
            postprocess_collapse(cd);

            // update queue
            for (or_it = one_ring.begin(), or_end = one_ring.end(); or_it != or_end;
                 ++or_it)
                enqueue_vertex(*or_it);
        }

        // clean up
        delete queue_;
        mesh_->collect_garbage();
        mesh_->remove_vertex_property(vpriority_);
        mesh_->remove_vertex_property(heap_pos_);
        mesh_->remove_vertex_property(vtarget_);

        // remove added properties
        mesh_->remove_vertex_property(vquadric_);
        mesh_->remove_face_property(normal_cone_);
        mesh_->remove_face_property(face_points_);
    }

    //-----------------------------------------------------------------------------

    void SurfaceMeshSimplification::enqueue_vertex(SurfaceMesh::Vertex v) {
        float prio, min_prio(FLT_MAX);
        SurfaceMesh::Halfedge min_h;

        // find best out-going halfedge
        for (auto h : mesh_->halfedges(v)) {
            CollapseData cd(mesh_, h);
            if (is_collapse_legal(cd)) {
                prio = priority(cd);
                if (prio != -1.0 && prio < min_prio) {
                    min_prio = prio;
                    min_h = h;
                }
            }
        }

        // target found -> put vertex on heap
        if (min_h.is_valid()) {
            vpriority_[v] = min_prio;
            vtarget_[v] = min_h;

            if (queue_->is_stored(v))
                queue_->update(v);
            else
                queue_->insert(v);
        }

            // not valid -> remove from heap
        else {
            if (queue_->is_stored(v))
                queue_->remove(v);

            vpriority_[v] = -1;
            vtarget_[v] = min_h;
        }
    }

    //-----------------------------------------------------------------------------

    bool SurfaceMeshSimplification::is_collapse_legal(const CollapseData &cd) {
        // test selected vertices
        if (has_selection_) {
            if (!vselected_[cd.v0])
                return false;
        }

        // test features
        if (has_features_) {
            if (vfeature_[cd.v0] && !efeature_[mesh_->edge(cd.v0v1)])
                return false;

            if (cd.vl.is_valid() && efeature_[mesh_->edge(cd.vlv0)])
                return false;

            if (cd.vr.is_valid() && efeature_[mesh_->edge(cd.v0vr)])
                return false;
        }

        // do not collapse boundary vertices to interior vertices
        if (mesh_->is_border(cd.v0) && !mesh_->is_border(cd.v1))
            return false;

        // there should be at least 2 incident faces at v0
        if (mesh_->next_around_source(mesh_->next_around_source(cd.v0v1)) ==
            cd.v0v1)
            return false;

        // topological check
        if (!mesh_->is_collapse_ok(cd.v0v1))
            return false;

        // check maximal valence
        if (max_valence_ > 0) {
            unsigned int val0 = mesh_->valence(cd.v0);
            unsigned int val1 = mesh_->valence(cd.v1);
            unsigned int val = val0 + val1 - 1;
            if (cd.fl.is_valid())
                --val;
            if (cd.fr.is_valid())
                --val;
            if (val > max_valence_ && (val >= std::max(val0, val1)))
                return false;
        }

        // remember the positions of the endpoints
        const vec3 p0 = vpoint_[cd.v0];
        const vec3 p1 = vpoint_[cd.v1];

        // check for maximum edge length
        if (std::abs(edge_length_) > std::numeric_limits<float>::min()) {
            for (auto v : mesh_->vertices(cd.v0)) {
                if (v != cd.v1 && v != cd.vl && v != cd.vr) {
                    if (norm(vpoint_[v] - p1) > edge_length_)
                        return false;
                }
            }
        }

        // check for flipping normals
        if (normal_deviation_ == 0.0) {
            vpoint_[cd.v0] = p1;
            for (auto f : mesh_->faces(cd.v0)) {
                if (f != cd.fl && f != cd.fr) {
                    vec3 n0 = fnormal_[f];
                    vec3 n1 = mesh_->compute_face_normal(f);
                    if (dot(n0, n1) < 0.0) {
                        vpoint_[cd.v0] = p0;
                        return false;
                    }
                }
            }
            vpoint_[cd.v0] = p0;
        }

            // check normal cone
        else {
            vpoint_[cd.v0] = p1;

            SurfaceMesh::Face fll, frr;
            if (cd.vl.is_valid())
                fll = mesh_->face(
                        mesh_->opposite(mesh_->prev(cd.v0v1)));
            if (cd.vr.is_valid())
                frr = mesh_->face(
                        mesh_->opposite(mesh_->next(cd.v1v0)));

            for (auto f : mesh_->faces(cd.v0)) {
                if (f != cd.fl && f != cd.fr) {
                    NormalCone nc = normal_cone_[f];
                    nc.merge(mesh_->compute_face_normal(f));

                    if (f == fll)
                        nc.merge(normal_cone_[cd.fl]);
                    if (f == frr)
                        nc.merge(normal_cone_[cd.fr]);

                    if (nc.angle() > 0.5 * normal_deviation_) {
                        vpoint_[cd.v0] = p0;
                        return false;
                    }
                }
            }

            vpoint_[cd.v0] = p0;
        }

        // check aspect ratio
        if (std::abs(aspect_ratio_) > std::numeric_limits<float>::min()) {
            float ar0(0), ar1(0);

            for (auto f : mesh_->faces(cd.v0)) {
                if (f != cd.fl && f != cd.fr) {
                    // worst aspect ratio after collapse
                    vpoint_[cd.v0] = p1;
                    ar1 = std::max(ar1, aspect_ratio(f));
                    // worst aspect ratio before collapse
                    vpoint_[cd.v0] = p0;
                    ar0 = std::max(ar0, aspect_ratio(f));
                }
            }

            // aspect ratio is too bad, and it does also not improve
            if (ar1 > aspect_ratio_ && ar1 > ar0)
                return false;
        }

        // check Hausdorff error
        if (std::abs(hausdorff_error_) > std::numeric_limits<float>::min()) {
            Points points;
            bool ok = false;

            // collect points to be tested
            for (auto f : mesh_->faces(cd.v0)) {
                std::copy(face_points_[f].begin(), face_points_[f].end(),
                          std::back_inserter(points));
            }
            points.push_back(vpoint_[cd.v0]);

            // test points against all faces
            vpoint_[cd.v0] = p1;
            for (auto point : points) {
                ok = false;

                for (auto f : mesh_->faces(cd.v0)) {
                    if (f != cd.fl && f != cd.fr) {
                        if (distance(f, point) < hausdorff_error_) {
                            ok = true;
                            break;
                        }
                    }
                }

                if (!ok) {
                    vpoint_[cd.v0] = p0;
                    return false;
                }
            }
            vpoint_[cd.v0] = p0;
        }

        // collapse passed all tests -> ok
        return true;
    }

    //-----------------------------------------------------------------------------

    float SurfaceMeshSimplification::priority(const CollapseData &cd) {
        // computer quadric error metric
        Quadric Q = vquadric_[cd.v0];
        Q += vquadric_[cd.v1];
        return Q(vpoint_[cd.v1]);
    }

    //-----------------------------------------------------------------------------

    void SurfaceMeshSimplification::postprocess_collapse(const CollapseData &cd) {
        // update error quadrics
        vquadric_[cd.v1] += vquadric_[cd.v0];

        // update normal cones
        if (std::abs(normal_deviation_) > std::numeric_limits<float>::min()) {
            for (auto f : mesh_->faces(cd.v1)) {
                normal_cone_[f].merge(mesh_->compute_face_normal(f));
            }

            if (cd.vl.is_valid()) {
                SurfaceMesh::Face f = mesh_->face(cd.v1vl);
                if (f.is_valid())
                    normal_cone_[f].merge(normal_cone_[cd.fl]);
            }

            if (cd.vr.is_valid()) {
                SurfaceMesh::Face f = mesh_->face(cd.vrv1);
                if (f.is_valid())
                    normal_cone_[f].merge(normal_cone_[cd.fr]);
            }
        }

        // update Hausdorff error
        if (std::abs(hausdorff_error_) > std::numeric_limits<float>::min()) {
            Points points;

            // collect points to be distributed

            // points of v1's one-ring
            for (auto f : mesh_->faces(cd.v1)) {
                std::copy(face_points_[f].begin(), face_points_[f].end(),
                          std::back_inserter(points));
                face_points_[f].clear();
            }

            // points of the 2 removed triangles
            if (cd.fl.is_valid()) {
                std::copy(face_points_[cd.fl].begin(), face_points_[cd.fl].end(),
                          std::back_inserter(points));
                Points().swap(face_points_[cd.fl]); // free mem
            }
            if (cd.fr.is_valid()) {
                std::copy(face_points_[cd.fr].begin(), face_points_[cd.fr].end(),
                          std::back_inserter(points));
                Points().swap(face_points_[cd.fr]); // free mem
            }

            // the removed vertex
            points.push_back(vpoint_[cd.v0]);

            // test points against all faces
            float d, dd;
            SurfaceMesh::Face ff;

            for (auto point : points) {
                dd = FLT_MAX;

                for (auto f : mesh_->faces(cd.v1)) {
                    d = distance(f, point);
                    if (d < dd) {
                        ff = f;
                        dd = d;
                    }
                }

                face_points_[ff].push_back(point);
            }
        }
    }

    //-----------------------------------------------------------------------------

    float SurfaceMeshSimplification::aspect_ratio(SurfaceMesh::Face f) const {
        // min height is area/maxLength
        // aspect ratio = length / height
        //              = length * length / area

        SurfaceMesh::VertexAroundFaceCirculator fvit = mesh_->vertices(f);

        const vec3 p0 = vpoint_[*fvit];
        const vec3 p1 = vpoint_[*(++fvit)];
        const vec3 p2 = vpoint_[*(++fvit)];

        const vec3 d0 = p0 - p1;
        const vec3 d1 = p1 - p2;
        const vec3 d2 = p2 - p0;

        const float l0 = length2(d0);
        const float l1 = length2(d1);
        const float l2 = length2(d2);

        // max squared edge length
        const float l = std::max(l0, std::max(l1, l2));

        // triangle area
        float a = norm(cross(d0, d1));

        return l / a;
    }

    //-----------------------------------------------------------------------------

    float SurfaceMeshSimplification::distance(SurfaceMesh::Face f, const vec3 &p) const {
        SurfaceMesh::VertexAroundFaceCirculator fvit = mesh_->vertices(f);

        const vec3 p0 = vpoint_[*fvit];
        const vec3 p1 = vpoint_[*(++fvit)];
        const vec3 p2 = vpoint_[*(++fvit)];

        vec3 n;
        return geom::dist_point_triangle(p, p0, p1, p2, n);
    }

    //-----------------------------------------------------------------------------

    SurfaceMeshSimplification::CollapseData::CollapseData(SurfaceMesh *sm, SurfaceMesh::Halfedge h)
            : mesh(sm) {
        v0v1 = h;
        v1v0 = mesh->opposite(v0v1);
        v0 = mesh->target(v1v0);
        v1 = mesh->target(v0v1);
        fl = mesh->face(v0v1);
        fr = mesh->face(v1v0);

        // get vl
        if (fl.is_valid()) {
            v1vl = mesh->next(v0v1);
            vlv0 = mesh->next(v1vl);
            vl = mesh->target(v1vl);
        }

        // get vr
        if (fr.is_valid()) {
            v0vr = mesh->next(v1v0);
            vrv1 = mesh->prev(v0vr);
            vr = mesh->source(vrv1);
        }
    }

} // namespace easy3d