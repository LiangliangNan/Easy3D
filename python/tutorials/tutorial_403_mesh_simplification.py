# -----------------------------------------------------------------------------
# Tutorial: Simplifying a Surface Mesh Using Easy3D
# -----------------------------------------------------------------------------
# This tutorial explains how to:
# 1. Simplify the surface mesh using the `SurfaceMeshSimplification` class.
# 3. Visualize the original and simplified meshes side-by-side using `MultiViewer`.
# -----------------------------------------------------------------------------
# Mesh simplification is a crucial process in computer graphics and geometry
# processing. It reduces the number of vertices and faces in a 3D mesh while
# preserving its overall structure and visual appearance. This technique is
# widely used in:
# - Real-time rendering (e.g., games and VR applications).
# - Reducing storage and computational requirements.
# - Level-of-detail (LOD) modeling in computer graphics.
# -----------------------------------------------------------------------------

# If Easy3D is not installed via `pip`, you may need to add its Python bindings
# to the system path. For detailed instructions on building and installing
# # Python bindings of Easy3D, visit:
# # https://github.com/LiangliangNan/Easy3D/blob/main/ReadMe.md
import sys
sys.path.append("../../cmake-build-release/lib/python")

# Import Easy3D for 3D geometry processing and visualization.
import easy3d
from easy3d import SurfaceMeshIO, SurfaceMeshSimplification

# Setting `True` will provide detailed output for debugging purposes.
easy3d.initialize(False)

# Load a 3D surface mesh file using the `SurfaceMeshIO` class.
# Here, we use the Stanford Bunny model,
mesh = SurfaceMeshIO.load(easy3d.directory() + "/data/bunny.ply")
# Print details about the original mesh.
print(f"Number of vertices in the input mesh: {mesh.n_vertices()}")

# -----------------------------------------------------------------------------
# Simplify the Surface Mesh
# -----------------------------------------------------------------------------
# The `SurfaceMeshSimplification` class simplifies a 3D mesh by reducing the number
# of vertices and faces while preserving important geometric features.

# To compare the original and simplified meshes, we make a copy of the input mesh.
copied_mesh = easy3d.SurfaceMesh(mesh)  # Create a copy for simplification.

# Initialize the mesh simplifier.
simplifier = SurfaceMeshSimplification(copied_mesh)

# Set the target number of vertices to 333 (you can play with this parameter)
simplifier.simplify(333)

# Print details about the simplified mesh.
print(f"Number of vertices in the simplified mesh: {copied_mesh.n_vertices()}")

# -----------------------------------------------------------------------------
# Visualize Original and Simplified Meshes Side-by-Side
# -----------------------------------------------------------------------------
# Use the `MultiViewer` class to create a side-by-side comparison of the meshes.

# Create a MultiViewer instance with 1 row and 2 columns.
viewer = easy3d.MultiViewer(1, 2, "Easy3D Viewer - Mesh simplification")

# Add the original mesh to the viewer.
viewer.add_model(mesh)
viewer.assign(0, 0, mesh)  # Assign it to the left view (row=0, column=0).

# Add the simplified mesh to the viewer.
viewer.add_model(copied_mesh)
viewer.assign(0, 1, copied_mesh)  # Assign it to the right view (row=0, column=1).

# Add instructions for the viewer (optional).
viewer.set_usage(
    "- Left: Original mesh.\n"
    "- Right: Simplified mesh."
)

# Set the camera view direction and up vector for a better view position.
viewer.camera().setViewDirection(easy3d.vec3(0, 1, 0))   # Set the view direction.
viewer.camera().setUpVector(easy3d.vec3(0, 0, 1))        # Set the up vector.

# The bunny model included in Easy3D has a per-face scalar property, which will be rendered by
# default. To better reveal the effect of simplification, let's show the bunny with a uniform color.
mesh.renderer().get_triangles_drawable("faces").set_coloring_method(easy3d.State.UNIFORM_COLOR)
copied_mesh.renderer().get_triangles_drawable("faces").set_coloring_method(easy3d.State.UNIFORM_COLOR)

# By default, the edges/wireframes of a surface mesh is hidden. Let's also show them.
mesh.renderer().get_lines_drawable("edges").set_visible(True)
copied_mesh.renderer().get_lines_drawable("edges").set_visible(True)

# Launch the viewer.
viewer.run()
