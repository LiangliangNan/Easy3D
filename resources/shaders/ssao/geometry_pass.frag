#version 150

in vec3 FragPos;
in vec3 Normal;

out vec3 gPosition;
out vec3 gNormal;

void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;

    gNormal = normalize(Normal);

    // to handle two sided lighting
    if (dot(gNormal, vec3(0, 0, 1)) < 0)
        gNormal = -gNormal;
}
