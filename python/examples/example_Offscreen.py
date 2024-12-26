import sys
sys.path.append("../../cmake-build-release/lib/python")
import easy3d


# Initialize Easy3D
easy3d.initialize(True) # True to print detailed log

file_name = easy3d.directory() + "/data/bunny.ply"
print("input file name: " + file_name)

renderer = easy3d.OffScreen()
model = renderer.add_model(file_name)
renderer.render(file_name + ".png")

camera = renderer.camera()
pos = camera.position()
pos = easy3d.Vec_3UL_float_t(pos.x + 0.1, pos.y + 0.2, pos.z + 0.2)
camera.setPosition(pos)
renderer.render(file_name + "2.png")
print("done")

# ToDo: fix the crash due to reference valued in add_model()
#       Solution: use std::shared_ptr in add_model()