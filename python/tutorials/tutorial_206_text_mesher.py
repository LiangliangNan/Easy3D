"""
Tutorial: Generating and Visualizing 3D Text with Easy3D
========================================================

This tutorial demonstrates how to use Easy3D's `TextMesher` to:

- Generate 3D surface mesh models from text.
- Visualize the generated 3D text meshes using the default Easy3D Viewer.

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

# -----------------------------------------------------------------------------
# Importing the Easy3D Library and Initializing
# -----------------------------------------------------------------------------
# Import the Easy3D library for visualization and 3D model generation.
import easy3d
# Initialization is required before using Easy3D functionalities.
# The `True` parameter enables detailed logging for debugging purposes.
easy3d.initialize(True)

# -----------------------------------------------------------------------------
# Setting Up the TextMesher
# -----------------------------------------------------------------------------
# The `TextMesher` class is used to create 3D meshes from text.
# A font file is required to define the text's style. Here, we use the
# "Roboto-Regular" font file included in Easy3D's default directory.

# Path to the font file (update the path if needed).
font_path = easy3d.resource_directory() + "/fonts/en_Roboto-Regular.ttf"

# Create an instance of the `TextMesher` using the specified font.
mesher = easy3d.TextMesher(font_path)

# -----------------------------------------------------------------------------
# Generating a 3D Mesh from Text
# -----------------------------------------------------------------------------
# Generate a 3D mesh with the text "Hello, Easy3D!".
# Key parameters:
# - `x` and `y`: Position of the text in 2D space.
# - `font_size`: Size of the text (affects the height and width).
# - `height`: Depth of the 3D text extrusion.
# - `collision_free`: Ensures that generated meshes do not have overlapping geometry.

text = "Hello, Easy3D!"  # Text to be converted to 3D.
mesh = mesher.generate(text, x=0, y=0, font_size=48, height=15, collision_free=True)

# Check if the mesh was successfully generated.
if mesh:
    print(f"3D mesh for '{text}' generated successfully!")
else:
    print("Failed to generate the 3D mesh. Check the parameters or font file.")

# -----------------------------------------------------------------------------
# Step 5: Visualizing the Generated 3D Mesh
# -----------------------------------------------------------------------------
# Use the Easy3D Viewer to visualize the generated 3D mesh.
# The viewer allows interaction with the model (e.g., rotate, zoom, pan).

# Create an instance of the Viewer with a custom title.
viewer = easy3d.Viewer("Easy3D Viewer - Text mesher")
viewer.set_usage("")
# Adjust the camera view to look at the text from the front.
viewer.camera().setViewDirection(easy3d.vec3(0, 0, -1))  # View direction.
viewer.camera().setUpVector(easy3d.vec3(0, 1, 0))        # Camera's up vector.
# Add the generated mesh to the viewer for visualization.
viewer.add_model(mesh)
# Launch the viewer to display the 3D text model.
viewer.run()

# -----------------------------------------------------------------------------
# Notes:
# 1. Ensure the font file exists at the specified path.
# 2. Modify the text, font size, or extrusion depth to experiment with different results.
# -----------------------------------------------------------------------------
