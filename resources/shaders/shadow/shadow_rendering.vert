#version 150

in  vec3 vtx_position;	// vertex position
in  vec3 vtx_normal;	// vertex normal
in  vec3 vtx_color;		// vertex color

uniform mat4 MVP;
uniform mat4 SHADOW;

uniform vec3 default_color = vec3(0.3, 0.3, 0.7);
uniform bool per_vertex_color = false;

// the data to be sent to the fragment shader
out Data {
	vec3 color;
	vec3 normal;
	vec3 position;
	vec4 shadowCoord;
} DataOut;


void main() {
	if (per_vertex_color)	
		DataOut.color = vtx_color;
	else
		DataOut.color = default_color;
	
        DataOut.normal = vtx_normal;
        DataOut.position = vtx_position;
        DataOut.shadowCoord = SHADOW * vec4(vtx_position, 1.0);

        // the position of the vertex as seen from the current camera, in clip space
        gl_Position = MVP * vec4(vtx_position, 1.0);
}
