#include "polyfit.h"

#include <unordered_map>

#include <CGAL/Cartesian.h>
#include <CGAL/Projection_traits_xy_3.h>
#include <CGAL/convex_hull_2.h>
#include <CGAL/bounding_box.h>
#include <CGAL/intersections.h>

#include <easy3d/core/principal_axes.h>


namespace easy3d {

    namespace internal {

        /// When an intersecting point (at an edge, computed from a plane and an edge)
        /// is very close to an existing vertex (i.e., an end point of an edge), we
        /// snap the intersecting point to the existing vertex. This way we can avoid
        /// many thin faces.
        /// \note Value really doesn't matter as long as it is small (default is 1e-10).
        ///       So this parameter is not intended to be changed by the user.
        /// TODO: make this in the API
        float snap_squared_distance_threshold() {
            return float(1e-10);
        }

        // A group of points (represented by their indices) belonging to a planar segment in a point set.
        class Planar_segment : public std::vector<std::size_t> {
        public:
            // \param point_set the point set that owns this planar segment.
            Planar_segment(const PointCloud *point_set) : cloud_(point_set), supporting_plane_(nullptr) {}

            ~Planar_segment() { delete supporting_plane_; }

            const PointCloud *cloud() const { return cloud_; }

            // Fits and returns the supporting plane of this planar segment
            Plane3 *fit_supporting_plane();

            // Returns the supporting plane of this planar segment.
            // \note Returned plane is valid only if fit_supporting_plane() has been called.
            Plane3 *supporting_plane() const { return supporting_plane_; }

        private:
            const PointCloud *cloud_;
            Plane3 *supporting_plane_; // The hypothesis generator owns this plane and manages the memory
        };


        // An enriched point set that stores the extracted planar segments
        class EnrichedPointCloud {
        public:
            EnrichedPointCloud(const PointCloud *cloud, PointCloud::VertexProperty<int> plane_indices) : cloud_(cloud) {
                // Gets to know the number of plane from the plane indices
                int max_plane_index = 0;
                for (auto v: cloud->vertices()) {
                    int plane_index = plane_indices[v];
                    if (plane_index > max_plane_index)
                        max_plane_index = plane_index;
                }
                std::size_t num_plane = max_plane_index + 1; // the first one has index 0

                for (std::size_t i = 0; i < num_plane; ++i)
                    planar_segments_.push_back(new Planar_segment(cloud));

                std::size_t idx = 0;
                for (auto v: cloud->vertices()) {
                    int plane_index = plane_indices[v];
                    if (plane_index != -1) {
                        Planar_segment *ps = planar_segments_[plane_index];
                        ps->push_back(idx);
                    }
                    ++idx;
                }
            }

            ~EnrichedPointCloud() {
                for (std::size_t i = 0; i < planar_segments_.size(); ++i)
                    delete planar_segments_[i];
            }

            const PointCloud *cloud() const { return cloud_; }

            std::vector<Planar_segment *> &planar_segments() { return planar_segments_; }

            const std::vector<Planar_segment *> &planar_segments() const { return planar_segments_; }

        private:
            const PointCloud *cloud_;
            std::vector<Planar_segment *> planar_segments_;
        };


        Plane3 *Planar_segment::fit_supporting_plane() {
            const auto &points = cloud_->points();
            PrincipalAxes<3, float> pca;
            pca.begin();
            for (std::size_t i = 0; i < size(); ++i) {
                std::size_t idx = at(i);
                pca.add(points[idx]);
            }
            pca.end();

            if (supporting_plane_)
                delete supporting_plane_;

            // the normal is the eigen vector corresponding to the smallest eigen value
            supporting_plane_ = new Plane3(pca.center(), pca.axis(2));
            return supporting_plane_;
        }


        // Generates candidate faces by pairwise intersecting of the supporting planes of the planar segments.
        class Hypothesis {
//        private:
//            typedef typename Kernel::FT                                FT;
//            typedef typename Kernel::Point_3                        vec3;
//            typedef typename Kernel::Point_2                        Point2;
//            typedef typename Kernel::Vector_3                        Vector;
//            typedef typename Kernel::Line_3                                Line;
//            typedef typename Kernel::Segment_3                        Segment;
//            typedef typename Kernel::Plane_3                        Plane;
//            typedef internal::Planar_segment<Kernel>                Planar_segment;
//            typedef internal::Point_set_with_planes<Kernel>                Point_set_with_planes;
//
//            typedef CGAL::Surface_mesh<vec3>                        SurfaceMesh;
//            typedef typename SurfaceMesh::Face_index                SurfaceMesh::Face;
//            typedef typename SurfaceMesh::Edge_index                SurfaceMesh::Edge;
//            typedef typename SurfaceMesh::Vertex_index                SurfaceMesh::Vertex;
//
//            typedef typename SurfaceMesh::Halfedge_index        SurfaceMesh::Halfedge;

        public:
            Hypothesis() {}

            ~Hypothesis();

            // Generates candidate faces (stored as a SurfaceMesh object) and returns the pointer to the candidate faces
            SurfaceMesh *generate(EnrichedPointCloud &point_set);

            /// 'Intersection' represents a set of faces intersecting at a common edge.
            /// \note The faces are represented by their halfedges.
            struct Intersection : public std::vector<SurfaceMesh::Halfedge> {
                const vec3 *s;
                const vec3 *t;
            };
            typedef typename std::vector<Intersection> Adjacency;

            /// Extracts the adjacency of the pairwise intersection.
            /// The extracted adjacency will be used to formulate the hard constraints
            /// in the face selection stage.
            Adjacency extract_adjacency(const SurfaceMesh &candidate_faces);

        private:
            // Merges near co-planar segments
            void refine_planes();

            // Constructs a mesh representing the bounding box of the point set
            SurfaceMesh construct_bbox_mesh();

            // Construct a mesh from the segments bounded by the bounding box mesh
            SurfaceMesh construct_proxy_mesh(SurfaceMesh &bbox_mesh);

            // Pairwise intersection
            void pairwise_intersection(SurfaceMesh &candidate_faces);

            // Counts the number of points that are with the dist_threshold to its supporting plane
            std::size_t number_of_points_on_plane(const Planar_segment *s, const Plane3 *plane, float dist_threshold);

            // Merges two planar segments;
            void merge(Planar_segment *s1, Planar_segment *s2);

            // Pre-computes all potential intersections of plane triplets
            void compute_triplet_intersections();

            // Queries the intersecting point for a plane triplet
            const vec3 *query_intersection(const Plane3 *plane1, const Plane3 *plane2, const Plane3 *plane3);

            bool halfedge_exists(SurfaceMesh::Vertex v1, SurfaceMesh::Vertex v2, const SurfaceMesh &mesh);

            // Tests if 'face' insects 'plane'
            bool do_intersect(const SurfaceMesh &mesh, SurfaceMesh::Face face, const Plane3 *plane);

            // Cuts face using the cutting_plane and returns the new faces
            std::vector<SurfaceMesh::Face> cut(SurfaceMesh::Face face, const Plane3 *cutting_plane, SurfaceMesh &mesh);

            // Collects all faces in 'mesh' that intersect 'face'. Store in std::set() so easier to erase
            std::set<SurfaceMesh::Face> collect_intersecting_faces(SurfaceMesh::Face face, const SurfaceMesh &mesh);

