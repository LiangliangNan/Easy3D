// Shared per-draw layout, packed explicitly by the Vulkan backend.
layout(set=1,binding=0,std140) uniform MaterialState {
    mat4 model_view; mat4 projection; mat4 inverse_projection;
    vec4 dimensions; vec4 light; vec4 clip_planes[2]; vec4 clip_info;
    mat4 model; vec4 eye; vec4 material_light;
    vec4 ambient; vec4 specular; vec4 backside; vec4 shading;
} material;
vec4 shade_material(vec4 color,vec3 normal,vec3 position,bool back) {
    vec3 n=normalize(normal);
    vec3 view_dir=normalize(material.eye.xyz-position);
    vec3 light_dir=normalize(material.material_light.xyz);
    if (material.shading.z!=0 && back) color.rgb=material.backside.rgb;
    float df=dot(light_dir,n);
    df=material.shading.y!=0 ? abs(df) : max(df,0);
    float sf=0;
    if(df>0) {
        sf=dot(normalize(light_dir+view_dir),n);
        sf=material.shading.y!=0 ? abs(sf) : max(sf,0);
        sf=pow(sf,material.shading.w);
    }
    return vec4(color.rgb*df+material.specular.rgb*sf+material.ambient.rgb,color.a);
}
