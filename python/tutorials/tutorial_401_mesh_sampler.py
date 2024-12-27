# -----------------------------------------------------------------------------
# Tutorial: Sampling a Surface Mesh into a Point Cloud Using Easy3D
# -----------------------------------------------------------------------------
# This tutorial demonstrates how to:
# 1. Use the Easy3D `SurfaceMeshSampler` class to sample points from a surface mesh.
# 2. Visualize the original surface mesh and the sampled point cloud side-by-side
#    using Easy3D's `MultiViewer`.
# The sampling process generates a near-uniform distribution of points over the
# surface of the mesh.
# -----------------------------------------------------------------------------

# If Easy3D is not installed via `pip` but built locally, add its Python bindings 
# to the system path. For detailed instructions on building and installing 
# Python bindings of Easy3D, visit:
# https://github.com/LiangliangNan/Easy3D/blob/main/ReadMe.md
import sys
sys.path.append("../../cmake-build-release/lib/python")  # Update this path as needed.

# Import the Easy3D library for 3D visualization and processing.
import easy3d

# Easy3D must be initialized before use.
easy3d.initialize(False)

# -----------------------------------------------------------------------------
# Setting Up the Multi-View Viewer
# -----------------------------------------------------------------------------
# See detailed explanations in "python/tutorials/tutorial_203_multiview.py"

# Create a MultiViewer instance with 1 row and 2 columns.
viewer = easy3d.MultiViewer(1, 2, "Easy3D Viewer - Mesh sampling")

# -----------------------------------------------------------------------------
# Loading the Surface Mesh
# -----------------------------------------------------------------------------
# Load the Bunny surface mesh from a file. Ensure the file path is correct.
mesh = viewer.add_model(easy3d.directory() + "/data/bunny.ply")
# Assign the surface mesh to the left view (row=0, column=0).
viewer.assign(0, 0, mesh)

# -----------------------------------------------------------------------------
# Sampling Points from the Surface Mesh
# -----------------------------------------------------------------------------
# Use the `SurfaceMeshSampler` class to sample points from the mesh surface.
# The sampling process generates a near-uniform distribution of points.

# Perform the sampling. 20000 is the desired number of points to sample.
point_cloud = easy3d.SurfaceMeshSampler.apply(mesh, 20000)

# Print the number of sampled points.
print(f"Sampled point cloud with {point_cloud.n_vertices()} points.")

# Add the sampled point cloud to the viewer and assign it to the right view (row=0, column=1).
viewer.add_model(point_cloud)
viewer.assign(0, 1, point_cloud)

# Add instructions for the viewer (optional).
viewer.set_usage(
    "- Left: Original mesh.\n"
    "- Right: Sampled point cloud."
)
# Launch the viewer.
viewer.run()

# -----------------------------------------------------------------------------
# Notes:
# 1. The left view shows the original Bunny surface mesh.
# 2. The right view shows the sampled point cloud with a near-uniform point
#    distribution.
# 3. Modify the `num_points` variable to experiment with different sampling densities.
# -----------------------------------------------------------------------------