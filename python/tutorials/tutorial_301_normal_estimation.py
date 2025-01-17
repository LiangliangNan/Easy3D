"""
Tutorial: Estimate Point Cloud Normals Using Easy3D
===================================================

This script demonstrates how to estimate and reorient normals for a point cloud
using Easy3D. The script loads a point cloud from a file, visualizes it in a
viewer, and allows you to interactively estimate and reorient the normals using
keyboard shortcuts.

"""

# -------------------------------------------------------------------------------
# Adding Easy3D Python Bindings to the System Path
# -------------------------------------------------------------------------------
# This is required if the bindings are not installed via `pip` but are located in
# a local build directory. For building and installing Python bindings of Easy3D,
# please refer to: https://github.com/LiangliangNan/Easy3D/blob/main/README.md
# -------------------------------------------------------------------------------
import sys
sys.path.append("../../cmake-build-release/lib/python")  # Update this path to point to your Easy3D build directory.

import sys
sys.path.append("../../cmake-build-release/lib/python")  # Update this path to point to your Easy3D build directory.

# Import the Easy3D library, which provides tools for 3D visualization and geometry processing.
import easy3d

# Initialization is required before using any Easy3D functionalities.
# The `False` parameter disables detailed logging. You can set it to `True`
# if you need more detailed output, which might be useful for debugging.
easy3d.initialize(False)

# -------------------------------------------------------------------------------
# Defining Functions for Normal Estimation and Reorientation
# -------------------------------------------------------------------------------
# Here, we define two functions: one for estimating normals and another for reorienting them.
# These functions will be bound to keyboard shortcuts, allowing you to execute them
# interactively while the viewer is running.

def estimate(viewer, cloud):
    """
    Estimate the normals of a point cloud.
    This function uses Easy3D's built-in normal estimation algorithm.
    """
    # Call the normal estimation algorithm of Easy3D.
    # The second argument (16) specifies the number of nearest neighbors for normal estimation.
    easy3d.PointCloudNormals.estimate(cloud, 16)

    # Update the drawable so the rendering will reflect the changes in the normals.
    cloud.renderer().get_points_drawable("vertices").update()

    print("Done normal estimation.")
    return True


def reorient(viewer, cloud):
    """
    Reorient the normals of a point cloud.
    This function uses Easy3D's built-in normal reorientation algorithm.
    """
    # Call the normal reorientation algorithm of Easy3D.
    # The second argument (16) specifies the number of nearest neighbors for normal reorientation.
    easy3d.PointCloudNormals.reorient(cloud, 16)

    # Update the drawable so the rendering will reflect the changes in the normals.
    cloud.renderer().get_points_drawable("vertices").update()

    print("Done normal reorientation.")
    return True


# -------------------------------------------------------------------------------
# Creating a Viewer Instance
# -------------------------------------------------------------------------------
# The Easy3D Viewer is a powerful tool for visualizing and interacting with 3D data.
# Here, we create an instance of the Viewer with a custom title.
viewer = easy3d.Viewer(title="Easy3D Viewer - Interactive Normal Estimation and Reorientation")

# -------------------------------------------------------------------------------
# Loading a Point Cloud
# -------------------------------------------------------------------------------
# Load a point cloud from the Easy3D resource directory. This directory contains
# sample data for testing and demonstration purposes. You can replace this with
# your own 3D data file (e.g., .ply, .obj, .bin, etc.).
point_cloud = viewer.add_model(easy3d.resource_directory() + "/data/bunny.bin")

# -------------------------------------------------------------------------------
# Binding Functions to Keyboard Shortcuts
# -------------------------------------------------------------------------------
# Bind the custom functions to keyboard shortcuts. This allows you to execute
# the functions interactively while the viewer is running.
# - `estimate` is bound to the shortcut `Ctrl + E`.
# - `reorient` is bound to the shortcut `Ctrl + R`.
viewer.bind(estimate, point_cloud, easy3d.Viewer.KEY_E, easy3d.Viewer.MODIF_CTRL) # Modifier is optional
viewer.bind(reorient, point_cloud, easy3d.Viewer.KEY_R, easy3d.Viewer.MODIF_CTRL) # Modifier is optional

# Display usage instructions in the viewer window.
viewer.set_usage("",
                 "  Ctrl + e: Estimate normals\n"
                 "  Ctrl + r: Reorient normals")

# -------------------------------------------------------------------------------
# Enhancing Visualization
# -------------------------------------------------------------------------------
# To clearly observe any inconsistency in normals, let's set different colors for
# the two sides of the model. This makes it easier to visualize the orientation of the normals.
point_cloud.renderer().get_points_drawable("vertices").set_distinct_back_color(True)

# Set the camera view direction and up vector for a better view.
viewer.camera().setViewDirection(easy3d.vec3(0, -1, 0))     # Set the view direction.
viewer.camera().setUpVector(easy3d.vec3(-1, 0, 0))          # Set the up vector.

# -------------------------------------------------------------------------------
# Running the Viewer
# -------------------------------------------------------------------------------
# Start the Easy3D Viewer. This will open a window where you can interact with
# the 3D data and execute the two functions using the bound shortcuts.
viewer.run()