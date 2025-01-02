# -------------------------------------------------------------------------------
# Example: Working with the SurfaceMesh class in Easy3D
# -------------------------------------------------------------------------------
# This script demonstrates how to use the Easy3D `SurfaceMesh` class:
# - Initialize a `SurfaceMesh` from vertices and vertex indices of the faces.
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

# Iterate all the faces and print the number of its vertices
for face in mesh.faces():
    print(f"Face {face.idx()} has {mesh.valence(face)} vertices")