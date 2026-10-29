#version 150
// Fetch the existing position/index buffers and emit one instanced triangle quad
// per segment. This keeps width portable without wide lines or geometry shaders.
uniform samplerBuffer positions;
uniform usamplerBuffer indices;
uniform bool indexed;
uniform vec2 viewport;
uniform float line_width;
uniform float depth;
void main() {
    int first = gl_InstanceID*2;
    int i0 = indexed ? int(texelFetch(indices,first).r) : first;
    int i1 = indexed ? int(texelFetch(indices,first+1).r) : first+1;
    vec2 a = texelFetch(positions,i0).xy;
    vec2 b = texelFetch(positions,i1).xy;
    vec2 delta = (b-a)*viewport*0.5;
    float segment_length = length(delta);
    vec2 offset = segment_length>0 ? vec2(-delta.y,delta.x)/segment_length*line_width/viewport : vec2(0);
    const int ends[6] = int[6](0,1,1,0,1,0);
    const float sides[6] = float[6](-1,-1,1,-1,1,1);
    vec2 p = ends[gl_VertexID]==0 ? a : b;
    gl_Position = vec4(p+sides[gl_VertexID]*offset,depth,1);
}
