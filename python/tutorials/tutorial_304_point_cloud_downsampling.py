# -----------------------------------------------------------------------------
# Tutorial: Point Cloud Downsampling Using Easy3D
# -----------------------------------------------------------------------------
# This tutorial demonstrates how to downsample a 3D point cloud using the Easy3D
# library. Downsampling reduces the number of points while preserving the overall
# structure of the cloud, which can speed up processing and reduce memory usage.
#
# In this tutorial, we will:
# 1. Load a point cloud from a file.
# 2. Apply different downsampling techniques, including grid and uniform
#    simplifications, and measure their effects on the point cloud size.
# -----------------------------------------------------------------------------

# If you built Easy3D locally, you'll need to add its Python bindings to the
# system path to access them. If Easy3D is globally installed via `pip`, this
# step is not necessary. For building and installing Python bindings of Easy3D,
# # please refer to: https://github.com/LiangliangNan/Easy3D/blob/main/ReadMe.md
import sys
sys.path.append("../../cmake-build-release/lib/python")  # Update this path as needed

# Importing Easy3D and Initializing the Library
# Initialization is required before using Easy3D, and the 'True' parameter enables
# detailed logging for debugging purposes.
import easy3d
easy3d.initialize(True)

# -----------------------------------------------------------------------------
# Loading a Point Cloud
# -----------------------------------------------------------------------------
# Easy3D supports various file formats for point clouds (e.g., .bin, .xyz).
# Here, we load a sample file provided by Easy3D. Replace the path with your
# own file if needed.
pointcloud_file = easy3d.directory() + "/data/polyhedron.bin"  # Update as needed
point_cloud = easy3d.PointCloudIO.load(pointcloud_file)  # Load the point cloud
print(f"Loaded point cloud with {point_cloud.n_vertices()} points.")  # Print the initial point count

# -----------------------------------------------------------------------------
# Downsampling a Point Cloud
# -----------------------------------------------------------------------------
# Downsampling reduces the number of points in a cloud while maintaining its
# structure. Below, we apply two common techniques: grid and uniform simplifications.

# --- Grid Simplification ---
# Grid simplification reduces the point cloud's density based on a 3D grid.
# Points falling within the same grid cell are simplified into one representative point.
# The `cell_size` parameter determines the resolution of the grid. Larger values result
# in fewer points.
cell_size = 0.01  # Adjust based on the scale of your data
grid_indices = easy3d.PointCloudSimplification.grid_simplification(point_cloud, cell_size)
print(f"Grid simplification: {len(grid_indices)} points will be removed.")

# --- Uniform Simplification ---
# Uniform simplification ensures a minimum distance between remaining points in the cloud.
# Option 1: Specify a distance threshold (`epsilon`) to control the spacing between points.
epsilon = 0.005
uniform_indices_by_epsilon = easy3d.PointCloudSimplification.uniform_simplification(point_cloud, epsilon=epsilon)
print(f"Uniform simplification (distance threshold): {len(uniform_indices_by_epsilon)} points will be removed.")

# Option 2: Specify the desired number of points in the final point cloud (`num`).
target_point_count = 50000
uniform_indices_by_number = easy3d.PointCloudSimplification.uniform_simplification(point_cloud, num=target_point_count)
print(f"Uniform simplification (target point count): {len(uniform_indices_by_number)} points will be removed.")

# -----------------------------------------------------------------------------
# Applying Simplification
# -----------------------------------------------------------------------------
# The indices returned by simplification methods indicate the points to be removed.
# We use `delete_points()` to apply the simplifications to the point cloud.
point_cloud.delete_points(uniform_indices_by_number)
print(f"Final point cloud size after simplification: {point_cloud.n_vertices()} points.")

# Optional: Save the result to a file if needed (or visualize using the Easy3D viewer).
# output_file = "downsampled_point_cloud.ply"  # Specify the output file name
# easy3d.PointCloudIO.save(output_file, point_cloud)
# print(f"Downsampled point cloud saved to {output_file}")