            // Represents an intersecting point at an edge
            struct EdgePos {
                EdgePos(SurfaceMesh::Edge e, const vec3 *p) : edge(e), pos(p) {}

                SurfaceMesh::Edge edge;
                const vec3 *pos;
            };

            // Computes the intersecting points of face and cutting_plane. The intersecting points are returned
            // by 'existing_vts' (if the plane intersects the face at its vertices) and 'new_vts' (if the plane
            // intersects the face at its edges).
            void compute_intersections(const SurfaceMesh &mesh,
                                       SurfaceMesh::Face face, const Plane3 *cutting_plane,
                                       std::vector<SurfaceMesh::Vertex> &existing_vts,
                                       std::vector<EdgePos> &new_vts
            );

            // This function will
            // - split an edge denoted by 'ep'
            // - assign the new edges the supporting faces
            // - return the halfedge pointing to the new vertex
            // Internally it uses Euler split_edge().
            SurfaceMesh::Halfedge split_edge(SurfaceMesh &mesh, const EdgePos &ep, const Plane3 *cutting_plane);

            // Clears cached intermediate results
            void clear();

        private:
            // The input point cloud with planes
            EnrichedPointCloud *point_set_;

            SurfaceMesh candidate_faces_;

            // The intersection of the planes can be unreliable when the planes are near parallel.
            // Here are the tricks we use in our implementation:
            //   - We first test if an intersection exists for every pair of planes. We then collect
            //     plane triplets such that every pair in the plane triplet intersect. This is achieved
            //     by testing each plane against the known intersecting pairs.
            //         - The 3D vertices of the final faces are obtained by computing the intersections of
            //     the plane triplets. To cope with limited floating point precision, each vertex is
            //     identified by the pointers of (in an increasing order) of the three planes from
            //     which it is computed. By doing so, two vertices with almost identical positions can
            //     be distinguished. This turned out to be quite robust in handling very close and near
            //     parallel planes.

            // The supporting planes of all planar segments and the bounding box faces
            std::vector<const Plane3 *> supporting_planes_;

            // Precomputed intersecting points of all plane triplets
            std::vector<const vec3 *> intersecting_points_;

            typedef typename std::unordered_map<const Plane3 *, const vec3 *> Plane_to_point_map;
            typedef typename std::unordered_map<const Plane3 *, Plane_to_point_map> Two_planes_to_point_map;
            typedef typename std::unordered_map<const Plane3 *, Two_planes_to_point_map> Planes_to_point_map;
            Planes_to_point_map triplet_intersections_;
        };


        //////////////////////////////////////////////////////////////////////////

        // implementation
        Hypothesis::~Hypothesis() {
            clear();
        }


        void Hypothesis::clear() {
            for (std::size_t i = 0; i < supporting_planes_.size(); ++i)
                delete supporting_planes_[i];
            supporting_planes_.clear();

            for (std::size_t i = 0; i < intersecting_points_.size(); ++i)
                delete intersecting_points_[i];
            intersecting_points_.clear();

            triplet_intersections_.clear();
        }


        SurfaceMesh *Hypothesis::generate(EnrichedPointCloud &point_set) {
            point_set_ = &point_set;

            refine_planes();

            SurfaceMesh bbox_mesh = construct_bbox_mesh();
            candidate_faces_ = construct_proxy_mesh(bbox_mesh);
            pairwise_intersection(candidate_faces_);

            return &candidate_faces_;
        }


        /// \cond SKIP_IN_MANUAL

        template<typename Planar_segment>
        class SegmentSizeIncreasing {
        public:
            SegmentSizeIncreasing() {}

            bool operator()(const Planar_segment *s0, const Planar_segment *s1) const {
                return s0->size() < s1->size();
            }
        };

        template<typename Planar_segment>
        class SegmentSizeDecreasing {
        public:
            SegmentSizeDecreasing() {}

            bool operator()(const Planar_segment *s0, const Planar_segment *s1) const {
                return s0->size() > s1->size();
            }
        };

        template<typename BBox>
        typename BBox::FT bbox_radius(const BBox &box) {
            typedef typename BBox::FT FT;
            FT dx = box.xmax() - box.xmin();
            FT dy = box.ymax() - box.ymin();
            FT dz = box.zmax() - box.zmin();
            return FT(0.5) * std::sqrt(dx * dx + dy * dy + dz * dz);
        }

        template<typename VT>
        void sort_increasing(VT &v1, VT &v2, VT &v3) {
            VT vmin = 0;
            if (v1 < v2 && v1 < v3)
                vmin = v1;
            else if (v2 < v1 && v2 < v3)
                vmin = v2;
            else
                vmin = v3;

            VT vmid = 0;
            if ((v1 > v2 && v1 < v3) || (v1 < v2 && v1 > v3))
                vmid = v1;
            else if ((v2 > v1 && v2 < v3) || (v2 < v1 && v2 > v3))
                vmid = v2;
            else
                vmid = v3;

            VT vmax = 0;
            if (v1 > v2 && v1 > v3)
                vmax = v1;
            else if (v2 > v1 && v2 > v3)
                vmax = v2;
            else
                vmax = v3;

            v1 = vmin;
            v2 = vmid;
            v3 = vmax;
        }
        /// \endcond



        std::size_t
        Hypothesis::number_of_points_on_plane(const Planar_segment *s, const Plane3 *plane, float dist_threshold) {
            assert(const_cast<Planar_segment *>(s)->cloud() == point_set_->cloud());

            std::size_t count = 0;
            const auto &points = point_set_->cloud()->points();
            for (std::size_t i = 0; i < s->size(); ++i) {
                std::size_t idx = s->at(i);
                const vec3 &p = points[idx];

                const float sdist = plane->squared_ditance(p);
                const float dist = std::sqrt(sdist);
                if (dist < dist_threshold)
                    ++count;
            }
            return count;
        }


        void Hypothesis::merge(Planar_segment *s1, Planar_segment *s2) {
            assert(const_cast<Planar_segment *>(s1)->cloud() == point_set_->cloud());
            assert(const_cast<Planar_segment *>(s2)->cloud() == point_set_->cloud());
            std::vector<Planar_segment *> &segments = point_set_->planar_segments();

            std::vector<std::size_t> points_indices;
            points_indices.insert(points_indices.end(), s1->begin(), s1->end());
            points_indices.insert(points_indices.end(), s2->begin(), s2->end());

            Planar_segment *s = new Planar_segment(point_set_->cloud());
            s->insert(s->end(), points_indices.begin(), points_indices.end());
            s->fit_supporting_plane();
            segments.push_back(s);

            typename std::vector<Planar_segment *>::iterator pos = std::find(segments.begin(), segments.end(), s1);
            if (pos != segments.end()) {
                Planar_segment *tmp = *pos;
                const Plane3 *plane = tmp->supporting_plane();
                segments.erase(pos);
                delete tmp;
                delete plane;
            } else
                std::cerr << "Fatal error: should not reach here" << std::endl;

            pos = std::find(segments.begin(), segments.end(), s2);
            if (pos != segments.end()) {
                Planar_segment *tmp = *pos;
                const Plane3 *plane = tmp->supporting_plane();
                segments.erase(pos);
                delete tmp;
                delete plane;
            } else
                std::cerr << "Fatal error: should not reach here" << std::endl;
        }


