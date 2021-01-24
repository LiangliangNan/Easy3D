#version 150

in  vec3 vtx_position;	// vertex position
in  vec3 vtx_normal;	// vertex normal
in  vec3 vtx_color;		// vertex color
in  vec2 vtx_texcoord;

uniform mat4 SHADOW;
uniform mat4 MVP;
uniform mat4 MANIP = mat4(1.0);
uniform mat3 NORMAL = mat3(1.0);

uniform bool planeClippingDiscard = false;
uniform bool clippingPlaneEnabled = false;
uniform bool crossSectionEnabled = false;
uniform vec4 clippingPlane0;
uniform vec4 clippingPlane1;

uniform vec3 default_color = vec3(0.3, 0.3, 0.7);
uniform bool per_vertex_color = false;

// the data to be sent to the fragment shader
out Data {
        vec2 texcoord;
        vec3 color;
        vec3 normal;
        vec3 position;
        vec4 shadowCoord;
        float clipped;
} DataOut;


void main() {
        vec4 new_position = MANIP * vec4(vtx_position, 1.0);

        DataOut.clipped = 0.0;
        if (clippingPlaneEnabled) {
                gl_ClipDistance[0] = dot(new_position, clippingPlane0);
                if (planeClippingDiscard && gl_ClipDistance[0] < 0)
                DataOut.clipped = 1.0;
                if (crossSectionEnabled) {
                        gl_ClipDistance[1] = dot(new_position, clippingPlane1);
                        if (planeClippingDiscard && gl_ClipDistance[1] < 0)
                        DataOut.clipped = 1.0;
                }
        }

        if (per_vertex_color)
                DataOut.color = vtx_color;
        else
                DataOut.color = default_color;

        DataOut.normal = NORMAL * vtx_normal;
        DataOut.position = new_position.xyz;
        DataOut.shadowCoord = SHADOW * new_position;
        DataOut.texcoord = vtx_texcoord;

        // the position of the vertex as seen from the current camera, in clip space
        gl_Position = MVP * new_position;
}
