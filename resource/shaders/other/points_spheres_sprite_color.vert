//#version 150		// for point sprite to work
#version 430 core	// for shader storage buffer

// please send comments or report bug to: liangliang.nan@gmail.com
layout(std140) uniform Matrices {
	mat4 MV;
	mat4 invMV;
	mat4 PROJ;
	mat4 MVP;
	mat4 MANIP;
	mat3 NORMAL;
	mat4 SHADOW;
	bool clippingPlaneEnabled;
	bool crossSectionEnabled;
	vec4 clippingPlane0;
	vec4 clippingPlane1;
};

in vec3  vtx_position;
in vec3  vtx_color;
//in float sphere_radius;

uniform int		screen_width;   // scale to calculate size in pixels
uniform float	sphere_radius;
uniform vec3	default_color;
uniform bool	per_vertex_color;

uniform int		hightlight_id;
uniform bool	selected;

layout(std430, binding = 1) buffer selection_t {
	uint data[];
} selection;


out Data{
	vec4	position; // in eye space
	vec3	sphere_color;
	//float	sphere_radius;
} DataOut;


void main()
{
	vec4 new_position = MANIP * vec4(vtx_position, 1.0);

	vec3 color = default_color;
	if (per_vertex_color)
		color = vtx_color;

	//uint addr = gl_VertexID / 32;
	//uint offs = gl_VertexID % 32;
	//if ((selection.data[addr] & (1 << offs)) != 0)
	if ((selection.data[gl_VertexID >> 5] & (1 << (gl_VertexID & 31))) != 0)
		color = mix(color, vec3(1.0, 0.0, 0.0), 0.8);
	else if (hightlight_id == gl_VertexID)
		color = vec3(1.0, 0.0, 0.0);

	if (selected)
		color = vec3(1.0, 0, 0);

	DataOut.sphere_color = color;

	//DataOut.sphere_radius = sphere_radius;

	// Output vertex position
	DataOut.position = MV * new_position; // eye space

	// http://stackoverflow.com/questions/8608844/resizing-point-sprites-based-on-distance-from-the-camera
	vec4 projCorner = PROJ * vec4(sphere_radius, sphere_radius, DataOut.position.z, DataOut.position.w);
	gl_PointSize = screen_width * projCorner.x / projCorner.w;
	// you can also compute the point size using both screen width and height information.
	// http://stackoverflow.com/questions/17397724/point-sprites-for-particle-system
	//vec2 projSize = screen_size * projCorner.xy / projCorner.w;  // screen_size = vec2(screen_width, screen_height);
	//gl_PointSize = 0.5 * (projSize.x + projSize.y);

	gl_Position = PROJ * DataOut.position;

	if (clippingPlaneEnabled) {
		gl_ClipDistance[0] = dot(new_position, clippingPlane0);
		if (crossSectionEnabled)
			gl_ClipDistance[1] = dot(new_position, clippingPlane1);
	}
}