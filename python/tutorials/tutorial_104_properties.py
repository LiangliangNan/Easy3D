"""
Tutorial: Per-Element Properties in Easy3D
==========================================

This tutorial demonstrates how to create, set, and retrieve per-element
properties for a 3D model using the Easy3D library. Per-element properties can
be attached to vertices or faces of a surface mesh, enabling detailed data
storage and manipulation for each element.
The same principles apply to PointCloud as well.
"""

# -------------------------------------------------------------------------------
# Adding Easy3D Python Bindings to the System Path
# -------------------------------------------------------------------------------
# This is required if the bindings are not installed via `pip` but are located in
# a local build directory. For building and installing Python bindings of Easy3D,
# please refer to: https://github.com/LiangliangNan/Easy3D/blob/main/README.md
# -------------------------------------------------------------------------------
import sys
sys.path.append("../../cmake-build-release/lib/python")  # Update this path to point to your Easy3D build directory.

# Import the Easy3D library, which is used for 3D visualization and geometry processing.
import easy3d

# Initialize Easy3D. This is required before using any of its functionalities.
# The parameter `False` disables detailed logging. Set it to `True` for debug logs.
easy3d.initialize(False)

# -----------------------------------------------------------------------------
# Creating a Unit Cube as a Surface Mesh
# -----------------------------------------------------------------------------
# A SurfaceMesh is a collection of vertices and faces that represents a 3D model.

# Define the vertices of a unit cube (corner coordinates).
vertices = [
    [-0.5, -0.5, -0.5],  # Vertex 0
    [ 0.5, -0.5, -0.5],  # Vertex 1
    [ 0.5,  0.5, -0.5],  # Vertex 2
    [-0.5,  0.5, -0.5],  # Vertex 3
    [-0.5, -0.5,  0.5],  # Vertex 4
    [ 0.5, -0.5,  0.5],  # Vertex 5
    [ 0.5,  0.5,  0.5],  # Vertex 6
    [-0.5,  0.5,  0.5],  # Vertex 7
]

# Define the indices of the cube faces (each face is defined by 4 vertices).
indices = [
    [0, 3, 2, 1],  # Bottom face
    [4, 5, 6, 7],  # Top face
    [0, 4, 7, 3],  # Left face
    [1, 2, 6, 5],  # Right face
    [0, 1, 5, 4],  # Front face
    [2, 3, 7, 6],  # Back face
]

# Create the SurfaceMesh object.
mesh = easy3d.SurfaceMesh(vertices, indices)

# -----------------------------------------------------------------------------
# Adding Per-Element Properties
# -----------------------------------------------------------------------------
# Properties allow storing custom data for each vertex or face of the mesh.
# The `add_vertex_property` and `add_face_property` methods are used to add properties.

# Adding per-vertex properties
vertex_elevation = mesh.add_vertex_property("v:elevation", dtype=float) # Elevation (float)
vertex_class = mesh.add_vertex_property("v:class", dtype=int)           # Class id (int)
vertex_visited = mesh.add_vertex_property("v:visited", dtype=bool)      # Visited status (bool)
vertex_color = mesh.add_vertex_property("v:color", dtype=easy3d.vec3)   # Color (vec3 for RGB)

# Adding per-face properties
face_area = mesh.add_face_property("f:area", dtype=float)               # Area (float)
face_class = mesh.add_face_property("f:class", dtype=int)               # Class id (int)
face_status = mesh.add_face_property("f:status", dtype=bool)            # Status (bool)
face_color = mesh.add_face_property("f:color", dtype=easy3d.vec3)       # Color (vec3 for RGB)

# -----------------------------------------------------------------------------
# Setting Property Values
# -----------------------------------------------------------------------------
# Use loops to iterate through all vertices or faces and assign values to their properties.

# Assigning values to vertex properties
for v in mesh.vertices():
    vertex_elevation[v] = 1.23                           # Set elevation to 1.23 for this vertex
    vertex_visited[v] = True                             # Mark this vertex as visited
    vertex_class[v] = 10                                 # Assign this vertex to class 10
    vertex_color[v] = easy3d.vec3(0.0, 1.0, 0.0)         # Set the color of this vertex to green (RGB: 0, 1, 0)

# Assigning values to face properties
for f in mesh.faces():
    face_area[f] = 1.0                                   # Set face area to 1.0 for this face
    face_status[f] = False                               # Mark this face as negative
    face_class[f] = 5                                    # Assign this face to class 10
    face_color[f] = easy3d.vec3(0.0, 0.0, 1.0)           # Set the color of this face to blue (RGB: 0, 0, 1)

# -----------------------------------------------------------------------------
# Retrieving Properties
# -----------------------------------------------------------------------------
# Retrieve the properties using their names. Ensure the name and type match those used during creation.

# Retrieve vertex properties
vertex_elevation_retrieved = mesh.get_vertex_property("v:elevation", dtype=float)
vertex_class_retrieved = mesh.get_vertex_property("v:class", dtype=int)
vertex_visited_retrieved = mesh.get_vertex_property("v:visited", dtype=bool)
vertex_color_retrieved = mesh.get_vertex_property("v:color", dtype=easy3d.vec3)

# Retrieve face properties
face_area_retrieved = mesh.get_face_property("f:area", dtype=float)
face_class_retrieved= mesh.get_face_property("f:class", dtype=int)
face_status_retrieved = mesh.get_face_property("f:status", dtype=bool)
face_color_retrieved = mesh.get_face_property("f:color", dtype=easy3d.vec3)

# -----------------------------------------------------------------------------
# Accessing Property Values
# -----------------------------------------------------------------------------
# Iterate over vertices or faces to read and process property values.

# Print vertex properties
print("Vertex Properties:")
for v in mesh.vertices():
    print(f"Vertex {v.idx()} --- Elevation: {vertex_elevation_retrieved[v]}, Class: {vertex_class_retrieved[v]}, "
          f"Visited: {vertex_visited_retrieved[v]}, Color: {vertex_color_retrieved[v]}")

# Print face properties
print("\nFace Properties:")
for f in mesh.faces():
    print(f"Face {f.idx()} --- Area: {face_area_retrieved[f]}, Class: {face_class_retrieved[f]}, "
          f"Status: {face_status_retrieved[f]}, Color: {face_color_retrieved[f]}")

# -----------------------------------------------------------------------------
# Final Notes
# -----------------------------------------------------------------------------
# The printed output will display the values assigned to each vertex and face property.
# You can use these properties to store and manipulate custom data for 3D models.
