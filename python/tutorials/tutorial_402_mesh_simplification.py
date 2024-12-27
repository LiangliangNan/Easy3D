# -----------------------------------------------------------------------------
# Tutorial: Simplifying a Surface Mesh Using Easy3D
# -----------------------------------------------------------------------------
# This tutorial explains how to:
# 1. Load a surface mesh using Easy3D's `SurfaceMeshIO` class.
# 2. Simplify the surface mesh using the `SurfaceMeshSimplification` class.
# 3. Visualize the simplified mesh.
# -----------------------------------------------------------------------------
# Mesh simplification is a key concept in computer graphics and geometry
# processing. It reduces the number of vertices and faces in a mesh while
# preserving its overall shape and appearance. Simplified meshes are
# computationally more efficient and are often used in applications like
# real-time rendering, collision detection, and data visualization.
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# If Easy3D is built locally, you need to add its Python bindings to the system path.
# For installation instructions, visit: https://github.com/LiangliangNan/Easy3D/blob/main/ReadMe.md
import sys
sys.path.append("../../cmake-build-release/lib/python")  # Update this path as needed.

# Import Easy3D for 3D geometry processing and visualization.
import easy3d
from easy3d import SurfaceMeshIO, SurfaceMeshSimplification

# Easy3D must be initialized before use.
# Setting `True` to enable detailed output, which is helpful for debugging.
easy3d.initialize(True)

# -----------------------------------------------------------------------------
# Loading the Surface Mesh
# -----------------------------------------------------------------------------
# Use the `SurfaceMeshIO` class to load a mesh file. We use the Bunny mesh.
mesh = SurfaceMeshIO.load(easy3d.directory() + "/data/bunny.ply")
# Print the number of vertices in the original mesh.
print(f"Input mesh has {mesh.n_vertices()} vertices.")

# -----------------------------------------------------------------------------
# Simplifying the Surface Mesh
# -----------------------------------------------------------------------------
# Mesh simplification reduces the complexity of a 3D model by decreasing
# the number of vertices and faces while retaining its overall shape.
# The `SurfaceMeshSimplification` class performs this operation.

# Initialize the mesh simplifier.
simplifier = SurfaceMeshSimplification(mesh)

# Simplify the mesh to a target number of vertices.
target_vertices = 333  # Desired number of vertices after simplification.
simplifier.simplify(n_vertices=target_vertices)

# Print the number of vertices in the simplified mesh.
print(f"After simplification, the mesh has {mesh.n_vertices()} vertices.")

# -----------------------------------------------------------------------------
# Use Easy3D's `Viewer` to display the simplified mesh.
# -----------------------------------------------------------------------------

# Create a viewer instance with a custom title.
viewer = easy3d.Viewer("Mesh Simplification Tutorial")

# Add the simplified mesh to the viewer.
viewer.add_model(mesh)
viewer.set_usage("")

# Run the viewer.
# The viewer window remains open until the user closes it manually.
viewer.run()

# -----------------------------------------------------------------------------
# Notes for Beginners:
# -----------------------------------------------------------------------------
# 1. Why simplify a mesh?
#    - Simplified meshes are faster to render and process.
#    - They are used in applications like gaming, VR/AR, and CAD tools.
#
# 2. What happens during simplification?
#    - Redundant vertices and faces are removed.
#    - Important features of the mesh (e.g., sharp edges and corners) are preserved.
#
# 3. Experiment with the `target_vertices` parameter to see how it affects
#    the appearance of the simplified mesh.
# -----------------------------------------------------------------------------