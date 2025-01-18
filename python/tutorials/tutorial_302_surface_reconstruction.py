"""
302: Surface reconstruction
===========================

This tutorial demonstrates how to perform Poisson surface reconstruction
using the Easy3D library. The algorithm reconstructs a surface mesh from a
3D point cloud with consistent normals, resulting in a smooth and complete
surface mesh representation.

In this example, we will:

- Load a point cloud.
- Initialize the PoissonReconstruction algorithm.
- Set reconstruction parameters.
- Apply the reconstruction to the point cloud.
- Visualize the reconstructed surface mesh in an interactive viewer.

"""

# sphinx_gallery_thumbnail_path = '_static/sphx_glr_tutorial_302_surface_reconstruction_thumb.png'

# -------------------------------------------------------------------------------
# Adding Easy3D Python Bindings to the System Path
# -------------------------------------------------------------------------------
# This is required if the bindings are not installed via `pip` but are located in
# a local build directory. For building and installing Python bindings of Easy3D,
# please refer to: https://github.com/LiangliangNan/Easy3D/blob/main/README.md
# -------------------------------------------------------------------------------
import sys
sys.path.append("../../cmake-build-release/lib/python")  # Update this path to point to your Easy3D build directory.

# -----------------------------------------------------------------------------
# Importing the Necessary Easy3D Library
# -----------------------------------------------------------------------------
# Import the Easy3D library, which provides functions for 3D processing and
# visualization.
import easy3d

# -----------------------------------------------------------------------------
# Initializing Easy3D
# -----------------------------------------------------------------------------
# Before using any functionality of Easy3D, we need to initialize the library.
# The 'True' parameter enables detailed logging, which is useful for debugging
# and understanding the internal workings of the library.
easy3d.initialize(True)  # Initialize Easy3D with detailed logging

# -----------------------------------------------------------------------------
# Creating the Poisson Reconstruction Object
# -----------------------------------------------------------------------------
# Now, we create an instance of the PoissonReconstruction class. This object
# will be used to reconstruct a surface mesh from a 3D point cloud using
# the Poisson surface reconstruction algorithm.
reconstructor = easy3d.PoissonReconstruction()  # Create PoissonReconstruction object

# -----------------------------------------------------------------------------
# Setting Reconstruction Parameters
# -----------------------------------------------------------------------------
# Poisson surface reconstruction involves constructing an octree to represent
# the point cloud. The 'depth' parameter controls the maximum depth of the
# octree and the resolution of the voxel grid. Larger depths result in higher
# resolution but also increased computational cost.
#
# In this example, we set the depth to 8, which offers good balance between
# accuracy and performance. Feel free to experiment with different values.
reconstructor.set_depth(8)  # Adjust this value as needed.

# -----------------------------------------------------------------------------
# Loading the 3D Point Cloud
# -----------------------------------------------------------------------------
# The point cloud is loaded from a file. The Easy3D library supports various
# formats, including .bin files. The example uses a point cloud file located
# in the Easy3D library's data directory.
#
# The `PointCloudIO.load()` function loads the point cloud into memory.
pointcloud_file = easy3d.resource_directory() + "/data/polyhedron.bin"  # Update the path if necessary
point_cloud = easy3d.PointCloudIO.load(pointcloud_file)  # Load the point cloud from the file

# -----------------------------------------------------------------------------
# Applying Poisson Surface Reconstruction
# -----------------------------------------------------------------------------
# We now apply the Poisson surface reconstruction algorithm to the loaded
# point cloud. The `apply()` method returns a surface mesh.
#
# The surface mesh is stored in the `surface_mesh` variable, and the number
# of faces in the reconstructed mesh is printed for reference.
surface_mesh = reconstructor.apply(point_cloud)  # Apply reconstruction
print("Number of faces in reconstructed mesh: ", surface_mesh.n_faces())  # Print number of faces

# -----------------------------------------------------------------------------
# Optional: Trimming the Reconstructed Mesh (If Necessary)
# -----------------------------------------------------------------------------
# If the point cloud is incomplete or noisy, you can trim the resulting
# surface mesh. The trimming operation removes faces based on the density
# attribute. This step is optional and may not be needed for every dataset.
#
# The method below trims faces based on a minimum density threshold (6.0)
# and a maximum allowed density variation (0.01). Set these values based on
# your specific dataset for optimal results.
#
# trimmed_mesh = reconstructor.trim(surface_mesh, 6.0, 0.01, True)

# -----------------------------------------------------------------------------
# Visualizing the Reconstructed Surface Mesh
# -----------------------------------------------------------------------------
# To visualize the reconstructed surface mesh, we use the Easy3D viewer.
# The viewer provides an interactive window where you can rotate, zoom, and
# explore the 3D model.
#
# First, we create a viewer instance, set its usage description (optional),
# and add the surface mesh to the viewer.
viewer = easy3d.Viewer("Easy3D Viewer - Surface reconstruction")
viewer.set_usage("")  # Optional: Set an empty usage description for a clean UI

# Add the reconstructed mesh to the viewer
viewer.add_model(surface_mesh)  # Add surface mesh to viewer

# The reconstructed model also has a scalar field, which will be rendered by default.
# In this example, we are interested in the reconstructed geometry. Thus, we render
# the surface mesh with a uniform color.
surface = surface_mesh.renderer().get_triangles_drawable("faces")
surface.set_coloring_method(easy3d.State.UNIFORM_COLOR)    # Set uniform color rendering.

# Running the Viewer
viewer.run()