        void Hypothesis::refine_planes() {
            std::vector<Planar_segment *> &segments = point_set_->planar_segments();
            const auto &points = point_set_->cloud()->points();

            float avg_max_dist = 0;
            for (std::size_t i = 0; i < segments.size(); ++i) {
                Planar_segment *s = segments[i];
                const Plane3 *plane = s->fit_supporting_plane(); // user may provide invalid plane fitting (we always fit)

                float max_dist = -(std::numeric_limits<float>::max)();
                for (std::size_t j = 0; j < s->size(); ++j) {
                    std::size_t idx = s->at(j);
                    const vec3 &p = points[idx];
                    const float sdist = plane->squared_ditance(p);
                    max_dist = (std::max)(max_dist, std::sqrt(sdist));
                }

                avg_max_dist += max_dist;
            }
            avg_max_dist /= segments.size();
            avg_max_dist /= 2.0f;

            const float theta = static_cast<float>(CGAL_PI * 10.0 / 180.0f);        // in radian
            bool merged = false;
            do {
                merged = false;
                // Segments with less points have less confidences and thus should be merged first.
                // So we sort the segments according to their sizes.
                std::sort(segments.begin(), segments.end(), internal::SegmentSizeIncreasing<Planar_segment>());

                for (std::size_t i = 0; i < segments.size(); ++i) {
                    Planar_segment *s1 = segments[i];
                    const Plane3 *plane1 = s1->supporting_plane();
                    const vec3 n1 = plane1->normal().normalize();

                    const float num_threshold = s1->size() / 5.0f;
                    for (std::size_t j = i + 1; j < segments.size(); ++j) {
                        Planar_segment *s2 = segments[j];
                        const Plane3 *plane2 = s2->supporting_plane();
                        const vec3 n2 = plane2->normal().normalize();

                        if (std::abs(dot(n1, n2)) > std::cos(theta)) {
                            std::size_t set1on2 = number_of_points_on_plane(s1, plane2, avg_max_dist);
                            std::size_t set2on1 = number_of_points_on_plane(s2, plane1, avg_max_dist);
                            if (set1on2 > num_threshold || set2on1 > num_threshold) {
                                merge(s1, s2);
                                merged = true;
                                break;
                            }
                        }
                    }
                    if (merged)
                        break;
                }
            } while (merged);

            std::sort(segments.begin(), segments.end(), internal::SegmentSizeDecreasing<Planar_segment>());

            // Stores all the supporting planes
            for (std::size_t i = 0; i < segments.size(); ++i) {
                Planar_segment *s = segments[i];
                const Plane3 *plane = s->supporting_plane();
                supporting_planes_.push_back(plane);
            }
        }


        SurfaceMesh Hypothesis::construct_bbox_mesh() {
            SurfaceMesh mesh;
            const auto &points = point_set_->cloud()->points();

            Box3 box = point_set_->cloud()->bounding_box();
            const float dx = box.range(0);
            const float dy = box.range(1);
            const float dz = box.range(2);
            const float radius = 0.5f * std::sqrt(dx * dx + dy * dy + dz * dz);
            const float offset = radius * 0.05f;

            // make the box larger to ensure all points are enclosed.
            const float xmin = box.min_coord(0) - offset, xmax = box.max_coord(0) + offset;
            const float ymin = box.min_coord(1) - offset, ymax = box.max_coord(1) + offset;
            const float zmin = box.min_coord(2) - offset, zmax = box.max_coord(2) + offset;

            mesh.clear();

            SurfaceMesh::Vertex v0 = mesh.add_vertex(vec3(xmin, ymin, zmin));  // 0
            SurfaceMesh::Vertex v1 = mesh.add_vertex(vec3(xmax, ymin, zmin));  // 1
            SurfaceMesh::Vertex v2 = mesh.add_vertex(vec3(xmax, ymin, zmax));  // 2
            SurfaceMesh::Vertex v3 = mesh.add_vertex(vec3(xmin, ymin, zmax));  // 3
            SurfaceMesh::Vertex v4 = mesh.add_vertex(vec3(xmax, ymax, zmax));  // 4
            SurfaceMesh::Vertex v5 = mesh.add_vertex(vec3(xmax, ymax, zmin));  // 5
            SurfaceMesh::Vertex v6 = mesh.add_vertex(vec3(xmin, ymax, zmin));  // 6
            SurfaceMesh::Vertex v7 = mesh.add_vertex(vec3(xmin, ymax, zmax));  // 7

            mesh.add_quad(v0, v1, v2, v3);
            mesh.add_quad(v1, v5, v4, v2);
            mesh.add_quad(v1, v0, v6, v5);
            mesh.add_quad(v4, v5, v6, v7);
            mesh.add_quad(v0, v3, v7, v6);
            mesh.add_quad(v2, v4, v7, v3);

            // The supporting plane of each face
            auto face_supporting_planes = mesh.add_face_property<const Plane3 *>("f:supp_plane");

            // The supporting planes of each edge
            auto edge_supporting_planes = mesh.add_edge_property<std::set<const Plane3 *> >("e:supp_plane");

            // The supporting planes of each vertex
            auto vertex_supporting_planes = mesh.add_vertex_property<std::set<const Plane3 *> >("v:supp_plane");

            // Assigns the original plane for each face
            const auto &coords = mesh.points();
            for (auto fd: mesh.faces()) {
                SurfaceMesh::Halfedge h = mesh.halfedge(fd);
                SurfaceMesh::Vertex va = mesh.target(h);
                const vec3 &pa = mesh.position(va);
                h = mesh.next(h);
                SurfaceMesh::Vertex vb = mesh.target(h);
                const vec3 &pb = mesh.position(vb);
                h = mesh.next(h);
                SurfaceMesh::Vertex vc = mesh.target(h);
                const vec3 &pc = mesh.position(vc);
                const Plane3 *plane = new Plane3(pa, pb, pc);
                supporting_planes_.push_back(plane);
                face_supporting_planes[fd] = plane;
            }

            // Assigns the original planes for each edge
            for (auto ed: mesh.edges()) {
                SurfaceMesh::Halfedge h1 = mesh.halfedge(ed, 0);
                SurfaceMesh::Halfedge h2 = mesh.halfedge(ed, 1);

                SurfaceMesh::Face f1 = mesh.face(h1);
                SurfaceMesh::Face f2 = mesh.face(h2);
                assert(f1 != SurfaceMesh::Face()); // the bbox mesh is closed
                assert(f2 != SurfaceMesh::Face()); // the bbox mesh is closed

                const Plane3 *plane1 = face_supporting_planes[f1];
                const Plane3 *plane2 = face_supporting_planes[f2];
                assert(plane1 && plane2 && plane1 != plane2);

                edge_supporting_planes[ed].insert(plane1);
                edge_supporting_planes[ed].insert(plane2);
                assert(edge_supporting_planes[ed].size() == 2);
            }

            // Assigns the original planes for each vertex
            for (auto vd: mesh.vertices()) {
                assert(vertex_supporting_planes[vd].size() == 0);
                SurfaceMesh::HalfedgeAroundVertexCirculator hbegin(&mesh, vd), done(hbegin);
                do {
                    SurfaceMesh::Halfedge h = *hbegin;
                    SurfaceMesh::Face f = mesh.face(h);
                    const Plane3 *plane = face_supporting_planes[f];
                    vertex_supporting_planes[vd].insert(plane);
                    ++hbegin;
                } while (hbegin != done);
                assert(vertex_supporting_planes[vd].size() == 3);
            }

            std::sort(supporting_planes_.begin(), supporting_planes_.end());
            return mesh;
        }


