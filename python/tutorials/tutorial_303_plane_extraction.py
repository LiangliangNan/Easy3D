# -----------------------------------------------------------------------------
# Tutorial: Primitive Extraction from Point Clouds Using Easy3D
# -----------------------------------------------------------------------------
# This tutorial demonstrates how to extract basic geometric primitives
# (planes, cylinders, cones, spheres, and toruses) from 3D point clouds
# using the Easy3D library.
#
# In this example, we will:
# 1. Load a 3D point cloud from a file.
# 2. Configure the PrimitivesRansac class for primitive extraction.
# 3. Extract planes as geometric primitives.
# 4. Visualize the extracted planar segments.
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Adding Easy3D Python Bindings to the System Path
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
# Importing the Easy3D Library
# -----------------------------------------------------------------------------
# Import the Easy3D library, which provides tools for 3D data processing,
# primitive extraction, and visualization.

import easy3d

# -----------------------------------------------------------------------------
# Initializing Easy3D
# -----------------------------------------------------------------------------
# Before using any Easy3D functionality, we must initialize the library.
# The 'True' parameter enables detailed logging, useful for debugging and
# understanding the library's internal processes.

easy3d.initialize(True)

# -----------------------------------------------------------------------------
# Loading a Point Cloud
# -----------------------------------------------------------------------------
# The Easy3D library supports various point cloud formats (e.g., .bin, .xyz).
# Here, we load a sample point cloud file provided by Easy3D. Replace the path
# with your own dataset as needed.

pointcloud_file = easy3d.resource_directory() + "/data/polyhedron.bin"  # Update if necessary
point_cloud = easy3d.PointCloudIO.load(pointcloud_file)  # Load the point cloud
print(f"Loaded point cloud with {point_cloud.n_vertices()} points.")  # Print point count

# -----------------------------------------------------------------------------
# Creating a PrimitivesRansac Instance
# -----------------------------------------------------------------------------
# The PrimitivesRansac class is responsible for detecting geometric primitives
# (e.g., planes, cylinders) in a point cloud using RANSAC (Random Sample Consensus).
# We configure this instance to extract planes in this example.

ransac = easy3d.PrimitivesRansac()  # Create a PrimitivesRansac instance

# -----------------------------------------------------------------------------
# Configuring Primitive Types for Extraction
# -----------------------------------------------------------------------------
# Specify the types of primitives to extract. In this example, we add planes.
# Other available types include cylinders, cones, spheres, and toruses.

ransac.add_primitive_type(easy3d.PrimitivesRansac.PLANE)  # Add plane extraction

# -----------------------------------------------------------------------------
# Detecting Primitives in the Point Cloud
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
# Retrieving Extracted Planes
# -----------------------------------------------------------------------------
# Extracted primitives are retrieved using getter methods such as `get_planes`.
# Each plane object contains attributes like:
# - `primitive_index`: Unique index for the plane.
# - `position`: A point on the plane.
# - `normal`: The normal vector of the plane.
# - `vertices`: Indices of points belonging to the plane.
# Note: The extracted primitives are also stored as properties:
#   - "v:primitive_type"  (one of PLANE, SPHERE, CYLINDER, CONE, TORUS, and UNKNOWN)
#   - "v:primitive_index" (-1, 0, 1, 2...). -1 meaning a vertex does not belong to any primitive (thus its
#     primitive_type must be UNKNOWN.
planes = ransac.get_planes()  # Retrieve detected planes
print(f"Number of planes extracted: {len(planes)}")  # Print the number of detected planes

for i, plane in enumerate(planes):
    print(f"Plane {i}:")
    print(f"  Index: {plane.primitive_index}")
    print(f"  Position: {plane.position}")
    print(f"  Normal: {plane.normal}")

    # Print up to 20 vertex indices (for demonstration purposes)
    vertex_count = len(plane.vertices)
    print(f"  Vertices ({min(vertex_count, 20)} out of {vertex_count} shown): {plane.vertices[:20]}")  # e.g., [5788, 5801, 5777, 5796, ... ]

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
#     # Print up to 20 vertex indices (for demonstration purposes)
#     vertex_count = len(cylinder.vertices)
#     print(f"  Vertices ({min(vertex_count, 20)} out of {vertex_count} shown): {cylinder.vertices[:20]}")  # e.g., [5788, 5801, 5777, 5796, ... ]

# -----------------------------------------------------------------------------
# Visualizing the extracted planar segments
# -----------------------------------------------------------------------------

viewer = easy3d.Viewer("Easy3D Viewer - Plane extraction")  # Create a viewer
viewer.set_usage("") # Optional. Just to hide the lengthy manual in console window
viewer.add_model(point_cloud)  # Add the point cloud to the viewer

# The extracted primitives are also stored in int type vertex property "v:primitive_index".
# We can color the point cloud by this property.
point_cloud.renderer().color_by_segmentation("v:primitive_index")

# Run the viewer
viewer.run()