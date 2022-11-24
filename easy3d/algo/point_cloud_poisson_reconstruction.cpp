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

#include <easy3d/algo/point_cloud_poisson_reconstruction.h>

#include <algorithm>

#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/stop_watch.h>

#include <3rd_party/poisson/MyTime.h>
#include <3rd_party/poisson/MemoryUsage.h>
#include <3rd_party/poisson/MarchingCubes.h>
#include <3rd_party/poisson/Octree.h>
#include <3rd_party/poisson/MultiGridOctreeData.h>
#include <3rd_party/poisson/SurfaceTrimmer.h>


namespace easy3d {


#if defined( _WIN32 ) || defined( _WIN64 )
    double PeakMemoryUsageMB(void)
    {
        HANDLE h = GetCurrentProcess();
        PROCESS_MEMORY_COUNTERS pmc;
        return GetProcessMemoryInfo(h, &pmc, sizeof(pmc)) ? ((double)pmc.PeakWorkingSetSize) / (1 << 20) : 0;
    }
#endif // _WIN32 || _WIN64

#define REAL    float

#define DEGREE           2
#define DATA_DEGREE      1        // The order of the B-Spline used to splat in data for color interpolation
#define WEIGHT_DEGREE    2        // The order of the B-Spline used to splat in the weights for density estimation
#define NORMAL_DEGREE    2        // The order of the B-Spline used to splat int the normals for constructing the Laplacian constraints

#define BType  BOUNDARY_NEUMANN


    template<class Real>
    struct OctreeProfiler {
        Octree<Real> &tree;
        double t;

        explicit OctreeProfiler(Octree<Real> &_t) : tree(_t), t(0) {}

        void start() { t = Time(), tree.resetLocalMemoryUsage(); }

        void print(const char *header) const {
            tree.memoryUsage();
#if defined( _WIN32 ) || defined( _WIN64 )
            if (header)
                printf("%s %9.1f (s), %9.1f (MB) / %9.1f (MB) / %9.1f (MB)\n", header, Time() - t, tree.localMemoryUsage(), tree.maxMemoryUsage(), PeakMemoryUsageMB());
            else
                printf("%9.1f (s), %9.1f (MB) / %9.1f (MB) / %9.1f (MB)\n", Time() - t, tree.localMemoryUsage(), tree.maxMemoryUsage(), PeakMemoryUsageMB());
#else // !_WIN32 && !_WIN64
            if (header)
                printf("%s %9.1f (s), %9.1f (MB) / %9.1f (MB)\n", header, Time() - t, tree.localMemoryUsage(),
                       tree.maxMemoryUsage());
            else
                printf("%9.1f (s), %9.1f (MB) / %9.1f (MB)\n", Time() - t, tree.localMemoryUsage(),
                       tree.maxMemoryUsage());
#endif // _WIN32 || _WIN64
        }
    };


    PoissonReconstruction::PoissonReconstruction()
            : depth_(8), samples_per_node_(1.0f), triangulate_mesh_(true) {
        // other default parameters
        full_depth_ = 5;
        cgDepth_ = 0;
        scale_ = 1.1f;
        pointWeight_ = 4.0f;
        gsIter_ = 8;
        threads_ = omp_get_num_procs();
        LOG(INFO) << "number of threads: " << threads_;

        verbose_ = false;
    }

    PoissonReconstruction::~PoissonReconstruction() = default;

    // \cond
    namespace internal {

