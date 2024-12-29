# -------------------------------------------------------------------------------
# Example: Estimate Point Cloud Normals Using Easy3D
# -------------------------------------------------------------------------------
# This script demonstrates how to estimate normals for a point cloud using Easy3D.
# The script loads a point cloud from a file, estimates its normals, reorients
# them, saves the resulting point cloud, and visualizes it in a viewer.
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

# -------------------------------------------------------------------------------
# Importing Necessary Libraries
# -------------------------------------------------------------------------------
import easy3d  # Easy3D library for 3D visualization and processing

# -------------------------------------------------------------------------------
# Initializing Easy3D
# -------------------------------------------------------------------------------
# Initialize the Easy3D library. The `True` parameter enables detailed logging,
# which is helpful for debugging.
easy3d.initialize(True)

# -------------------------------------------------------------------------------
# Loading the Point Cloud
# -------------------------------------------------------------------------------
# Define the input point cloud file. In this case, a point cloud in .bin format.
file_name = easy3d.directory() + "/data/bunny.bin"
print(f"Input file name: {file_name}")

# Load the point cloud from the specified file.
point_cloud = easy3d.PointCloudIO.load(file_name)

# -------------------------------------------------------------------------------
# Estimating Point Cloud Normals
# -------------------------------------------------------------------------------
# Since we want to visualize both input and resulted point clouds, let's make a
# copy of the input point cloud. The algorithm will be applied to the copy one.
copied_point_cloud = easy3d.PointCloud(point_cloud)  # Create a copy
easy3d.PointCloudNormals.estimate(copied_point_cloud, 16)

# Reorient the normals to ensure consistency.
easy3d.PointCloudNormals.reorient(copied_point_cloud, 16)

# -------------------------------------------------------------------------------
# Saving the Processed Point Cloud
# -------------------------------------------------------------------------------
# Save the point cloud with estimated normals to a new file in .ply format.
output_file = file_name + "-result.ply"
easy3d.PointCloudIO.save(output_file, copied_point_cloud)
print(f"Processed point cloud saved to: {output_file}")

# -------------------------------------------------------------------------------
# Visualizing the input and the resulted point clouds side-by-side
# -------------------------------------------------------------------------------

# Create a MultiViewer instance with 1 row and 2 columns.
viewer = easy3d.MultiViewer(1, 2, "Easy3D Viewer - Normal estimation")

# Add the input point cloud to the viewer and assign it to the left view (row=0, column=0).
viewer.add_model(point_cloud)
viewer.assign(0, 0, point_cloud)

# Add the resulted point cloud to the viewer and assign it to the right view (row=0, column=1).
viewer.add_model(copied_point_cloud)
viewer.assign(0, 1, copied_point_cloud)

# Add instructions for the viewer (optional).
viewer.set_usage(
    "- Left: Original point cloud.\n"
    "- Right: Resulted point cloud with normal information."
)

# # Set the camera view direction and up vector for a better view position.
viewer.camera().setViewDirection(easy3d.vec3(0, 0, -1))   # Set the view direction.
viewer.camera().setUpVector(easy3d.vec3(0, 1, 0))         # Set the up vector.

# Launch the viewer.
viewer.run()