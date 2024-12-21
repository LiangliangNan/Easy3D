import sys
sys.path.append("../cmake-build-release/")

import easy3d_core
import easy3d_algo
import easy3d_fileio
import easy3d_util
import easy3d_viewer

# Initialize Easy3D
easy3d_util.initialize(True)

file_name = easy3d_util.resource.directory() + "/data/bunny.bin"
print("input file name: " + file_name)

# Load the point cloud from a file
pc = easy3d_fileio.PointCloudIO.load(file_name)
if not pc:
    raise ValueError("Failed to load the point cloud.")

# Estimate normals for the point cloud
if not easy3d_algo.PointCloudNormals.estimate(pc, 16, False):
    raise ValueError("Normal estimation failed.")

# Optional: Reorient normals if needed
if not easy3d_algo.PointCloudNormals.reorient(pc, 16):
    raise ValueError("Normal reorientation failed.")

# # Visualize the point cloud
# ToDo: python binding for the viewer is not ready
#       The viewer crashes when closing it (due to the ownership of pc?)
viewer = easy3d_viewer.Viewer("Easy3D Viewer")
viewer.set_usage("")
# print(pc.name())
viewer.add_model(pc)
viewer.run()

# Save the point cloud into a file
if not easy3d_fileio.PointCloudIO.save(file_name + "-result.ply", pc):
    raise ValueError("Saving file failed.")