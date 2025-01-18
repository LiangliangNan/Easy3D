"""
208: Interactive function
=========================

This tutorial demonstrates how to bind customized functions to shortcuts in
the Easy3D Viewer. This allows you to execute your own geometry processing
algorithms interactively while the viewer is running. This is particularly
useful for demonstrating and testing 3D data processing algorithms.
"""

# sphinx_gallery_thumbnail_path = '_static/sphx_glr_tutorial_208_interactive_function_execution_thumb.png'

# -------------------------------------------------------------------------------
# Adding Easy3D Python Bindings to the System Path
# -------------------------------------------------------------------------------
# This is required if the bindings are not installed via `pip` but are located in
# a local build directory. For building and installing Python bindings of Easy3D,
# please refer to: https://github.com/LiangliangNan/Easy3D/blob/main/README.md
# -------------------------------------------------------------------------------
import sys
sys.path.append("../../cmake-build-release/lib/python")  # Update this path to point to your Easy3D build directory.

# Import the Easy3D library.
import easy3d

# Initialization is required before using any Easy3D functionalities.
# The `False` parameter disables detailed logging. You can set it to `True`
# if you need more detailed output, which might be useful for debugging.
easy3d.initialize(False)

# -----------------------------------------------------------------------------
# Defining Custom Functions for Processing 3D Data
# -----------------------------------------------------------------------------
# Here, we define two example functions that will be bound to keyboard shortcuts.
# These functions demonstrate how to interact with 3D data (e.g., point clouds)
# in the Easy3D Viewer.

def process_1(viewer, cloud):
    """
    Example function to process a point cloud.
    This function prints the name of the point cloud and a placeholder message.
    Replace the placeholder with your own geometry processing algorithm.
    """
    print("------------ running process_1................")
    print(f"Input point cloud is: {cloud.name()}.")
    print(f"You should implement your process_1 function here.")
    # TODO: Implement your algorithm for processing the point cloud here...
    return True

def process_2(viewer, cloud):
    """
    Example function to process a point cloud.
    This function prints the number of points in the point cloud and a placeholder message.
    Replace the placeholder with your own geometry processing algorithm.
    """
    print("------------ running process_2................")
    print(f"Input point cloud has {cloud.n_vertices()} points.")
    print(f"You should implement your process_2 function here.")
    # TODO: Implement your algorithm for processing the point cloud here...
    return True

# -----------------------------------------------------------------------------
# Creating a Viewer Instance
# -----------------------------------------------------------------------------
# The Easy3D Viewer is a powerful tool for visualizing and interacting with 3D data.
# Here, we create an instance of the Viewer with a custom title.
viewer = easy3d.Viewer(title="Easy3D Viewer - Interactive Function Execution")

# -----------------------------------------------------------------------------
# Loading a 3D Model (Point Cloud)
# -----------------------------------------------------------------------------
# Load a point cloud from the Easy3D resource directory. This directory contains
# sample data for testing and demonstration purposes. You can replace this with
# your own 3D data file (e.g., .ply, .obj, .bin, etc.).
point_cloud = viewer.add_model(easy3d.resource_directory() + "/data/polyhedron.bin")

# -----------------------------------------------------------------------------
# Binding Custom Functions to Keyboard Shortcuts
# -----------------------------------------------------------------------------
# Bind the custom functions to keyboard shortcuts. This allows you to execute
# your functions interactively while the viewer is running. In this tutorial,
# let's create two functions for different processing purposes:
# - `process_1`, bound to the shortcut `e`.
# - `process_2`, bound to the shortcut `Ctrl + r`.
viewer.bind(process_1, point_cloud, easy3d.Viewer.KEY_E) # without key modifier
viewer.bind(process_2, point_cloud, easy3d.Viewer.KEY_R, easy3d.Viewer.MODIF_CTRL) # with key modifier

# Display usage instructions in the viewer window.
viewer.set_usage("",
                 "       e: Run process_1\n"
                 "Ctrl + r: Run process_2")

# -----------------------------------------------------------------------------
# Running the Viewer
# -----------------------------------------------------------------------------
# Start the Easy3D Viewer. This will open a window where you can interact with
# the 3D data and execute your custom functions using the bound shortcuts.
viewer.run()