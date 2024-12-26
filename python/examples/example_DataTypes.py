import sys
sys.path.append("../../cmake-build-release/lib/python")
import easy3d
import numpy as np


# Initialize Easy3D
easy3d.initialize(True) # True to print detailed log

p = easy3d.vec3(1, 2, 3)
print(p.x, p.y, p.z)

# Initialize PointCloud with points
points = np.array([[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]])
point_cloud = easy3d.PointCloud(points)

# Add more points
more_points = np.array([[7.0, 8.0, 9.0], [10.0, 11.0, 12.0]])
point_cloud.add_points(more_points)

# Export points to NumPy
exported_points = point_cloud.to_numpy()
print("Exported Points:", exported_points)