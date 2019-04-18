#version 430
// please send comments or report bug to: liangliang.nan@gmail.com

in  vec3		vtx_position;	// vertex position

uniform mat4	MVP_MANIP;
uniform ivec4	viewport;
uniform bool	deselect;


layout(std430, binding = 0) buffer polygon_t {
	vec2 data[];
} polygon;


layout(std430, binding = 1) buffer selection_t {
	uint data[];
} selection;



bool point_in_polygon(vec2 p)
{
	bool inside = false;
	uint n = polygon.data.length();
	for (uint i = 0, j = n - 1; i < n; j = i, ++i) {
		vec2 u0 = polygon.data[i];
		vec2 u1 = polygon.data[j];  // current edge

		if (((u0.y <= p.y) && (p.y < u1.y)) ||  // U1 is above the ray, U0 is on or below the ray
			((u1.y <= p.y) && (p.y < u0.y)))    // U0 is above the ray, U1 is on or below the ray
		{
			// find x-intersection of current edge with the ray. 
			// Only consider edge crossings on the ray to the right of P.
			float x = u0.x + (p.y - u0.y) * (u1.x - u0.x) / (u1.y - u0.y);
			if (x > p.x)
				inside = !inside;
		}
	}

	return inside;
}


void main()
{
	vec4 p = MVP_MANIP * vec4(vtx_position, 1.0);
	float x = p.x / p.w * 0.5 + 0.5;
	float y = p.y / p.w * 0.5 + 0.5;
	x = x * viewport[2] + viewport[0];
	y = y * viewport[3] + viewport[1];

	vec2 v = vec2(x, viewport[3] - 1 - y);
	if (point_in_polygon(v)) {
		//int addr = gl_VertexID / 32;
		//int offs = gl_VertexID % 32;
		uint addr = gl_VertexID >> 5;
		uint offs = gl_VertexID & 31;
		if (deselect) {
			atomicAnd(selection.data[addr], ~(1 << offs));
			//selection.data[addr] &= ~(1 << offs);
		}
		else {
			atomicOr(selection.data[addr], (1 << offs));
			//selection.data[addr] |= (1 << offs);
		}
	}

	gl_Position = p;
}
