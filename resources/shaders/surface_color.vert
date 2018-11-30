#version 330
// please send comments or report bug to: liangliang.nan@gmail.com

in  vec3 vtx_position;	// vertex position

uniform mat4 MVP;

out vec3 position;

void main() {
	position = vtx_position;
	gl_Position = MVP * vec4(vtx_position, 1.0);
}
