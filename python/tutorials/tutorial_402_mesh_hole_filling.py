# -----------------------------------------------------------------------------
# Tutorial: Filling Small Holes in a Surface Mesh using Easy3D
# -----------------------------------------------------------------------------
# This tutorial demonstrates how to use Easy3D Python bindings to fill small
# holes in a surface mesh. A "hole" is defined as a boundary loop of manifold
# vertices in the mesh. We will show how to:
# 1. Load a 3D mesh.
# 2. Fill the holes smaller than a given size in the mesh.
# 3. Visualize the original and modified meshes side-by-side.
# -----------------------------------------------------------------------------
# Filling holes in 3D meshes is important for applications like:
# - 3D printing: Ensuring the mesh is watertight.
# - Computer graphics: Avoiding rendering artifacts caused by holes.
# - Geometric modeling: Preparing meshes for further processing.

# What is a "Hole" in a Mesh?
# A hole in a mesh is typically represented by a boundary loop, where the
# vertices are connected but do not form a closed surface. The size of the hole
# is measured by the number of boundary edges in the loop.

# -------------------------------------------------------------------------------
# Adding Easy3D Python Bindings to the System Path
# -------------------------------------------------------------------------------
# This is required if the bindings are not installed via `pip` but are located in
# a local build directory. For building and installing Python bindings of Easy3D,
# please refer to: https://github.com/LiangliangNan/Easy3D/blob/main/README.md
# -------------------------------------------------------------------------------
import sys
sys.path.append("../../cmake-build-release/lib/python")  # Update this path to point to your Easy3D build directory.

# Import Easy3D for 3D geometry processing and visualization.
import easy3d

# Initialize Easy3D. You can set `True` for detailed output for debugging.
easy3d.initialize(False)

# Load a 3D surface mesh file using the `SurfaceMeshIO` class. We use the Stanford Bunny.
mesh = easy3d.SurfaceMeshIO.load(easy3d.resource_directory() + "/data/bunny.ply")
# Print details about the loaded mesh.
print(f"Input mesh has {mesh.n_vertices()} vertices and {mesh.n_faces()} faces")

# -----------------------------------------------------------------------------
# Fill Small Holes in the Mesh
# -----------------------------------------------------------------------------

# To compare the original and modified meshes, we make a copy of the original mesh.
copied_mesh = easy3d.SurfaceMesh(mesh)  # Create a copy for hole filling.

# Initialize the mesh hole filler. We will process the copied mesh.
filler = easy3d.SurfaceMeshHoleFilling(copied_mesh)

# The `SurfaceMeshHoleFilling` class fills all holes smaller than a specified size.
# In this example, we set a hole size threshold (in terms of number of boundary edges).
hole_size_threshold = 50  # Fill holes with a boundary size smaller than 50 edges.
print(f"Filling holes with size smaller than {hole_size_threshold} edges.")
filler.fill_holes(hole_size_threshold)

# Print details about the modified mesh after filling holes.
print(f"Modified mesh has {mesh.n_vertices()} vertices and {mesh.n_faces()} faces")

# -----------------------------------------------------------------------------
# Visualize the Original and Modified Meshes Side-by-Side
# -----------------------------------------------------------------------------

# Create a MultiViewer instance with 1 row and 2 columns (side-by-side comparison).
viewer = easy3d.MultiViewer(1, 2, "Easy3D Viewer - Mesh hole filling")

# Add the original mesh to the left view (row=0, column=0).
viewer.add_model(mesh)
viewer.assign(0, 0, mesh)

# Highlight the holes in the original mesh for better visualization.
holes = mesh.renderer().get_lines_drawable("borders")
holes.set_visible(True)  # Ensure the holes are visible.
holes.set_impostor_type(easy3d.LinesDrawable.CYLINDER)  # Render edges as cylinders.
holes.set_line_width(3)  # Set the width of the hole boundaries.
holes.set_color(easy3d.vec4(1.0, 0.0, 0.0, 1.0))  # Set hole boundary color to red.

# Add the modified mesh (after hole filling) to the right view (row=0, column=1).
viewer.add_model(copied_mesh)
viewer.assign(0, 1, copied_mesh)

# Add instructions for the viewer (optional).
viewer.set_usage("",
    "Left view: Original mesh with holes highlighted in red\n"
    "Right view: Modified mesh with holes filled"
)

# Set the camera view direction and up vector for a better view position.
viewer.camera().setViewDirection(easy3d.vec3(0, 0, 1))   # Set the view direction.
viewer.camera().setUpVector(easy3d.vec3(1, 0, 0))        # Set the up vector.

# The bunny model included in Easy3D has a per-face scalar property, which will be rendered by
# default. To better reveal the effect of hole filling, let's show the bunny with a uniform color.
mesh.renderer().get_triangles_drawable("faces").set_coloring_method(easy3d.State.UNIFORM_COLOR)
copied_mesh.renderer().get_triangles_drawable("faces").set_coloring_method(easy3d.State.UNIFORM_COLOR)

# Launch the viewer.
viewer.run()

# -----------------------------------------------------------------------------
# Notes for Beginners:
# -----------------------------------------------------------------------------
# 1. Why is hole filling important?
#    - Filling holes ensures that the mesh is watertight, which is crucial for tasks like 3D printing.
#    - It also prevents rendering artifacts that might appear when holes exist in the mesh.
#
# 2. What happens during hole filling?
#    - The algorithm identifies boundary loops (holes) in the mesh and fills them, effectively "closing" the surface.
#    - Small holes with a number of edges below the specified threshold are filled automatically.
#
# 3. Experiment with the `hole_size_threshold` parameter:
#    - Changing the threshold determines how large a hole must be before it is filled.
#    - Larger thresholds fill more holes.
# -----------------------------------------------------------------------------
