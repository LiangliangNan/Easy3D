import sys
sys.path.append("../../cmake-build-release/lib")
# sys.path.append("/Users/lnan/Documents/Projects/PyEasy3D/cmake-build-release")

import PyEasy3D

# Initialize Easy3D
PyEasy3D.initialize(True) # True to print detailed log

file_name = PyEasy3D.directory() + "/data/bunny.bin"
print("input file name: " + file_name)

stopwatch = PyEasy3D.StopWatch()
stopwatch.start()

# Load the point cloud from a file
pc = PyEasy3D.PointCloudIO.load(file_name)
if not pc:
    raise ValueError("Failed to load the point cloud.")

# Estimate normals for the point cloud
if not PyEasy3D.PointCloudNormals.estimate(pc, 16, False):
    raise ValueError("Normal estimation failed.")

# Optional: Reorient normals if needed
if not PyEasy3D.PointCloudNormals.reorient(pc, 16):
    raise ValueError("Normal reorientation failed.")

print("Done. Time: " + stopwatch.time_string())

# # Visualize the point cloud
# ToDo: python binding for the viewer is not ready
#       The viewer crashes when closing it (due to the ownership of pc?)
#       Solution: use std::shared_ptr in Viewer::add_model()
viewer = PyEasy3D.Viewer("Easy3D Viewer")
viewer.set_usage("")
# print(pc.name())
viewer.add_model(pc)
# viewer.add_model(file_name)
viewer.run()

# Save the point cloud into a file
if not PyEasy3D.PointCloudIO.save(file_name + "-result.ply", pc):
    raise ValueError("Saving file failed.")