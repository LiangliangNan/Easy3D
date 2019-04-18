#version 150

in  vec3 vtx_position;	// vertex position

uniform mat4 MVP;

void main() {
        gl_Position = MVP * vec4(vtx_position, 1.0);
}