        SurfaceMesh Hypothesis::construct_proxy_mesh(SurfaceMesh &bbox_mesh) {
            // Properties of the bbox_mesh
            // The supporting planes of each edge
            auto bbox_edge_supporting_planes = bbox_mesh.add_edge_property<std::set<const Plane3 *> >("e:supp_plane");

            // The supporting planes of each vertex
            auto bbox_vertex_supporting_planes = bbox_mesh.add_vertex_property<std::set<const Plane3 *> >(
                    "v:supp_plane");

            SurfaceMesh proxy_mesh;
            // The properties of the proxy mesh
            // The supporting plane of each face
            auto face_supporting_planes = proxy_mesh.add_face_property<const Plane3 *>("f:supp_plane");

            // The supporting planar segment of each face
            auto face_supporting_segments = proxy_mesh.add_face_property<Planar_segment *>("f:supp_segment");

            // The supporting planes of each edge
            auto edge_supporting_planes = bbox_mesh.add_edge_property<std::set<const Plane3 *> >("e:supp_plane");

            // The supporting planes of each vertex
            auto vertex_supporting_planes = bbox_mesh.add_vertex_property<std::set<const Plane3 *> >("v:supp_plane");

            const std::vector<Planar_segment *> &segments = point_set_->planar_segments();
            for (std::size_t i = 0; i < segments.size(); ++i) {
                Planar_segment *g = segments[i];
                const Plane3 *cutting_plane = g->supporting_plane();

                std::vector<vec3> intersecting_points;
                std::vector<std::set<const Plane3 *> > intersecting_points_source_planes;

                for (auto ed: bbox_mesh.edges()) {
                    SurfaceMesh::Vertex sd = bbox_mesh.vertex(ed, 0);
                    SurfaceMesh::Vertex td = bbox_mesh.vertex(ed, 1);
                    const vec3 &s = bbox_mesh.position(sd);
                    const vec3 &t = bbox_mesh.position(td);

                    const int ss = cutting_plane->orient(s);
                    const int st = cutting_plane->orient(t);

                    if ((ss == 1 && st == -1) || (ss == -1 && st == 1)) {
                        vec3 p;
                        if (cutting_plane->intersect(Line3::from_two_points(s, t))) {
                            intersecting_points.push_back(p);
                            std::set<const Plane3 *> planes = bbox_edge_supporting_planes[ed];
                            planes.insert(cutting_plane);
                            assert(planes.size() == 3);
                            intersecting_points_source_planes.push_back(planes);
                        } else
                            std::cerr << "Fatal error: should have intersection" << std::endl;
                    } else {
                        if (ss == 0 && st != 0) {
                            intersecting_points.push_back(s);
                            const std::set<const Plane3 *> &planes = bbox_vertex_supporting_planes[sd];
                            assert(planes.size() == 3);
                            intersecting_points_source_planes.push_back(planes);
                        } else if (st == 0 && ss != 0) {
                            intersecting_points.push_back(t);
                            const std::set<const Plane3 *> &planes = bbox_vertex_supporting_planes[td];
                            assert(planes.size() == 3);
                            intersecting_points_source_planes.push_back(planes);
                        } else {
                            // The intersection is the current edge, nothing to do
                        }
                    }
                }

                // Decides the orientation of the points
                if (intersecting_points.size() >= 3) {
                    std::list<vec3> pts;
                    for (std::size_t i = 0; i < intersecting_points.size(); ++i) {
                        const vec3 &p = intersecting_points[i];
                        const vec2 &q = cutting_plane->to_2d(p);
                        pts.emplace_back(vec3(q.x, q.y, float(i))); // the z component stores the point index
                    }

                    typedef CGAL::Cartesian<float> Kernel;
                    typedef CGAL::Projection_traits_xy_3<Kernel> Projection;
                    std::list<vec3> hull;
                    CGAL::convex_hull_2(pts.begin(), pts.end(), std::back_inserter(hull), Projection());

                    std::vector<vec3> ch;
                    std::vector<std::set<const Plane3 *> > ch_source_planes;
                    for (typename std::list<vec3>::iterator it = hull.begin(); it != hull.end(); ++it) {
                        std::size_t idx = std::size_t(it->z);
                        ch.push_back(intersecting_points[idx]);
                        ch_source_planes.push_back(intersecting_points_source_planes[idx]);
                    }

                    if (ch.size() >= 3) {
                        std::vector<SurfaceMesh::Vertex> descriptors;
                        for (std::size_t j = 0; j < ch.size(); ++j) {
                            SurfaceMesh::Vertex vd = proxy_mesh.add_vertex(ch[j]);
                            descriptors.push_back(vd);
                            vertex_supporting_planes[vd] = ch_source_planes[j];
                            assert(vertex_supporting_planes[vd].size() == 3);
                        }

                        SurfaceMesh::Face fd = proxy_mesh.add_face(descriptors);
                        face_supporting_segments[fd] = g;
                        face_supporting_planes[fd] = cutting_plane;

                        // Assigns each edge the supporting planes
                        SurfaceMesh::HalfedgeAroundFaceCirculator hbegin(&proxy_mesh, fd), done(hbegin);
                        do {
                            SurfaceMesh::Halfedge hd = *hbegin;
                            SurfaceMesh::Edge ed = proxy_mesh.edge(hd);

                            SurfaceMesh::Vertex s_vd = proxy_mesh.source(hd);
                            SurfaceMesh::Vertex t_vd = proxy_mesh.target(hd);
                            const std::set<const Plane3 *> &s_planes = vertex_supporting_planes[s_vd];
                            const std::set<const Plane3 *> &t_planes = vertex_supporting_planes[t_vd];
                            std::set<const Plane3 *> common_planes;
                            std::set_intersection(s_planes.begin(), s_planes.end(), t_planes.begin(), t_planes.end(),
                                                  std::inserter(common_planes, common_planes.begin()));
                            if (common_planes.size() == 2) {
                                assert(edge_supporting_planes[ed].size() == 0);
                                edge_supporting_planes[ed] = common_planes;
                                assert(edge_supporting_planes[ed].size() == 2);
                            } else // If reached here, there must be topological errors.
                                std::cerr << "topological error" << std::endl;

                            ++hbegin;
                        } while (hbegin != done);
                    }
                }
            }

            return proxy_mesh;
        }


        void Hypothesis::compute_triplet_intersections() {
            triplet_intersections_.clear();
            if (supporting_planes_.size() < 4) // no closed surface will be constructed from fewer than 4 planes
                return;

            for (std::size_t i = 0; i < supporting_planes_.size(); ++i) {
                const Plane3 *plane1 = supporting_planes_[i];
                for (std::size_t j = i + 1; j < supporting_planes_.size(); ++j) {
                    const Plane3 *plane2 = supporting_planes_[j];
                    for (std::size_t k = j + 1; k < supporting_planes_.size(); ++k) {
                        const Plane3 *plane3 = supporting_planes_[k];
                        assert(plane1 < plane2 && plane2 < plane3);
                        vec3 p;
                        if (intersection(*plane1, *plane2, *plane3, p)) {
                            // Stores the intersection for future query
                            vec3 *new_point = new vec3(p);
                            triplet_intersections_[plane1][plane2][plane3] = new_point;
                            intersecting_points_.push_back(new_point);
                        }
                    }
                }
            }
        }


