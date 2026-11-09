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

#include "viewer.h"
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_406_CollisionDetection/main.cpp
 *
 * \brief Real-time collision detection between two meshes, with intersecting faces highlighted.
 *
 * <b>Overview</b>
 * This tutorial loads two meshes (a bunny and a mannequin), animates the bunny, and
 * continuously reports which faces of the two meshes intersect, painting them red. Pausing
 * the animation lets the user drag the bunny around (as in Tutorial_405_ObjectManipulation)
 * and watch the contact regions update in real time. Unlike the other 4xx tutorials, all
 * scene setup happens in the viewer's constructor, so main() only creates the viewer and
 * runs it.
 *
 * <b>The technique</b>
 * Testing every face of one mesh against every face of the other is O(n*m) — far too slow
 * for interactive rates on large meshes. The standard solution is a bounding volume
 * hierarchy (BVH): each mesh is preprocessed once into a tree of axis-aligned bounding boxes
 * (AABBs), with the actual triangles at the leaves. A collision query then walks both trees
 * simultaneously, discarding whole branches whose boxes do not overlap, and only performs
 * exact triangle-triangle intersection tests on the few leaf pairs that survive. Easy3D's
 * Collider wraps the Opcode library (Pierre Terdiman), which achieves real-time performance
 * even for large meshes. Crucially, the trees are built in each mesh's local space and the
 * current transformation matrices are passed to every detect() call — so moving or rotating
 * a mesh never requires rebuilding the trees. The demo visualizes the result by storing a
 * per-face color property: all faces start in the mesh's base color, and every reported
 * intersecting face pair is painted red.
 *
 * <b>Easy3D API</b>
 * - Collider: constructed from two triangle meshes (builds the AABB trees); detect(t0, t1)
 *   returns the intersecting face pairs for the given model transforms.
 * - Manipulator / ManipulatedFrame: pose the bunny interactively (Alt + drag) and provide
 *   the transformation matrices passed to detect().
 * - SurfaceMesh face properties + State::COLOR_PROPERTY: per-face colors drive the contact
 *   visualization; Viewer::animation_func_ / set_animation() run the demo animation.
 *
 * <b>Running it</b>
 * On start, an animation rotates the bunny and highlights intersecting faces in red. Press
 * 'space' (or F2) to pause/resume. While paused, Alt + left-drag rotates the bunny and
 * Alt + right-drag translates it; collisions are recomputed during every drag.
 *
 * The header file of the viewer class:
 * \include{lineno} Tutorial_406_CollisionDetection/viewer.h
 * The source file of the viewer class:
 * \include{lineno} Tutorial_406_CollisionDetection/viewer.cpp
 *
 * \see Collider, SurfaceMesh, Manipulator, Tutorial_405_ObjectManipulation
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // initialize Easy3D.
    initialize();

    TutorialCollisionDetection viewer(EXAMPLE_TITLE);

    // run the viewer
    return viewer.run();
}
