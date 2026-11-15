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
#include <chrono>
#include <cmath>
#include <set>

#include <easy3d/algo/delaunay_2d.h>
#include <easy3d/algo/delaunay_3d.h>
#include <easy3d/algo/point_cloud_normals.h>
#include <easy3d/algo/point_cloud_poisson_reconstruction.h>
#include <easy3d/algo/point_cloud_ransac.h>
#include <easy3d/algo/point_cloud_simplification.h>
#include <easy3d/algo/surface_mesh_components.h>
#include <easy3d/algo/surface_mesh_enumerator.h>
#include <easy3d/algo/surface_mesh_fairing.h>
#include <easy3d/algo/surface_mesh_geodesic.h>
#include <easy3d/algo/surface_mesh_geometry.h>
#include <easy3d/algo/surface_mesh_hole_filling.h>
#include <easy3d/algo/surface_mesh_parameterization.h>
#include <easy3d/algo/surface_mesh_polygonization.h>
#include <easy3d/algo/surface_mesh_remeshing.h>
#include <easy3d/algo/surface_mesh_sampler.h>
#include <easy3d/algo/surface_mesh_simplification.h>
#include <easy3d/algo/surface_mesh_smoothing.h>
#include <easy3d/algo/surface_mesh_subdivision.h>
#include <easy3d/algo/surface_mesh_triangulation.h>
#ifdef HAS_CGAL
#include <easy3d/algo_ext/surfacer.h>
#endif
#include <easy3d/core/graph.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/poly_mesh.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/texture.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>

#include <3rd_party/imgui/imgui.h>

using namespace easy3d;


const char *MappleViewer::process_title(ProcessOp op) {
    switch (op) {
        case ProcessOp::PC_ESTIMATE_NORMALS:
            return "Estimate Normals";
        case ProcessOp::PC_REORIENT_NORMALS:
            return "Re-orient Normals";
        case ProcessOp::PC_SIMPLIFY:
            return "Simplify Point Cloud";
        case ProcessOp::PC_POISSON:
            return "Poisson Reconstruction";
        case ProcessOp::PC_RANSAC:
            return "Extract Primitives";
        case ProcessOp::PC_DELAUNAY_2D:
            return "Delaunay 2D (XY)";
        case ProcessOp::PC_DELAUNAY_3D:
            return "Delaunay 3D";
        case ProcessOp::MESH_SMOOTH:
            return "Smooth Surface Mesh";
        case ProcessOp::MESH_SIMPLIFY:
            return "Simplify Surface Mesh";
        case ProcessOp::MESH_REMESH:
            return "Remesh Surface Mesh";
        case ProcessOp::MESH_FILL_HOLES:
            return "Fill Holes";
        case ProcessOp::MESH_POLYGONIZE:
            return "Polygonize";
        case ProcessOp::MESH_FAIRING:
            return "Fair Surface Mesh";
        case ProcessOp::MESH_SAMPLE:
            return "Sample Surface";
        case ProcessOp::MESH_COMPONENTS:
            return "Connected Components";
        case ProcessOp::MESH_PLANAR_PARTITION:
            return "Planar Partition";
        case ProcessOp::MESH_TRIANGULATE:
            return "Triangulate";
        case ProcessOp::MESH_DUAL:
            return "Dual Mesh";
        case ProcessOp::MESH_SUBDIVIDE_CC:
            return "Subdivide (Catmull-Clark)";
        case ProcessOp::MESH_SUBDIVIDE_LOOP:
            return "Subdivide (Loop)";
        case ProcessOp::MESH_SUBDIVIDE_SQRT3:
            return "Subdivide (Sqrt3)";
        case ProcessOp::MESH_PARAMETERIZE:
            return "Parameterize Surface Mesh";
        case ProcessOp::MESH_GEODESIC:
            return "Geodesic Distance";
        case ProcessOp::MESH_CLIP:
            return "Clip by Clipping Plane";
        case ProcessOp::MESH_SPLIT:
            return "Split by Clipping Plane";
        case ProcessOp::MESH_SLICE:
            return "Slice by Clipping Plane";
        default:
            return "";
    }
}

void MappleViewer::open_process(ProcessOp op) {
    if (process_job_)
        return;
    Model *model = current_model();
    if (!model)
        return;

    auto *cloud = dynamic_cast<PointCloud *>(model);
    auto *mesh = dynamic_cast<SurfaceMesh *>(model);
    const bool point_cloud_op = op == ProcessOp::PC_ESTIMATE_NORMALS ||
                                op == ProcessOp::PC_REORIENT_NORMALS || op == ProcessOp::PC_SIMPLIFY ||
                                op == ProcessOp::PC_POISSON || op == ProcessOp::PC_RANSAC ||
                                op == ProcessOp::PC_DELAUNAY_2D || op == ProcessOp::PC_DELAUNAY_3D;
    if ((point_cloud_op && !cloud) || (!point_cloud_op && !mesh))
        return;

    // Targets depend on the current model; refresh them so reopening a window always
    // starts from a sensible value.
    if (op == ProcessOp::PC_SIMPLIFY && cloud)
        process_params_.point_target = static_cast<int>(std::max(1u, cloud->n_vertices() / 2));
    if (op == ProcessOp::MESH_SIMPLIFY && mesh)
        process_params_.simplify_target = static_cast<int>(std::max(4u, mesh->n_vertices() / 2));
    if (op == ProcessOp::MESH_SAMPLE && mesh)
        process_params_.sample_points = static_cast<int>(std::max<std::size_t>(1000000, mesh->n_vertices() + 1));

    process_op_ = op;
}

