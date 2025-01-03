# -------------------------------------------------------------------------------
# Example: Basic rendering styles in Easy3D
# -------------------------------------------------------------------------------
# This script demonstrates how to use the two most common imposters for rendering
# points and lines in Easy3D:
#   - Render vertices as spheres.
#   - Render edges/wireframe as cylinders.
# This tutorial also show how to customize the surface color and visibility.
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
sphere_path = easy3d.resource_directory() + "/data/sphere.obj"  # File path for the sphere model.
sphere = viewer.add_model(sphere_path)  # Add the model to the viewer.

# -------------------------------------------------------------------------------
# Customize the point imposters: render points as spheres
# -------------------------------------------------------------------------------
# Get the drawable responsible for rendering the vertices of the model.
vertices = sphere.renderer().get_points_drawable("vertices")
vertices.set_visible(True)  # Ensure the vertices are visible.
# sphere.update_vertex_normals()  # This line is for "SURFEL" (SURFEL requires vertex normals).
vertices.set_impostor_type(easy3d.PointsDrawable.SPHERE)  # Render vertices as spheres. Please try "SURFEL"
vertices.set_point_size(25)  # Set the size of the vertex spheres.
vertices.set_uniform_coloring(easy3d.vec4(1.0, 0.3, 0.3, 1.0))  # Set color (RGBA).

# -------------------------------------------------------------------------------
# Customize the line imposters: render lines/edges/wireframe as cylinders
# -------------------------------------------------------------------------------
# Get the drawable responsible for rendering the edges (wireframe) of the model.
edges = sphere.renderer().get_lines_drawable("edges")
edges.set_visible(True)  # Ensure the wireframe is visible.
edges.set_impostor_type(easy3d.LinesDrawable.CYLINDER)  # Render edges as cylinders. Please try "CONE".
edges.set_line_width(5)  # Set the thickness of the wireframe cylinders.
edges.set_uniform_coloring(easy3d.vec4(0.6, 0.6, 1.0, 1.0))  # Set color (RGBA).

# -------------------------------------------------------------------------------
# Customizing the surface rendering
# -------------------------------------------------------------------------------
# Get the drawable responsible for rendering the triangles (surface) of the model.
faces = sphere.renderer().get_triangles_drawable("faces")
faces.set_visible(True)  # Ensure the surface is visible.
# This "/data/sphere.obj" file contains texture coordinates, and by defauly it will
# be rendered with a checkboard texture. You can comment the two lines below to check
# out the textured rendering.
faces.set_uniform_coloring(easy3d.vec4(0.8, 0.8, 0.3, 1.0))  # Set color (RGBA).

# Running the Viewer
viewer.run()