#version 150
// One invocation owns a packed selection word, so no vertex atomics/SSBOs are
// required on OpenGL 3.2 (including macOS). Positions alias the rendering VBO.
in uint old_word;
out uint new_word;
uniform samplerBuffer positions, polygon;
uniform usamplerBuffer active_words;
uniform mat4 mvp;
uniform vec4 planes[2];
uniform int plane_count, polygon_count, source_count;
uniform vec2 dimensions;
uniform bool deselect;
bool contains(vec2 p) {
    bool inside=false;
    for(int i=0,j=polygon_count-1;i<polygon_count;j=i++) {
        vec2 a=texelFetch(polygon,j).xy,b=texelFetch(polygon,i).xy,edge=b-a,delta=p-a;
        float length2=dot(edge,edge);
        if(length2>0) {
            float t=clamp(dot(delta,edge)/length2,0,1);
            vec2 d=delta-t*edge;
            if(dot(d,d)<=1e-8) return true;
        }
        if((a.y>p.y)!=(b.y>p.y) && p.x<a.x+(p.y-a.y)*(b.x-a.x)/(b.y-a.y)) inside=!inside;
    }
    return inside;
}
void main() {
    new_word=old_word;
    uint active_bits=texelFetch(active_words,gl_VertexID).r;
    for(int bit=0;bit<32;++bit) {
        int id=gl_VertexID*32+bit;uint mask=1u<<uint(bit);
        if(id>=source_count || (active_bits&mask)==0u) continue;
        vec4 local=vec4(texelFetch(positions,id*3).r,texelFetch(positions,id*3+1).r,texelFetch(positions,id*3+2).r,1);
        vec4 clip=mvp*local;
        if(any(isnan(clip)) || any(isinf(clip)) || clip.w<=0 ||
           any(lessThan(clip.xyz,vec3(-clip.w))) || any(greaterThan(clip.xyz,vec3(clip.w)))) continue;
        bool clipped=false;
        for(int i=0;i<plane_count;++i) clipped=clipped || dot(planes[i],local)<0;
        if(!clipped && contains(vec2((clip.x/clip.w+1)*.5*dimensions.x,(1-clip.y/clip.w)*.5*dimensions.y)))
            new_word=deselect?new_word&~mask:new_word|mask;
    }
    gl_Position=vec4(0);
}
