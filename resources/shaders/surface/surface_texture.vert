#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

in vec3 vtx_position;
in vec2 vtx_texcoord;
in vec3 vtx_normal;

uniform mat4 MVP;

out Data{
	vec2 texcoord;
	vec3 normal;
	vec3 position;
} DataOut;

void main() {

	DataOut.texcoord = vtx_texcoord;
        DataOut.normal = vtx_normal;
        DataOut.position = vtx_position;

        gl_Position = MVP * vec4(vtx_position, 1.0);
}
