# -------------------------------------------------------------------------------
# Example: The basic Easy3D viewer
# -------------------------------------------------------------------------------
# This script demonstrates how to use the Easy3D `Viewer` class to:
# 1. Create a 3D viewer with a custom title.
# 2. Load and visualize a 3D mesh model (`SurfaceMesh`) from a file.
# 3. Load and visualize a 3D point cloud model (`PointCloud`) from a file.
# 4. Run the viewer to interact with the models (e.g., rotate, zoom, pan).


# -------------------------------------------------------------------------------
# Adding Easy3D Python Bindings to the System Path
# -------------------------------------------------------------------------------
# This is required if the bindings are not installed via `pip` but are located in
# a local build directory. To install the bindings, use the following command:
#       "pip install YOUR_BUILD_DIRECTORY/lib/python/setup.py"
# -------------------------------------------------------------------------------
import sys
sys.path.append("../../cmake-build-release/lib/python")

# Importing necessary libraries
import easy3d           # Easy3D library for 3D visualization and processing
import numpy as np      # NumPy library (used for handling data if needed)

# -------------------------------------------------------------------------------
# Initializing Easy3D
# -------------------------------------------------------------------------------
# The `easy3d.initialize(False)` function initializes the Easy3D library.
# Use `True` to enable detailed logging, which is useful for debugging.
easy3d.initialize(False)

# -------------------------------------------------------------------------------
# Creating a Viewer
# -------------------------------------------------------------------------------
# The `Viewer` class provides a GUI window for visualizing 3D models.
# Here, we create a viewer instance with a custom title, "Easy3D Viewer".
viewer = easy3d.Viewer("Easy3D Viewer")

# -------------------------------------------------------------------------------
# Loading and Adding a 3D Mesh Model
# -------------------------------------------------------------------------------
# Load a 3D mesh file (e.g., Stanford Bunny in .ply format).
# The `easy3d.directory()` function retrieves the root directory of Easy3D's
# resources. The model is located in the `data` subdirectory.
mesh_file = easy3d.directory() + "/data/bunny.ply"

# Add the mesh model to the viewer.
# This can be done directly using the file path, as shown below:
viewer.add_model(mesh_file)

# Alternatively, you can load the model as a `SurfaceMesh` object and add it:
# model = easy3d.SurfaceMeshIO.load(mesh_file)
# viewer.add_model(model)

# -------------------------------------------------------------------------------
# Loading and Adding a 3D Point Cloud Model
# -------------------------------------------------------------------------------
# Load a 3D point cloud file (e.g., a polyhedron in .bin format).
pointcloud_file = easy3d.directory() + "/data/polyhedron.bin"

# The `PointCloudIO.load()` function reads the point cloud data from the file.
pointcloud = easy3d.PointCloudIO.load(pointcloud_file)

# Add the point cloud model to the viewer.
# You can add the point cloud object directly as shown below:
viewer.add_model(pointcloud)

# Alternatively, the point cloud file path can also be added directly:
# viewer.add_model(pointcloud_file)

# -------------------------------------------------------------------------------
# Running the Viewer
# -------------------------------------------------------------------------------
# The `viewer.run()` function launches the viewer window, where users can:
# - Interact with the 3D models (e.g., rotate, zoom, pan).
# - Explore the structure and details of the loaded models.
viewer.run()
