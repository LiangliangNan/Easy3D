* easy3d (on going):
    - Add a Font class that handles fonts
            * use it in TextRenderer
            * use it in TextMesher
    - Add tutorials for algorithms: 
            * Delaunay2D, 
            * simplification
            * subdivision
            * smoothing
            * hole filling
            * curvature
            * parameterization
            * CSG operations
    - Support tetrahedron formats (e.g., *.mesh, *.meshb, *.tet); visualizing interiors and cross-sections;
    - Transparency on macOS with AMD graphics has artifact along the edges (an issue with dFdx/dFdy in the fragment shader). 
      An workaround is to provide a per-face normal (instead of using the normal computed from dFdx/dFdy calls);
    - The Graph data structure is not ready, not tested yet;
    - Rendering performance can still be improved
        * Drawables (e.g., faces, edges, and vertices) of the same model can share the same vertex buffer;
    - Previous timer events may interrupt the current one when visualizing pivot points;
	- The current way handling high-dpi support is not optimal. Maybe always use framebuffer sizes?
	  This will ensure viewport[2] == camera()->screenWidth(), and viewport[3] == camera()->screenHeight():
	            int viewport[4];
                glGetIntegerv(GL_VIEWPORT, viewport);
	- Save/load camera/viewer states, and settings (e.g., default rendering parameters);
	- Walkthrough and animation; allow to modify the cameras interactively;
	- Pick/select mechanism (points, faces, objects); object manipulation;
	    * for OpenGL >= 4.3, use GPU;
	    * otherwise, use multi-thread;
	- Visualize the labels of vertices/edges/faces/halfedges (useful for debugging);
	    
Long term:
    - switch to Vulkan.
    
    
Text to 3D models
    - http://www.mxs.de/textstl/