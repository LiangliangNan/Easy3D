#version 150

in  vec3 vOutColor;
out vec4 outputF;

void main()
{
     outputF = vec4(vOutColor, 1.0);
}
