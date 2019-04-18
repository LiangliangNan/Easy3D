#version 150

uniform bool smooth_shading;

in vec3 FragPos;
in vec3 Normal;

out vec3 gPosition;
out vec3 gNormal;

void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;
    // also store the per-fragment normals into the gbuffer
	if (smooth_shading)
		gNormal = normalize(Normal);
	else {
		gNormal = normalize(cross(dFdx(FragPos), dFdy(FragPos)));
		if (dot(gNormal, Normal) < 0)
			gNormal = -gNormal;
	}

	// to handle two sided lighting
	if (dot(gNormal, vec3(0, 0, 1)) < 0)
		gNormal = -gNormal;
}