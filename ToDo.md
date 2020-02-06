* easy3d (on going):
    - Rendering performance can still be improved
        * Drawables (e.g., faces, edges, and vertices) of the same model can share the same vertex buffer;
        * Use index buffer to reduce the num of vertices sent to GPU (already done).
    - Previous timer events may interrupt the current one when visualizing pivot points;
	- The current way handling high-dpi support is not optimal. Maybe always use framebuffer sizes?
	  This will ensure viewport[2] == camera()->screenWidth(), and viewport[3] == camera()->screenHeight():
	            int viewport[4];
                glGetIntegerv(GL_VIEWPORT, viewport);
	- Add an application 'Mapple' showcasing the features in Easy3D;
	- Cross-section; able to be manipulated;
	- Save/load camera/viewer states, and settings (e.g., default rendering parameters);
	- Pick/select mechanism (points, faces, objects); object manipulation;
	    * for OpenGL >= 4.3, use GPU;
	    * otherwise, use multi-thread
	-...