        const vec3 *
        Hypothesis::query_intersection(const Plane3 *min_plane, const Plane3 *mid_plane, const Plane3 *max_plane) {
            assert(min_plane < mid_plane);
            assert(mid_plane < max_plane);

            if (triplet_intersections_.find(min_plane) == triplet_intersections_.end())
                return nullptr;

            Two_planes_to_point_map &map2 = triplet_intersections_[min_plane];
            if (map2.find(mid_plane) == map2.end())
                return nullptr;

            Plane_to_point_map &map1 = map2[mid_plane];
            if (map1.find(max_plane) == map1.end())
                return nullptr;

            return map1[max_plane];
        }


        SurfaceMesh::Halfedge
        Hypothesis::split_edge(SurfaceMesh &mesh, const EdgePos &ep, const Plane3 *cutting_plane) {
            // The supporting planes of each edge
            auto edge_supporting_planes = mesh.add_edge_property<std::set<const Plane3 *> >("e:supp_plane");

            // The supporting planes of each vertex
            auto vertex_supporting_planes = mesh.add_vertex_property<std::set<const Plane3 *> >("v:supp_plane");

            // We cannot use const reference, because it will become invalid after splitting
            std::set<const Plane3 *> sfs = edge_supporting_planes[ep.edge];
            assert(sfs.size() == 2);

            SurfaceMesh::Vertex v = mesh.add_vertex(*ep.pos);
            if (!v.is_valid())        // failed adding vertex
                return SurfaceMesh::Halfedge();

            SurfaceMesh::Halfedge h = mesh.insert_vertex(ep.edge, v);
            if (!h.is_valid()) // failed splitting edge
                return h;

            SurfaceMesh::Edge e1 = mesh.edge(h);
            edge_supporting_planes[e1] = sfs;
            SurfaceMesh::Edge e2 = mesh.edge(mesh.next(h));
            edge_supporting_planes[e2] = sfs;

            vertex_supporting_planes[v] = sfs;
            vertex_supporting_planes[v].insert(cutting_plane);
            assert(vertex_supporting_planes[v].size() == 3);

            return h;
        }


        // Cuts f using the cutter and returns the new faces

        std::vector<SurfaceMesh::Face>
        Hypothesis::cut(SurfaceMesh::Face face, const Plane3 *cutting_plane, SurfaceMesh &mesh) {
            std::vector<SurfaceMesh::Face> new_faces;

            // The supporting plane of each face
            auto face_supporting_planes = mesh.face_property<const Plane3 *>("f:supp_plane");
            const Plane3 *supporting_plane = face_supporting_planes[face];

            if (supporting_plane == cutting_plane)
                return new_faces;

            // The supporting planar segment of each face
            auto face_supporting_segments = mesh.face_property<Planar_segment *>("f:supp_segment");

            // The supporting planes of each edge
            auto edge_supporting_planes = mesh.edge_property<std::set<const Plane3 *> >("e:supp_plane");

            Planar_segment *supporting_segment = face_supporting_segments[face];

            std::vector<SurfaceMesh::Vertex> existing_vts;
            std::vector<EdgePos> new_vts;
            compute_intersections(mesh, face, cutting_plane, existing_vts, new_vts);

            // We need to check here because new faces are emerging
            if (existing_vts.size() + new_vts.size() != 2)
                return new_faces;

            else if (existing_vts.size() == 2) {
                // Tests if the two intersecting points are both very close to an existing vertex.
                // Since we allow snapping, we test if the two intersecting points are the same.
                if (existing_vts[0] == existing_vts[1])
                    return new_faces;

                // Tests if an edge already exists, i.e., the plane cuts at this edge
                if (mesh.find_halfedge(existing_vts[0], existing_vts[1]).is_valid())
                    return new_faces;
            }

            SurfaceMesh::Halfedge h0 = SurfaceMesh::Halfedge();
            SurfaceMesh::Halfedge h1 = SurfaceMesh::Halfedge();

            if (existing_vts.size() == 2) { // cutting_plane cuts the face at two existing vertices (not an edge)
                h0 = mesh.opposite(mesh.out_halfedge(existing_vts[0]));
                h1 = mesh.opposite(mesh.out_halfedge(existing_vts[1]));
            } else if (existing_vts.size() == 1) {
                h0 = mesh.opposite(mesh.out_halfedge(existing_vts[0]));
                h1 = split_edge(mesh, new_vts[0], cutting_plane);
            } else if (new_vts.size() == 2) {
                h0 = split_edge(mesh, new_vts[0], cutting_plane);
                h1 = split_edge(mesh, new_vts[1], cutting_plane);
            }
            assert(h0.is_valid());
            assert(h1.is_valid());

            // To split the face, `h0` and `h1` must be incident to the same face
            if (mesh.face(h0) != face) {
                SurfaceMesh::Halfedge end = h0;
                do {
                    h0 = mesh.opposite(mesh.next(h0));        // h0 = h0->next()->opposite();
                    if (mesh.face(h0) == face)
                        break;
                } while (h0 != end);
            }
            assert(mesh.face(h0) == face);

            if (mesh.face(h1) != face) {
                SurfaceMesh::Halfedge end = h1;
                do {
                    h1 = mesh.opposite(mesh.next(h1)); // h1 = h1->next()->opposite();
                    if (mesh.face(h1) == face)
                        break;
                } while (h1 != end);
            }
            assert(mesh.face(h1) == face);

            SurfaceMesh::Halfedge h = mesh.insert_edge(h0, h1);
            if (!h.is_valid() || !mesh.face(h).is_valid()) {
                std::cerr << "Fatal error. could not split face" << std::endl;
                return new_faces;
            }

            SurfaceMesh::Edge e = mesh.edge(h);
            edge_supporting_planes[e].insert(supporting_plane);
            edge_supporting_planes[e].insert(cutting_plane);
            assert(edge_supporting_planes[e].size() == 2);

            // Now the two faces
            SurfaceMesh::Face f1 = mesh.face(h);
            face_supporting_segments[f1] = supporting_segment;
            face_supporting_planes[f1] = supporting_plane;
            new_faces.push_back(f1);

            SurfaceMesh::Face f2 = mesh.face(mesh.opposite(h));
            face_supporting_segments[f2] = supporting_segment;
            face_supporting_planes[f2] = supporting_plane;
            new_faces.push_back(f2);

            return new_faces;
        }


