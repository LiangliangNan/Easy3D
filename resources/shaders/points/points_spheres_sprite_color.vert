#version 150		// for point sprite to work

// please send comments or report bug to: liangliang.nan@gmail.com
uniform mat4 MV;
uniform mat4 PROJ;

in vec3  vtx_position;
in vec3  vtx_color;
//in float sphere_radius;

uniform int	    screen_width;   // scale to calculate size in pixels
uniform float   sphere_radius;
uniform vec4    default_color;
uniform bool    per_vertex_color;

uniform bool clippingPlaneEnabled = false;
uniform bool crossSectionEnabled = false;
uniform vec4 clippingPlane0;
uniform vec4 clippingPlane1;

out Data {
    vec4    position;// in eye space
    vec4    sphere_color;
//float	sphere_radius;
} DataOut;


void main()
{
	if (per_vertex_color)
    DataOut.sphere_color = vec4(vtx_color, 1.0);
        else
            DataOut.sphere_color = default_color;
	//DataOut.sphere_radius = sphere_radius;

	// Output vertex position
        DataOut.position = MV * vec4(vtx_position, 1.0); // eye space

	// http://stackoverflow.com/questions/8608844/resizing-point-sprites-based-on-distance-from-the-camera
	vec4 projCorner = PROJ * vec4(sphere_radius, sphere_radius, DataOut.position.z, DataOut.position.w);
	gl_PointSize = screen_width * projCorner.x / projCorner.w;
	// you can also compute the point size using both screen width and height information.
	// http://stackoverflow.com/questions/17397724/point-sprites-for-particle-system
	//vec2 projSize = screen_size * projCorner.xy / projCorner.w;  // screen_size = vec2(screen_width, screen_height);
	//gl_PointSize = 0.5 * (projSize.x + projSize.y);

	gl_Position = PROJ * DataOut.position;
}
