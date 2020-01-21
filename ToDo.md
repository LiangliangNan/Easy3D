* easy3d (on going):
    - Modify SurfaceMesh to allow non-manifold meshes;
    - Previous timer events may interrupt the current one when visualizing pivot points;
	- The current way handling high-dpi support is not optimal. Maybe always use framebuffer sizes?
	- Add an application 'Mapple' showcasing the features in Easy3D;
	- Use glog;
	- Cross-section; able to be manipulated;
	- Save/load camera/viewer states, and settings (default rendering parameters);
	- Pick/select mechanism;
	- For drawables, have fewer functions (instead a function for each buffer) like
    			void update_array_buffer(Drawable::Buffer, const std::vector<vec3>& vertices);
    			void update_element_buffer(...);
          Check VertexArrayObject and the AttribType in ShaderProgram. Maybe inherit Drawable from VAO?
	-...
