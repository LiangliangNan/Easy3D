import sys
sys.path.append("../cmake-build-release/")
# sys.path.append("/Users/lnan/Documents/Projects/PyEasy3D/cmake-build-release")

import easy3d

# Initialize Easy3D
easy3d.initialize(True) # True to print detailed log

file_name = easy3d.directory() + "/data/bunny.bin"
print("input file name: " + file_name)

stopwatch = easy3d.StopWatch()
stopwatch.start()

# Load the point cloud from a file
pc = easy3d.PointCloudIO.load(file_name)
if not pc:
    raise ValueError("Failed to load the point cloud.")

# Estimate normals for the point cloud
if not easy3d.PointCloudNormals.estimate(pc, 16, False):
    raise ValueError("Normal estimation failed.")

# Optional: Reorient normals if needed
if not easy3d.PointCloudNormals.reorient(pc, 16):
    raise ValueError("Normal reorientation failed.")

print("Done. Time: " + stopwatch.time_string())

# # Visualize the point cloud
# ToDo: python binding for the viewer is not ready
#       The viewer crashes when closing it (due to the ownership of pc?)
#       Solution: use std::shared_ptr in Viewer::add_model()
viewer = easy3d.Viewer("Easy3D Viewer")
viewer.set_usage("")
# print(pc.name())
viewer.add_model(pc)
# viewer.add_model(file_name)
viewer.run()

# Save the point cloud into a file
if not easy3d.PointCloudIO.save(file_name + "-result.ply", pc):
    raise ValueError("Saving file failed.")