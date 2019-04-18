#version 150		

// please send comments or report bug to: liangliang.nan@gmail.com

in vec3  vtx_position;

uniform mat4	MV;
uniform mat4	PROJ;
uniform mat4	MANIP;

uniform int		screen_width;   // scale to calculate size in pixels
uniform float	sphere_radius;


out vec4	position; // in eye space

void main()
{
	position = MV * MANIP * vec4(vtx_position, 1.0);

	// http://stackoverflow.com/questions/8608844/resizing-point-sprites-based-on-distance-from-the-camera
	vec4 projCorner = PROJ * vec4(sphere_radius, sphere_radius, position.z, position.w);
	gl_PointSize = screen_width * projCorner.x / projCorner.w;

	gl_Position = PROJ * position;
}