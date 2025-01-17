"""
Tutorial: Rendering a Scalar Field on a Surface Mesh Using Easy3D
=================================================================

This tutorial demonstrates how to visualize a scalar field (e.g., the
elevation of vertices) on a 3D surface mesh using Easy3D's Python bindings.

It covers the following topics:

- Loading and displaying a 3D surface mesh model.
- Computing and assigning scalar values to mesh vertices.
- Visualizing the scalar field by mapping scalar values to colors.

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

# Import the Easy3D library.
import easy3d

# Initialization is required before using any Easy3D functionalities.
# The `False` parameter disables detailed logging. You can set it to `True`
# if you need more detailed output, which might be useful for debugging.
easy3d.initialize(False)

# -----------------------------------------------------------------------------
# Create a Viewer
# -----------------------------------------------------------------------------
# A Viewer is the main window used to visualize 3D objects. In this example, we
# create a Viewer named "Easy3D Viewer - Scalar Field". The Viewer will display
# the loaded 3D model and scalar field.
viewer = easy3d.Viewer("Easy3D Viewer - Scalar Field")

# Optionally, clear the default usage message printed in the console when the viewer starts.
viewer.set_usage("")

# -----------------------------------------------------------------------------
# Load a Surface Mesh Model
# -----------------------------------------------------------------------------
# Load a 3D surface mesh model into the viewer. Here, we use a demo mesh file
# included in Easy3D's resources. The function `easy3d.resource_directory()`
# returns the directory containing Easy3D's bundled resources.
#
# Note: Replace the file path with your own 3D model file if needed. Supported
# formats include OFF, OBJ, PLY, etc.
mesh = viewer.add_model(easy3d.resource_directory() + "/data/building.off")

# Check if the loaded model is a surface mesh. If not, exit the program with an error.
if not isinstance(mesh, easy3d.SurfaceMesh):
    print("Failed to load model. Ensure the file exists and is in the correct format.")
    sys.exit(1)

# -----------------------------------------------------------------------------
# Define a Scalar Field (Elevation)
# -----------------------------------------------------------------------------
# A scalar field assigns a numeric value to each vertex (or face) of the mesh.
# In this example, we compute the elevation (Z-coordinate) of each vertex and
# assign it as a scalar value.
#
# The scalar values are stored as a vertex property named "v:elevation" using
# Easy3D's property system.

# Create a new vertex property to store elevation values. The `dtype=float`
# specifies that the property holds floating-point numbers.
elevation = mesh.add_vertex_property("v:elevation", dtype=float)

# Iterate through all vertices of the mesh and compute their elevation.
# The Z-component of each vertex's position is used as the elevation.
for v in mesh.vertices():
    elevation[v] = mesh.position(v).z

# -----------------------------------------------------------------------------
# Map the Scalar Field to Colors
# -----------------------------------------------------------------------------
# To visualize the scalar field, we map the scalar values to colors using
# a colormap. The `drawable.set_scalar_coloring()` function tells Easy3D
# how to color the mesh based on the scalar field.

# Retrieve the triangles drawable, which is responsible for rendering the mesh faces.
drawable = mesh.renderer().get_triangles_drawable("faces")

# Set up scalar coloring for the drawable.
# - `easy3d.State.VERTEX`: Indicates the scalar values are defined on vertices.
# - `"v:elevation"`: The name of the vertex property that stores the scalar field.
drawable.set_scalar_coloring(
    easy3d.State.VERTEX,  # Scalar field defined on vertices.
    "v:elevation"         # Name of the scalar field property.
)

# -----------------------------------------------------------------------------
# Optional: Use a Custom Colormap.
# -----------------------------------------------------------------------------
# Easy3D allows you to use custom textures as colormaps. For example, you can use
# a colormap like "rainbow.png" to visualize scalar fields.
#
# Uncomment the following two lines to load and apply a custom colormap. Make sure
# the colormap file exists in the Easy3D resource directory.
# texture_file = easy3d.resource_directory() + "/colormaps/rainbow.png"
# drawable.set_texture(texture_file)

# Run the viewer to display the surface mesh and the scalar field.
viewer.run()