void MappleViewer::run_process(ProcessOp op) {
    // A parameter window owns the command slot while it is open.
    if (process_op_ != ProcessOp::NONE)
        return;
    open_process(op);
    if (process_op_ == op)
        apply_process();
}

void MappleViewer::draw_process_dialog() {
    if (process_op_ == ProcessOp::NONE)
        return;
    Model *model = current_model();
    if (!model) {
        process_op_ = ProcessOp::NONE;
        return;
    }

    // Dim the scene and panels; the window itself stays fully lit.
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::GetBackgroundDrawList()->AddRectFilled(
            viewport->Pos, ImVec2(viewport->Pos.x + viewport->Size.x, viewport->Pos.y + viewport->Size.y),
            IM_COL32(0, 0, 0, 96));

    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(360.0f, 0.0f), ImGuiCond_Appearing);
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                                   ImGuiWindowFlags_AlwaysAutoResize;
    const bool visible = ImGui::Begin(process_title(process_op_), nullptr, flags);

    if (visible) {
        ImGui::TextDisabled("%s", model->name().c_str());
        ImGui::Separator();
        if (auto *mesh = dynamic_cast<SurfaceMesh *>(model))
            ImGui::Text("%u vertices, %u faces", mesh->n_vertices(), mesh->n_faces());
        else if (auto *cloud = dynamic_cast<PointCloud *>(model))
            ImGui::Text("%u points", cloud->n_vertices());
        ImGui::Spacing();

        ProcessParams &p = process_params_;
        switch (process_op_) {
            case ProcessOp::PC_ESTIMATE_NORMALS:
                ImGui::InputInt("Neighbors", &p.normals_k);
                ImGui::Checkbox("Compute curvature", &p.compute_curvature);
                break;

            case ProcessOp::PC_REORIENT_NORMALS:
                ImGui::InputInt("Neighbors", &p.normals_k);
                ImGui::TextDisabled("Orients the existing normals consistently; requires normals.");
                break;

            case ProcessOp::PC_SIMPLIFY:
                ImGui::InputInt("Remaining points", &p.point_target);
                ImGui::TextDisabled("Keeps a uniform subset of the current points.");
                break;

            case ProcessOp::PC_POISSON:
                ImGui::InputInt("Octree depth", &p.poisson_depth);
                ImGui::InputFloat("Samples per node", &p.poisson_samples, 1.0f, 5.0f, "%.1f");
                ImGui::Checkbox("Trim result", &p.poisson_trim);
                ImGui::BeginDisabled(!p.poisson_trim);
                ImGui::InputFloat("Trim density", &p.poisson_trim_value, 1.0f, 5.0f, "%.1f");
                ImGui::InputFloat("Island ratio", &p.poisson_island_ratio, 0.001f, 0.01f, "%.4f");
                ImGui::EndDisabled();
                ImGui::TextDisabled("Adds a new mesh; requires normals.");
                ImGui::TextDisabled("Runs in the background and can take a while.");
                break;

            case ProcessOp::PC_RANSAC: {
                ImGui::TextDisabled("Primitive types");
                ImGui::Checkbox("Plane", &p.ransac_plane);
                ImGui::SameLine();
                ImGui::Checkbox("Sphere", &p.ransac_sphere);
                ImGui::SameLine();
                ImGui::Checkbox("Cylinder", &p.ransac_cylinder);
                ImGui::Checkbox("Cone", &p.ransac_cone);
                ImGui::SameLine();
                ImGui::Checkbox("Torus", &p.ransac_torus);
                ImGui::InputInt("Min support", &p.ransac_min_support);
                ImGui::InputFloat("Distance threshold", &p.ransac_distance, 0.001f, 0.01f, "%.4f");
                ImGui::InputFloat("Bitmap resolution", &p.ransac_bitmap, 0.005f, 0.01f, "%.3f");
                ImGui::SliderFloat("Normal threshold", &p.ransac_normal, 0.0f, 1.0f, "%.2f");
                ImGui::InputFloat("Overlook probability", &p.ransac_overlook, 0.0005f, 0.001f, "%.4f");
                ImGui::Checkbox("Selected points only", &p.ransac_selected_only);
                ImGui::TextDisabled("Requires normals; colors points by primitive.");
                break;
            }

            case ProcessOp::MESH_SMOOTH:
                if (ImGui::RadioButton("Explicit", !p.smooth_implicit))
                    p.smooth_implicit = false;
                ImGui::SameLine();
                if (ImGui::RadioButton("Implicit", p.smooth_implicit))
                    p.smooth_implicit = true;
                if (!p.smooth_implicit)
                    ImGui::InputInt("Iterations", &p.smooth_iterations);
                ImGui::Checkbox("Uniform Laplacian", &p.smooth_uniform);
                break;

            case ProcessOp::MESH_SIMPLIFY:
                ImGui::InputInt("Remaining vertices", &p.simplify_target);
                ImGui::TextDisabled("Requires a triangle mesh.");
                break;

            case ProcessOp::MESH_REMESH:
                ImGui::SliderFloat("Edge length x", &p.remesh_edge_factor, 0.05f, 5.0f, "%.2f",
                                   ImGuiSliderFlags_Logarithmic);
                ImGui::TextDisabled("Factor times the current mean edge length.");
                break;

            case ProcessOp::MESH_FILL_HOLES:
                ImGui::InputInt("Max hole size", &p.hole_max_size);
                ImGui::TextDisabled("Holes with more boundary edges are left open.");
                break;

            case ProcessOp::MESH_POLYGONIZE:
                ImGui::SliderFloat("Angle threshold", &p.polygonize_angle, 0.1f, 45.0f, "%.2f deg");
                ImGui::TextDisabled("Coplanar neighboring faces are merged into convex polygons.");
                ImGui::TextDisabled("The mesh is rebuilt; its properties are discarded.");
                break;

            case ProcessOp::MESH_FAIRING:
                ImGui::TextDisabled("Criterion");
                if (ImGui::RadioButton("Minimize area", p.fairing_criterion == 0))
                    p.fairing_criterion = 0;
                if (ImGui::RadioButton("Minimize curvature", p.fairing_criterion == 1))
                    p.fairing_criterion = 1;
                if (ImGui::RadioButton("Minimize curvature variation", p.fairing_criterion == 2))
                    p.fairing_criterion = 2;
                ImGui::TextDisabled("Fixes the boundary and smooths the interior.");
                break;

            case ProcessOp::MESH_SAMPLE:
                ImGui::InputInt("Points", &p.sample_points);
                ImGui::TextDisabled("Adds a new point cloud sampled over the surface.");
                ImGui::TextDisabled("Must exceed the current vertex count.");
                break;

            case ProcessOp::MESH_PARAMETERIZE:
                ImGui::TextDisabled("Method");
                if (ImGui::RadioButton("LSCM", p.param_method == 0))
                    p.param_method = 0;
                if (ImGui::RadioButton("Discrete harmonic", p.param_method == 1))
                    p.param_method = 1;
                ImGui::BeginDisabled(p.param_method != 1);
                ImGui::Checkbox("Uniform weights", &p.param_uniform_weights);
                ImGui::EndDisabled();
                ImGui::Checkbox("Preview with checkerboard", &p.param_preview);
                ImGui::TextDisabled("Requires a triangle mesh with a boundary.");
                ImGui::TextDisabled("Stores texture coordinates in v:texcoord.");
                break;

            default:
                break;
        }

        ImGui::Spacing();
        ImGui::Separator();
        if (ImGui::Button("Apply", ImVec2(100, 0))) {
            apply_process();
            ImGui::End();
            return;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(100, 0)) || ImGui::IsKeyPressed(ImGuiKey_Escape))
            process_op_ = ProcessOp::NONE;
    }

    ImGui::End();
}

