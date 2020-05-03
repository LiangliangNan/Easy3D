* easy3d (on going):
    - Add tutorials for algorithms: Delaunay2D, simplification, subdivision, smoothing, hole filling, curvatures, parameterization.
    - Add a progress bar to Mapple;
    - Test if the hash function in tessellator.cpp works on both 64-bit and 32-bit machines:
                  inline void hash_combine(std::size_t &seed, T const &v) const {
                      std::hash<T> hasher;
                      seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                  }
    - Support tetrahedron formats (e.g., *.mesh, *.meshb, *.tet); visualizing interiors and cross-sections;
    - Transparency on macOS with AMD graphics has artifact along the edges (an issue with dFdx/dFdy in the fragment shader). 
      An workaround is to provide a per-face normal (instead of using the normal computed from dFdx/dFdy calls);
    - The Graph data structure is not ready, not tested yet;
    - Module dependence can be improved (core <- viewer; viewer <- core);  The IO examples should not depend on viewer.
    - Rendering performance can still be improved
        * Drawables (e.g., faces, edges, and vertices) of the same model can share the same vertex buffer;
        * Use index buffer to reduce the num of vertices sent to GPU (already done).
    - Previous timer events may interrupt the current one when visualizing pivot points;
	- The current way handling high-dpi support is not optimal. Maybe always use framebuffer sizes?
	  This will ensure viewport[2] == camera()->screenWidth(), and viewport[3] == camera()->screenHeight():
	            int viewport[4];
                glGetIntegerv(GL_VIEWPORT, viewport);
	- Save/load camera/viewer states, and settings (e.g., default rendering parameters);
	- Walkthrough and animation;
	- Pick/select mechanism (points, faces, objects); object manipulation;
	    * for OpenGL >= 4.3, use GPU;
	    * otherwise, use multi-thread
	    
Longer term:
    - switch to Vulkan.
    
    
    
Text to 3D models
    - http://www.mxs.de/textstl/