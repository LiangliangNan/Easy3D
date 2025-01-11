# -------------------------------------------------------------------------------
# Example: Working with the SurfaceMesh class in Easy3D
# -------------------------------------------------------------------------------
# This script demonstrates how to use the Easy3D `SurfaceMesh` class:
# - Initialize a `SurfaceMesh` from vertices and vertex indices of the faces.
# - Iterate all the faces, query the number of its vertices and print them.
# - Iterate all the vertices, query the number of its incident vertices and print
#   them.
# - Iterate all the edges, print the two end vertices and two incident faces of
#   each face.
# - Iterate all the halfedges, print its source and target vertices and its two
#   incident faces.
# It seems most Python users will only use this class for visualization, thus
# a lot of editing operations for SurfaceMesh have not been exposed to Python.
# Please let me know if you do need editing a mesh in Python.
# -------------------------------------------------------------------------------

# -------------------------------------------------------------------------------
# Adding Easy3D Python Bindings to the System Path
# -------------------------------------------------------------------------------
# This is required if the bindings are not installed via `pip` but are located in
# a local build directory. For building and installing Python bindings of Easy3D,
# please refer to: https://github.com/LiangliangNan/Easy3D/blob/main/README.md
# -------------------------------------------------------------------------------
import sys
sys.path.append("../../cmake-build-release/lib/python")  # Update this path to point to your Easy3D build directory.

import easy3d

# The `easy3d.initialize(False)` function initializes the Easy3D library.
# Use `True` to enable detailed logging, which is useful for debugging.
easy3d.initialize(False)

# -------------------------------------------------------------------------------
# Creating a SurfaceMesh
# -------------------------------------------------------------------------------
# In this tutorial, let's create a water-tight and consistently oriented unit cube.
vertices = [   # Vertices of a unit cube.
    [-0.5, -0.5, -0.5], [0.5, -0.5, -0.5], [0.5, 0.5, -0.5], [-0.5, 0.5, -0.5],
    [-0.5, -0.5,  0.5], [0.5, -0.5,  0.5], [0.5, 0.5,  0.5], [-0.5, 0.5,  0.5],
]
# You can also use Numpy array (don't forget "import numpy")
# vertices = numpy.array([
#     [-0.5, -0.5, -0.5], [0.5, -0.5, -0.5], [0.5, 0.5, -0.5], [-0.5, 0.5, -0.5],
#     [-0.5, -0.5,  0.5], [0.5, -0.5,  0.5], [0.5, 0.5,  0.5], [-0.5, 0.5,  0.5],
# ])

indices = [  # Indices of the faces (this cube has both triangle and rectangle faces).
    # Each element denotes the vertex indices of a face.
    # The SurfaceMesh class supports general polygonal meshes, and thus the faces
    # can have different numbers of vertex indices.
    [0, 3, 2, 1],    [4, 5, 6, 7],  [0, 4, 7, 3],
    [1, 2, 6, 5],    [0, 1, 5],    [0, 5, 4],    [2, 3, 7],    [2, 7, 6],
]

# You can also use Numpy array (don't forget "import numpy")
# indices = numpy.array([
#     [0, 3, 2, 1],    [4, 5, 6, 7],  [0, 4, 7, 3],
#     [1, 2, 6, 5],    [0, 1, 5],    [0, 5, 4],    [2, 3, 7],    [2, 7, 6],
# ], dtype=object)

# In case you want to represent the cube be a set of triangles (thus to create a triangle mesh), you can provide
# a 1D array of unsigned/signed integers, with each consecutive three indices representing a triangle face.
# indices = numpy.array([    # Indices for the 12 triangles (2 per face).
#     # Each consecutive 3 indices denote the vertex indices of a triangle face.
#     0, 2, 1,    0, 3, 2,    4, 5, 6,    4, 6, 7,    0, 7, 3,    0, 4, 7,
#     1, 2, 6,    1, 6, 5,    0, 1, 5,    0, 5, 4,    2, 3, 7,    2, 7, 6,
# ], dtype=numpy.uint32)

# Initialize a `SurfaceMesh` object with the vertices and indices.
mesh = easy3d.SurfaceMesh(vertices, indices)

# Set a name for this mesh.
mesh.set_name("Unit cube")
print(f"Mesh '{mesh.name()}' has {mesh.n_vertices()} vertices and {mesh.n_faces()} faces.")

