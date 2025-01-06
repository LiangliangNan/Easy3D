# -------------------------------------------------------------------------------
# Example: Working with the PointCloud class in Easy3D
# -------------------------------------------------------------------------------
# This script demonstrates how to use the Easy3D `PointCloud` class:
# - Initialize a `PointCloud` object with a NumPy array of points.
# - Add multiple points at once using a NumPy array.
# - Add individual points using various formats (vec3, lists, NumPy arrays).
# - Export the entire point cloud back to a NumPy array.
# -------------------------------------------------------------------------------

# -------------------------------------------------------------------------------
# Adding Easy3D Python Bindings to the System Path
# -------------------------------------------------------------------------------
# This is required if the bindings are not installed via `pip` but are located in
# a local build directory. For building and installing Python bindings of Easy3D,
# please refer to: https://github.com/LiangliangNan/Easy3D/blob/main/ReadMe.md
# -------------------------------------------------------------------------------
import sys
sys.path.append("../../cmake-build-release/lib/python")  # Update this path to point to your Easy3D build directory.

# -------------------------------------------------------------------------------
# Importing Necessary Libraries
# -------------------------------------------------------------------------------
import easy3d
import numpy as np      # NumPy library (used for handling data)

# -------------------------------------------------------------------------------
# Initializing Easy3D
# -------------------------------------------------------------------------------
# The `easy3d.initialize(False)` function initializes the Easy3D library.
# Use `True` to enable detailed logging, which is useful for debugging.
easy3d.initialize(False)

# -------------------------------------------------------------------------------
# Creating a PointCloud Object and Initializing with Points
# -------------------------------------------------------------------------------
# Create a NumPy array of initial points (each point is represented by [x, y, z]).
initial_points = np.array([[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]])  # Initial points

# Initialize a `PointCloud` object with the array of points.
pc = easy3d.PointCloud(initial_points)

# Set a name for the point cloud. This helps with identification in complex scenes.
pc.set_name("Small Point Cloud")

# Output the name of the point cloud.
print(f"Point cloud name: {pc.name()}")  # Output: Small Point Cloud
print(f"Point cloud has {pc.n_vertices()} vertices.")

# -------------------------------------------------------------------------------
# Adding Multiple Points to the PointCloud
# -------------------------------------------------------------------------------
# Create a NumPy array of additional points to add to the point cloud.
more_points = np.array([[7.0, 8.0, 9.0], [10.0, 11.0, 12.0]])  # Additional points

# Add the new points to the point cloud using `add_points`.
pc.add_points(more_points)
print(f"Point cloud has {pc.n_vertices()} vertices.")

# -------------------------------------------------------------------------------
# Adding Individual Points Using Various Formats
# -------------------------------------------------------------------------------
# Add individual points using the `vec3` format.
v3 = easy3d.vec3(3.0, 3.0, 3.0)  # Creating a vec3 object
pc.add_point(v3)  # Add point using vec3 format
pc.add_point(easy3d.vec3(1, 8, 8))  # Another point using vec3

# Add individual points using Python lists.
pc.add_point([20, 20, 20])  # Add point using list format
pc.add_point([5, 5, 5])     # Another point using list format

# Add individual points using a NumPy array.
point_np = np.array([4.0, 5.0, 6.0], dtype=np.float32)  # Point in NumPy array format
pc.add_point(point_np)  # Add point using NumPy array format

# -------------------------------------------------------------------------------
# Exporting the PointCloud Data to a NumPy Array
# -------------------------------------------------------------------------------
# Convert the point cloud data back to a NumPy array using the `to_numpy()` method.
points_np = pc.to_numpy()

# Output the point cloud data in NumPy array format.
print("Point cloud data as numpy array:\n", points_np)

# Iterate all the faces and print the number of its vertices
for v in pc.vertices():
    print(f"Vertex {v.idx()} : {pc.position(v)}")
