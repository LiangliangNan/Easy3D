* ToDo list (or on going):
    - Walkthrough and animation; allow to modify the cameras interactively
    - Implement potree for huge point cloud visualization (https://github.com/potree/potree)
    - Add a measuring tool
    - Use https://github.com/onqtam/doctest for tests
    - Add contents and brief info for each model in WidgetModelList.
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
    - Check if clipper can be used to handle overlapping faces.
    - Generate LODs using [meshoptimizer](https://github.com/zeux/meshoptimizer).
    - Transparency on macOS with AMD graphics has artifact along the edges (an issue with dFdx/dFdy in the fragment shader). 
      An workaround is to provide a per-face normal (instead of using the normal computed from dFdx/dFdy calls);
    - Previous timer events may interrupt the current one when visualizing pivot points;
	- Pick/select mechanism (points, faces, objects); object manipulation;
	    * for OpenGL >= 4.3, use GPU;
	    * otherwise, use multi-thread;
	    
Long term:
    - support Vulkan.
    - Python bindings.