bool MappleViewer::is_long_running(ProcessOp op) {
    switch (op) {
        case ProcessOp::PC_ESTIMATE_NORMALS:
        case ProcessOp::PC_REORIENT_NORMALS:
        case ProcessOp::PC_SIMPLIFY:
        case ProcessOp::PC_POISSON:
        case ProcessOp::MESH_SIMPLIFY:
        case ProcessOp::MESH_REMESH:
        case ProcessOp::MESH_SAMPLE:
            return true;
        default:
            return false;
    }
}

void MappleViewer::apply_process() {
    Model *model = current_model();
    if (!model || process_op_ == ProcessOp::NONE || process_job_)
        return;

    if (is_long_running(process_op_)) {
        start_process_job();
        return;
    }

    const auto start = std::chrono::steady_clock::now();
    const bool skip_selection_display = run_process_algorithm(model);
    if (process_op_ == ProcessOp::NONE) // unknown command; nothing to finalize
        return;
    finalize_process(model, start, skip_selection_display);
}

void MappleViewer::start_process_job() {
    process_job_.reset(new ProcessJob);
    process_job_->start = std::chrono::steady_clock::now();
    process_job_->target = current_model();
    process_job_->cancelable = process_op_ == ProcessOp::MESH_REMESH ||
                               process_op_ == ProcessOp::MESH_SAMPLE;
    process_job_->thread = std::thread([this] {
        run_process_algorithm(process_job_->target);
        process_job_->done.store(true, std::memory_order_release);
    });
    LOG(INFO) << process_title(process_op_) << " started in the background";
    update();
}

void MappleViewer::poll_process_job() {
    if (!process_job_ || !process_job_->done.load(std::memory_order_acquire))
        return;

    process_job_->thread.join();
    Model *target = process_job_->target;
    Model *result = process_job_->result;
    const auto start = process_job_->start;
    process_job_.reset();

    if (result)
        add_model(result);
    finalize_process(target, start, false);
}