        void Hypothesis::compute_intersections(const SurfaceMesh &mesh,
                                               SurfaceMesh::Face face, const Plane3 *cutting_plane,
                                               std::vector<SurfaceMesh::Vertex> &existing_vts,
                                               std::vector<EdgePos> &new_vts) {
            existing_vts.clear();
            new_vts.clear();

            // The supporting plane of each face
            auto face_supporting_planes = mesh.get_face_property<const Plane3 *>("f:supp_plane");
            const Plane3 *supporting_plane = face_supporting_planes[face];
            if (supporting_plane == cutting_plane)
                return;

            auto edge_supporting_planes = mesh.get_edge_property<std::set<const Plane3 *> >("e:supp_plane");

            auto coords = mesh.get_vertex_property<vec3>("v:point");

            SurfaceMesh::Halfedge cur = mesh.halfedge(face);
            SurfaceMesh::Halfedge end = cur;
            do {
                SurfaceMesh::Edge ed = mesh.edge(cur);
                const std::set<const Plane3 *> &supporting_planes = edge_supporting_planes[ed];
                if (supporting_planes.find(cutting_plane) !=
                    supporting_planes.end()) // the edge lies on the cutting plane
                    return;

                SurfaceMesh::Vertex s_vd = mesh.source(cur);
                SurfaceMesh::Vertex t_vd = mesh.target(cur);
                const vec3 &s = coords[s_vd];
                const vec3 &t = coords[t_vd];

                int s_side = cutting_plane->orient(s);
                int t_side = cutting_plane->orient(t);

                if (t_side == 0) {
                    if (s_side == 0)  // the edge lies on the cutting plane
                        return;
                    else
                        existing_vts.push_back(t_vd);
                } else if ((s_side == 1 && t_side == -1) ||
                           (s_side == -1 && t_side == 1)) // intersects at the interior of the edge
                {
                    float s_sdist = cutting_plane->squared_ditance(s);
                    float t_sdist = cutting_plane->squared_ditance(t);

                    if (s_sdist <= snap_squared_distance_threshold())                // plane cuts at vertex 's'
                        existing_vts.push_back(s_vd);
                    else if (t_sdist <= snap_squared_distance_threshold()) // plane cuts at vertex 't'
                        existing_vts.push_back(t_vd);
                    else {
                        const Plane3 *plane1 = *(supporting_planes.begin());
                        const Plane3 *plane2 = *(supporting_planes.rbegin());
                        const Plane3 *plane3 = const_cast<const Plane3 *>(cutting_plane);

                        if (plane3 != plane1 && plane3 != plane2) {
                            internal::sort_increasing(plane1, plane2, plane3);
                            const vec3 *p = query_intersection(plane1, plane2, plane3);
                            if (p) {
                                if (distance2(*p, s) <= snap_squared_distance_threshold())             // snap to 's'
                                    existing_vts.push_back(s_vd);
                                else if (distance2(*p, t) <= snap_squared_distance_threshold())        // snap to 't'
                                    existing_vts.push_back(t_vd);
                                else
                                    new_vts.push_back(EdgePos(ed, p));
                            } else
                                std::cerr << "Fatal error: should have intersection" << std::endl;
                        } else
                            std::cerr << "Fatal error: should not have duplicated planes." << std::endl;
                    }
                } else {
                    // Nothing needs to do here, we will test the next edge
                }

                cur = mesh.next(cur);
            } while (cur != end);
        }

        // Tests if face 'f' insects plane 'plane'
        bool Hypothesis::do_intersect(const SurfaceMesh &mesh, SurfaceMesh::Face f, const Plane3 *plane) {
            std::vector<SurfaceMesh::Vertex> existing_vts;
            std::vector<EdgePos> new_vts;
            compute_intersections(mesh, f, plane, existing_vts, new_vts);

            if (existing_vts.size() == 2) {
                if (!mesh.find_halfedge(existing_vts[0], existing_vts[1]).is_valid())
                    return true;
            } else if (existing_vts.size() + new_vts.size() == 2)
                return true;

            return false;
        }


        // Collects all faces in 'mesh' that intersect 'face', stored in std::set() so easier to erase

        std::set<SurfaceMesh::Face> Hypothesis::collect_intersecting_faces(SurfaceMesh::Face face, const SurfaceMesh &mesh) {
            // The supporting plane of each face
            auto face_supporting_planes = mesh.get_face_property<const Plane3 *>("f:supp_plane");

            // The supporting planar segment of each face
            auto face_supporting_segments = mesh.get_face_property<Planar_segment *>("f:supp_segment");

            std::set<SurfaceMesh::Face> intersecting_faces;
            for (auto f: mesh.faces()) {
                if (f == face ||
                    face_supporting_segments[f] == face_supporting_segments[face] ||
                    face_supporting_planes[f] == face_supporting_planes[face]) {
                    continue;
                }

                const Plane3 *plane = face_supporting_planes[face];
                assert(plane != nullptr);
                if (do_intersect(mesh, f, plane))
                    intersecting_faces.insert(f);
            }
            return intersecting_faces;
        }


        void Hypothesis::pairwise_intersection() {
            // Pre-computes all potential intersection of plane triplets
            compute_triplet_intersections();

            // Since we are going to split faces, we can not use the reference.
            // const SurfaceMesh::Face_range& all_faces = mesh.faces();
            // So we make a local copy
            std::vector<SurfaceMesh::Face> all_faces(candidate_faces.faces().begin(), candidate_faces.faces().end());

            // The supporting plane of each face
            typename SurfaceMesh::template Property_map<SurfaceMesh::Face, const Plane3 *> face_supporting_planes
                    = candidate_faces.template property_map<SurfaceMesh::Face, const Plane3 *>("f:supp_plane").first;

            for (std::size_t i = 0; i < all_faces.size(); ++i) {
                SurfaceMesh::Face face = all_faces[i];
                const Plane3 *face_plane = face_supporting_planes[face];

                std::set<SurfaceMesh::Face> intersecting_faces = collect_intersecting_faces(face, candidate_faces);
                if (intersecting_faces.empty())
                    continue;

                std::vector<SurfaceMesh::Face> cutting_faces(intersecting_faces.begin(), intersecting_faces.end());

                // 1. 'face' will be cut by all the intersecting faces
                //    \note After each cut, the original face doesn't exist any more and it is replaced by multiple pieces.
                //          then each piece will be cut by another face.
                std::vector<SurfaceMesh::Face> faces_to_be_cut;
                faces_to_be_cut.push_back(face);
                while (!intersecting_faces.empty()) {
                    SurfaceMesh::Face cutting_face = *(intersecting_faces.begin());
                    const Plane3 *cutting_plane = face_supporting_planes[cutting_face];

                    std::set<SurfaceMesh::Face> new_faces;                // stores the new faces
                    std::set<SurfaceMesh::Face> remained_faces;        // faces that will be cut later
                    for (std::size_t j = 0; j < faces_to_be_cut.size(); ++j) {
                        SurfaceMesh::Face current_face = faces_to_be_cut[j];
                        std::vector<SurfaceMesh::Face> tmp = cut(current_face, cutting_plane, candidate_faces);
                        new_faces.insert(tmp.begin(), tmp.end());
                        if (tmp.empty()) {  // no actual cut occurred. The face will be cut later
                            remained_faces.insert(current_face);
                        }
                    }

                    // The new faces might be cut by other faces
                    faces_to_be_cut = std::vector<SurfaceMesh::Face>(new_faces.begin(), new_faces.end());

                    // Don't forget the remained faces
                    faces_to_be_cut.insert(faces_to_be_cut.end(), remained_faces.begin(), remained_faces.end());

                    // The job of cutting_face is done, remove it
                    intersecting_faces.erase(cutting_face);
                }

                // 2. All the cutting_faces will be cut by f.
                for (std::size_t j = 0; j < cutting_faces.size(); ++j) {
                    cut(cutting_faces[j], face_plane, candidate_faces);
                }
            }

            assert(candidate_faces.is_valid());
        }


