import sys
sys.path.append("../../cmake-build-release/lib/python")
# sys.path.append("/Users/lnan/Documents/Projects/PyEasy3D/cmake-build-release")

import easy3d

# Initialize Easy3D
easy3d.initialize(True) # True to print detailed log

file_name = easy3d.directory() + "/data/bunny.ply"

# Load the point cloud from a file
pc = easy3d.SurfaceMeshIO.load(file_name)

viewer = easy3d.Viewer("Easy3D Viewer")
viewer.set_usage("")
viewer.add_model(pc)
# viewer.add_model(file_name)
viewer.run()
