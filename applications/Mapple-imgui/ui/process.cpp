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

#include <easy3d/algo/point_cloud_normals.h>
#include <easy3d/algo/point_cloud_simplification.h>
#include <easy3d/algo/surface_mesh_hole_filling.h>
#include <easy3d/algo/surface_mesh_polygonization.h>
#include <easy3d/algo/surface_mesh_remeshing.h>
#include <easy3d/algo/surface_mesh_simplification.h>
#include <easy3d/algo/surface_mesh_smoothing.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/logging.h>

#include <3rd_party/imgui/imgui.h>

using namespace easy3d;


const char *MappleViewer::process_title(ProcessOp op) {
    switch (op) {
        case ProcessOp::PC_ESTIMATE_NORMALS:
            return "Estimate Normals";
        case ProcessOp::PC_SIMPLIFY:
            return "Simplify Point Cloud";
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
        default:
            return "";
    }
}

void MappleViewer::open_process(ProcessOp op) {
    Model *model = current_model();
    if (!model)
        return;

    auto *cloud = dynamic_cast<PointCloud *>(model);
    auto *mesh = dynamic_cast<SurfaceMesh *>(model);
    const bool point_cloud_op = op == ProcessOp::PC_ESTIMATE_NORMALS || op == ProcessOp::PC_SIMPLIFY;
    if ((point_cloud_op && !cloud) || (!point_cloud_op && !mesh))
        return;

    // Targets depend on the current model; refresh them so reopening a window always
    // starts from a sensible value.
    if (op == ProcessOp::PC_SIMPLIFY && cloud)
        process_params_.point_target = static_cast<int>(std::max(1u, cloud->n_vertices() / 2));
    if (op == ProcessOp::MESH_SIMPLIFY && mesh)
        process_params_.simplify_target = static_cast<int>(std::max(4u, mesh->n_vertices() / 2));

    process_op_ = op;
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

            case ProcessOp::PC_SIMPLIFY:
                ImGui::InputInt("Remaining points", &p.point_target);
                ImGui::TextDisabled("Keeps a uniform subset of the current points.");
                break;

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

void MappleViewer::apply_process() {
    Model *model = current_model();
    if (!model || process_op_ == ProcessOp::NONE)
        return;

    auto *cloud = dynamic_cast<PointCloud *>(model);
    auto *mesh = dynamic_cast<SurfaceMesh *>(model);
    ProcessParams &p = process_params_;
    const auto start = std::chrono::steady_clock::now();

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

        default:
            process_op_ = ProcessOp::NONE;
            return;
    }

    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start);
    LOG(INFO) << process_title(process_op_) << " finished in " << elapsed.count() << " ms";

    // The geometry changed; keep the shading normals in sync before the buffers rebuild.
    if (mesh) {
        mesh->update_face_normals();
        mesh->update_vertex_normals();
    }
    if (model->renderer())
        model->renderer()->update();
    invalidate_selection_count();
    clear_hover();
    finish_selection_change(model);
    process_op_ = ProcessOp::NONE;
    update();
}
