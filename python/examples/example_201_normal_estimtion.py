# -------------------------------------------------------------------------------
# Example: Estimate Point Cloud Normals
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
pc = easy3d.PointCloudIO.load(file_name)

# -------------------------------------------------------------------------------
# Estimating Point Cloud Normals
# -------------------------------------------------------------------------------
# Estimate normals for the point cloud. The `16` parameter specifies the
# number of neighbors to use for normal estimation.
easy3d.PointCloudNormals.estimate(pc, 16)

# Reorient the normals to ensure consistency.
easy3d.PointCloudNormals.reorient(pc, 16)

# -------------------------------------------------------------------------------
# Saving the Processed Point Cloud
# -------------------------------------------------------------------------------
# Save the point cloud with estimated normals to a new file in .ply format.
output_file = file_name + "-result.ply"
easy3d.PointCloudIO.save(output_file, pc)
print(f"Processed point cloud saved to: {output_file}")

# -------------------------------------------------------------------------------
# Visualizing the Point Cloud
# -------------------------------------------------------------------------------
# Create a viewer instance to visualize the point cloud.
viewer = easy3d.Viewer("Easy3D Viewer")

# Set to an empty to get rid of the lengthy usage instructions
viewer.set_usage("")

# Add the point cloud model to the viewer and run the viewer to interact with the model.
viewer.add_model(pc)
viewer.run()
