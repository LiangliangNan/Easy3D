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
#include <easy3d/util/stop_watch.h>

#include <3rd_party/opcode/Opcode.h>

using namespace Opcode;
using namespace IceMaths;


namespace internal {

    class ColliderImpl {
    public:
        ColliderImpl(easy3d::SurfaceMesh *mesh0, easy3d::SurfaceMesh *mesh1) {
            model0_ = build(mesh0);
            model1_ = build(mesh1);
            if (!model0_ || !model1_)
                return;

            cache_ = new BVTCache;
            cache_->Model0 = model0_;
            cache_->Model1 = model1_;

            collider_ = new AABBTreeCollider;
            collider_->SetFirstContact(false);
            collider_->SetTemporalCoherence(false);
            collider_->SetPrimitiveTests(true);
            const char* msg = collider_->ValidateSettings();
            if (msg) {
                LOG(WARNING) << "failed setting AABB tree collider: " << msg;
                delete collider_;
                collider_ = nullptr;
            }
        }

        ~ColliderImpl() {
            auto mesh0_interface = model0_->GetMeshInterface();
            delete [] mesh0_interface->GetTris();
            delete [] mesh0_interface->GetVerts();
            delete mesh0_interface;
            delete model0_;

            auto mesh1_interface = model1_->GetMeshInterface();
            delete [] mesh1_interface->GetTris();
            delete [] mesh1_interface->GetVerts();
            delete mesh1_interface;
            delete model1_;

            delete cache_;
            delete collider_;
        }

    public:
        std::vector<std::pair<easy3d::SurfaceMesh::Face, easy3d::SurfaceMesh::Face> > detect(const easy3d::mat4 &t0, const easy3d::mat4 &t1) const {
            std::vector<std::pair<easy3d::SurfaceMesh::Face, easy3d::SurfaceMesh::Face> > result;
            if (!collider_) {
                LOG_EVERY_N(10, WARNING) << "the AABB tree collider was not built";
                return result;
            }

            Matrix4x4 trans0, trans1;
            for (auto i = 0; i < 4; ++i) {
                for (auto j = 0; j < 4; ++j) {
                    // ToDo: Why transpose? (opcode requires 'world' matrix)
                    trans0[i][j] = t0(j, i);
                    trans1[i][j] = t1(j, i);
                }
            }

            if (!collider_->Collide(*cache_, &trans0, &trans1)) {
                LOG(WARNING) << "failed detecting collision";
                return result;
            }

            if (collider_->GetContactStatus()) {
                const udword num = collider_->GetNbPairs();
                result.resize(num);
                const Pair* pairs = collider_->GetPairs();
                for (udword i = 0; i < num; ++i) {
                    const Pair& pair = pairs[i];
                    result[i] = { easy3d::SurfaceMesh::Face(static_cast<int>(pair.id0)), easy3d::SurfaceMesh::Face(static_cast<int>(pair.id1)) };
                }
            }
            return result;
        }

    private:
        Opcode::Model* build(const easy3d::SurfaceMesh* mesh) {
            if (!mesh->is_triangle_mesh()) {
                LOG(WARNING) << "the first mesh (" << mesh->name() << ") is not a triangle mesh";
                return nullptr;
            }

            if (mesh->n_vertices() <= 0 || mesh->n_faces() <= 0) {
                LOG(WARNING) << "invalid geometry";
                return nullptr;
            }

            const auto& pts = mesh->points();
            auto vertices = new Point[pts.size()];
            for (std::size_t i=0; i<pts.size(); ++i)
                    vertices[i].Set(pts[i]);

            auto indices = new IndexedTriangle[mesh->n_faces()];
            for (const auto& f : mesh->faces()) {
                    std::vector<int> ids;
                    for (const auto& v : mesh->vertices(f))
                            ids.push_back(v.idx());
                    indices[f.idx()] = IndexedTriangle(ids[0], ids[1], ids[2]);
                }

            auto mesh_interface = new MeshInterface();
            mesh_interface->SetNbTriangles(mesh->n_faces());
            mesh_interface->SetNbVertices(mesh->n_vertices());
            mesh_interface->SetPointers(indices, vertices);

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
            return model;
        }

    private:
        Opcode::Model* model0_;
        Opcode::Model* model1_;
        BVTCache* cache_;
        AABBTreeCollider* collider_;
    };
}


namespace easy3d {


    Collider::Collider(SurfaceMesh *mesh0, SurfaceMesh *mesh1) {
        StopWatch w;
        collider_ = new internal::ColliderImpl(mesh0, mesh1);
        LOG(INFO) << "building the collider (AABB trees): " << w.time_string();
    }


    Collider::~Collider() {
        delete collider_;
    }


    std::vector<std::pair<SurfaceMesh::Face, SurfaceMesh::Face> > Collider::detect(const mat4 &t0, const mat4 &t1) const {
        return collider_->detect(t0, t1);
    }
}
