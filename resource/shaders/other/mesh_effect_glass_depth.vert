#version 150
// please send comments or report bug to: liangliang.nan@gmail.com

in vec3 vtx_position;
in vec3 vtx_normal;

uniform mat4 MV;
uniform mat4 MVP;
uniform mat3 NORMAL;

out vec3 vNormal;
out vec3 vPosition;


void main()
{
	vPosition = (MV * vec4(vtx_position, 1.0)).xyz;
	vNormal = NORMAL * vtx_normal;
	gl_Position = MVP * vec4(vtx_position, 1.0);
}