void MappleViewer::request_process_cancel() {
    if (!process_job_ || !process_job_->cancelable || process_job_->cancel_requested)
        return;
    process_job_->cancel_requested = true;
    cancel();
    LOG(INFO) << "cancellation requested";
}

void MappleViewer::draw_process_progress() {
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::GetBackgroundDrawList()->AddRectFilled(
            viewport->Pos, ImVec2(viewport->Pos.x + viewport->Size.x, viewport->Pos.y + viewport->Size.y),
            IM_COL32(0, 0, 0, 96));

    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(360.0f, 0.0f), ImGuiCond_Appearing);
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                                   ImGuiWindowFlags_AlwaysAutoResize;
    if (!ImGui::Begin(process_title(process_op_), nullptr, flags)) {
        ImGui::End();
        return;
    }

    const std::chrono::duration<double> elapsed = std::chrono::steady_clock::now() - process_job_->start;
    ImGui::Text("Running for %.1f s", elapsed.count());

    const int percent = process_job_->percent.load();
    if (percent >= 0) {
        ImGui::ProgressBar(static_cast<float>(percent) / 100.0f);
        ImGui::TextDisabled("%d%%", percent);
    } else {
        // No progress callback in this algorithm: show a sweeping bar instead of a
        // frozen, meaningless percentage.
        const float sweep = std::fmod(static_cast<float>(ImGui::GetTime()) * 0.35f, 1.0f);
        ImGui::ProgressBar(sweep, ImVec2(-1, 0), "Working...");
    }

    ImGui::Spacing();
    if (process_job_->cancelable) {
        if (process_job_->cancel_requested)
            ImGui::TextDisabled("Cancellation requested...");
        else if (ImGui::Button("Cancel", ImVec2(100, 0)))
            request_process_cancel();
        if (!process_job_->cancel_requested) {
            ImGui::SameLine();
            ImGui::TextDisabled("Esc also cancels.");
        }
    } else
        ImGui::TextDisabled("This command cannot be interrupted.");
    ImGui::End();
}

