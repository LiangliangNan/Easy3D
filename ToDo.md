* easy3d (on going):
    - Add tutorials for algorithms:
        - Delaunay2D,
        - simplification
        - subdivision
        - smoothing
        - hole filling
        - curvature
        - parameterization
        - CSG operations
        - clipping plane
    - Save/load camera/viewer states, and settings (e.g., default rendering parameters);
    - Walkthrough and animation; allow to modify the cameras interactively: insert/remove/move cameras. Check CloudCompare.
    - Check if clipper can be used to handle overlapping faces.
    - Introduce HalfEdge to PolyMesh (like OpenVolumeMesh).
    - Generate LODs using [meshoptimizer](https://github.com/zeux/meshoptimizer); Check if it can serve as mesh 
      simplification, or if it can be used to merge close vertices.
    - Transparency on macOS with AMD graphics has artifact along the edges (an issue with dFdx/dFdy in the fragment shader). 
      An workaround is to provide a per-face normal (instead of using the normal computed from dFdx/dFdy calls);
    - Previous timer events may interrupt the current one when visualizing pivot points;
	- Pick/select mechanism (points, faces, objects); object manipulation;
	    * for OpenGL >= 4.3, use GPU;
	    * otherwise, use multi-thread;
	- Visualize the labels of vertices/edges/faces/halfedges (useful for debugging);
	    
Long term:
    - switch to Vulkan.