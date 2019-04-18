#version 150

in  vec3 vtx_position;	// vertex position
in  vec3 vtx_normal;	// vertex normal
in  vec3 vtx_color;		// vertex color

uniform vec3 default_color;
uniform bool per_vertex_color;
uniform mat4 MVP;


// the data to be sent to the fragment shader
out Data{
	vec3 color;
	vec3 normal;
	vec3 position;
} DataOut;


void main(void)
{
	if (per_vertex_color)
		DataOut.color = vtx_color;
	else
		DataOut.color = default_color;

        DataOut.normal = vtx_normal;
        DataOut.position = vtx_position;

        gl_Position = MVP * vec4(vtx_position, 1.0);
}
