#version 150

in  vec3 vtx_position;	// point position
in  vec3 vtx_color;	// point color
in  vec3 vtx_normal;// point normal

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