        Hypothesis::Adjacency Hypothesis::extract_adjacency(const SurfaceMesh &candidate_faces) {
            auto vertex_supporting_planes = candidate_faces.get_vertex_property<std::set<const Plane3 *> >("v:supp_plane");

            // An edge is denoted by its two end points
            typedef typename std::unordered_map<const vec3 *, std::set<SurfaceMesh::Halfedge> > Edge_map;
            typedef typename std::unordered_map<const vec3 *, Edge_map> Face_pool;
            Face_pool face_pool;

            for (auto h: candidate_faces.halfedges()) {
                SurfaceMesh::Face f = candidate_faces.face(h);
                if (!f.is_valid())
                    continue;

                SurfaceMesh::Vertex sd = candidate_faces.source(h);
                SurfaceMesh::Vertex td = candidate_faces.target(h);
                const std::set<const Plane3 *> &set_s = vertex_supporting_planes[sd];
                const std::set<const Plane3 *> &set_t = vertex_supporting_planes[td];
                assert(set_s.size() == 3);
                assert(set_t.size() == 3);

                std::vector<const Plane3 *> s_planes(set_s.begin(), set_s.end());
                assert(s_planes[0] < s_planes[1]);
                assert(s_planes[1] < s_planes[2]);
                const vec3 *s = triplet_intersections_[s_planes[0]][s_planes[1]][s_planes[2]];

                std::vector<const Plane3 *> t_planes(set_t.begin(), set_t.end());
                assert(t_planes[0] < t_planes[1]);
                assert(t_planes[1] < t_planes[2]);
                const vec3 *t = triplet_intersections_[t_planes[0]][t_planes[1]][t_planes[2]];

                if (s > t)
                    std::swap(s, t);
                face_pool[s][t].insert(candidate_faces.halfedge(f));
            }

            Adjacency fans;
            typename Face_pool::const_iterator it = face_pool.begin();
            for (; it != face_pool.end(); ++it) {
                const vec3 *s = it->first;
                const Edge_map &tmp = it->second;
                typename Edge_map::const_iterator cur = tmp.begin();
                for (; cur != tmp.end(); ++cur) {
                    const vec3 *t = cur->first;
                    const std::set<SurfaceMesh::Halfedge> &faces = cur->second;
                    Intersection fan;
                    fan.s = s;
                    fan.t = t;
                    fan.insert(fan.end(), faces.begin(), faces.end());
                    fans.push_back(fan);
                }
            }

            return fans;
        }

    }


    PolyFit::PolyFit(const PointCloud *cloud, PointCloud::VertexProperty<int> plane_indices) : error_message_("") {
        if (!cloud) {
            error_message_ = "empty input";
            return;
        }

        internal::EnrichedPointCloud point_set(cloud, plane_indices);
        const std::vector<internal::Planar_segment *> &planar_segments = point_set.planar_segments();
        if (planar_segments.size() < 4) {
            error_message_ = "at least 4 planes required to reconstruct a closed surface mesh (only "
                             + std::to_string(planar_segments.size()) + " provided)";
            return;
        }

        hypothesis_ = new internal::Hypothesis;
        candidate_faces_ = hypothesis_->generate(point_set);

        internal::Candidate_confidences conf;
        candidate_faces_ = conf.compute(point_set, candidate_faces_);
    }


    PolyFit::~PolyFit() {
        delete hypothesis_;
        delete candidate_faces_;
    }


