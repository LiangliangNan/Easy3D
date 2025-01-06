# -----------------------------------------------------------------------------
# Tutorial: Mesh Subdivision with Easy3D
# -----------------------------------------------------------------------------
# In this tutorial, we will demonstrate how to apply three popular subdivision
# algorithms to a 3D surface mesh using Easy3D Python bindings:
# 1. Catmull-Clark Subdivision
# 2. Loop Subdivision
# 3. Sqrt3 Subdivision
# These subdivision algorithms are useful for smoothing and refining meshes while
# maintaining their topological properties. The Catmull-Clark algorithm is widely
# used for quad meshes, the Loop subdivision is ideal for triangular meshes, and
# the Sqrt3 subdivision is another option for triangular meshes, providing smoother
# results.
# We will also visually compare the input and subdivided model using Easy3D's
# built-in visualization tools.
# -----------------------------------------------------------------------------

# If Easy3D is not installed via `pip`, you may need to add its Python bindings
# to the system path. For detailed instructions on building and installing
# Python bindings of Easy3D, visit:
# https://github.com/LiangliangNan/Easy3D/blob/main/ReadMe.md

import sys
sys.path.append("../../cmake-build-release/lib/python")  # Update this path to point to your Easy3D build directory.

# Import Easy3D
import easy3d

# Setting `True` will provide detailed output for debugging purposes.
easy3d.initialize(False)

# -----------------------------------------------------------------------------
# Load a Surface Mesh
# -----------------------------------------------------------------------------
# For this tutorial, let's use the torusknot model.
input = easy3d.SurfaceMeshIO.load(easy3d.resource_directory() + "/data/torusknot.obj")
# Print details about the original mesh.
print(f"Original mesh has {input.n_vertices()} vertices and {input.n_faces()} faces.")

# -----------------------------------------------------------------------------
# Apply Subdivision
# -----------------------------------------------------------------------------
# Let's make a copy of the input and apply subdivision on the copied mesh.
output = easy3d.SurfaceMesh(input)
success = easy3d.SurfaceMeshSubdivision.loop(output) # apply Loop subdivision
# success = easy3d.SurfaceMeshSubdivision.catmull_clark(copied_mesh)    # apply Catmull-Clark
# success = easy3d.SurfaceMeshSubdivision.sqrt3(copied_mesh)            # apply sqrt3
if success:
    print(f"Subdivided mesh has {output.n_vertices()} vertices and {output.n_faces()} faces.")
else:
    print("Subdivision failed.")

# -----------------------------------------------------------------------------
# Visualize the original mesh and the subdivision result side-by-side using the MultiViewer.
# -----------------------------------------------------------------------------

# Create a MultiViewer instance with 1 row and 2 columns.
viewer = easy3d.MultiViewer(1, 2, "Easy3D Viewer - Mesh subdivision")

# Add the original mesh to the viewer and assign it to the first column.
viewer.add_model(input)
viewer.assign(0, 0, input)

# Add the subdivided mesh to the viewer and assign it to the second column.
viewer.add_model(output)
viewer.assign(0, 1, output)

# This "/data/torusknot.obj" file contains texture coordinates, and by default
# it will be rendered with the checkboard texture. To better reveal the effect
# of subdivision, let's show them with a uniform color and overlay the edges.
input.renderer().get_triangles_drawable("faces").set_coloring_method(easy3d.State.UNIFORM_COLOR)    # Set uniform color rendering.
output.renderer().get_triangles_drawable("faces").set_coloring_method(easy3d.State.UNIFORM_COLOR)   # Set uniform color rendering.
input.renderer().get_lines_drawable("edges").set_visible(True)    # Set edges/wireframe visible.
output.renderer().get_lines_drawable("edges").set_visible(True)   # Set edges/wireframe visible.

# Add instructions for the viewer (optional).
viewer.set_usage(
    "- Left: Input mesh.\n"
    "- Right: Subdivision result."
)

# Launch the viewer
viewer.run()