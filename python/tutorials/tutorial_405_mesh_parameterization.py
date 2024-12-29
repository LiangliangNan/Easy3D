# -----------------------------------------------------------------------------
# Tutorial: Mesh Parameterization Using Easy3D
# -----------------------------------------------------------------------------
# This tutorial demonstrates how to:
# 1. Perform surface mesh parameterization using the `SurfaceMeshParameterization`
#   class of Easy3D.
# 2. Visualize the resulting parameterized meshes with texture mapping.
# -----------------------------------------------------------------------------
# Mesh parameterization is the process of mapping a 3D surface mesh onto a 2D plane
# while preserving certain geometric properties such as angles, areas, or overall
# shape. It is a crucial technique in computer graphics, particularly for texture
# mapping, mesh analysis, and surface manipulation.
#
# The `SurfaceMeshParameterization` class in Easy3D implements two widely used
# mesh parameterization algorithms:
#
#  - **Discrete Harmonic Parameterization**: This method seeks to map the 3D mesh
#    onto a 2D plane by solving harmonic functions that minimize geometric distortion.
#    It ensures that the mesh's boundaries are respected and that the surface's
#    properties (like curvature) are preserved during the mapping.
#
#  - **Least-Squares Conformal Mapping (LSCM)**: This algorithm minimizes angular
#    distortion by applying conformal mapping techniques. The goal is to preserve
#    local angles and minimize stretching while flattening the 3D surface into a
#    2D plane. LSCM is often used when creating texture maps, as it reduces texture
#    distortion in the parameterized space.
# -----------------------------------------------------------------------------


# If Easy3D is not installed via `pip`, you may need to add its Python bindings
# to the system path. For detailed instructions on building and installing
# Python bindings of Easy3D, visit:
# https://github.com/LiangliangNan/Easy3D/blob/main/ReadMe.md

import sys
sys.path.append("../../cmake-build-release/lib/python")

# Import and initialize Easy3D
import easy3d
easy3d.initialize(False)


# In this example, we load a 3D surface mesh file of a girl's face.
mesh_harmonic = easy3d.SurfaceMeshIO.load(easy3d.directory() + "/data/girl_face.ply")

# -----------------------------------------------------------------------------
# Prepare Mesh for LSCM Parameterization
# -----------------------------------------------------------------------------
# LSCM algorithm works only with triangle meshes, so we need to triangulate the mesh
# before applying this method. Let's make a copy of the mesh for LSCM processing.

mesh_lscm = easy3d.SurfaceMesh(mesh_harmonic)
mesh_lscm.triangulate()  # Triangulate the mesh to make it compatible with LSCM.

# -----------------------------------------------------------------------------
# Apply Mesh Parameterization
# -----------------------------------------------------------------------------
# Now, we apply the two parameterization algorithms to the meshes.

# Apply the Discrete Harmonic Parameterization
harmonic_method = easy3d.SurfaceMeshParameterization(mesh_harmonic)
harmonic_method.harmonic()

# Apply the Least-Squares Conformal Mapping (LSCM)
lscm_method = easy3d.SurfaceMeshParameterization(mesh_lscm)
lscm_method.lscm()

# -----------------------------------------------------------------------------
# Visualize the Results
# -----------------------------------------------------------------------------
# Use Easy3D's `MultiViewer` class to visualize the two results side-by-side.

# Create a MultiViewer instance with 1 row and 2 columns.
viewer = easy3d.MultiViewer(1, 2, "Easy3D Viewer - Mesh parameterization")

# Add the result of the Harmonic parameterization to the left view.
viewer.add_model(mesh_harmonic)
# Assign it to the left view (row=0, column=0).
viewer.assign(0, 0, mesh_harmonic)

# Add the result of the LSCM parameterization to the viewer.
viewer.add_model(mesh_lscm)
# Assign it to the right view (row=0, column=1).
viewer.assign(0, 1, mesh_lscm)

# Set the camera view direction and up vector for a better view.
viewer.camera().setViewDirection(easy3d.vec3(0, 0, 1))   # Set the view direction.
viewer.camera().setUpVector(easy3d.vec3(0, 1, 0))        # Set the up vector.

# Add instructions for the viewer (optional).
viewer.set_usage(
    "- Left: Result of Discrete Harmonic Parameterization.\n"
    "- Right: Result of Least-Squares Conformal Mapping (LSCM)."
)

# Launch the viewer
viewer.run()
