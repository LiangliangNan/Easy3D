# -----------------------------------------------------------------------------
# Tutorial: Primitive Extraction from Point Clouds
# -----------------------------------------------------------------------------
# This tutorial demonstrates how to extract basic geometric primitives
# (planes, cylinders, cones, spheres, and toruses) from 3D point clouds
# using the Easy3D library.
#
# In this example, we will:
# 1. Load a 3D point cloud from a file.
# 2. Configure the PrimitivesRansac class for primitive extraction.
# 3. Extract planes as geometric primitives.
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Step 1: Adding Easy3D Python Bindings to the System Path
# -----------------------------------------------------------------------------
# If the Easy3D Python bindings are located in a custom directory (e.g., from
# a local build), you'll need to add that directory to the system path to
# access them. You can skip this step if the bindings are installed globally
# via `pip`.
#
# Please refer to the official documentation for building and installing
# Easy3D Python bindings:
# https://github.com/LiangliangNan/Easy3D/blob/main/ReadMe.md

import sys

sys.path.append("../../cmake-build-release/lib/python")  # Update this path as needed

# -----------------------------------------------------------------------------
# Step 2: Importing the Easy3D Library
# -----------------------------------------------------------------------------
# Import the Easy3D library, which provides tools for 3D data processing,
# primitive extraction, and visualization.

import easy3d

# -----------------------------------------------------------------------------
# Step 3: Initializing Easy3D
# -----------------------------------------------------------------------------
# Before using any Easy3D functionality, we must initialize the library.
# The 'True' parameter enables detailed logging, useful for debugging and
# understanding the library's internal processes.

easy3d.initialize(True)

# -----------------------------------------------------------------------------
# Step 4: Loading a Point Cloud
# -----------------------------------------------------------------------------
# The Easy3D library supports various point cloud formats (e.g., .bin, .xyz).
# Here, we load a sample point cloud file provided by Easy3D. Replace the path
# with your own dataset as needed.

pointcloud_file = easy3d.directory() + "/data/polyhedron.bin"  # Update if necessary
point_cloud = easy3d.PointCloudIO.load(pointcloud_file)  # Load the point cloud
print(f"Loaded point cloud with {point_cloud.n_vertices()} points.")  # Print point count

# -----------------------------------------------------------------------------
# Step 5: Creating a PrimitivesRansac Instance
# -----------------------------------------------------------------------------
# The PrimitivesRansac class is responsible for detecting geometric primitives
# (e.g., planes, cylinders) in a point cloud using RANSAC (Random Sample Consensus).
# We configure this instance to extract planes in this example.

ransac = easy3d.PrimitivesRansac()  # Create a PrimitivesRansac instance

# -----------------------------------------------------------------------------
# Step 6: Configuring Primitive Types for Extraction
# -----------------------------------------------------------------------------
# Specify the types of primitives to extract. In this example, we add planes.
# Other available types include cylinders, cones, spheres, and toruses.

ransac.add_primitive_type(easy3d.PrimitivesRansac.PLANE)  # Add plane extraction

# -----------------------------------------------------------------------------
# Step 7: Detecting Primitives in the Point Cloud
# -----------------------------------------------------------------------------
# The detect() method applies the RANSAC algorithm to extract primitives.
# Parameters:
# - point_cloud: The input point cloud.
# - min_support: Minimum number of points required for a primitive.
# - dist_threshold: Distance threshold relative to the bounding box's max dimension.
# - bitmap_resolution: Bitmap resolution relative to the bounding box width.
# - normal_threshold: Cosine of the maximum normal deviation.
# - overlook_probability: Probability of overlooking a primitive.

ransac.detect(point_cloud,
              min_support=200,
              dist_threshold=0.005,
              bitmap_resolution=0.02,
              normal_threshold=0.8,
              overlook_probability=0.001,
              )

# -----------------------------------------------------------------------------
# Step 8: Retrieving Extracted Planes
# -----------------------------------------------------------------------------
# Extracted primitives are retrieved using getter methods such as `get_planes`.
# Each plane object contains attributes like:
# - `primitive_index`: Unique index for the plane.
# - `position`: A point on the plane.
# - `normal`: The normal vector of the plane.
# - `vertices`: Indices of points belonging to the plane.

planes = ransac.get_planes()  # Retrieve detected planes
print(f"Number of planes extracted: {len(planes)}")  # Print the number of detected planes

for i, plane in enumerate(planes):
    print(f"Plane {i}:")
    print(f"  Index: {plane.primitive_index}")
    print(f"  Position: {plane.position}")
    print(f"  Normal: {plane.normal}")
    print(f"  Vertices: {plane.vertices}")  # e.g., [5788, 5801, 5777, 5796, ... ]

# -----------------------------------------------------------------------------
# Optional: Detecting Cylinders
# -----------------------------------------------------------------------------
# To detect other primitives (e.g., cylinders), configure the RANSAC instance
# and retrieve the results. Uncomment the following code to detect cylinders.

# ransac.add_primitive_type(easy3d.PrimitivesRansac.CYLINDER)
# ransac.detect(point_cloud, 200, 0.005, 0.02, 0.8, 0.001)
# cylinders = ransac.get_cylinders()
# print(f"Number of cylinders extracted: {len(cylinders)}")
# for i, cylinder in enumerate(cylinders):
#     print(f"Cylinder {i}:")
#     print(f"  Index: {cylinder.primitive_index}")
#     print(f"  Position: {cylinder.position}")
#     print(f"  Direction: {cylinder.direction}")
#     print(f"  Radius: {cylinder.radius}")
