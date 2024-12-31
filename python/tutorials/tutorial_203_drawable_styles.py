# -------------------------------------------------------------------------------
# Example: Basic rendering styles in Easy3D
# -------------------------------------------------------------------------------
# This script demonstrates how to control the rendering styles of basic drawables。
#   - Render vertices as spheres.
#   - Render edges as cylinders (wireframe).
#   - Customize the surface color and visibility.
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
import easy3d   # Easy3D library for 3D visualization and processing

# -------------------------------------------------------------------------------
# Initializing Easy3D
# -------------------------------------------------------------------------------
# The `easy3d.initialize(False)` function initializes the Easy3D library.
# Use `True` to enable detailed logging, which is useful for debugging.
easy3d.initialize(False)

# -------------------------------------------------------------------------------
# Creating a Viewer Instance
# -------------------------------------------------------------------------------
# Create a Viewer with a custom title.
viewer = easy3d.Viewer("Easy3D Viewer - Rendering styles")
viewer.set_usage("") # Optional. Just to hide the lengthy manual in console window

# -------------------------------------------------------------------------------
# Loading a 3D Model (Sphere)
# -------------------------------------------------------------------------------
# Load a sphere model from an OBJ file located in the Easy3D data directory.
# Replace the file path with the location of your model if necessary.
sphere_path = easy3d.directory() + "/data/sphere.obj"  # File path for the sphere model.
sphere = viewer.add_model(sphere_path)  # Add the model to the viewer.

# -------------------------------------------------------------------------------
# Customizing the Rendering Style
# -------------------------------------------------------------------------------
# By default, only the surface of a SurfaceMesh is rendered. In this example:
#  - The surface color is modified but remains visible.
#  - The vertices are displayed as spheres.
#  - The edges (wireframe) are rendered as cylinders.

# Customize the surface rendering:
# Get the drawable responsible for rendering the triangles (surface) of the model.
sphere_faces = sphere.renderer().get_triangles_drawable("faces")
sphere_faces.set_visible(True)  # Ensure the surface is visible.
# This "/data/sphere.obj" file contains texture coordinates, and it will be rendered
# with the default checkboard texture. You can comment the two lines below to check
# out the textured rendering.
sphere_faces.set_coloring_method(easy3d.State.UNIFORM_COLOR)    # Set uniform color rendering.
sphere_faces.set_color(easy3d.vec4(0.8, 0.8, 0.3, 1.0))         # Set the surface color (RGBA).

# Customize the vertex rendering:
# Get the drawable responsible for rendering the vertices of the model.
sphere_vertices = sphere.renderer().get_points_drawable("vertices")
sphere_vertices.set_visible(True)  # Ensure the vertices are visible.
sphere_vertices.set_impostor_type(easy3d.PointsDrawable.SPHERE)  # Render vertices as spheres.
sphere_vertices.set_point_size(25)  # Set the size of the vertex spheres.
sphere_vertices.set_color(easy3d.vec4(1.0, 0.3, 0.3, 1.0))  # Set the vertex color (RGBA).

# Customize the edge rendering:
# Get the drawable responsible for rendering the edges (wireframe) of the model.
sphere_wireframe = sphere.renderer().get_lines_drawable("edges")
sphere_wireframe.set_visible(True)  # Ensure the wireframe is visible.
sphere_wireframe.set_impostor_type(easy3d.LinesDrawable.CYLINDER)  # Render edges as cylinders.
sphere_wireframe.set_line_width(5)  # Set the thickness of the wireframe cylinders.
sphere_wireframe.set_uniform_coloring(easy3d.vec4(0.6, 0.6, 1.0, 1.0))  # Set the wireframe color (RGBA).

# -------------------------------------------------------------------------------
# Running the Viewer
# -------------------------------------------------------------------------------
viewer.run()


# -------------------------------------------------------------------------------
# Notes:
# 1. Ensure the file paths are correct, or replace them with paths to your own models.
# 2. The viewer window remains open until the user manually closes it.
# 3. You can modify the impostor type, colors, and sizes to explore different visual styles.
# -------------------------------------------------------------------------------
