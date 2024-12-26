import sys
sys.path.append("../../cmake-build-release/lib/python")


import easy3d

easy3d.initialize()
viewer = easy3d.Viewer("Test")
viewer.add_model(easy3d.directory() + "/data/bunny.ply")
viewer.run()