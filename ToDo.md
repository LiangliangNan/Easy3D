* easy3d (on going):
	- data is too big (>100 M). Consider using the bunny for more tutorials;
	- in the draw() function of drawables, check if necessary buffers are valid;
	- have fewer functions like
			void update_array_buffer(Drawable::Buffer, const std::vector<vec3>& vertices);
			void update_element_buffer(...);
	  to replace
    		void update_vertex_buffer(const std::vector<vec3>& vertices);
            void update_color_buffer(const std::vector<vec3>& colors);
            void update_normal_buffer(const std::vector<vec3>& normals);
     		void update_texcoord_buffer(const std::vector<vec2>& texcoords);
    		void update_index_buffer(const std::vector<unsigned int>& indices);
      check VertexArrayObject and the AttribType in ShaderProgram. Maybe inherit Drawable from VAO?
	- current way handling high-dpi support is not optimal. Maybe always use framebuffer sizes?
	- add an application 'Mapple' showcasing the features in Easy3D;
	- use glog;
	- cross-section; able to be manipulated;
	- save/load camera/viewer states, and settings (default rendering parameters);
	- pick/select mechanism;
	-...