bool MappleViewer::run_process_algorithm(Model *model) {
    auto *cloud = dynamic_cast<PointCloud *>(model);
    auto *mesh = dynamic_cast<SurfaceMesh *>(model);
    ProcessParams &p = process_params_;
    // RANSAC replaces the cloud's appearance with segmentation colors, which must
    // not be overwritten by a selection display refresh afterwards.
    bool skip_selection_display = false;

    switch (process_op_) {
        case ProcessOp::PC_ESTIMATE_NORMALS:
            if (cloud) {
                const int k = std::max(3, p.normals_k);
                if (PointCloudNormals::estimate(cloud, static_cast<unsigned int>(k), p.compute_curvature))
                    LOG(INFO) << "estimated normals for " << cloud->n_vertices() << " points (k = " << k << ")";
                else
                    LOG(WARNING) << "normal estimation failed";
            }
            break;

        case ProcessOp::PC_REORIENT_NORMALS:
            if (cloud) {
                if (!cloud->get_vertex_property<vec3>("v:normal")) {
                    LOG(WARNING) << "re-orienting normals requires normals "
                                    "(Process > Point Cloud > Estimate Normals)";
                    break;
                }
                const int k = std::max(3, p.normals_k);
                if (PointCloudNormals::reorient(cloud, static_cast<unsigned int>(k)))
                    LOG(INFO) << "re-oriented normals of " << cloud->n_vertices() << " points (k = " << k << ")";
                else
                    LOG(WARNING) << "normal reorientation failed";
            }
            break;

        case ProcessOp::PC_SIMPLIFY:
            if (cloud) {
                const auto before = cloud->n_vertices();
                const int target = std::max(1, p.point_target);
                const auto removed = PointCloudSimplification::uniform_simplification(
                        cloud, static_cast<unsigned int>(target));
                for (auto v : removed)
                    cloud->delete_vertex(v);
                cloud->collect_garbage();
                LOG(INFO) << "point cloud simplified: " << (before - cloud->n_vertices())
                          << " points removed, " << cloud->n_vertices() << " remain";
            }
            break;

        case ProcessOp::PC_POISSON:
            if (cloud) {
                if (!cloud->get_vertex_property<vec3>("v:normal")) {
                    LOG(WARNING) << "poisson reconstruction requires normals (Process > Point Cloud > Estimate Normals)";
                    break;
                }
                PoissonReconstruction recon;
                recon.set_depth(std::max(5, std::min(14, p.poisson_depth)));
                recon.set_samples_per_node(std::max(1.0f, p.poisson_samples));
                SurfaceMesh *result = recon.apply(cloud);
                if (!result) {
                    LOG(WARNING) << "poisson reconstruction failed";
                    break;
                }
                if (p.poisson_trim) {
                    SurfaceMesh *trimmed = PoissonReconstruction::trim(
                            result, p.poisson_trim_value, p.poisson_island_ratio, false);
                    if (trimmed) {
                        delete result;
                        result = trimmed;
                    }
                }
                result->set_name(file_system::name_less_extension(cloud->name()) + "_poisson.ply");
                LOG(INFO) << "poisson reconstruction: " << result->n_vertices() << " vertices, "
                          << result->n_faces() << " faces";
                if (process_job_)
                    process_job_->result = result;
                else
                    add_model(result);
            }
            break;

        case ProcessOp::PC_RANSAC:
            if (cloud) {
                if (!cloud->get_vertex_property<vec3>("v:normal")) {
                    LOG(WARNING) << "primitive extraction requires normals (Process > Point Cloud > Estimate Normals)";
                    break;
                }
                PrimitivesRansac ransac;
                if (p.ransac_plane)
                    ransac.add_primitive_type(PrimitivesRansac::PLANE);
                if (p.ransac_sphere)
                    ransac.add_primitive_type(PrimitivesRansac::SPHERE);
                if (p.ransac_cylinder)
                    ransac.add_primitive_type(PrimitivesRansac::CYLINDER);
                if (p.ransac_cone)
                    ransac.add_primitive_type(PrimitivesRansac::CONE);
                if (p.ransac_torus)
                    ransac.add_primitive_type(PrimitivesRansac::TORUS);
                if (!p.ransac_plane && !p.ransac_sphere && !p.ransac_cylinder && !p.ransac_cone && !p.ransac_torus) {
                    LOG(WARNING) << "no primitive type selected";
                    break;
                }

                const unsigned int min_support = static_cast<unsigned int>(std::max(3, p.ransac_min_support));
                int num = 0;
                if (p.ransac_selected_only) {
                    auto selected = cloud->get_vertex_property<bool>("v:select");
                    std::vector<int> indices;
                    if (selected) {
                        for (auto v : cloud->vertices())
                            if (selected[v])
                                indices.push_back(v.idx());
                    }
                    if (indices.size() < 3) {
                        LOG(WARNING) << "select at least three points before extracting from them";
                        break;
                    }
                    num = ransac.detect(cloud, indices, min_support, p.ransac_distance,
                                        p.ransac_bitmap, p.ransac_normal, p.ransac_overlook);
                } else
                    num = ransac.detect(cloud, min_support, p.ransac_distance,
                                        p.ransac_bitmap, p.ransac_normal, p.ransac_overlook);
                LOG(INFO) << num << " primitive(s) extracted";

                if (num > 0) {
                    auto segments = cloud->vertex_property<int>("v:primitive_index");
                    auto coloring = cloud->vertex_property<vec3>("v:color-segments", vec3(0.0f));
                    Renderer::color_from_segmentation(cloud, segments, coloring);
                    if (auto *drawable = cloud->renderer() ?
                                         cloud->renderer()->get_points_drawable("vertices", false) : nullptr)
                        drawable->set_property_coloring(State::VERTEX, "v:color-segments");
                    selection_styles_.erase(model);
                    skip_selection_display = true;
                }
            }
            break;

        case ProcessOp::PC_DELAUNAY_2D:
            if (cloud) {
                if (cloud->n_vertices() < 3) {
                    LOG(WARNING) << "delaunay 2D needs at least three points";
                    break;
                }
                const std::vector<vec3> &pts = cloud->points();
                std::vector<vec2> projected;
                projected.reserve(pts.size());
                for (const auto &p : pts)
                    projected.emplace_back(p.x, p.y);

                Delaunay2 delaunay;
                delaunay.set_vertices(projected);
                if (delaunay.nb_triangles() == 0) {
                    LOG(WARNING) << "delaunay 2D produced no triangles (degenerate XY projection)";
                    break;
                }

                auto *result = new SurfaceMesh;
                result->set_name(file_system::name_less_extension(cloud->name()) + "_delaunay_XY.ply");
                for (const auto &p : pts)
                    result->add_vertex(p);
                const int num_points = static_cast<int>(pts.size());
                std::size_t added = 0;
                for (unsigned int t = 0; t < delaunay.nb_triangles(); ++t) {
                    SurfaceMesh::Vertex tri[3];
                    bool valid = true;
                    for (int j = 0; j < 3; ++j) {
                        const int v = delaunay.tri_vertex(t, j);
                        if (v < 0 || v >= num_points) {
                            valid = false;
                            break;
                        }
                        tri[j] = SurfaceMesh::Vertex(v);
                    }
                    if (valid && result->add_triangle(tri[0], tri[1], tri[2]).is_valid())
                        ++added;
                }
                result->update_face_normals();
                result->update_vertex_normals();
                LOG(INFO) << "delaunay 2D (XY): " << added << " triangles from " << pts.size() << " points";
                add_model(result);
            }
            break;

        case ProcessOp::PC_DELAUNAY_3D:
            if (cloud) {
                if (cloud->n_vertices() < 4) {
                    LOG(WARNING) << "delaunay 3D needs at least four points";
                    break;
                }
                const std::vector<vec3> &pts = cloud->points();
                Delaunay3 delaunay;
                delaunay.set_vertices(pts);
                if (delaunay.nb_tets() == 0) {
                    LOG(WARNING) << "delaunay 3D produced no tetrahedra";
                    break;
                }

                auto *result = new PolyMesh;
                result->set_name(file_system::name_less_extension(cloud->name()) + "_delaunay.ply");
                for (const auto &p : pts)
                    result->add_vertex(p);
                const int num_points = static_cast<int>(pts.size());
                for (unsigned int t = 0; t < delaunay.nb_tets(); ++t) {
                    PolyMesh::Vertex tet[4];
                    bool valid = true;
                    for (int j = 0; j < 4; ++j) {
                        const int v = delaunay.tet_vertex(t, j);
                        if (v < 0 || v >= num_points) {
                            valid = false;
                            break;
                        }
                        tet[j] = PolyMesh::Vertex(v);
                    }
                    if (valid)
                        result->add_tetra(tet[0], tet[1], tet[2], tet[3]);
                }
                result->update_face_normals();
                result->update_vertex_normals();
                LOG(INFO) << "delaunay 3D: " << delaunay.nb_tets() << " tetrahedra from " << pts.size() << " points";
                add_model(result);
            }
            break;

        case ProcessOp::MESH_SMOOTH:
            if (mesh) {
                SurfaceMeshSmoothing smoother(mesh);
                if (p.smooth_implicit) {
                    // Cotan weights scale with the scene; only re-center and rescale
                    // when no boundary fixes the geometry (matching the Qt dialog).
                    const float radius = camera()->scene_radius();
                    const float dt = p.smooth_uniform ? 0.001f : 0.001f * radius * radius;
                    bool has_boundary = false;
                    for (auto v : mesh->vertices()) {
                        if (mesh->is_border(v)) {
                            has_boundary = true;
                            break;
                        }
                    }
                    smoother.implicit_smoothing(dt, p.smooth_uniform, !has_boundary);
                    LOG(INFO) << "implicit smoothing (timestep " << dt << ")";
                } else {
                    const int iterations = std::max(1, p.smooth_iterations);
                    smoother.explicit_smoothing(static_cast<unsigned int>(iterations), p.smooth_uniform);
                    LOG(INFO) << "explicit smoothing: " << iterations << " iterations";
                }
            }
            break;

        case ProcessOp::MESH_SIMPLIFY:
            if (mesh) {
                const auto before = mesh->n_vertices();
                const int target = std::max(4, p.simplify_target);
                SurfaceMeshSimplification ss(mesh);
                ss.initialize(10.0f, 0.0f, 0u, 180.0f, 0.0f);
                ss.simplify(static_cast<unsigned int>(target));
                LOG(INFO) << "mesh simplified: " << before << " -> " << mesh->n_vertices()
                          << " vertices, " << mesh->n_faces() << " faces";
            }
            break;

        case ProcessOp::MESH_REMESH:
            if (mesh) {
                float mean_edge = 0.0f;
                for (auto e : mesh->edges())
                    mean_edge += distance(mesh->position(mesh->vertex(e, 0)), mesh->position(mesh->vertex(e, 1)));
                if (mesh->n_edges())
                    mean_edge /= static_cast<float>(mesh->n_edges());
                if (mean_edge <= 0.0f) {
                    LOG(WARNING) << "cannot remesh a mesh without edges";
                    break;
                }
                SurfaceMeshRemeshing(mesh).uniform_remeshing(mean_edge * p.remesh_edge_factor);
                LOG(INFO) << "remeshed to edge length " << (mean_edge * p.remesh_edge_factor)
                          << " (" << mesh->n_vertices() << " vertices, " << mesh->n_faces() << " faces)";
            }
            break;

        case ProcessOp::MESH_FILL_HOLES:
            if (mesh) {
                const int filled = SurfaceMeshHoleFilling(mesh).fill_holes(std::max(3, p.hole_max_size));
                LOG(INFO) << filled << " hole(s) filled, " << mesh->n_faces() << " faces";
            }
            break;

        case ProcessOp::MESH_POLYGONIZE:
            if (mesh) {
                const auto before = mesh->n_faces();
                SurfaceMeshPolygonization polygonizer;
                polygonizer.apply(mesh, p.polygonize_angle);
                LOG(INFO) << "polygonized " << before << " faces into " << mesh->n_faces()
                          << " faces (mesh properties were discarded)";
            }
            break;

        case ProcessOp::MESH_FAIRING:
            if (mesh) {
                SurfaceMeshFairing fair(mesh);
                if (p.fairing_criterion == 0) {
                    fair.minimize_area();
                    LOG(INFO) << "faired by minimizing surface area";
                } else if (p.fairing_criterion == 1) {
                    fair.minimize_curvature();
                    LOG(INFO) << "faired by minimizing surface curvature";
                } else {
                    fair.fair(3);
                    LOG(INFO) << "faired by minimizing curvature variation";
                }
            }
            break;

        case ProcessOp::MESH_SAMPLE:
            if (mesh) {
                const int target = std::max(static_cast<int>(mesh->n_vertices()) + 1, p.sample_points);
                PointCloud *cloud = SurfaceMeshSampler::apply(mesh, target);
                if (!cloud) {
                    LOG(WARNING) << "surface sampling failed";
                    break;
                }
                cloud->set_name(file_system::name_less_extension(mesh->name()) + "_sampled.ply");
                LOG(INFO) << "sampled " << cloud->n_vertices() << " points from " << mesh->n_faces() << " faces";
                if (process_job_)
                    process_job_->result = cloud;
                else
                    add_model(cloud);
            }
            break;

        case ProcessOp::MESH_PARAMETERIZE:
            if (mesh) {
                if (!mesh->is_triangle_mesh()) {
                    LOG(WARNING) << "parameterization requires a triangle mesh (Process > Surface Mesh > Triangulate)";
                    break;
                }
                bool has_boundary = false;
                for (auto v : mesh->vertices()) {
                    if (mesh->is_border(v)) {
                        has_boundary = true;
                        break;
                    }
                }
                if (!has_boundary) {
                    LOG(WARNING) << "parameterization requires a boundary (a closed surface has none)";
                    break;
                }

                SurfaceMeshParameterization para(mesh);
                if (p.param_method == 0) {
                    LOG(INFO) << "parameterization (least squares conformal map)...";
                    para.lscm();
                } else {
                    LOG(INFO) << "parameterization (discrete harmonic, "
                              << (p.param_uniform_weights ? "uniform" : "cotan") << " weights)...";
                    para.harmonic(p.param_uniform_weights);
                }

                if (p.param_preview) {
                    if (auto *drawable = mesh->renderer() ?
                                         mesh->renderer()->get_triangles_drawable("faces", false) : nullptr)
                        drawable->set_texture_coloring(State::VERTEX, "v:texcoord");
                    selection_styles_.erase(model);
                    skip_selection_display = true;
                }
                LOG(INFO) << "texture coordinates stored in v:texcoord";
            }
            break;

        case ProcessOp::MESH_GEODESIC:
            if (mesh) {
                // Faces are the selection unit of a mesh, so their vertices seed the
                // distance field; this reuses the existing selection tools.
                auto selected = mesh->get_face_property<bool>("f:select");
                std::vector<SurfaceMesh::Vertex> seeds;
                std::set<SurfaceMesh::Vertex> unique;
                std::size_t selected_faces = 0;
                if (selected) {
                    for (auto f : mesh->faces()) {
                        if (!selected[f])
                            continue;
                        ++selected_faces;
                        for (auto v : mesh->vertices(f)) {
                            if (unique.insert(v).second)
                                seeds.push_back(v);
                        }
                    }
                }
                if (seeds.empty()) {
                    LOG(WARNING) << "select one or more faces to seed the geodesic distance";
                    break;
                }

                const auto components = SurfaceMeshComponent::extract(mesh);
                if (components.size() > 1)
                    LOG(WARNING) << "mesh has " << components.size()
                                 << " connected components; distances stop at component boundaries";

                SurfaceMeshGeodesic geodesic(mesh);
                geodesic.compute(seeds);
                // The selection view would hide the distance coloring, and the seeds
                // have served their purpose.
                clear_selection();
                if (auto *drawable = mesh->renderer() ?
                                     mesh->renderer()->get_triangles_drawable("faces", false) : nullptr)
                    drawable->set_coloring(State::SCALAR_FIELD, State::VERTEX, "v:geodesic:distance");
                LOG(INFO) << "geodesic distance from " << seeds.size() << " seed vertices ("
                          << selected_faces << " selected faces)";
            }
            break;

        case ProcessOp::MESH_CLIP:
        case ProcessOp::MESH_SPLIT:
        case ProcessOp::MESH_SLICE:
#ifdef HAS_CGAL
            if (mesh) {
                if (!mesh->is_triangle_mesh()) {
                    LOG(WARNING) << process_title(process_op_) << " requires a triangle mesh "
                                 << "(Process > Surface Mesh > Triangulate first)";
                    break;
                }
                auto *plane = ClippingPlane::instance();
                if (!plane->is_enabled()) {
                    LOG(WARNING) << "the clipping plane is not enabled "
                                 << "(Rendering panel > Clipping Plane)";
                    break;
                }
                const Plane3 cut = plane->plane0();

                if (process_op_ == ProcessOp::MESH_CLIP) {
                    const auto before = mesh->n_faces();
                    if (Surfacer::clip(mesh, cut, false))
                        LOG(INFO) << "clipped with the clipping plane: " << before << " -> "
                                  << mesh->n_faces() << " faces";
                    else
                        LOG(WARNING) << "nothing to clip: the plane does not cut the mesh";
                } else if (process_op_ == ProcessOp::MESH_SPLIT) {
                    const auto before = mesh->n_faces();
                    Surfacer::split(mesh, cut);
                    LOG(INFO) << "split with the clipping plane: " << before << " -> "
                              << mesh->n_faces() << " faces";
                } else {
                    const auto polylines = Surfacer::slice(mesh, cut);
                    if (polylines.empty()) {
                        LOG(WARNING) << "nothing to slice: the plane does not cut the mesh";
                        break;
                    }
                    auto *graph = new Graph;
                    graph->set_name(file_system::name_less_extension(mesh->name()) + "_slice");
                    std::size_t num_edges = 0;
                    for (const auto &polyline : polylines) {
                        if (polyline.size() < 2)
                            continue;
                        // Closed polylines repeat their first point at the end.
                        const bool closed = (polyline.front() - polyline.back()).length() < 1e-6f;
                        const std::size_t n = closed ? polyline.size() - 1 : polyline.size();
                        std::vector<Graph::Vertex> vertices;
                        vertices.reserve(n);
                        for (std::size_t i = 0; i < n; ++i)
                            vertices.push_back(graph->add_vertex(polyline[i]));
                        for (std::size_t i = 1; i < vertices.size(); ++i) {
                            graph->add_edge(vertices[i - 1], vertices[i]);
                            ++num_edges;
                        }
                        if (closed && vertices.size() > 2) {
                            graph->add_edge(vertices.back(), vertices.front());
                            ++num_edges;
                        }
                    }
                    add_model(graph);
                    LOG(INFO) << "sliced with the clipping plane: " << polylines.size()
                              << " polylines, " << num_edges << " edges";
                }
            }
#else
            LOG(WARNING) << process_title(process_op_) << " requires CGAL support "
                         << "(build Easy3D with -DEasy3D_ENABLE_CGAL=ON)";
#endif
            break;

        case ProcessOp::MESH_COMPONENTS:
            if (mesh) {
                const auto components = SurfaceMeshComponent::extract(mesh);
                auto ids = mesh->face_property<int>("f:components", -1);
                int id = 0;
                for (const auto &comp : components) {
                    for (auto f : comp.faces())
                        ids[f] = id;
                    ++id;
                }
                auto coloring = mesh->face_property<vec3>("f:color_components", vec3(0.5f));
                Renderer::color_from_segmentation(mesh, ids, coloring);
                if (auto *drawable = mesh->renderer() ?
                                     mesh->renderer()->get_triangles_drawable("faces", false) : nullptr)
                    drawable->set_property_coloring(State::FACE, "f:color_components");
                selection_styles_.erase(model);
                skip_selection_display = true;
                LOG(INFO) << components.size() << " connected component(s)";
                if (!components.empty())
                    LOG(INFO) << "largest component has " << components.front().n_faces() << " faces";
            }
            break;

        case ProcessOp::MESH_PLANAR_PARTITION:
            if (mesh) {
                mesh->update_face_normals();
                auto ids = mesh->face_property<int>("f:planar_partition", -1);
                const int num = SurfaceMeshEnumerator::enumerate_planar_components(mesh, ids, 1.0f);
                auto coloring = mesh->face_property<vec3>("f:color_planar_partition", vec3(0.0f));
                Renderer::color_from_segmentation(mesh, ids, coloring);
                if (auto *drawable = mesh->renderer() ?
                                     mesh->renderer()->get_triangles_drawable("faces", false) : nullptr)
                    drawable->set_property_coloring(State::FACE, "f:color_planar_partition");
                selection_styles_.erase(model);
                skip_selection_display = true;
                LOG(INFO) << num << " planar patch(es) at a 1 degree normal threshold";
            }
            break;

        case ProcessOp::MESH_TRIANGULATE:
            if (mesh) {
                const auto before = mesh->n_faces();
                SurfaceMeshTriangulation triangulator(mesh);
                triangulator.triangulate(SurfaceMeshTriangulation::MIN_AREA);
                LOG(INFO) << "triangulated " << before << " faces into " << mesh->n_faces() << " triangles";
            }
            break;

        case ProcessOp::MESH_DUAL:
            if (mesh) {
                const auto before = mesh->n_faces();
                geom::dual(mesh);
                LOG(INFO) << "dual mesh: " << before << " -> " << mesh->n_faces()
                          << " faces (mesh properties were discarded)";
            }
            break;

        case ProcessOp::MESH_SUBDIVIDE_CC:
        case ProcessOp::MESH_SUBDIVIDE_LOOP:
        case ProcessOp::MESH_SUBDIVIDE_SQRT3:
            if (mesh) {
                const auto before = mesh->n_vertices();
                bool ok = false;
                if (process_op_ == ProcessOp::MESH_SUBDIVIDE_CC)
                    ok = SurfaceMeshSubdivision::catmull_clark(mesh);
                else if (process_op_ == ProcessOp::MESH_SUBDIVIDE_LOOP)
                    ok = SurfaceMeshSubdivision::loop(mesh);
                else
                    ok = SurfaceMeshSubdivision::sqrt3(mesh);
                if (ok)
                    LOG(INFO) << "subdivided: " << before << " -> " << mesh->n_vertices()
                              << " vertices, " << mesh->n_faces() << " faces";
                else
                    LOG(WARNING) << "subdivision failed";
            }
            break;

        default:
            process_op_ = ProcessOp::NONE;
            return skip_selection_display;
    }

    return skip_selection_display;
}

void MappleViewer::finalize_process(Model *model, std::chrono::steady_clock::time_point start,
                                    bool skip_selection_display) {
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start);
    LOG(INFO) << process_title(process_op_) << " finished in " << elapsed.count() << " ms";

    // The geometry changed; keep the shading normals in sync before the buffers rebuild.
    if (auto *mesh = dynamic_cast<SurfaceMesh *>(model)) {
        mesh->update_face_normals();
        mesh->update_vertex_normals();
    }
    if (model && model->renderer())
        model->renderer()->update();
    invalidate_selection_count();
    clear_hover();
    if (!skip_selection_display)
        finish_selection_change(model);
    process_op_ = ProcessOp::NONE;
    update();
}