    bool PolyFit::reconstruct(
            PolygonMesh &output_mesh,
            double wt_fitting /* = 0.43 */,
            double wt_coverage /* = 0.27 */,
            double wt_complexity /* = 0.30 */) {
        if (!error_message_.empty()) { // an error has occurred in the constructor
            return false;
        }

        if (candidate_faces_.num_faces() < 4) {
            error_message_ = "at least 4 candidate faces required to reconstruct a closed surface mesh (only "
                             + std::to_string(candidate_faces_.num_faces()) + " computed)";
            return false;
        }

        typedef typename internal::Hypothesis<GeomTraits>::Adjacency Adjacency;
        const Adjacency &adjacency = hypothesis_.extract_adjacency(candidate_faces_);

        // Internal data structure
        SurfaceMesh target_mesh = candidate_faces_;

        // The number of supporting points of each face
        typename SurfaceMesh::template Property_map<SurfaceMesh::Face, std::size_t> face_num_supporting_points =
                target_mesh.template add_property_map<SurfaceMesh::Face, std::size_t>("f:num_supporting_points").first;

        // The area of each face
        typename SurfaceMesh::template Property_map<SurfaceMesh::Face, FT> face_areas =
                target_mesh.template add_property_map<SurfaceMesh::Face, FT>("f:face_area").first;

        // The point covered area of each face
        typename SurfaceMesh::template Property_map<SurfaceMesh::Face, FT> face_covered_areas =
                target_mesh.template add_property_map<SurfaceMesh::Face, FT>("f:covered_area").first;

        // The supporting plane of each face
        typename SurfaceMesh::template Property_map<SurfaceMesh::Face, const Plane3 *> face_supporting_planes =
                target_mesh.template add_property_map<SurfaceMesh::Face, const Plane3 *>("f:supp_plane").first;

        // Gives each face an index
        typename SurfaceMesh::template Property_map<SurfaceMesh::Face, std::size_t> face_indices =
                target_mesh.template add_property_map<SurfaceMesh::Face, std::size_t>("f:index").first;

        double total_points = 0.0;
        std::size_t idx = 0;
        for (auto f: target_mesh.faces()) {
            total_points += face_num_supporting_points[f];
            face_indices[f] = idx;
            ++idx;
        }


        typedef MixedIntegerProgramTraits MIP_Solver;
        typedef typename MixedIntegerProgramTraits::Variable Variable;
        typedef typename MixedIntegerProgramTraits::Linear_objective Linear_objective;
        typedef typename MixedIntegerProgramTraits::Linear_constraint Linear_constraint;

        MIP_Solver solver;

        // Adds variables

        // Binary variables:
        // x[0] ... x[num_faces - 1] : binary labels of all the input faces
        // x[num_faces] ... x[num_faces + num_edges - 1] : binary labels of all the intersecting edges (remain or not)
        // x[num_faces + num_edges] ... x[num_faces + num_edges + num_edges] : binary labels of corner edges (sharp edge of not)

        std::size_t num_faces = target_mesh.number_of_faces();
        std::size_t num_edges(0);

        typedef typename internal::Hypothesis<GeomTraits>::Intersection Intersection;

        std::unordered_map<const Intersection *, std::size_t> edge_usage_status;        // keep or remove an intersecting edges
        for (std::size_t i = 0; i < adjacency.size(); ++i) {
            const Intersection &fan = adjacency[i];
            if (fan.size() == 4) {
                std::size_t var_idx = num_faces + num_edges;
                edge_usage_status[&fan] = var_idx;
                ++num_edges;
            }
        }

        std::size_t total_variables = num_faces + num_edges + num_edges;

        const std::vector<Variable *> &variables = solver.create_variables(total_variables);
        for (std::size_t i = 0; i < total_variables; ++i) {
            Variable *v = variables[i];
            v->set_variable_type(Variable::BINARY);
        }

        // Adds objective

        const typename SurfaceMesh::template Property_map<SurfaceMesh::Vertex, vec3> &coords = target_mesh.points();
        std::vector<vec3> vertices(target_mesh.number_of_vertices());
        idx = 0;
        for (auto v: target_mesh.vertices()) {
            vertices[idx] = coords[v];
            ++idx;
        }

        typedef typename GeomTraits::Iso_cuboid_3 Box;

        const Box &box = CGAL::bounding_box(vertices.begin(), vertices.end());
        FT dx = box.xmax() - box.xmin();
        FT dy = box.ymax() - box.ymin();
        FT dz = box.zmax() - box.zmin();
        FT box_area = FT(2.0) * (dx * dy + dy * dz + dz * dx);

        // Chooses a better scale: all actual values multiplied by total number of points
        double coeff_data_fitting = wt_fitting;
        double coeff_coverage = total_points * wt_coverage / box_area;
        double coeff_complexity = total_points * wt_complexity / double(adjacency.size());

        Linear_objective *objective = solver.create_objective(Linear_objective::MINIMIZE);

        std::unordered_map<const Intersection *, std::size_t> edge_sharp_status;        // the edge is sharp or not
        std::size_t num_sharp_edges = 0;
        for (std::size_t i = 0; i < adjacency.size(); ++i) {
            const Intersection &fan = adjacency[i];
            if (fan.size() == 4) {
                std::size_t var_idx = num_faces + num_edges + num_sharp_edges;
                edge_sharp_status[&fan] = var_idx;

                // Accumulates model complexity term
                objective->add_coefficient(variables[var_idx], coeff_complexity);
                ++num_sharp_edges;
            }
        }
        assert(num_edges == num_sharp_edges);

        for (auto f: target_mesh.faces()) {
            std::size_t var_idx = face_indices[f];

            // Accumulates data fitting term
            std::size_t num = face_num_supporting_points[f];
            objective->add_coefficient(variables[var_idx], -coeff_data_fitting * num);

            // Accumulates model coverage term
            double uncovered_area = (face_areas[f] - face_covered_areas[f]);
            objective->add_coefficient(variables[var_idx], coeff_coverage * uncovered_area);
        }

        // Adds constraints: the number of faces associated with an edge must be either 2 or 0
        std::size_t var_edge_used_idx = 0;
        for (std::size_t i = 0; i < adjacency.size(); ++i) {
            Linear_constraint *c = solver.create_constraint(0.0, 0.0);
            const Intersection &fan = adjacency[i];
            for (std::size_t j = 0; j < fan.size(); ++j) {
                SurfaceMesh::Face f = target_mesh.face(fan[j]);
                std::size_t var_idx = face_indices[f];
                c->add_coefficient(variables[var_idx], 1.0);
            }

            if (fan.size() == 4) {
                std::size_t var_idx = num_faces + var_edge_used_idx;
                c->add_coefficient(variables[var_idx], -2.0);  //
                ++var_edge_used_idx;
            } else { // boundary edge
                // will be set to 0 (i.e., we don't allow open surface)
            }
        }

        // Adds constraints: for the sharp edges. The explanation of posing this constraint can be found here:
        // https://user-images.githubusercontent.com/15526536/30185644-12085a9c-942b-11e7-831d-290dd2a4d50c.png
        double M = 1.0;
        for (std::size_t i = 0; i < adjacency.size(); ++i) {
            const Intersection &fan = adjacency[i];
            if (fan.size() != 4)
                continue;

            // If an edge is sharp, the edge must be selected first:
            // X[var_edge_usage_idx] >= X[var_edge_sharp_idx]
            Linear_constraint *c = solver.create_constraint(0.0);
            std::size_t var_edge_usage_idx = edge_usage_status[&fan];
            c->add_coefficient(variables[var_edge_usage_idx], 1.0);
            std::size_t var_edge_sharp_idx = edge_sharp_status[&fan];
            c->add_coefficient(variables[var_edge_sharp_idx], -1.0);

            for (std::size_t j = 0; j < fan.size(); ++j) {
                SurfaceMesh::Face f1 = target_mesh.face(fan[j]);
                const Plane3 *plane1 = face_supporting_planes[f1];
                std::size_t fid1 = face_indices[f1];
                for (std::size_t k = j + 1; k < fan.size(); ++k) {
                    SurfaceMesh::Face f2 = target_mesh.face(fan[k]);
                    const Plane3 *plane2 = face_supporting_planes[f2];
                    std::size_t fid2 = face_indices[f2];

                    if (plane1 != plane2) {
                        // The constraint is:
                        //X[var_edge_sharp_idx] + M * (3 - (X[fid1] + X[fid2] + X[var_edge_usage_idx])) >= 1
                        // which equals to
                        //X[var_edge_sharp_idx] - M * X[fid1] - M * X[fid2] - M * X[var_edge_usage_idx] >= 1 - 3M
                        c = solver.create_constraint(1.0 - 3.0 * M);
                        c->add_coefficient(variables[var_edge_sharp_idx], 1.0);
                        c->add_coefficient(variables[fid1], -M);
                        c->add_coefficient(variables[fid2], -M);
                        c->add_coefficient(variables[var_edge_usage_idx], -M);
                    }
                }
            }
        }

        // Optimization

        if (solver.solve()) {

            // Marks results
            const std::vector<double> &X = solver.solution();

            std::vector<SurfaceMesh::Face> to_delete;
            std::size_t f_idx(0);
            for (auto f: target_mesh.faces()) {
                if (static_cast<int>(std::round(X[f_idx])) == 0)
                    to_delete.push_back(f);
                ++f_idx;
            }

            for (std::size_t i = 0; i < to_delete.size(); ++i) {
                SurfaceMesh::Face f = to_delete[i];
                SurfaceMesh::Halfedge h = target_mesh.halfedge(f);
                Euler::remove_face(h, target_mesh);
            }

            // Marks the sharp edges
            typename SurfaceMesh::template Property_map<SurfaceMesh::Edge, bool> edge_is_sharp =
                    target_mesh.template add_property_map<SurfaceMesh::Edge, bool>("e:sharp_edges").first;
            for (auto e: target_mesh.edges())
                edge_is_sharp[e] = false;

            for (std::size_t i = 0; i < adjacency.size(); ++i) {
                const Intersection &fan = adjacency[i];
                if (fan.size() != 4)
                    continue;

                std::size_t idx_sharp_var = edge_sharp_status[&fan];
                if (static_cast<int>(X[idx_sharp_var]) == 1) {
                    for (std::size_t j = 0; j < fan.size(); ++j) {
                        SurfaceMesh::Halfedge h = fan[j];
                        SurfaceMesh::Face f = target_mesh.face(h);
                        if (f != SurfaceMesh::null_face()) { // some faces may be deleted
                            std::size_t fid = face_indices[f];
                            if (static_cast<int>(std::round(X[fid])) == 1) {
                                SurfaceMesh::Edge e = target_mesh.edge(h);
                                edge_is_sharp[e] = true;
                                break;
                            }
                        }
                    }
                }
            }

            // Converts from internal data structure to the required `PolygonMesh`.
            output_mesh.clear();        // make sure it is empty.
            CGAL::copy_face_graph(target_mesh, output_mesh);
        } else {
            error_message_ = "solving the binary program failed";
            return false;
        }

        return true;
    }

} // namespace easy3d
