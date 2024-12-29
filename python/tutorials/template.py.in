# -----------------------------------------------------------------------------
# Tutorial: XXX with Easy3D
# -----------------------------------------------------------------------------
# In this tutorial, we will demonstrate how to apply XXX.
# The XXX algorithm is ... It is useful for XXX...
# We will also visualize the result using Easy3D's built-in visualization tools.
# -----------------------------------------------------------------------------

# If Easy3D is not installed via `pip`, you may need to add its Python bindings
# to the system path. For detailed instructions on building and installing
# Python bindings of Easy3D, visit:
# https://github.com/LiangliangNan/Easy3D/blob/main/ReadMe.md

import sys
sys.path.append("../../cmake-build-release/lib/python")

# Import Easy3D
import easy3d

# Setting `True` will provide detailed output for debugging purposes.
easy3d.initialize(False)

# -----------------------------------------------------------------------------
# Load a XXX model from file
# -----------------------------------------------------------------------------
# For this tutorial, let's use the XXX model.
input = easy3d.SurfaceMeshIO.load(easy3d.directory() + "/data/torusknot.obj")
# Print details about the original mesh.
# print(f"Original mesh has {input.n_vertices()} vertices and {input.n_faces()} faces.")

# -----------------------------------------------------------------------------
# Apply the XXX algorithm
# -----------------------------------------------------------------------------
# Let's make a copy of the input and apply the algorithm on the copied mesh.
output = easy3d.SurfaceMesh(input)
success = easy3d.XXXXXX.XXXX(output)

# -----------------------------------------------------------------------------
# Visualize the input and result side-by-side using the MultiViewer.
# -----------------------------------------------------------------------------

# Create a MultiViewer instance with 1 row and 2 columns.
viewer = easy3d.MultiViewer(1, 2, "Easy3D Viewer - XXX")

# Add the input model to the viewer and assign it to the first column.
viewer.add_model(input)
viewer.assign(0, 0, input)

# Add the output model to the viewer and assign it to the second column.
viewer.add_model(output)
viewer.assign(0, 1, output)

# Add instructions for the viewer (optional).
viewer.set_usage(
    "- Left: Input.\n"
    "- Right: Output."
)

# Launch the viewer
viewer.run()