print("------- Iterate all the faces ------")
for f in mesh.faces():
    # print the number of its vertices
    print(f"Face {f.idx()} has {mesh.valence(f)} vertices")
    # iterate all vertices of this face
    vcir = easy3d.SurfaceMesh.VertexAroundFaceCirculator(mesh, f)
    vertex_indices = [v.idx() for v in vcir]    # Collect all its vertex indices and print them in a single line
    print(f"The incident vertices are: {' '.join(map(str, vertex_indices))}")
    # alternative to VertexAroundFaceCirculator, one can use mesh.vertices(f) to access all vertices incident to this face
    vertex_indices = [v.idx() for v in mesh.vertices(f)]
    print(f"The incident vertices are: {' '.join(map(str, vertex_indices))}")
    # iterate all halfedges of this face
    hcir = easy3d.SurfaceMesh.HalfedgeAroundFaceCirculator(mesh, f)
    halfedge_indices = [h.idx() for h in hcir]    # Collect all its halfedges indices and print them in a single line
    print(f"The incident halfedges are: {' '.join(map(str, halfedge_indices))}")
    # alternative to HalfedgeAroundFaceCirculator, one can use mesh.halfedges(f) to access all halfedges incident to this face
    halfedge_indices = [h.idx() for h in mesh.halfedges(f)]
    print(f"The incident halfedges are: {' '.join(map(str, halfedge_indices))}")

print("------- Iterate all the vertices ------")
for v in mesh.vertices():
    #  print the number of its adjacent vertices
    print(f"Vertex {v.idx()} ({mesh.position(v).x}, {mesh.position(v).y}, {mesh.position(v).z}) has {mesh.valence(v)} adjacent vertices")
    # iterate all vertices incident to this vertex
    vcir = easy3d.SurfaceMesh.VertexAroundVertexCirculator(mesh, v)
    vertex_indices = [v.idx() for v in vcir]    # Collect indices of all incident vertices and print them in a single line
    print(f"The incident vertices are: {' '.join(map(str, vertex_indices))}")
    # alternative to VertexAroundVertexCirculator, one can use mesh.vertices(v) to access all vertices incident to this vertex
    vertex_indices = [v.idx() for v in mesh.vertices(v)]
    print(f"The incident vertices are: {' '.join(map(str, vertex_indices))}")
    # iterate all halfedges incident to this vertex
    hcir = easy3d.SurfaceMesh.HalfedgeAroundVertexCirculator(mesh, v)
    halfedge_indices = [h.idx() for h in hcir]    # Collect indices of all incident halfedges and print them in a single line
    print(f"The incident halfedges are: {' '.join(map(str, halfedge_indices))}")
    # alternative to HalfedgeAroundVertexCirculator, one can use mesh.halfedges(v) to access all halfedges incident to this vertex
    halfedge_indices = [h.idx() for h in mesh.halfedges(v)]
    print(f"The incident halfedges are: {' '.join(map(str, halfedge_indices))}")
    # iterate all faces incident to this vertex
    fcir = easy3d.SurfaceMesh.FaceAroundVertexCirculator(mesh, v)
    face_indices = [f.idx() for f in fcir]    # Collect indices of all incident faces and print them in a single line
    print(f"The incident faces are: {' '.join(map(str, face_indices))}")
    # alternative to FaceAroundVertexCirculator, one can use mesh.faces(v) to access all faces incident to this vertex
    face_indices = [f.idx() for f in mesh.faces(v)]
    print(f"The incident faces are: {' '.join(map(str, face_indices))}")

print("------- Iterate all the edges ------")
for e in mesh.edges():
    # print the indices of the two end vertices
    print(f"Edge {e.idx()}: vertex {mesh.vertex(e, 0).idx()} - vertex {mesh.vertex(e, 1).idx()}")
    # print the indices of the two incident faces
    print(f"The two incident faces: face {mesh.face(e, 0).idx()} - face {mesh.face(e, 1).idx()}")

print("------- Iterate all the halfedges ------")
for h in mesh.halfedges():
    #  print its vertices
    print(f"Halfedge {h.idx()}: vertex {mesh.source(h).idx()} -> vertex {mesh.target(h).idx()}")
    #  print the incident face and that incident to its opposite
    print(f"Halfedge {h.idx()} -> face {mesh.face(h).idx()}. Opposite halfedge {mesh.opposite(h).idx()} -> face {mesh.face(mesh.opposite(h)).idx()}")



# # Uncomment the following line to visualize the model (overlaid with edges and vertices)
# viewer = easy3d.Viewer("Easy3D Viewer")
# viewer.add_model(mesh)
# mesh.renderer().get_lines_drawable("edges").set_visible(True)       # Ensure the wireframe is visible.
# mesh.renderer().get_points_drawable("vertices").set_visible(True)   # Ensure the vertices is visible.
# viewer.run()
