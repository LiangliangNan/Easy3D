#version 150

in vec3 vtx_position;
in vec3 vtx_normal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 MV;
uniform mat4 PROJ;
uniform mat4 MANIP;
uniform mat3 NORMAL;

uniform bool clippingPlaneEnabled;
uniform bool crossSectionEnabled;
uniform vec4 clippingPlane0;
uniform vec4 clippingPlane1;

void main()
{
	vec4 new_position = MANIP * vec4(vtx_position, 1.0);

    vec4 viewPos = MV * new_position;
    FragPos = viewPos.xyz; 
    
    Normal = NORMAL * vtx_normal;
    
    gl_Position = PROJ * viewPos;

	if (clippingPlaneEnabled) {
		gl_ClipDistance[0] = dot(new_position, clippingPlane0);
		if (crossSectionEnabled)
			gl_ClipDistance[1] = dot(new_position, clippingPlane1);
	}
}