        template<class Vertex>
        SurfaceMesh *
        convert_to_mesh(CoredFileMeshData<Vertex> &mesh, const XForm4x4<REAL> &iXForm,
                        const std::string &density_attr_name,
                        bool has_colors) {
            std::size_t num_ic_pts = mesh.inCorePoints.size();
            int num_ooc_pts = mesh.outOfCorePointCount();
            int num_face = mesh.polygonCount();
            if (num_face <= 0) {
                LOG(ERROR) << "reconstructed mesh has 0 facet";
                return nullptr;
            }

            auto result = new SurfaceMesh;
            SurfaceMesh::VertexProperty<float> density = result->add_vertex_property<float>(density_attr_name);
            SurfaceMesh::VertexProperty<vec3> color;
            if (has_colors)
                color = result->add_vertex_property<vec3>("v:color");

            std::vector<SurfaceMesh::Vertex> all_vertices;

            REAL min_density = FLT_MAX;
            REAL max_density = -FLT_MAX;
            mesh.resetIterator();
            for (std::size_t i = 0; i < num_ic_pts; ++i) {
                const Vertex &v = mesh.inCorePoints[i];
                const Point3D<REAL> &pt = iXForm * v.point;
                SurfaceMesh::Vertex vv = result->add_vertex(vec3(pt.coords[0], pt.coords[1], pt.coords[2]));
                all_vertices.push_back(vv);
                density[vv] = v.value;
                min_density = std::min(min_density, v.value);
                max_density = std::max(max_density, v.value);

                if (has_colors) {
                    vec3 c(v.color);
                    color[vv] = c / 255.0f;
                }
            }

            for (int i = 0; i < num_ooc_pts; ++i) {
                Vertex v;
                mesh.nextOutOfCorePoint(v);
                const Point3D<REAL> &pt = iXForm * v.point;
                SurfaceMesh::Vertex vv = result->add_vertex(vec3(pt.coords[0], pt.coords[1], pt.coords[2]));
                all_vertices.push_back(vv);
                density[vv] = v.value;
                min_density = std::min(min_density, v.value);
                max_density = std::max(max_density, v.value);

                if (has_colors) {
                    vec3 c(v.color);
                    color[vv] = c / 255.0f;
                }
            }

            for (int i = 0; i < num_face; ++i) {
                std::vector<CoredVertexIndex> vertices;
                mesh.nextPolygon(vertices);

                std::vector<SurfaceMesh::Vertex> face_vts;
                for (auto v : vertices) {
                    std::size_t id = v.idx;
                    if (!v.inCore)
                        id += num_ic_pts;
                    face_vts.push_back(all_vertices[id]);
                }
                result->add_face(face_vts);
            }

            LOG(INFO)
                    << "vertex property \'" << density_attr_name << "\' added with range ["
                    << min_density << ", " << max_density << "]";
            if (has_colors)
                LOG(INFO) << "vertex property 'v:color' added.";

            return result;
        }
    }
    // \endcond

