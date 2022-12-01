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


#include "dialog_surface_mesh_hole_filling.h"

#include <easy3d/core/surface_mesh.h>
#include <easy3d/algo/surface_mesh_hole_filling.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/progress.h>

#include "main_window.h"
#include "paint_canvas.h"


using namespace easy3d;

DialogSurfaceMeshHoleFilling::DialogSurfaceMeshHoleFilling(MainWindow *window)
        : Dialog(window) {
    setupUi(this);

    layout()->setSizeConstraint(QLayout::SetFixedSize);
    connect(okButton, SIGNAL(clicked()), this, SLOT(apply()));
}


void DialogSurfaceMeshHoleFilling::apply() {
    auto mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
    if (!mesh)
        return;

    /*
    // find the smallest hole
    SurfaceMesh::Halfedge hmin;
    unsigned int lmin(mesh->n_halfedges());
    for (auto h : mesh->halfedges()) {
        if (mesh->is_border(h)) {
            float l(0);
            SurfaceMesh::Halfedge hh = h;
            do {
                ++l;
                if (!mesh->is_manifold(mesh->target(hh))) {
                    l += 123456;
                    break;
                }
                hh = mesh->next(hh);
            } while (hh != h);

            if (l < lmin) {
                lmin = l;
                hmin = h;
            }
        }
    }
     */

    // first, find all the boundaries to be filled
    const int allowed_boundary_size = spinBoxAllowedBoundarySize->value();

    // prepare some information about the smallest and largest holes to the user
    int min_hole_size = max<int>();
    int max_hole_size = -max<int>();
    std::vector< std::pair<SurfaceMesh::Halfedge, int> > holes;

    auto visited = mesh->add_halfedge_property<bool>("DialogSurfaceMeshHoleFilling::h::visited", false);
    for (auto h : mesh->halfedges()) {
        if (!visited[h] && mesh->is_border(h)) {
            int size = 0;
            SurfaceMesh::Halfedge hh = h;
            do {
                visited[hh] = true;
                ++size;
                if (!mesh->is_manifold(mesh->target(hh))) {
                    size += 123456;
                    break;
                }
                hh = mesh->next(hh);
            } while (hh != h);

            min_hole_size = std::min(min_hole_size, size);
            max_hole_size = std::max(max_hole_size, size);
            if (size <= allowed_boundary_size) {
                holes.emplace_back(std::make_pair(h, size));
            }
        }
    }
    mesh->remove_halfedge_property(visited);

    // close holes whose sizes are smaller than the min allowed boundary size
    int num_closed = 0;

    ProgressLogger progress(holes.size(), true, false);
    for (const auto &hole : holes) {
        if (progress.is_canceled()) {
            LOG(WARNING) << "hole filling cancelled";
            return;
        }
        SurfaceMeshHoleFilling hf(mesh);
        hf.fill_hole(hole.first);
        mesh->renderer()->update();
        viewer_->update();
        window_->updateUi();
        ++num_closed;
        progress.next();
    }

    if (holes.empty()) {
        if (min_hole_size == max<int>() && max_hole_size == -max<int>())
            LOG(WARNING) << "model is closed and no holes to fill";
        else
            LOG(WARNING) << "no holes meet the requirement (smallest: " << min_hole_size << ", largest: " << max_hole_size << ")";
    } else {
        LOG(INFO) << num_closed << " (out of " << holes.size() << ") holes filled";
    }
}

