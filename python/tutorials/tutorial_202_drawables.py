# -------------------------------------------------------------------------------
# Tutorial: Using Drawables in Easy3D
# -------------------------------------------------------------------------------
# This tutorial demonstrates how to visualize 3D data using Easy3D's drawables
# without explicitly defining a 3D model. We'll cover:
#   1. Visualizing a surface model using `TrianglesDrawable`.
#   2. Visualizing points with `PointsDrawable`.
#   3. Visualizing bounding boxes using `LinesDrawable`.
#   4. Understanding and configuring drawables for specific rendering purposes.
#   5. Running the Easy3D Viewer to display the visualizations.
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

# Create a Viewer instance
# -------------------------------------------------------------------------------
viewer = easy3d.Viewer("Easy3D Viewer - Drawables")
viewer.set_usage("")  # Optionally set usage information for the viewer

# Load 3D data
# -------------------------------------------------------------------------------
# Easy3D provides some predefined models, such as the Stanford Bunny. 
# Here, we load the Bunny's vertex positions (`bunny_vertices`) and triangle face
# indices (`bunny_indices`).
points = easy3d.bunny_vertices  # Vertex coordinates
indices = easy3d.bunny_indices  # Triangle face indices

#  Visualize the Bunny surface using `TrianglesDrawable`
# -------------------------------------------------------------------------------
# `TrianglesDrawable` is used to render triangular meshes. We upload the vertex
# positions and indices of the Bunny model for visualization.

# Create a `TrianglesDrawable` named "faces"
surface = easy3d.TrianglesDrawable("faces")
surface.update_vertex_buffer(points)  # Upload vertex positions
surface.update_element_buffer(indices)  # Upload triangle indices
viewer.add_drawable(surface)  # Add the drawable to the viewer

# Visualize the Bunny vertices using `PointsDrawable`
# -------------------------------------------------------------------------------
# `PointsDrawable` is used to render point clouds. We upload the Bunny's vertices 
# and configure their appearance (e.g., color, size).

# Create a `PointsDrawable` named "vertices"
vertices = easy3d.PointsDrawable("vertices")
vertices.update_vertex_buffer(points)  # Upload vertex positions
vertices.set_uniform_coloring(easy3d.vec4(1.0, 0.0, 0.0, 1.0))  # Set color to red
vertices.set_impostor_type(easy3d.PointsDrawable.SPHERE)  # Render points as spheres
vertices.set_point_size(10)  # Set point size
viewer.add_drawable(vertices)  # Add the drawable to the viewer

# Compute the bounding box of the Bunny model
# -------------------------------------------------------------------------------
# The `bounding_box` function computes the axis-aligned bounding box (AABB) of
# a set of points. We'll use this to render a bounding box around the Bunny.

# Compute the bounding box
bbox = easy3d.bounding_box(points)
# Retrieve the min and max coordinates of the bounding box
min_point = bbox.min_point()
max_point = bbox.max_point()

# Define the 8 corner points of the bounding box
bbox_points = [
    easy3d.vec3(min_point.x, min_point.y, max_point.z),
    easy3d.vec3(max_point.x, min_point.y, max_point.z),
    easy3d.vec3(min_point.x, max_point.y, max_point.z),
    easy3d.vec3(max_point.x, max_point.y, max_point.z),
    easy3d.vec3(min_point.x, min_point.y, min_point.z),
    easy3d.vec3(max_point.x, min_point.y, min_point.z),
    easy3d.vec3(min_point.x, max_point.y, min_point.z),
    easy3d.vec3(max_point.x, max_point.y, min_point.z),
]

# Define the indices of the lines that connect the bounding box corners
bbox_indices = [
    0, 1, 2, 3, 4, 5, 6, 7,  # Lines connecting top and bottom
    0, 2, 4, 6, 1, 3, 5, 7,  # Lines connecting vertical edges
    0, 4, 2, 6, 1, 5, 3, 7   # Lines connecting horizontal edges
]

# Visualize the bounding box using `LinesDrawable`
# -------------------------------------------------------------------------------
# `LinesDrawable` is used to render lines. We upload the bounding box corners 
# and the indices of the lines connecting them.

# Create a `LinesDrawable` named "bbox"
bbox_drawable = easy3d.LinesDrawable("bbox")
bbox_drawable.update_vertex_buffer(bbox_points)  # Upload corner points
bbox_drawable.update_element_buffer(bbox_indices)  # Upload line indices
bbox_drawable.set_uniform_coloring(easy3d.vec4(0.0, 0.0, 1.0, 1.0))  # Set color to blue
bbox_drawable.set_line_width(5.0)  # Set line width
viewer.add_drawable(bbox_drawable)  # Add the drawable to the viewer

# Run the viewer
# -------------------------------------------------------------------------------
# The `viewer.run()` function launches the GUI window for visualization.
viewer.run()