    SurfaceMesh *PoissonReconstruction::apply(const PointCloud *cloud, const std::string &density_attr_name) const {
        if (!cloud) {
            LOG(ERROR) << "nullptr point cloud";
            return nullptr;
        }

        PointCloud::VertexProperty<vec3> normals = cloud->get_vertex_property<vec3>("v:normal");
        if (!normals) {
            LOG(ERROR) << "normal information not exist for Poisson surface reconstruction method";
            return nullptr;
        }

        typedef typename Octree<REAL>::template DensityEstimator<WEIGHT_DEGREE> DensityEstimator;
        typedef typename Octree<REAL>::template InterpolationInfo<false> InterpolationInfo;

        REAL isoValue = 0;

        //if (OctNode< TreeNodeData >::NodeAllocator.blockSize != MEMORY_ALLOCATOR_BLOCK_SIZE)
        OctNode<TreeNodeData>::SetAllocator(MEMORY_ALLOCATOR_BLOCK_SIZE);

        Reset<REAL>();
        Octree<REAL> tree;
        OctreeProfiler<REAL> profiler(tree);
        tree.threads = threads_;

        int maxSolveDepth = depth_;
        int kernelDepth = depth_ - 2;
        if (kernelDepth > depth_) {
            LOG(ERROR) << "kernelDepth (" << kernelDepth << ") cannot be greater than tree depth (" << depth_ << ")";
            kernelDepth = depth_;
        }

        //////////////////////////////////////////////////////////////////////////

        LOG(INFO) << "Screened Poisson Reconstruction (V9.0.1)";
        StopWatch t, w;

        //////////////////////////////////////////////////////////////////////////

        int pointCount;

        REAL pointWeightSum;
        auto samples = new std::vector<typename Octree<REAL>::PointSample>();
        std::vector<ProjectiveData<Point3D<REAL>, REAL> > *sampleData = nullptr;
        DensityEstimator *density = nullptr;
        SparseNodeData<Point3D<REAL>, NORMAL_DEGREE> *normalInfo = nullptr;
        REAL targetValue = (REAL) 0.5;
        XForm4x4<REAL> xForm, iXForm;

        { // Load the samples (and color data)
            LOG(INFO) << "loading data into tree... ";
            t.restart();
            profiler.start();
            const float *pts = cloud->points()[0];
            const float *nms = normals.vector()[0];
            const float *cls = nullptr;
            PointCloud::VertexProperty<vec3> colors = cloud->get_vertex_property<vec3>("v:color");
            if (colors) {
                cls = colors.vector()[0];
                sampleData = new std::vector<ProjectiveData<Point3D<REAL>, REAL> >();
            }

            xForm = XForm4x4<REAL>::Identity();
            {
                const Box3 &box = cloud->bounding_box();

                Point3D<REAL> min_p(box.min_coord(0), box.min_coord(1), box.min_coord(2)), max_p(box.max_coord(0), box.max_coord(1), box.max_coord(2));
                Point3D<REAL> center = (max_p + min_p) / 2;
                REAL scale = std::max<REAL>(max_p[0] - min_p[0],
                                            std::max<REAL>(max_p[1] - min_p[1], max_p[2] - min_p[2]));
                scale *= scale_;
                for (int i = 0; i < 3; i++)
                    center[i] -= scale / 2;
                XForm4x4<REAL> tXForm = XForm4x4<REAL>::Identity(), sXForm = XForm4x4<REAL>::Identity();
                for (int i = 0; i < 3; i++)
                    sXForm(i, i) = (REAL) (1. / scale), tXForm(3, i) = -center[i];
                xForm = (sXForm * tXForm) * xForm;
            }

            pointCount = tree.init<Point3D<REAL> >(cloud->n_vertices(), pts, nms, cls, xForm, maxSolveDepth, false,
                                                   *samples, sampleData);
            iXForm = xForm.inverse();

#pragma omp parallel for num_threads(threads_)
            for (int i = 0; i < (int) samples->size(); i++)
                (*samples)[i].sample.data.n *= (REAL) -1;

            if (verbose_)
                profiler.print(" - Load input into tree: ");

            LOG(INFO) << "input points/samples: " << pointCount << "/" << samples->size() <<
                      ". memory usage: " << float(MemoryInfo::Usage()) / (1 << 20) << " MB. " << t.time_string();
        }

        //////////////////////////////////////////////////////////////////////////

        DenseNodeData<REAL, DEGREE> solution;
        {
            DenseNodeData<REAL, DEGREE> constraints;
            InterpolationInfo *iInfo = nullptr;
            int solveDepth = maxSolveDepth;

            tree.resetNodeIndices();

            // Get the kernel density estimator [If discarding, compute anew. Otherwise, compute once.]
            {
                t.restart();
                profiler.start();
                density = tree.setDensityEstimator<WEIGHT_DEGREE>(*samples, kernelDepth, samples_per_node_);
                if (verbose_)
                    profiler.print(" - Got kernel density:   ");
            }

            // Transform the Hermite samples into a vector field [If discarding, compute anew. Otherwise, compute once.]
            {
                LOG(INFO) << "setting normal field... ";
                t.restart();
                profiler.start();
                normalInfo = new SparseNodeData<Point3D<REAL>, NORMAL_DEGREE>();
                *normalInfo = tree.setNormalField<NORMAL_DEGREE>(*samples, *density, pointWeightSum, true);
                if (verbose_)
                    profiler.print(" - Got normal field:     ");

                LOG(INFO) << "memory usage: " << float(MemoryInfo::Usage()) / (1 << 20) << " MB. "
                          << t.time_string();
            }

            // Trim the tree and prepare for multi-grid
            {
                LOG(INFO) << "trimming tree and preparing for multi-grid... ";

                t.restart();
                profiler.start();
                std::vector<int> indexMap;

                constexpr int MAX_DEGREE = (NORMAL_DEGREE > DEGREE) ? NORMAL_DEGREE : DEGREE;
                tree.inalizeForBroodedMultigrid<MAX_DEGREE, DEGREE, BType>(full_depth_,
                                                                           typename Octree<REAL>::template HasNormalDataFunctor<NORMAL_DEGREE>(
                                                                                   *normalInfo), &indexMap);

                normalInfo->remapIndices(indexMap);
                density->remapIndices(indexMap);
                if (verbose_)
                    profiler.print(" - Finalized tree:       ");

                LOG(INFO) << "memory usage: " << float(MemoryInfo::Usage()) / (1 << 20) << " MB. "
                          << t.time_string();
            }

            // Add the FEM constraints
            {
                t.restart();
                profiler.start();
                constraints = tree.initDenseNodeData<DEGREE>();
                tree.addFEMConstraints<DEGREE, BType, NORMAL_DEGREE, BType>(
                        FEMVFConstraintFunctor<NORMAL_DEGREE, BType, DEGREE, BType>(1., 0.), *normalInfo, constraints,
                        solveDepth);
                if (verbose_)
                    profiler.print(" - Set FEM constraints:  ");
            }

            // Free up the normal info [If we don't need it for subsequent iterations.]
            delete normalInfo;
            normalInfo = nullptr;

            // Add the interpolation constraints
            if (pointWeight_ > 0) {
                t.restart();
                profiler.start();
                int AdaptiveExponent = 1;
                iInfo = new InterpolationInfo(tree, *samples, targetValue, AdaptiveExponent,
                                              (REAL) pointWeight_ * pointWeightSum, (REAL) 0);
                tree.addInterpolationConstraints<DEGREE, BType>(*iInfo, constraints, solveDepth);
                if (verbose_)
                    profiler.print(" - Set point constraints:");
            }

            if (verbose_) {
                LOG(INFO) << " - Leaf Nodes / Active Nodes / Ghost Nodes: " << (int) tree.leaves() << " / "
                          << (int) tree.nodes() << " / " << (int) tree.ghostNodes();
            }

            // Solve the linear system
            {
                LOG(INFO) << "solving the linear system... ";

                t.restart();
                profiler.start();
                typename Octree<REAL>::SolverInfo solverInfo;

                bool showResidual = false;
                REAL solverAccuracy = 1e-3f;

                solverInfo.cgDepth = cgDepth_, solverInfo.iters = gsIter_, solverInfo.cgAccuracy = solverAccuracy, solverInfo.verbose = verbose_, solverInfo.showResidual = showResidual, solverInfo.lowResIterMultiplier = std::max<double>(
                        1., 1.);
                solution = tree.solveSystem<DEGREE, BType>(FEMSystemFunctor<DEGREE, BType>(0, 1., 0), iInfo,
                                                           constraints, solveDepth, solverInfo);
                if (verbose_)
                    profiler.print(" - Linear system solved: ");
                if (iInfo)
                    delete iInfo, iInfo = nullptr;

                LOG(INFO) << "memory usage: " << float(MemoryInfo::Usage()) / (1 << 20) << " MB. "
                          << t.time_string();
            }
        }

        // 	CoredFileMeshData< PlyVertex< Real > > mesh;		// no depth recorded, so can not trim
        // 	CoredFileMeshData< PlyColorVertex< Real > > mesh;	// no depth, but has color
        CoredFileMeshData<PlyColorAndValueVertex<REAL> > mesh;
        {
            t.restart();
            profiler.start();
            double valueSum = 0, weightSum = 0;
            typename Octree<REAL>::template MultiThreadedEvaluator<DEGREE, BType> evaluator(&tree, solution, threads_);
#pragma omp parallel for num_threads(threads_) reduction( + : valueSum, weightSum )
            for (int j = 0; j < samples->size(); j++) {
                ProjectiveData<OrientedPoint3D<REAL>, REAL> &sample = (*samples)[j].sample;
                if (sample.weight > 0)
                    weightSum += sample.weight, valueSum += evaluator.value(sample.data.p / sample.weight, omp_get_thread_num(),
                                                                (*samples)[j].node) * sample.weight;
            }
            isoValue = (REAL) (valueSum / weightSum);

            PointCloud::VertexProperty<vec3> colors = cloud->get_vertex_property<vec3>("v:color");
            if (!colors && samples)
                delete samples, samples = nullptr;
            if (verbose_) {
                profiler.print(" - Got average:          ");
                LOG(INFO) << " - Iso-Value: " << isoValue;
            }
        }

        {
            LOG(INFO) << "extracting mesh... ";

            t.restart();
            profiler.start();
            SparseNodeData<ProjectiveData<Point3D<REAL>, REAL>, DATA_DEGREE> *colorData = nullptr;
            if (sampleData) {
                float colorValue = 16.0f;
                colorData = new SparseNodeData<ProjectiveData<Point3D<REAL>, REAL>, DATA_DEGREE>();
                *colorData = tree.template setDataField<DATA_DEGREE, false>(*samples, *sampleData,
                                                                            (DensityEstimator *) nullptr);
                delete sampleData, sampleData = nullptr;
                for (const OctNode<TreeNodeData> *n = tree.tree().nextNode(); n; n = tree.tree().nextNode(n)) {
                    ProjectiveData<Point3D<REAL>, REAL> *clr = (*colorData)(n);
                    if (clr)
                        (*clr) *= (REAL) pow(colorValue, tree.depth(n));
                }
            }

            bool nonLinearFit = true;
            bool addBarycenter = true;
            tree.getMCIsoSurface<DEGREE, BType, WEIGHT_DEGREE, DATA_DEGREE>(density, colorData, solution, isoValue,
                                                                            mesh, nonLinearFit, addBarycenter,
                                                                            !triangulate_mesh_);

            if (verbose_) {
                LOG(INFO) << " - Vertices / Polygons: " << mesh.outOfCorePointCount() + mesh.inCorePoints.size()
                          << " / " << mesh.polygonCount();
                if (!triangulate_mesh_)
                    profiler.print(" - Got polygons:         ");
                else
                    profiler.print(" - Got triangles:        ");
            }

            if (colorData) {
                delete colorData;
                colorData = nullptr;
            }
            if (density) {
                delete density;
                density = nullptr;
            }
            if (samples) {
                delete samples;
                samples = nullptr;
            }

            LOG(INFO) << "memory usage: " << float(MemoryInfo::Usage()) / (1 << 20) << " MB. " << t.time_string();
        }

        //////////////////////////////////////////////////////////////////////////

        PointCloud::VertexProperty<vec3> colors = cloud->get_vertex_property<vec3>("v:color");
        SurfaceMesh *result = internal::convert_to_mesh(mesh, iXForm, density_attr_name, colors);
        const std::string &file_name = file_system::name_less_extension(cloud->name()) + "_Poisson.ply";
        result->set_name(file_name);
        LOG(INFO) << "total reconstruction time: " << w.time_string();

        return result;
    }


