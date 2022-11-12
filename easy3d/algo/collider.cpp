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

#include <easy3d/algo/collider.h>

#include <map>
#include <3rd_party/opcode/Opcode.h>

using namespace Opcode;
using namespace IceMaths;


namespace easy3d {

    std::map<Opcode::Model*, MeshInterface*> mesh_interfaces;

    namespace details {

        Opcode::Model* build(const SurfaceMesh* mesh) {
            if (!mesh->is_triangle_mesh()) {
                LOG(WARNING) << "the first mesh (" << mesh->name() << ") is not a triangle mesh";
                return nullptr;
            }

            const auto& pts = mesh->points();
            std::vector<Point> vertices(pts.size());
            for (std::size_t i=0; i<pts.size(); ++i)
                    vertices[i].Set(pts[i]);

            std::vector<IndexedTriangle> indices;
            indices.reserve(mesh->n_faces());
            for (const auto& f : mesh->faces()) {
                std::vector<int> ids;
                for (const auto& v : mesh->vertices(f))
                    ids.push_back(v.idx());
                indices.emplace_back(IndexedTriangle(ids[0], ids[1], ids[2]));
            }

            udword nVertices = vertices.size();
            udword nTriangles = indices.size();

            MeshInterface *mesh_interface = new MeshInterface();
            mesh_interface->SetNbTriangles(nTriangles);
            mesh_interface->SetNbVertices(nVertices);
            bool pointersSet = mesh_interface->SetPointers(indices.data(), vertices.data());
            if (!pointersSet) {
                LOG(WARNING) << "failed creating object pointers";
                return nullptr;
            }

            udword degenerated_faces = mesh_interface->CheckTopology();
            if (degenerated_faces != 0) {
                LOG(WARNING) << "model has " << degenerated_faces << " degenerated faces and cannot be processed";
                return nullptr;
            }
            if (!mesh_interface->IsValid()) {
                LOG(WARNING) << "the mesh if not valid and cannot be processed";
                return nullptr;
            }

            BuildSettings settings;
            settings.mLimit = 1;
            settings.mRules = SPLIT_SPLATTER_POINTS | SPLIT_GEOM_CENTER;

            OPCODECREATE data;
            data.mIMesh = mesh_interface;
            data.mCanRemap = false;
            data.mKeepOriginal = false;
            data.mNoLeaf = true;
            data.mQuantized = true;
            data.mSettings = settings;

            auto model = new Opcode::Model();
            if (!model->Build(data)) {
                LOG(WARNING) << "failed building AABB tree for the mesh";
                return nullptr;
            }

            mesh_interfaces[model] = mesh_interface;
            return model;
        }

    }


    Collider::Collider(SurfaceMesh *mesh0, SurfaceMesh *mesh1)
    {
        model0_ = details::build(mesh0);
        model1_ = details::build(mesh1);
        if (!model0_ || !model1_)
            return;
    }


    Collider::~Collider() {
        delete mesh_interfaces[model0_];
        delete mesh_interfaces[model1_];
        mesh_interfaces.erase(model0_);
        mesh_interfaces.erase(model1_);
        delete model0_;
        delete model1_;
    }


    std::vector<std::pair<SurfaceMesh::Face, SurfaceMesh::Face> > Collider::detect(const mat4 &t0, const mat4 &t1) const {
        std::vector<std::pair<SurfaceMesh::Face, SurfaceMesh::Face> > result;
        if (!model0_) {
            LOG_EVERY_N(10, WARNING) << "AABB tree for the first mesh not built";
            return result;
        }
        if (!model1_) {
            LOG_EVERY_N(10, WARNING) << "AABB tree for the second mesh not built";
            return result;
        }

        BVTCache cache;
        cache.Model0 = model0_;
        cache.Model1 = model1_;

        Matrix4x4 trans0, trans1;
        for (auto i = 0; i < 4; ++i) {
            for (auto j = 0; j < 4; ++j) {
                trans0[i][j] = t0(i, j);
                trans1[i][j] = t1(i, j);
            }
        }

        // std::cout << "collision detection ...";
        // t.start();
        AABBTreeCollider collider;
        collider.SetFirstContact(false);
        collider.SetTemporalCoherence(false);
        collider.SetPrimitiveTests(true);
        const char* msg = collider.ValidateSettings();
        if (msg) {
            LOG(WARNING) << "failed setting AABB tree collider: " << msg;
            return result;
        }

        if (!collider.Collide(cache, &trans0, &trans1)) {
            LOG(WARNING) << "failed detecting collision";
            return result;
        }

        //std::cout << "  done. time: " << t.time_string() << std::endl;
        if (collider.GetContactStatus()) {
            const udword num = collider.GetNbPairs();
            result.resize(num);
            const Pair* pairs = collider.GetPairs();
            for (udword i = 0; i < num; ++i) {
                const Pair& pair = pairs[i];
                udword id0 = pair.id0;
                udword id1 = pair.id1;
                result[i] = { SurfaceMesh::Face(id0), SurfaceMesh::Face(id1) };
            }
        }
        return result;
    }
}
