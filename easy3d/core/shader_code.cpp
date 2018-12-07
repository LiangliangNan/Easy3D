#include "shader_code.h"


namespace easy3d {

	namespace shadercode {


		std::string points_color_frag = R"(
			#version 150			
			uniform vec3	wLightPos;
			uniform vec3	wCamPos;
			uniform vec3	ambient = vec3(0.05f, 0.05f, 0.05f);	
			uniform vec3	specular = vec3(0.4f, 0.4f, 0.4f);
			uniform float	shininess = 64.0f;
			uniform bool	lighting = true;			
			in  vec3 vsPosition;
			in  vec3 vsColor;
			in  vec3 vsNormal;
			out vec4 outputF;
			void main(void) {
				vec3 view_dir = normalize(wCamPos - vsPosition);	// compute view direction and normalize it 
				vec3 normal = vsNormal;
				vec3 color = vsColor;
				if (lighting) {
					vec3 light_dir = normalize(wLightPos);
					float df = max(dot(light_dir, normal), 0);
					float sf = 0.0;	// specular factor
					if (df > 0.0) {	// if the vertex is lit compute the specular color
						vec3 half_vector = normalize(light_dir + view_dir);		// compute the half vector
						sf = max(dot(half_vector, normal), 0.0);
						sf = pow(sf, shininess);
					}
					color = color * df + specular * sf + ambient;
				}
				else 
					color = color + ambient;
				outputF = vec4(color, 1.0f);
			})";


		std::string points_color_vert = R"(
			#version 150			
			in  vec3 vtx_position;	// point position
			in  vec3 vtx_color;		// point color
			in	vec3 vtx_normal;	// point normal
			uniform mat4	MVP;
			uniform vec3	default_color = vec3(0.0f, 1.0f, 0.0f);
			uniform bool	per_vertex_color = false;
			// the data to be sent to the fragment shader
			out vec3 vsPosition;
			out vec3 vsColor;
			out vec3 vsNormal;
			void main(void) {			
				vsPosition = vtx_position;
				vsNormal = vtx_normal;
				vsColor = default_color;
				if (per_vertex_color)
					vsColor = vtx_color;
				gl_Position = MVP * vec4(vtx_position, 1.0);
			}
			)";


		std::string lines_color_frag = R"(
			#version 150
			in  vec3 vOutColor;
			out vec4 outputF;
			void main()
			{
				outputF = vec4(vOutColor, 1.0);
			})";


		std::string lines_color_vert = R"(
			#version 150
			in  vec3 vtx_position;	// point position
			in  vec3 vtx_color;		// point color
			uniform mat4	MVP;
			uniform vec3	default_color = vec3(0.0f, 0.0f, 0.0f);
			uniform bool	per_vertex_color = false;
			out vec3 vOutColor;
			void main()
			{
				gl_Position = MVP *  vec4(vtx_position, 1.0);

				if (per_vertex_color)
					vOutColor = vtx_color;
				else 
					vOutColor = default_color;
			})";


		std::string surface_color_frag = R"(
			#version 150		
			uniform vec3	wLightPos;
			uniform vec3	wCamPos;
			uniform vec3	ambient = vec3(0.05f, 0.05f, 0.05f);	
			uniform vec3	specular = vec3(0.4f, 0.4f, 0.4f);
			uniform float	shininess = 64.0f;
			uniform vec3	defaultColor = vec3(0.4f, 0.8f, 0.8f);
			in Data{
				vec3 color;
				vec3 position;
			} DataIn;

			out vec4 outputF;

			void main(void) {
				vec3 color = DataIn.color;
				vec3 normal = normalize(cross(dFdx(DataIn.position), dFdy(DataIn.position)));
				vec3 view_dir = normalize(wCamPos - DataIn.position);
				vec3 light_dir = normalize(wLightPos);
				float df = abs(dot(light_dir, normal));
				vec3 half_vector = normalize(light_dir + view_dir);	
				float sf = abs(dot(half_vector, normal));
				sf = pow(sf, shininess);
				outputF = vec4(color * df + specular * sf + ambient, 1.0);
			})";


		std::string surface_color_vert = R"(
			#version 150
			in  vec3 vtx_position;
			in  vec3 vtx_color;
			uniform mat4	MVP;
			uniform vec3	default_color = vec3(0.4f, 0.8f, 0.8f);
			uniform bool	per_vertex_color = false;
			out Data{
				vec3 color;
				vec3 position;
			} DataOut;
			void main() {
				if (per_vertex_color)	
					DataOut.color = vtx_color;
				else
					DataOut.color = default_color;
				DataOut.position = vtx_position;
				gl_Position = MVP *  vec4(vtx_position, 1.0);
			})";

	}

}