    SurfaceMesh *PoissonReconstruction::trim(
            SurfaceMesh *mesh,
            const std::string &density_attr_name,
            float trim_value,
            float area_ratio,
            bool triangulate) {
        if (!mesh)
            return nullptr;

        auto density = mesh->get_vertex_property<float>(density_attr_name);
        if (!density) {
            LOG(ERROR) << "density is not available";
            return nullptr;
        }

        auto color = mesh->get_vertex_property<vec3>("v:color");
        bool has_color = color;

        std::vector<PlyColorAndValueVertex<REAL> > vertices;
        std::vector<std::vector<int> > polygons;

        auto points = mesh->get_vertex_property<vec3>("v:point");
        for (auto vv : mesh->vertices()) {
            const vec3 &p = points[vv];
            PlyColorAndValueVertex<REAL> v;
            v.point = Point3D<REAL>(p.x, p.y, p.z);
            v.value = density[vv];
            if (has_color) {
                const vec3 &c = color[vv];
                v.color[0] = static_cast<unsigned char>(c.r * 255); // Liangliang: I don't want to change Misha's code
                v.color[1] = static_cast<unsigned char>(c.g * 255);
                v.color[2] = static_cast<unsigned char>(c.b * 255);
            }
            vertices.push_back(v);
        }

        for (auto f : mesh->faces()) {
            std::vector<int> plg;
            SurfaceMesh::VertexAroundFaceCirculator cir(mesh, f);
            SurfaceMesh::VertexAroundFaceCirculator end = cir;
            do {
                int id = (*cir).idx();
                plg.push_back(id);
                ++cir;
            } while (cir != end);
            polygons.push_back(plg);
        }

//	StopWatch t; t.start();
        LOG(INFO) << "Surface Trimmer (V5)";
        trim_mesh(vertices, polygons, REAL(trim_value), REAL(area_ratio), triangulate);

        //////////////////////////////////////////////////////////////////////////

        auto trimmed_mesh = new SurfaceMesh;
        density = trimmed_mesh->add_vertex_property<float>(density_attr_name);
        if (has_color)
            color = trimmed_mesh->add_vertex_property<vec3>("v:color");

        std::vector<SurfaceMesh::Vertex> all_vertices;
        for (auto vert : vertices) {
            const Point3D<REAL> &pt = vert.point;
            SurfaceMesh::Vertex v = trimmed_mesh->add_vertex(vec3(pt.coords[0], pt.coords[1], pt.coords[2]));
            all_vertices.push_back(v);
            density[v] = vert.value;

            if (has_color) {
                unsigned char *c = vert.color;
                color[v] = vec3(static_cast<float>(c[0]) / 255.0f, static_cast<float>(c[1]) / 255.0f, static_cast<float>(c[2]) / 255.0f);
            }
        }

        for (const auto& plg : polygons) {
            std::vector<SurfaceMesh::Vertex> face_vts;
            for (auto idx : plg) {
                face_vts.push_back(all_vertices[idx]);
            }
            trimmed_mesh->add_face(face_vts);
        }

//    LOG(INFO) << "done. " << t.time_string();

        return trimmed_mesh;
    }

} // namespace easy3d
