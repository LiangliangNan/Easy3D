#version 150

in vec3 vtx_position;
in vec3 vtx_normal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 MV;
uniform mat3 invMV;
uniform mat4 PROJ;


void main()
{
    vec4 viewPos = MV * vec4(vtx_position, 1.0);
    FragPos = viewPos.xyz;
    
    Normal = invMV * vtx_normal;
    
    gl_Position = PROJ * viewPos;
}
