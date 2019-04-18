#version 430
// please send comments or report bug to: liangliang.nan@gmail.com

in vec3 vtx_position;
in vec2 vtx_texcoord;
in vec3 vtx_normal;

layout(std140) uniform Matrices {
	mat4 MV;
	mat4 invMV;
	mat4 PROJ;
	mat4 MVP;
	mat4 MANIP;
	mat3 NORMAL;
	mat4 SHADOW;
	bool cs_enabled;
	bool cs_thin;
	vec4 cs_plane0;
	vec4 cs_plane1;
};

// the data to be sent to the fragment shader
out Data{
	vec3 position;
	vec2 texcoord;
	vec3 normal;
	vec4 shadowCoord;
} DataOut;


void main() {

	vec4 new_position = MANIP * vec4(vtx_position, 1.0);

	DataOut.texcoord = vtx_texcoord;
	DataOut.normal = NORMAL * vtx_normal;
	DataOut.shadowCoord = SHADOW * new_position;

	// Output position of the vertex, in clip space : MVP * position
	gl_Position = MVP * new_position;

	if (cs_enabled) {
		gl_ClipDistance[0] = dot(new_position, cs_plane0);
		if (cs_thin)
			gl_ClipDistance[1] = dot(new_position, cs_plane1);
	}
}
