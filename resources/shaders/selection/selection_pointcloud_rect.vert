#version 430
// please send comments or report bug to: liangliang.nan@gmail.com

in  vec3		vtx_position;	// vertex position

uniform mat4	MVP;		// model-view-projection matrix
uniform ivec4	viewport;		// view port
uniform vec4	rect;			// the rectangle region
uniform bool	deselect;		// select or deselect

layout(std430, binding = 1) buffer selection_t {
	uint data[];
} selection;


void main()
{
	vec4 p = MVP * vec4(vtx_position, 1.0);
	float x = p.x / p.w * 0.5 + 0.5;
	float y = p.y / p.w * 0.5 + 0.5;
	x = x * viewport[2] + viewport[0];
	y = y * viewport[3] + viewport[1];
	y = viewport[3] - 1 - y;
	if (x >= rect[0] && x <= rect[1] && y >= rect[2] && y <= rect[3]) 
	{
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
