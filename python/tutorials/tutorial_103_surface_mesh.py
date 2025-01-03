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
# please refer to: https://github.com/LiangliangNan/Easy3D/blob/main/ReadMe.md
# -------------------------------------------------------------------------------
import sys
sys.path.append("../../cmake-build-release/lib/python")

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
    cir = easy3d.SurfaceMesh.VertexAroundFaceCirculator(mesh, f)
    vertex_indices = [v.idx() for v in cir]    # Collect all its vertex indices and print them in a single line
    print(f"The vertices are: {' '.join(map(str, vertex_indices))}")

print("------- Iterate all the vertices ------")
for v in mesh.vertices():
    #  print the number of its adjacent vertices
    print(f"Vertex {v.idx()} ({mesh.position(v).x}, {mesh.position(v).y}, {mesh.position(v).z}) has {mesh.valence(v)} adjacent vertices")
    # iterate all vertices incident to this vertex
    cir = easy3d.SurfaceMesh.VertexAroundVertexCirculator(mesh, v)
    vertex_indices = [v.idx() for v in cir]    # Collect indices of all incident vertices and print them in a single line
    print(f"The vertices are: {' '.join(map(str, vertex_indices))}")

print("------- Iterate all the edges ------")
for e in mesh.edges():
    # print the indices of the two end vertices
    print(f"Edge {e.idx()}： vertex {mesh.vertex(e, 0).idx()} - vertex {mesh.vertex(e, 1).idx()}")
    # print the indices of the two incident faces
    print(f"The two incident faces: face {mesh.face(e, 0).idx()} - face {mesh.face(e, 1).idx()}")

print("------- Iterate all the halfedges ------")
for h in mesh.halfedges():
    #  print its vertices
    print(f"Halfedge {h.idx()}： vertex {mesh.source(h).idx()} -> vertex {mesh.target(h).idx()}")
    #  print the incident face and that incident to its opposite
    print(f"Halfedge {h.idx()} -> face {mesh.face(h).idx()}. Opposite halfedge {mesh.opposite(h).idx()} -> face {mesh.face(mesh.opposite(h)).idx()}")