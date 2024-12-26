# -------------------------------------------------------------------------------
# Example: Offscreen Rendering
# -------------------------------------------------------------------------------
# This script demonstrates how to render 3D models into images without using a viewer.
# The model is rendered offscreen and saved as PNG images.
# The script includes steps for changing the camera position to get different views.
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
import easy3d  # Easy3D library for 3D visualization and processing

# -------------------------------------------------------------------------------
# Initializing Easy3D
# -------------------------------------------------------------------------------
# The `easy3d.initialize(False)` function initializes the Easy3D library.
# Use `True` to enable detailed logging, which is useful for debugging.
easy3d.initialize(False)

# -------------------------------------------------------------------------------
# Setting Up the Model and Renderer
# -------------------------------------------------------------------------------
# Define the input model file. In this case, a 3D model of a bunny in .ply format.
file_name = easy3d.directory() + "/data/bunny.ply"
print(f"Input file name: {file_name}")

# Create an offscreen renderer instance. This allows rendering without a viewer.
renderer = easy3d.OffScreen()

# Add the model to the renderer.
model = renderer.add_model(file_name)

# Render the model to a PNG file.
renderer.render(file_name + ".png")

# -------------------------------------------------------------------------------
# Modifying Camera Position
# -------------------------------------------------------------------------------
# Get the current camera of the renderer.
camera = renderer.camera()

# Move the camera position slightly by adding a small offset.
# This gives a different view of the model.
pos = camera.position() + easy3d.vec3(0.2, 0.2, 0.2)

# Set the new camera position.
camera.setPosition(pos)

# Render the model again with the new camera position.
renderer.render(file_name + "2.png")
