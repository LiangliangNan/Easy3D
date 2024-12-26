# -----------------------------------------------------------------------------
# Tutorial: Poisson Surface Reconstruction
# -----------------------------------------------------------------------------
# This tutorial demonstrates how to perform Poisson surface reconstruction
# using the Easy3D library. The algorithm reconstructs a surface mesh from a
# 3D point cloud with consistent normals, resulting in a smooth and complete
# surface mesh representation.
#
# In this example, we will:
# 1. Load a point cloud.
# 2. Initialize the PoissonReconstruction algorithm.
# 3. Set reconstruction parameters.
# 4. Apply the reconstruction to the point cloud.
# 5. Visualize the reconstructed surface mesh in an interactive viewer.
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Step 1: Adding Easy3D Python Bindings to the System Path
# -----------------------------------------------------------------------------
# If the Easy3D Python bindings are located in a custom directory (e.g., from
# a local build), you'll need to add that directory to the system path to
# access them. You can skip this step if the bindings are installed globally
# via `pip'. For building and installing Python bindings of Easy3D,
# # please refer to: https://github.com/LiangliangNan/Easy3D/blob/main/ReadMe.md
import sys
sys.path.append("../../cmake-build-release/lib/python")  # Update this path based on your setup

# -----------------------------------------------------------------------------
# Step 2: Importing the Necessary Easy3D Library
# -----------------------------------------------------------------------------
# Import the Easy3D library, which provides functions for 3D processing and
# visualization.
import easy3d

# -----------------------------------------------------------------------------
# Step 3: Initializing Easy3D
# -----------------------------------------------------------------------------
# Before using any functionality of Easy3D, we need to initialize the library.
# The 'True' parameter enables detailed logging, which is useful for debugging
# and understanding the internal workings of the library.
easy3d.initialize(True)  # Initialize Easy3D with detailed logging

# -----------------------------------------------------------------------------
# Step 4: Creating the Poisson Reconstruction Object
# -----------------------------------------------------------------------------
# Now, we create an instance of the PoissonReconstruction class. This object
# will be used to reconstruct a surface mesh from a 3D point cloud using
# the Poisson surface reconstruction algorithm.
reconstructor = easy3d.PoissonReconstruction()  # Create PoissonReconstruction object

# -----------------------------------------------------------------------------
# Step 5: Setting Reconstruction Parameters
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
# Step 6: Loading the 3D Point Cloud
# -----------------------------------------------------------------------------
# The point cloud is loaded from a file. The Easy3D library supports various
# formats, including .bin files. The example uses a point cloud file located
# in the Easy3D library's data directory.
#
# The `PointCloudIO.load()` function loads the point cloud into memory.
pointcloud_file = easy3d.directory() + "/data/polyhedron.bin"  # Update the path if necessary
point_cloud = easy3d.PointCloudIO.load(pointcloud_file)  # Load the point cloud from the file

# -----------------------------------------------------------------------------
# Step 7: Applying Poisson Surface Reconstruction
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
# Step 8: Visualizing the Reconstructed Surface Mesh
# -----------------------------------------------------------------------------
# To visualize the reconstructed surface mesh, we use the Easy3D viewer.
# The viewer provides an interactive window where you can rotate, zoom, and
# explore the 3D model.
#
# First, we create a viewer instance, set its usage description (optional),
# and add the surface mesh to the viewer.
viewer = easy3d.Viewer("Easy3D Viewer")  # Create a viewer instance
viewer.set_usage("")  # Optional: Set an empty usage description for a clean UI

# Add the reconstructed mesh to the viewer
viewer.add_model(surface_mesh)  # Add surface mesh to viewer

# -----------------------------------------------------------------------------
# Step 9: Running the Viewer
# -----------------------------------------------------------------------------
# Finally, we run the viewer to display the reconstructed surface mesh
# in an interactive window.
viewer.run()  # Start the viewer and display the mesh
