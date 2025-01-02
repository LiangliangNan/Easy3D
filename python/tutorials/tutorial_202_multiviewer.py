# -------------------------------------------------------------------------------
# Example: The Easy3D Multi-View Viewer of Easy3D
# -------------------------------------------------------------------------------
# This script demonstrates how to use the Easy3D `MultiViewer` class to:
# - Create a multi-view 3D viewer with a custom title.
# - Set up a 2x2 layout for visualizing multiple 3D models simultaneously.
# - Load and visualize different types of 3D models.
# - Assign models to specific views in the multi-view grid.
# - Run the viewer to interact with the models (e.g., rotate, zoom, pan).
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

# Importing necessary libraries
import easy3d           # Easy3D library for 3D visualization and processing

# -------------------------------------------------------------------------------
# Initializing Easy3D
# -------------------------------------------------------------------------------
# The `easy3d.initialize(False)` function initializes the Easy3D library.
# Use `True` to enable detailed logging, which is useful for debugging.
easy3d.initialize(False)

# -------------------------------------------------------------------------------
# Setting Up a Multi-View Layout with Easy3D MultiViewer
# -------------------------------------------------------------------------------

# Create a `MultiViewer` instance with a 2x2 grid layout and a custom window title.
# The MultiViewer allows displaying multiple views in a single window.
viewer = easy3d.MultiViewer(2, 2, "Easy3D Viewer - MultiViewer")
viewer.set_usage("") # Optional. Just to hide the lengthy manual in console window

# -------------------------------------------------------------------------------
# Loading Models for Each View in the 2x2 Layout
# -------------------------------------------------------------------------------

# Each model is loaded and assigned to a specific view in the MultiViewer grid.
# The grid layout uses (row, column) coordinates to specify each view's position.

## View (0, 0) - Bunny Model (SurfaceMesh)
# This classic 3D model represents a bunny and is loaded as a surface mesh.
bunny_model = viewer.add_model(easy3d.resource_directory() + "/data/bunny.ply" )  # Load the model.
viewer.assign(0, 0, bunny_model)  # Assign the Bunny model to the top-left view (0, 0).

## View (0, 1) - Mannequin Model (SurfaceMesh)
# The mannequin is a human-like 3D model and is loaded as a surface mesh.
mannequin_model = viewer.add_model(easy3d.resource_directory() + "/data/mannequin.ply")  # Load the model.
viewer.assign(0, 1, mannequin_model)  # Assign the Mannequin model to the top-right view (0, 1).

## View (1, 0) - Polyhedron Model (PointCloud)
# The polyhedron model is a point cloud, representing a collection of 3D points.
polyhedron_model = viewer.add_model(easy3d.resource_directory() + "/data/polyhedron.bin")  # Load the model.
viewer.assign(1, 0, polyhedron_model)  # Assign the Polyhedron model to the bottom-left view (1, 0).

## View (1, 1) - Fandisk Model (SurfaceMesh)
# The fandisk is a commonly used 3D model and is loaded as a surface mesh.
fandisk_model = viewer.add_model(easy3d.resource_directory() + "/data/fandisk.off")  # Load the model.
viewer.assign(1, 1, fandisk_model)  # Assign the Fandisk model to the bottom-right view (1, 1).

# -------------------------------------------------------------------------------
# Running the Viewer
# -------------------------------------------------------------------------------
# Run the viewer to display the models in the multi-view layout.
# Note:
# - The viewer window remains open until the user manually closes it.
viewer.run()