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
    - Visualizing interiors and cross-sections of tetrahedron models ;
    - Transparency on macOS with AMD graphics has artifact along the edges (an issue with dFdx/dFdy in the fragment shader). 
      An workaround is to provide a per-face normal (instead of using the normal computed from dFdx/dFdy calls);
    - The Graph data structure is not ready, not tested yet;
    - Previous timer events may interrupt the current one when visualizing pivot points;
	- The current way handling high-dpi support is not optimal. Maybe always use framebuffer sizes?
	  This will ensure viewport[2] == camera()->screenWidth(), and viewport[3] == camera()->screenHeight():
	            int viewport[4];
                glGetIntegerv(GL_VIEWPORT, viewport);
	- Pick/select mechanism (points, faces, objects); object manipulation;
	    * for OpenGL >= 4.3, use GPU;
	    * otherwise, use multi-thread;
	- Visualize the labels of vertices/edges/faces/halfedges (useful for debugging);
	    
Long term:
    - switch to Vulkan.