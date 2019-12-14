#version 330 core
// please send comments or report bug to: liangliang.nan@gmail.com

//#extension GL_EXT_geometry_shader4 : enable

layout(points) in;
layout(triangle_strip, max_vertices=4) out;


uniform mat4    MV;
uniform mat4    PROJ;
uniform float	sphere_radius;

in vec3	sphere_color_in[];
//in float	sphere_radius_in;

out Data {
	flat	vec3	sphere_color;
	smooth	vec2	tex;
	flat	vec4	position;
	//flat	float	sphere_radius;
} DataOut;


void main()
{
  // Output vertex position
  DataOut.position = MV * gl_in[0].gl_Position; 
  DataOut.sphere_color = sphere_color_in[0];
  //sphere_radius = sphere_radius_in;
    
  // Vertex 1
  DataOut.tex = vec2(-1.0, -1.0);
  gl_Position = DataOut.position;
  gl_Position.xy += vec2(-sphere_radius, -sphere_radius);
  gl_Position = PROJ  * gl_Position;
  EmitVertex();

  // Vertex 2
  DataOut.tex = vec2(-1.0, 1.0);
  gl_Position = DataOut.position;
  gl_Position.xy += vec2(-sphere_radius, sphere_radius);
  gl_Position = PROJ  * gl_Position;
  EmitVertex();

  // Vertex 3
  DataOut.tex = vec2(1.0, -1.0);
  gl_Position = DataOut.position;
  gl_Position.xy += vec2(sphere_radius, -sphere_radius);
  gl_Position = PROJ  * gl_Position;
  EmitVertex();

  // Vertex 4
  DataOut.tex = vec2(1.0, 1.0);
  gl_Position = DataOut.position;
  gl_Position.xy += vec2(sphere_radius, sphere_radius);
  gl_Position = PROJ  * gl_Position;
  EmitVertex();

  EndPrimitive();
}
