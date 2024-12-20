import sys
sys.path.append("cmake-build-release/")

import easy3d_core
import easy3d_algo
import easy3d_fileio
import easy3d_util
import easy3d_viewer

# Initialize Easy3D
# easy3d_util.initialize()

file_name = "/home/geo3d/Documents/Projects/Easy3D/resources/data/bunny.bin"
# Load the point cloud from a file
point_cloud = easy3d_fileio.PointCloudIO.load(file_name)
if not point_cloud:
    raise ValueError("Failed to load the point cloud.")

# Estimate normals for the point cloud
if not easy3d_algo.PointCloudNormals.estimate(point_cloud, 16, False):
    raise ValueError("Normal estimation failed.")

# Optional: Reorient normals if needed
if not easy3d_algo.PointCloudNormals.reorient(point_cloud, 16):
    raise ValueError("Normal reorientation failed.")

# Visualize the point cloud
# viewer = easy3d_viewer.Viewer("Easy3D Viewer")  # Replace with your actual viewer class
# viewer.add_model(point_cloud)
# viewer.run()

# Save the point cloud into a file
if not easy3d_fileio.PointCloudIO.save(file_name + "-result.ply", point_cloud):
    raise ValueError("Saving file failed.")