#version 150

in Data{
    vec3 position;
    vec3 normal;
} DataIn;

// smooth shading
uniform bool        smooth_shading = true;

out vec3 gPosition;
out vec3 gNormal;

void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPosition = DataIn.position;

    vec3 normal;
    if (smooth_shading)
        gNormal = normalize(DataIn.normal);
    else {
        gNormal = normalize(cross(dFdx(DataIn.position), dFdy(DataIn.position)));
        if (dot(gNormal, DataIn.normal) < 0)
            gNormal = -gNormal;
    }

    // to handle two sided lighting
    if (dot(gNormal, vec3(0, 0, 1)) < 0)
        gNormal = -gNormal;
}
