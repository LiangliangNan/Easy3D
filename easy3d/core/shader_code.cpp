#include "shader_code.h"


namespace easy3d {

	namespace shadercode {
		std::string points_color_frag = R"(
			#version 150			
			uniform vec3	wLightPos;
			uniform vec3	eLightPos;
			uniform vec3	wCamPos;
			uniform vec3	ambient;		
			uniform vec3	specular;		
			uniform float	shininess;
			uniform bool	lighting;		// true if lighting is on			
			in  vec3 vsPosition;
			in  vec3 vsColor;
			in  vec3 vsNormal;
			out vec4 outputF;
			void main(void) {
				vec3 view_dir = normalize(wCamPos - vsPosition);	// compute view direction and normalize it 
				vec3 normal = vsNormal;
				vec3 color = vsColor;
				if (lighting) {
					vec3 light_dir = normalize(wLightPos);
					float df = max(dot(light_dir, normal), 0);
					float sf = 0.0;	// specular factor
					if (df > 0.0) {	// if the vertex is lit compute the specular color
						vec3 half_vector = normalize(light_dir + view_dir);		// compute the half vector
						sf = max(dot(half_vector, normal), 0.0);
						sf = pow(sf, shininess);
					}
					color = color * df + specular * sf + ambient;
				}
				else 
					color = color + ambient;
				outputF = vec4(color, 1.0f);
			})";


		std::string points_color_vert = R"(
			#version 150			
			in  vec3 vtx_position;	// point position
			in  vec3 vtx_color;		// point color
			in	vec3 vtx_normal;	// point normal
			uniform mat4	MVP;
			uniform vec3	default_color;
			uniform bool	per_vertex_color;
			// the data to be sent to the fragment shader
			out vec3 vsPosition;
			out vec3 vsColor;
			out vec3 vsNormal;
			void main(void) {			
				vsPosition = vtx_position;
				vsNormal = vtx_normal;
				vsColor = default_color;
				if (per_vertex_color)
					vsColor = vtx_color;
				gl_Position = MVP * vec4(vtx_position, 1.0);
			}
			)";


		std::string lines_color_frag = R"(
#version 150
in  vec3 vOutColor;
out vec4 outputF;
void main()
{
	outputF = vec4(vOutColor, 1.0);
})";


		std::string lines_color_vert = R"(
#version 150
in  vec3 vtx_position;	// point position
in  vec3 vtx_color;		// point color
uniform mat4	MVP;
uniform bool	per_vertex_color;
uniform vec3	default_color;
out vec3 vOutColor;
void main()
{
	gl_Position = MVP *  vec4(vtx_position, 1.0);

	if (per_vertex_color)
		vOutColor = vtx_color;
	else 
		vOutColor = default_color;
})";


		std::string surface_color_frag = R"(
#version 150
uniform vec3	wLightPos;
uniform vec3	wCamPos;
uniform vec3	ambient;		
uniform vec3	specular;
uniform float	shininess;
uniform vec3	defaultColor;

in Data{
	vec3 color;
	vec3 position;
} DataIn;

out vec4 outputF;

void main(void) {
	vec3 color = DataIn.color;
	vec3 normal = normalize(cross(dFdx(DataIn.position), dFdy(DataIn.position)));
	vec3 view_dir = normalize(wCamPos - DataIn.position);
	vec3 light_dir = normalize(wLightPos);
	float df = abs(dot(light_dir, normal));
	vec3 half_vector = normalize(light_dir + view_dir);	
	float sf = abs(dot(half_vector, normal));
	sf = pow(sf, shininess);
	outputF = vec4(color * df + specular * sf + ambient, 1.0);
})";


		std::string surface_color_vert = R"(
#version 150
in  vec3 vtx_position;
in  vec3 vtx_color;
uniform mat4	MVP;
uniform bool	per_vertex_color;
uniform vec3	default_color;
out Data{
	vec3 color;
	vec3 position;
} DataOut;
void main() {
	if (per_vertex_color)	
		DataOut.color = vtx_color;
	else
		DataOut.color = default_color;
	DataOut.position = vtx_position;
	gl_Position = MVP *  vec4(vtx_position, 1.0);
})";

	}
}

//
//std::string average_color_final_frag = R"(
//#version 150
//
//in vec2 texCoord;
//
//uniform sampler2D ColorTex0;
//uniform sampler2D ColorTex1;
//uniform vec3 BackgroundColor;
//
//out vec4 fragOutput;
//
//void main(void)
//{
//	vec4 SumColor = texture(ColorTex0, texCoord);
//	float n = texture(ColorTex1, texCoord).r;
//
//	if (n == 0.0) {
//		fragOutput.rgb = BackgroundColor;
//		// Liangliang: I don't want transparent images
//		fragOutput.a = 1.0f;
//		return;
//	}
//
//	vec3 AvgColor = SumColor.rgb / SumColor.a;
//	float AvgAlpha = SumColor.a / n;
//
//	float T = pow(1.0-AvgAlpha, n);
//	fragOutput.rgb = AvgColor * (1 - T) + BackgroundColor * T;
//
//	// Liangliang: I don't want transparent images
//	fragOutput.a = 1.0f;
//}
//)";
//
//
//std::string average_color_final_vert = R"(
//#version 150
//
//
//in vec4 vtx_position;
//in vec2 vtx_texcoord;
//
//out vec2 texCoord;
//
//void main()
//{
//	gl_Position = vtx_position;
//	texCoord = vtx_texcoord;
//}
//)";
//
//
//
//
//std::string average_color_init_frag = R"(
//#version 150
//
//
//// Liangliang: it is weired that computing normals in shader results in artifacts along the 
////			   edges (Perhaps this effect comes from the peeling?).
//// Note: if you change here, you should also uncomment the following in the source code:
////		 "program->set_uniform("smooth_shading", smooth_shading);"
//#define USE_PROVIDED_NORMALS
//
//
//// the output of this shader
//out vec4 fragOutput0;
//out vec4 fragOutput1;
//
//uniform sampler2D ColorTex0;
//uniform sampler2D ColorTex1;
//
//#ifndef USE_PROVIDED_NORMALS
//uniform bool	smooth_shading;
//#endif // !USE_PROVIDED_NORMALS
//
//uniform float	Alpha;
//
//layout(std140) uniform Lighting {
//	vec3	wLightPos;
//	vec3	eLightPos;
//	vec3	wCamPos;
//	vec3	ambient;		// in [0, 1], r==g==b;
//	vec3	specular;		// in [0, 1], r==g==b;
//	float	shininess;
//};
//
//in Data{
//	vec3 color;
//	vec3 normal;
//	vec3 position;
//} DataIn;
//
//
//vec4 ShadeFragment()
//{
//	vec3 normal;
//#ifdef USE_PROVIDED_NORMALS
//	normal = normalize(DataIn.normal);
//#else  // compute normals using dFdx and dFdy
//	if (smooth_shading)
//		normal = normalize(DataIn.normal);
//	else {
//		normal = normalize(cross(dFdx(DataIn.position), dFdy(DataIn.position)));
//		if (dot(normal, DataIn.normal) < 0)
//			normal = -normal;
//	}
//#endif 
//
//	vec3 view_dir = normalize(wCamPos - DataIn.position);
//	vec3 light_dir = normalize(wLightPos);
//
//	// diffuse factor
//	float df = abs(dot(light_dir, normal));				// light up both sides
//
//	// specular factor
//	vec3 half_vector = normalize(light_dir + view_dir);	// compute the half vector
//	float sf = max(dot(half_vector, normal), 0.0);		// only the front side can have specular
//	sf = pow(sf, shininess);
//
//	return vec4(DataIn.color * df + specular * sf + ambient, Alpha);
//}
//
//void main(void)
//{
//	vec4 color = ShadeFragment();
//	fragOutput0 = vec4(color.rgb * color.a, color.a);
//	fragOutput1 = vec4(1.0);
//}
//)";
//
//
//std::string average_color_init_vert = R"(
//#version 150
//
//in  vec3 vtx_position;	// vertex position
//in	vec3 vtx_normal;	// vertex normal
//in  vec3 vtx_color;		// vertex color
//
//uniform vec3 default_color;
//uniform bool per_vertex_color;
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//// the data to be sent to the fragment shader
//out Data{
//	vec3 color;
//	vec3 normal;
//	vec3 position;
//} DataOut;
//
//
//void main(void)
//{
//	vec4 new_position = MANIP * vec4(vtx_position, 1.0);
//
//	if (per_vertex_color)
//		DataOut.color = vtx_color;
//	else
//		DataOut.color = default_color;
//
//	DataOut.normal = NORMAL * vtx_normal;
//	DataOut.position = new_position.xyz;
//
//	gl_Position = MVP * new_position;
//}
//)";
//
//
//
//
//std::string basic_lighting_frag = R"(
//#version 330                                                                        
//                                                                                    
//const int MAX_POINT_LIGHTS = 2;                                                     
//const int MAX_SPOT_LIGHTS = 2;                                                      
//                                                                                    
//in vec2 TexCoord0;                                                                  
//in vec3 Normal0;                                                                    
//in vec3 WorldPos0;                                                                  
//                                                                                    
//out vec4 FragColor;                                                                 
//                                                                                    
//struct BaseLight                                                                    
//{                                                                                   
//    vec3 Color;                                                                     
//    float AmbientIntensity;                                                         
//    float DiffuseIntensity;                                                         
//};                                                                                  
//                                                                                    
//struct DirectionalLight                                                             
//{                                                                                   
//    BaseLight Base;                                                                 
//    vec3 Direction;                                                                 
//};                                                                                  
//                                                                                    
//struct Attenuation                                                                  
//{                                                                                   
//    float Constant;                                                                 
//    float Linear;                                                                   
//    float Exp;                                                                      
//};                                                                                  
//                                                                                    
//struct PointLight                                                                           
//{                                                                                           
//    BaseLight Base;                                                                         
//    vec3 Position;                                                                          
//    Attenuation Atten;                                                                      
//};                                                                                          
//                                                                                            
//struct SpotLight                                                                            
//{                                                                                           
//    PointLight Base;                                                                        
//    vec3 Direction;                                                                         
//    float Cutoff;                                                                           
//};                                                                                          
//                                                                                            
//uniform int gNumPointLights;                                                                
//uniform int gNumSpotLights;                                                                 
//uniform DirectionalLight gDirectionalLight;                                                 
//uniform PointLight gPointLights[MAX_POINT_LIGHTS];                                          
//uniform SpotLight gSpotLights[MAX_SPOT_LIGHTS];                                             
//uniform sampler2D gColorMap;                                                                
//uniform vec3 gEyeWorldPos;                                                                  
//uniform float gMatSpecularIntensity;                                                        
//uniform float gSpecularPower;                                                               
//                                                                                            
//vec4 CalcLightInternal(BaseLight Light, vec3 LightDirection, vec3 Normal)                   
//{                                                                                           
//    vec4 AmbientColor = vec4(Light.Color * Light.AmbientIntensity, 1.0f);
//    float DiffuseFactor = dot(Normal, -LightDirection);                                     
//                                                                                            
//    vec4 DiffuseColor  = vec4(0, 0, 0, 0);                                                  
//    vec4 SpecularColor = vec4(0, 0, 0, 0);                                                  
//                                                                                            
//    if (DiffuseFactor > 0) {                                                                
//        DiffuseColor = vec4(Light.Color * Light.DiffuseIntensity * DiffuseFactor, 1.0f);    
//                                                                                            
//        vec3 VertexToEye = normalize(gEyeWorldPos - WorldPos0);                             
//        vec3 LightReflect = normalize(reflect(LightDirection, Normal));                     
//        float SpecularFactor = dot(VertexToEye, LightReflect);                              
//        if (SpecularFactor > 0) {                                                           
//            SpecularFactor = pow(SpecularFactor, gSpecularPower);
//            SpecularColor = vec4(Light.Color * gMatSpecularIntensity * SpecularFactor, 1.0f);                         
//        }                                                                                   
//    }                                                                                       
//                                                                                            
//    return (AmbientColor + DiffuseColor + SpecularColor);                                   
//}                                                                                           
//                                                                                            
//vec4 CalcDirectionalLight(vec3 Normal)                                                      
//{                                                                                           
//    return CalcLightInternal(gDirectionalLight.Base, gDirectionalLight.Direction, Normal);  
//}                                                                                           
//                                                                                            
//vec4 CalcPointLight(PointLight l, vec3 Normal)                                              
//{                                                                                           
//    vec3 LightDirection = WorldPos0 - l.Position;                                           
//    float Distance = length(LightDirection);                                                
//    LightDirection = normalize(LightDirection);                                             
//                                                                                            
//    vec4 Color = CalcLightInternal(l.Base, LightDirection, Normal);                         
//    float Attenuation =  l.Atten.Constant +                                                 
//                         l.Atten.Linear * Distance +                                        
//                         l.Atten.Exp * Distance * Distance;                                 
//                                                                                            
//    return Color / Attenuation;                                                             
//}                                                                                           
//                                                                                            
//vec4 CalcSpotLight(SpotLight l, vec3 Normal)                                                
//{                                                                                           
//    vec3 LightToPixel = normalize(WorldPos0 - l.Base.Position);                             
//    float SpotFactor = dot(LightToPixel, l.Direction);                                      
//                                                                                            
//    if (SpotFactor > l.Cutoff) {                                                            
//        vec4 Color = CalcPointLight(l.Base, Normal);                                        
//        return Color * (1.0 - (1.0 - SpotFactor) * 1.0/(1.0 - l.Cutoff));                   
//    }                                                                                       
//    else {                                                                                  
//        return vec4(0,0,0,0);                                                               
//    }                                                                                       
//}                                                                                           
//                                                                                            
//void main()                                                                                 
//{                                                                                           
//    vec3 Normal = normalize(Normal0);                                                       
//    vec4 TotalLight = CalcDirectionalLight(Normal);                                         
//                                                                                            
//    for (int i = 0 ; i < gNumPointLights ; i++) {                                           
//        TotalLight += CalcPointLight(gPointLights[i], Normal);                              
//    }                                                                                       
//                                                                                            
//    for (int i = 0 ; i < gNumSpotLights ; i++) {                                            
//        TotalLight += CalcSpotLight(gSpotLights[i], Normal);                                
//    }                                                                                       
//                                                                                            
//    FragColor = texture(gColorMap, TexCoord0.xy) * TotalLight;
//}
//)";
//
//
//std::string basic_lighting_vert = R"(
//#version 330                                                                        
//                                                                                    
//layout (location = 0) in vec3 Position;                                             
//layout (location = 1) in vec2 TexCoord;                                             
//layout (location = 2) in vec3 Normal;                                               
//                                                                                    
//uniform mat4 gWVP;                                                                  
//uniform mat4 gWorld;                                                                
//                                                                                    
//out vec2 TexCoord0;                                                                 
//out vec3 Normal0;                                                                   
//out vec3 WorldPos0;                                                                 
//                                                                                    
//void main()                                                                         
//{                                                                                   
//    gl_Position = gWVP * vec4(Position, 1.0);                                       
//    TexCoord0   = TexCoord;                                                         
//    Normal0     = (gWorld * vec4(Normal, 0.0)).xyz;                                 
//    WorldPos0   = (gWorld * vec4(Position, 1.0)).xyz;                               
//})";
//
//
//
//
//std::string dual_peeling_blend_frag = R"(
//#version 150
//
//
//uniform sampler2D TempTex;
//
//in vec2 texCoord;
//
//out vec4 fragOutput;
//
//void main(void)
//{	
//	fragOutput = texture(TempTex, texCoord, 0);
//
//	// for occlusion query
//	if (fragOutput.a == 0) 
//		discard;
//}
//)";
//
//
//std::string dual_peeling_blend_vert = R"(
//#version 150
//
//
//in vec4 vtx_position;
//in vec2 vtx_texcoord;
//
//out vec2 texCoord;
//
//void main()
//{
//	gl_Position = vtx_position;
//	texCoord = vtx_texcoord;
//}
//)";
//
//
//
//
//std::string dual_peeling_final_frag = R"(
//#version 150
//
//in vec2 texCoord;
//
//uniform sampler2D FrontBlenderTex;
//uniform sampler2D BackBlenderTex;
//
//out vec4 fragOutput;
//
//void main(void)
//{
//	vec4 frontColor = texture(FrontBlenderTex, texCoord);
//	vec3 backColor = texture(BackBlenderTex, texCoord).rgb;
//	float alphaMultiplier = 1.0 - frontColor.w;
//
//	// front + back
//	fragOutput.rgb = frontColor.rgb + backColor * alphaMultiplier;
//
//	// Liangliang: I don't want transparent images
//	fragOutput.a = 1.0f;
//}
//
//
//
//)";
//
//
//std::string dual_peeling_final_vert = R"(
//#version 150
//
//in vec4 vtx_position;
//in vec2 vtx_texcoord;
//
//out vec2 texCoord;
//
//void main()
//{
//	gl_Position = vtx_position;
//	texCoord = vtx_texcoord;
//}
//)";
//
//
//
//
//std::string dual_peeling_init_frag = R"(
//#version 150
//
//out vec4 fragOutput;
//
//void main(void)
//{
//	fragOutput.xy = vec2(-gl_FragCoord.z, gl_FragCoord.z);
//}
//)";
//
//
//std::string dual_peeling_init_vert = R"(
//#version 150
//
//in	vec4 vtx_position;
//
//uniform mat4 MVP;
//uniform mat4 MANIP;
//
//void main(void)
//{
//     gl_Position = MVP * MANIP * vtx_position;
//}
//)";
//
//
//
//
//std::string dual_peeling_peel_color_frag = R"(
//#version 430
//
//
//#define MAX_DEPTH 1.0
//
//// Liangliang: it is weired that computing normals in shader results in artifacts along the 
////			   edges (Perhaps this effect comes from the peeling?).
//// Note: if you change here, you should also uncomment the following in the source code:
////		 "program->set_uniform("smooth_shading", smooth_shading);"
//#define USE_PROVIDED_NORMALS
//
//// the output of this shader
//out vec4 fragOutput0;
//out vec4 fragOutput1;
//out vec4 fragOutput2;
//
//uniform sampler2D DepthBlenderTex;
//uniform sampler2D FrontBlenderTex;
//
//#ifndef USE_PROVIDED_NORMALS
//uniform bool	smooth_shading;
//#endif // !USE_PROVIDED_NORMALS
//
//uniform float	Alpha;
//
//layout(std140) uniform Lighting {
//	vec3	wLightPos;
//	vec3	eLightPos;
//	vec3	wCamPos;
//	vec3	ambient;		// in [0, 1], r==g==b;
//	vec3	specular;		// in [0, 1], r==g==b;
//	float	shininess;
//};
//
//uniform bool lightingEnabled = true;
//uniform int  hightlight_id_min;
//uniform int  hightlight_id_max;
//
//uniform bool selected;
//
//layout(std430, binding = 1) buffer selection_t {
//	uint data[];
//} selection;
//
//in Data{
//	vec3 color;
//	vec3 normal;
//	vec3 position;
//} DataIn;
//
//
//// Recent OSX/ATI drivers perform some out-of-order execution that's causing
//// the dFdx/dFdy calls to be conditionally executed. Specifically, it looks
//// like the early returns when the depth is not on a current peel layer are 
//// moved before the dFdx/dFdy calls used to compute normals. Disable the 
//// early returns on apple for now, I don't think most GPUs really benefit 
//// from them anyway at this point.
//#ifdef __APPLE__
//#define NO_PRECOLOR_EARLY_RETURN
//#endif
//
//vec4 ShadeFragment()
//{
//	vec3 color = DataIn.color;
//	//uint addr = gl_PrimitiveID / 32;
//	//uint offs = gl_PrimitiveID % 32;
//	uint addr = gl_PrimitiveID >> 5;
//	uint offs = gl_PrimitiveID & 31;
//	if ((selection.data[addr] & (1 << offs)) != 0)
//		color = mix(color, vec3(1.0, 0.0, 0.0), 0.8);
//	else if (gl_PrimitiveID >= hightlight_id_min && gl_PrimitiveID <= hightlight_id_max)
//		color = vec3(1.0, 0.0, 0.0);
//
//	if (selected) {
//		color = vec3(1.0, 0.0, 0.0);
//	}
//
//	if (lightingEnabled) {
//		vec3 normal;
//#ifdef USE_PROVIDED_NORMALS
//		normal = normalize(DataIn.normal);
//#else  // compute normals using dFdx and dFdy
//		if (smooth_shading)
//			normal = normalize(DataIn.normal);
//		else {
//			normal = normalize(cross(dFdx(DataIn.position), dFdy(DataIn.position)));
//			if (dot(normal, DataIn.normal) < 0)
//				normal = -normal;
//		}
//#endif 
//
//		vec3 view_dir = normalize(wCamPos - DataIn.position);
//		vec3 light_dir = normalize(wLightPos);
//
//		// diffuse factor
//		float df = abs(dot(light_dir, normal));				// light up both sides
//
//		// specular factor
//		vec3 half_vector = normalize(light_dir + view_dir);	// compute the half vector
//		float sf = max(dot(half_vector, normal), 0.0);		// only the front side can have specular
//		sf = pow(sf, shininess);
//
//		return vec4(color * df + specular * sf + ambient, Alpha);
//	}
//	else 
//		return vec4(color, Alpha);
//}
//
//
//void main(void)
//{
//	// Set gl_FragDepth here (gl_FragCoord.z by default)
//	// window-space depth interpolated linearly in screen space
//	gl_FragDepth = gl_FragCoord.z;
//
//	// Liangliang: gl_FragCoord contains the window-relative coordinates (non-normalized) 
//	//			   of the current fragment, so texture() cannot be used. Direct accessing
//	//			   of a texel value with non-normalized coordinates can be done through 
//	//             the texelFetch() function. See https://www.khronos.org/opengl/wiki/Sampler_(GLSL)
//	ivec2 pixelCoord = ivec2(gl_FragCoord.xy);
//	vec2 depthBlender = texelFetch(DepthBlenderTex, pixelCoord, 0).xy;
//	vec4 forwardTemp = texelFetch(FrontBlenderTex, pixelCoord, 0);
//	
//	// Depths and 1.0-alphaMult always increase
//	// so we can use pass-through by default with MAX blending
//	fragOutput0.xy = vec2(-MAX_DEPTH);
//	
//	// Front colors always increase (DST += SRC*ALPHA_MULT)
//	// so we can use pass-through by default with MAX blending
//	fragOutput1 = forwardTemp;
//	
//	// Because over blending makes color increase or decrease,
//	// we cannot pass-through by default.
//	// Each pass, only one fragment writes a color greater than 0
//	fragOutput2 = vec4(0.0);
//
//	float nearestDepth = -depthBlender.x;
//	float farthestDepth = depthBlender.y;
//
//	// the .0000001 below is an epsilon.  It turns out that
//	// graphics cards can render the same polygon two times
//	// in a row with different z values. I suspect it has to
//	// do with how rasterization of the polygon is broken up.
//	// A different breakup across fragment shaders can result in
//	// very slightly different z values for some of the pixels.
//	// The end result is that with depth peeling, you can end up
//	// counting/accumulating pixels of the same surface twice
//	// simply due to this randomness in z values. So we introduce
//	// an epsilon into the transparent test to require some
//	// minimal z seperation between pixels. This value was determined
//	// through trial-and-error -- it may need to be increased at
//	// some point. See also the comment in vtkDepthPeelingPass's shader.
//	//float epsilon = 0.000001; // VTK uses this value
//	float epsilon = 0.0000001;
//
//	// Is this fragment outside the current peels?
//	if (gl_FragDepth < nearestDepth - epsilon || gl_FragDepth > farthestDepth + epsilon) {
//#ifndef NO_PRECOLOR_EARLY_RETURN
//		return;
//#endif
//	}
//
//	// Is this fragment inside the current peels?
//	if (gl_FragDepth > nearestDepth + epsilon && gl_FragDepth < farthestDepth - epsilon) {
//		// Write out depth so this frag will be peeled later:
//		fragOutput0.xy = vec2(-gl_FragDepth, gl_FragDepth);
//
//#ifndef NO_PRECOLOR_EARLY_RETURN
//		return;
//#endif
//	}
//	
//	// If we made it here, this fragment is on the peeled layer from last pass
//	// therefore, we need to shade it, and make sure it is not peeled any farther
//	vec4 color = ShadeFragment();
//	fragOutput0.xy = vec2(-MAX_DEPTH);
//	
//	// Check if this fragment is on a current peel:
//	if (gl_FragDepth >= nearestDepth - epsilon && gl_FragDepth <= nearestDepth + epsilon)
//	{ // Front peel:
//		// We store the front alpha value as (1-alpha) to allow MAX
//		// blending. This also means it is really initialized to 1,
//		// as it should be for under-blending.
//		float alphaMultiplier = 1.0 - forwardTemp.w;
//
//		// Use under-blending to combine fragment with front color:
//		fragOutput1.xyz += color.rgb * color.a * alphaMultiplier;
//
//		// Write out (1-alpha):
//		fragOutput1.w = 1.0 - alphaMultiplier * (1.0 - color.a);
//	} 
//	else  // Back peel:
//	{	
//		// Dump premultiplied fragment, it will be blended later:
//		fragOutput2 += color;
//	}
//}
//)";
//
//
//std::string dual_peeling_peel_color_vert = R"(
//#version 150
//
//in  vec3 vtx_position;	// vertex position
//in	vec3 vtx_normal;	// vertex normal
//in  vec3 vtx_color;		// vertex color
//
//uniform vec3 default_color;
//uniform bool per_vertex_color;
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//// the data to be sent to the fragment shader
//out Data{
//	vec3 color;
//	vec3 normal;
//	vec3 position; 
//} DataOut;
//
//
//void main(void)
//{
//	vec4 new_position = MANIP * vec4(vtx_position, 1.0);
//
//	if (per_vertex_color)
//		DataOut.color = vtx_color;
//	else
//		DataOut.color = default_color;
//
//	DataOut.normal = NORMAL * vtx_normal;
//	DataOut.position = new_position.xyz;
//
//	gl_Position = MVP * new_position;
//
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(new_position, clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(new_position, clippingPlane1);
//	}
//}
//)";
//
//
//
//
//std::string dual_peeling_peel_texture_frag = R"(
//#version 430
//
//#define MAX_DEPTH 1.0
//
//// Liangliang: it is weired that computing normals in shader results in artifacts along the 
////			   edges (Perhaps this effect comes from the peeling?).
//// Note: if you change here, you should also uncomment the following in the source code:
////		 "program->set_uniform("smooth_shading", smooth_shading);"
//#define USE_PROVIDED_NORMALS
//
//// the output of this shader
//out vec4 fragOutput0;
//out vec4 fragOutput1;
//out vec4 fragOutput2;
//
//uniform sampler2D DepthBlenderTex;
//uniform sampler2D FrontBlenderTex;
//
//uniform sampler2D ColorMapTex; // colormap texture
//uniform float	  texture_repeat;
//uniform float	  fractional_repeat;
//
//#ifndef USE_PROVIDED_NORMALS
//uniform bool	smooth_shading;
//#endif // !USE_PROVIDED_NORMALS
//
//uniform float	Alpha;
//
//uniform bool lightingEnabled = true;
//layout(std140) uniform Lighting {
//	vec3	wLightPos;
//	vec3	eLightPos;
//	vec3	wCamPos;
//	vec3	ambient;		// in [0, 1], r==g==b;
//	vec3	specular;		// in [0, 1], r==g==b;
//	float	shininess;
//};
//
//uniform int  hightlight_id_min;
//uniform int  hightlight_id_max;
//
//uniform bool selected;
//
//layout(std430, binding = 1) buffer selection_t {
//	uint data[];
//} selection;
//
//in Data{
//	vec2 texcoord;
//	vec3 normal;
//	vec3 position;
//} DataIn;
//
//
//// Recent OSX/ATI drivers perform some out-of-order execution that's causing
//// the dFdx/dFdy calls to be conditionally executed. Specifically, it looks
//// like the early returns when the depth is not on a current peel layer are 
//// moved before the dFdx/dFdy calls used to compute normals. Disable the 
//// early returns on apple for now, I don't think most GPUs really benefit 
//// from them anyway at this point.
//#ifdef __APPLE__
//#define NO_PRECOLOR_EARLY_RETURN
//#endif
//
//vec4 ShadeFragment()
//{
//	float repeat = texture_repeat + fractional_repeat / 100.0f;
//	vec3 color = texture(ColorMapTex, DataIn.texcoord * repeat).rgb;
//
//	//uint addr = gl_PrimitiveID / 32;
//	//uint offs = gl_PrimitiveID % 32;
//	uint addr = gl_PrimitiveID >> 5;
//	uint offs = gl_PrimitiveID & 31;
//	if ((selection.data[addr] & (1 << offs)) != 0)
//		color = mix(color, vec3(1.0, 0.0, 0.0), 0.8);
//	else if (gl_PrimitiveID >= hightlight_id_min && gl_PrimitiveID <= hightlight_id_max)
//		color = vec3(1.0, 0.0, 0.0);
//
//	if (selected) {
//		color = vec3(1.0, 0.0, 0.0);
//	}
//
//	if (lightingEnabled) {
//		vec3 normal;
//#ifdef USE_PROVIDED_NORMALS
//		normal = normalize(DataIn.normal);
//#else  // compute normals using dFdx and dFdy
//		if (smooth_shading)
//			normal = normalize(DataIn.normal);
//		else {
//			normal = normalize(cross(dFdx(DataIn.position), dFdy(DataIn.position)));
//			if (dot(normal, DataIn.normal) < 0)
//				normal = -normal;
//		}
//#endif 
//
//		vec3 view_dir = normalize(wCamPos - DataIn.position);
//		vec3 light_dir = normalize(wLightPos);
//
//		// diffuse factor
//		float df = abs(dot(light_dir, normal));				// light up both sides
//
//		// specular factor
//		vec3 half_vector = normalize(light_dir + view_dir);	// compute the half vector
//		float sf = max(dot(half_vector, normal), 0.0);		// only the front side can have specular
//		sf = pow(sf, shininess);
//
//		return vec4(color * df + specular * sf + ambient, Alpha);
//	}
//	else 
//		return vec4(color, Alpha);
//}
//
//
//void main(void)
//{
//	// Set gl_FragDepth here (gl_FragCoord.z by default)
//	// window-space depth interpolated linearly in screen space
//	gl_FragDepth = gl_FragCoord.z;
//
//	// Liangliang: gl_FragCoord contains the window-relative coordinates (non-normalized) 
//	//			   of the current fragment, so texture() cannot be used. Direct accessing
//	//			   of a texel value with non-normalized coordinates can be done through 
//	//             the texelFetch() function. See https://www.khronos.org/opengl/wiki/Sampler_(GLSL)
//	ivec2 pixelCoord = ivec2(gl_FragCoord.xy);
//	vec2 depthBlender = texelFetch(DepthBlenderTex, pixelCoord, 0).xy;
//	vec4 forwardTemp = texelFetch(FrontBlenderTex, pixelCoord, 0);
//	
//	// Depths and 1.0-alphaMult always increase
//	// so we can use pass-through by default with MAX blending
//	fragOutput0.xy = vec2(-MAX_DEPTH);
//	
//	// Front colors always increase (DST += SRC*ALPHA_MULT)
//	// so we can use pass-through by default with MAX blending
//	fragOutput1 = forwardTemp;
//	
//	// Because over blending makes color increase or decrease,
//	// we cannot pass-through by default.
//	// Each pass, only one fragment writes a color greater than 0
//	fragOutput2 = vec4(0.0);
//
//	float nearestDepth = -depthBlender.x;
//	float farthestDepth = depthBlender.y;
//
//	// the .0000001 below is an epsilon.  It turns out that
//	// graphics cards can render the same polygon two times
//	// in a row with different z values. I suspect it has to
//	// do with how rasterization of the polygon is broken up.
//	// A different breakup across fragment shaders can result in
//	// very slightly different z values for some of the pixels.
//	// The end result is that with depth peeling, you can end up
//	// counting/accumulating pixels of the same surface twice
//	// simply due to this randomness in z values. So we introduce
//	// an epsilon into the transparent test to require some
//	// minimal z seperation between pixels. This value was determined
//	// through trial-and-error -- it may need to be increased at
//	// some point. See also the comment in vtkDepthPeelingPass's shader.
//	//float epsilon = 0.000001; // VTK uses this value
//	float epsilon = 0.0000001;
//
//	// Is this fragment outside the current peels?
//	if (gl_FragDepth < nearestDepth - epsilon || gl_FragDepth > farthestDepth + epsilon) {
//#ifndef NO_PRECOLOR_EARLY_RETURN
//		return;
//#endif
//	}
//
//	// Is this fragment inside the current peels?
//	if (gl_FragDepth > nearestDepth + epsilon && gl_FragDepth < farthestDepth - epsilon) {
//		// Write out depth so this frag will be peeled later:
//		fragOutput0.xy = vec2(-gl_FragDepth, gl_FragDepth);
//
//#ifndef NO_PRECOLOR_EARLY_RETURN
//		return;
//#endif
//	}
//	
//	// If we made it here, this fragment is on the peeled layer from last pass
//	// therefore, we need to shade it, and make sure it is not peeled any farther
//	vec4 color = ShadeFragment();
//	fragOutput0.xy = vec2(-MAX_DEPTH);
//	
//	// Check if this fragment is on a current peel:
//	if (gl_FragDepth >= nearestDepth - epsilon && gl_FragDepth <= nearestDepth + epsilon)
//	{ // Front peel:
//		// We store the front alpha value as (1-alpha) to allow MAX
//		// blending. This also means it is really initialized to 1,
//		// as it should be for under-blending.
//		float alphaMultiplier = 1.0 - forwardTemp.w;
//
//		// Use under-blending to combine fragment with front color:
//		fragOutput1.xyz += color.rgb * color.a * alphaMultiplier;
//
//		// Write out (1-alpha):
//		fragOutput1.w = 1.0 - alphaMultiplier * (1.0 - color.a);
//	} 
//	else  // Back peel:
//	{	
//		// Dump premultiplied fragment, it will be blended later:
//		fragOutput2 += color;
//	}
//}
//)";
//
//
//std::string dual_peeling_peel_texture_vert = R"(
//#version 150
//
//in vec3 vtx_position;
//in vec2 vtx_texcoord;
//in vec3 vtx_normal;
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//// the data to be sent to the fragment shader
//out Data{
//	vec2 texcoord;
//	vec3 normal;
//	vec3 position; 
//} DataOut;
//
//
//void main(void)
//{
//	vec4 new_position = MANIP * vec4(vtx_position, 1.0);
//
//	DataOut.texcoord = vtx_texcoord;
//	DataOut.normal = NORMAL * vtx_normal;
//	DataOut.position = new_position.xyz;
//
//	gl_Position = MVP * new_position;
//
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(new_position, clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(new_position, clippingPlane1);
//	}
//}
//)";
//
//
//
//
//std::string edl_bilateral_filter_frag = R"(
//#version 150
//
///*=========================================================================
//
//Program: VTK
//Module:  vtkEDLBilateralFilterFS.glsl
//
//Copyright (c) 2005-2008 Sandia Corporation, Kitware Inc.
//All rights reserved.
//
//ParaView is a free software; you can redistribute it and/or modify it
//under the terms of the ParaView license version 1.2.
//
//See License_v1.2.txt for the full ParaView license.
//A copy of this license can be obtained by contacting
//Kitware Inc.
//28 Corporate Drive
//Clifton Park, NY 12065
//USA
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
//CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//=========================================================================*/
///*----------------------------------------------------------------------
//Acknowledgement:
//This algorithm is the result of joint work by Electricité de France,
//CNRS, Collège de France and Université J. Fourier as part of the
//Ph.D. thesis of Christian BOUCHENY.
//------------------------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////
////
////
////  Bilateral filtering
////
////  C.B. - 16 aout 2008
////
////    IN:
////      s2_I - Image to blur
////      s2_D - Modulating depth image
////
////    OUT:
////      Filtered image
////
////////////////////////////////////////////////////////////////////////////
//
//in	vec2	tcoordVC;
//
//out vec4	fragOutput;
//
///****************************************************/
//uniform sampler2D   s2_I;	// Image to blur
//uniform sampler2D   s2_D;	// Modulating depth image
//uniform float       SX;
//uniform float       SY;
//uniform int         N; // filter size (full width, necessarily odd, like 3, 5...)
//uniform float       sigma;
///****************************************************/
//
///****************************************************/
//float   sigmaz = 0.005;
///****************************************************/
//
//void main(void)
//{
//	vec3 C = texture2D(s2_I, tcoordVC.st).rgb;
//	float z = texture2D(s2_D, tcoordVC.st).r;
//
//	float ALL = 0.;       // sum of all weights
//	vec3  RES = vec3(0.); // sum of all contributions
//	int   hN = N / 2;      // filter half width
//	vec2  coordi = vec2(0., 0.);
//	vec3  Ci;
//	float zi;
//	float dist;
//	float dz;
//	float Fi, Gi;
//
//	int   c, d;
//	for (c = -hN; c<hN + 1; c++)
//	{
//		for (d = -hN; d<hN + 1; d++)
//		{
//			coordi = vec2(float(c)*SX, float(d)*SY);
//			Ci = texture2D(s2_I, tcoordVC.st + coordi).rgb;
//			zi = texture2D(s2_D, tcoordVC.st + coordi).r;
//
//			dist = clamp(float(c*c + d*d) / float(hN*hN), 0., 1.);
//			dz = (z - zi)*(z - zi);
//
//			Fi = exp(-dist*dist / (2.* sigma*sigma));
//			Gi = exp(-dz*dz / (2.* sigmaz*sigmaz));
//
//			RES += Ci * Fi * Gi;
//			ALL += Fi * Gi;
//		}
//	}
//	RES /= ALL;
//
//	fragOutput = vec4(RES, z);
//}
//)";
//
//
//std::string edl_bilateral_filter_vert = R"(
//#version 150
//
///*=========================================================================
//
//  Program:   Visualization Toolkit
//  Module:    vtktextureObjectVS.glsl
//
//  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
//  All rights reserved.
//  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.
//
//	 This software is distributed WITHOUT ANY WARRANTY; without even
//	 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//	 PURPOSE.  See the above copyright notice for more information.
//
//=========================================================================*/
//
//in	vec3	vertexMC;
//in	vec2	tcoordMC;
//
//out	vec2	tcoordVC;
//
//void main()
//{
//	tcoordVC = tcoordMC;
//	gl_Position = vec4(vertexMC, 1.0f);
//}
//)";
//
//
//
//
//std::string edl_compose_frag = R"(
//#version 150
//
///*=========================================================================
//
//Program: VTK
//Module:  vtkEDLComposeFS.glsl
//
//Copyright (c) 2005-2008 Sandia Corporation, Kitware Inc.
//All rights reserved.
//
//ParaView is a free software; you can redistribute it and/or modify it
//under the terms of the ParaView license version 1.2.
//
//See License_v1.2.txt for the full ParaView license.
//A copy of this license can be obtained by contacting
//Kitware Inc.
//28 Corporate Drive
//Clifton Park, NY 12065
//USA
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
//CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//=========================================================================*/
///*----------------------------------------------------------------------
//Acknowledgement:
//This algorithm is the result of joint work by Electricité de France,
//CNRS, Collège de France and Université J. Fourier as part of the
//Ph.D. thesis of Christian BOUCHENY.
//------------------------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////
////
////
////  EyeDome Lighting - Compositing - Simplified version for use in VTK\n
////
////    C.B. - 3 feb. 2009
////
////////////////////////////////////////////////////////////////////////////
//
//in	vec2	tcoordVC;
//
//out vec4	fragOutput;
//
///**************************************************/
//uniform sampler2D    s2_S1;  // fine scale
//uniform sampler2D    s2_S2;  // larger medium scale
//uniform sampler2D    s2_C;   // scene color image
//							 /**************************************************/
//
//void main(void)
//{
//	vec4 shade1 = texture2D(s2_S1, tcoordVC.st);
//	vec4 shade2 = texture2D(s2_S2, tcoordVC.st);
//	vec4  color = texture2D(s2_C, tcoordVC.st);
//
//	// if it is the background(ala depth > 0.99) just copy it
//	//if (shade1.a > 0.99)
//	//{
//	//	fragOutput = vec4(shade1.rgb, 1.) * color;
//	//}
//	//else
//	//{
//		float lum = mix(shade1.r, shade2.r, 0.3);
//		fragOutput = vec4(color.rgb*lum, color.a);
//	//}
//
//	gl_FragDepth = shade1.a; // write stored depth
//}
//)";
//
//
//std::string edl_compose_vert = R"(
//#version 150
//
///*=========================================================================
//
//  Program:   Visualization Toolkit
//  Module:    vtktextureObjectVS.glsl
//
//  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
//  All rights reserved.
//  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.
//
//	 This software is distributed WITHOUT ANY WARRANTY; without even
//	 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//	 PURPOSE.  See the above copyright notice for more information.
//
//=========================================================================*/
//
//in	vec3	vertexMC;
//in	vec2	tcoordMC;
//
//out	vec2	tcoordVC;
//
//void main()
//{
//	tcoordVC = tcoordMC;
//	gl_Position = vec4(vertexMC, 1.0f);
//}
//)";
//
//
//
//
//std::string edl_shade_frag = R"(
//#version 150
//
///*=========================================================================
//
//Program: VTK
//Module:  vtkEDLShadeFS.glsl
//
//Copyright (c) 2005-2008 Sandia Corporation, Kitware Inc.
//All rights reserved.
//
//ParaView is a free software; you can redistribute it and/or modify it
//under the terms of the ParaView license version 1.2.
//
//See License_v1.2.txt for the full ParaView license.
//A copy of this license can be obtained by contacting
//Kitware Inc.
//28 Corporate Drive
//Clifton Park, NY 12065
//USA
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
//CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//=========================================================================*/
///*----------------------------------------------------------------------
//Acknowledgement:
//This algorithm is the result of joint work by Electricité de France,
//CNRS, Collège de France and Université J. Fourier as part of the
//Ph.D. thesis of Christian BOUCHENY.
//------------------------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////
////
////
////    EyeDome Lighting - Simplified version for use in VTK
////        - oriented light
////        - no focus
////        - some uniforms transformed to local variables
////
////        C.B. - 3 feb. 2009
////
////      IN:    Depth buffer of the scene
////             r = recorded z, in [0:1]
////      OUT:   EDL shaded image
////
////////////////////////////////////////////////////////////////////////////
//
//in	vec2	tcoordVC;
//
//out vec4	fragOutput;
//
///**************************************************/
//uniform sampler2D    s2_depth; // - Z Map
//uniform float        d;        // [1.0 in full res - 2.0 at lower res]
//							   //- Extension in image space, in pixels
//uniform vec4         N[8];     //- Array of neighbours
//							   // [No support for TabUniform in VTK
//							   // --> constant array, hereafter]
//uniform float        F_scale;  // [5.] - Shading amplification factor
//
//uniform float        SX;      // - pixel horizontal step (image distance: 1/w)
//uniform float        SY;      //- pixel vertical step (image distance: 1/h)
//uniform float        Znear;     // near clipping plane
//uniform float        Zfar;      // far clipping plane
//uniform float        SceneSize; // typical scene size, to scale the depth by.
//
//uniform vec3         L;         // [0.,0.,-1.] - Light direction [frontal]
//								/**************************************************/
//
//								/**************************************************/
//int    Nnb = 1;  // nombre de voisins par rayon
//float  Zm = 0.; // minimal z in image
//float  ZM = 1.; // maximal z in image
//float  Z;        // initial Z
//
//vec3   WHITE3 = vec3(1., 1., 1.);
//
//float    t;
//vec4     Zn[8];  // profondeurs des voisins
//float    D[8];   // ombrage genere par les voisins
//vec4     tn, tnw, tw, tsw, ts, tse, te, tne;
//float    dn, dnw, dw, dsw, ds, dse, de, dne;
//float    S;      // image step, corresponds to one pixel size
///**************************************************/
//
////////////////////////////////////////////////////////////////////////////
////
////    Local shading functions
////
////    Pseudo angle, avec S (distance pixel) valant l'unite
////    zi      elevation of current pixel
////    zj      elevation of its neighbour
////    delta   distance between the two
//float angleP(float zi, float zj, float delta)
//{
//	return max(0., zj - zi) / (delta / S);
//}
//
////    zi      elevation of current pixel
////    zj      elevation of its neighbour
////    delta   distance between the two
//float obscurance(float zi, float zj, float delta)
//{
//	return angleP(zi, zj, delta);
//}
////
////    Local shading functions
////
////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////
////
////    Z transformation
////
//float zflip(float z)
//{
//	return 1. - z;
//}
//
//float zscale(float z)
//{
//	return clamp((z - Zm) / (ZM - Zm), 0., 1.);
//}
//
////    Inversion of OpenGL perspective projection
////    (should be adapted for orthographic projection)
////
//float ztransform(float z)
//{
//	float Z;
//	Z = (z - 0.5)*2.;
//	Z = -2.*Zfar*Znear / ((Zfar - Znear) * (Z - (Zfar + Znear) / (Zfar - Znear)));
//	Z = (Z - Znear) / SceneSize;
//	return 1. - Z;
//}
////
////      Z transformation
////
////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////
////
////      NEIGHBORHOOD    SHADING
////
//void computeNeighbours8(float dist)
//{
//	// Plan Lumiere-point
//	vec4  P = vec4(L.xyz, -dot(L.xyz, vec3(0., 0., t)));
//
//	// 0 at the back of the scene
//	int   c;
//	vec2  V;  // pixel voisin
//	float di = dist;
//	float Znp[8]; // profondeur des 8 voisins sur le plan
//
//	for (c = 0; c < 8; c++)
//	{
//		V = tcoordVC.st + di*vec2(SX, SY)*N[c].xy;
//		Zn[c].x = ztransform(texture2D(s2_depth, V).r);
//		// profondeur du voisin reel dans l'image
//
//		// VERSION qui ombre le fond
//		Znp[c] = dot(vec4(di*vec2(SX, SY)*N[c].xy, Zn[c].x, 1.0), P);
//	}
//
//	dn = obscurance(0., Znp[0], di*SX);
//	dnw = obscurance(0., Znp[1], di*SX);
//	dw = obscurance(0., Znp[2], di*SX);
//	dsw = obscurance(0., Znp[3], di*SX);
//	ds = obscurance(0., Znp[4], di*SX);
//	dse = obscurance(0., Znp[5], di*SX);
//	de = obscurance(0., Znp[6], di*SX);
//	dne = obscurance(0., Znp[7], di*SX);
//}
//
//float computeObscurance(float F, float scale, float weight)
//{
//	computeNeighbours8(scale);
//
//	float S = F;
//	float WE = weight;
//
//	S += dn  * WE;
//	S += dnw * WE;
//	S += dw  * WE;
//	S += dsw * WE;
//	S += ds  * WE;
//	S += dse * WE;
//	S += de  * WE;
//	S += dne * WE;
//
//	return S;
//}
//
//void ambientOcclusion()
//{
//	float F = 0.;
//	float weight = 20.; // 2. * 3.14159;
//
//	F = computeObscurance(F, d, weight);
//	F = exp(-F_scale*F);
//
//	fragOutput = vec4(F, F, F, Z);
//}
//
//void main(void)
//{
//	S = SX;
//	Z = texture2D(s2_depth, tcoordVC.st).r;
//	t = ztransform(Z);
//
//	ambientOcclusion();
//}
//)";
//
//
//std::string edl_shade_vert = R"(
//#version 150
//
///*=========================================================================
//
//  Program:   Visualization Toolkit
//  Module:    vtktextureObjectVS.glsl
//
//  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
//  All rights reserved.
//  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.
//
//	 This software is distributed WITHOUT ANY WARRANTY; without even
//	 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//	 PURPOSE.  See the above copyright notice for more information.
//
//=========================================================================*/
//
//in	vec3	vertexMC;
//in	vec2	tcoordMC;
//
//out	vec2	tcoordVC;
//
//void main()
//{
//	tcoordVC = tcoordMC;
//	gl_Position = vec4(vertexMC, 1.0f);
//}
//)";
//
//
//
//
//std::string lines_cones_color_frag = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//// It uses geometry shader for vertex generation.
//// The code does not cover round caps.
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//uniform bool	perspective;
//
//uniform vec3	color;
//uniform float	radius;
//
//layout(std140) uniform Lighting {
//	vec3	wLightPos;
//	vec3	eLightPos;
//	vec3	wCamPos;
//	vec3	ambient;		// in [0, 1], r==g==b;
//	vec3	specular;		// in [0, 1], r==g==b;
//	float	shininess;
//};
//
//in Data{
//	vec3 point;// camera space
//	vec3 axis;
//	vec3 base;
//	vec3 end;
//	vec3 U;
//	vec3 V;
//} DataIn;
//
//in  vec3 gOutColor;
//
//out vec4 outputF;
//
//
//vec3 ComputeLight(vec3 N, vec3 L, vec3 V, vec3 amb, vec3 spec, float sh, vec3 color) {
//	float df = max(dot(N, L), 0);
//
//	float sf = 0.0;	// specular factor
//	if (df > 0.0) {	// if the vertex is lit compute the specular color
//		vec3 half_vector = normalize(L + V);	// compute the half vector
//		sf = max(dot(half_vector, N), 0.0);
//		sf = pow(sf, sh);
//	}
//
//	return color * df + spec * sf + amb;
//}
//
//void main()
//{
//	// First of all, I need the correct point that we're pointing at
//
//	vec3 view_dir = vec3(0, 0, 1);	// this is view direction for orthographic mode in camera space
//	if (perspective) {
//		view_dir = normalize(vec3(0) - DataIn.point);	// camera pos is (0, 0, 0) in camera space
//	}
//
//	// basis = local coordinate system of cone 
//	mat3 basis = mat3(DataIn.U, DataIn.V, DataIn.axis);
//	vec3 diff = DataIn.point - DataIn.base;
//	vec3 P = diff * basis;
//
//	float radius2 = radius*radius;
//	float length2 = dot(DataIn.end - DataIn.base, DataIn.end - DataIn.base);
//	float coeff = radius2 / length2;
//	float h = dot(DataIn.end - DataIn.base, DataIn.axis);
//
//	// Now calculate distance to the intersecting point from ray origin
//	// See Ray tracing cones here: https://www.cl.cam.ac.uk/teaching/1999/AGraphHCI/SMAG/node2.html#eqn:rectray
//
//	// Direction of the ray in cone space
//	vec3 D = vec3(dot(DataIn.U, view_dir), dot(DataIn.V, view_dir), dot(DataIn.axis, view_dir));
//	// Now the intersection is between z-axis aligned cone and a ray
//	float a = D.x * D.x + D.y * D.y - D.z * D.z * coeff;
//	float b = 2.0 * (P.x * D.x + P.y * D.y - coeff * (P.z * D.z - h * D.z));
//	float c = P.x * P.x + P.y * P.y - coeff * (P.z - h) * (P.z - h);
//	// calculate a discriminant of the above quadratic equation
//	float d = b*b - 4 * a*c;
//	if (d < 0.0)		// outside of the cone
//		discard;
//
//	// point of intersection on cone surface
//	float dist = (-b + sqrt(d)) / (2 * a);
//	vec3 new_point = DataIn.point + dist * view_dir;
//
//	// now compute the normal of the new point (for computing the lighting).
//	vec3 tangent = cross(new_point - DataIn.base, new_point - DataIn.end);
//	vec3 normal = cross(new_point - DataIn.end, tangent);
//	normal = normalize(normal);
//
//	// to calculate caps, simply check the angle between the following two vectors:
//	//		- (point of intersection) - (the cone's base center)
//	//		- cap plane normal (which is the cone axis)
//	// if the angle < 0, the point is outside of cone
//	float cap_test = dot((new_point - DataIn.base), DataIn.axis);
//
//
//	// Now I need to determine if the projection of the cone's peak (onto the base plane) is within the base disc
//	float tt = dot(DataIn.base - DataIn.end, -DataIn.axis) / dot(view_dir, -DataIn.axis);
//	vec3 projection = DataIn.end + view_dir * tt;
//	bool within_base = dot(projection - DataIn.base, projection - DataIn.base) < radius2;
//
//
//	if (cap_test < 0.0 || (within_base && dot(view_dir, DataIn.axis) < 0.0))
//	{
//		normal = -DataIn.axis;
//
//		// ray-plane intersection
//		float t = dot(DataIn.base - DataIn.point, normal) / dot(view_dir, normal);
//		new_point = DataIn.point + view_dir * t;
//
//		// within the cap radius?
//		if (dot(new_point - DataIn.base, new_point - DataIn.base) > radius2)
//			discard;
//	}
//
//	// compute the depth. I can do it easier
//	//vec4 pos = PROJ * vec4(new_point, 1.0);
//	//float depth = 0.5*(pos.z / pos.w) + 0.5;
//	vec2 clipZW = new_point.z * PROJ[2].zw + PROJ[3].zw;
//	float depth = 0.5 + 0.5 * clipZW.x / clipZW.y;
//
//	// this is a workaround necessary for Mac, otherwise the modified fragment won't clip properly
//	if (depth <= 0.0 || depth >= 1.0)
//		discard;
//
//	gl_FragDepth = depth;
//
//	vec3 light_dir = normalize(eLightPos);
//	vec3 final_color = ComputeLight(normal, light_dir, view_dir, ambient, specular, shininess, gOutColor);
//	outputF = vec4(final_color, 1.0);
//})";
//
//
//std::string lines_cones_color_geom = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//// It uses geometry shader for vertex generation.
//// The code does not cover round caps.
//
////#extension GL_EXT_geometry_shader4 : enable
//
//layout(lines) in;
//layout(triangle_strip, max_vertices = 5) out;
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//uniform bool	perspective;
//uniform float	radius;
//
//in  vec3 vOutColor[];
//
//out vec3 gOutColor;
//
//out Data{
//	vec3 point;	// camera space
//	vec3 axis;
//	vec3 base;
//	vec3 end;	// the peak of the cone
//	vec3 U;
//	vec3 V;
//} DataOut;
//
//
//void main()
//{
//	vec3  base = gl_in[0].gl_Position.xyz;  // in camera space
//	vec3  top = gl_in[1].gl_Position.xyz;   // in camera space
//	vec3  len_dir = top - base;
//	vec3  view_dir = vec3(0,0,1);	// this is view direction for orthographic mode in camera space
//	if (perspective) 
//		view_dir = normalize(vec3(0) - base);	// camera pos is (0, 0, 0) in camera space
//
//	vec3 left = cross(view_dir, len_dir);
//	vec3 up = cross(left, len_dir);
//	DataOut.axis = normalize(len_dir);
//	DataOut.U = normalize(up);
//	DataOut.V = normalize(left);
//	left = radius * DataOut.V;
//	up = radius * DataOut.U;
//
//	DataOut.base = base;
//	DataOut.end = top;
//
//	float b = dot(view_dir, len_dir);
//	if (b < 0) {
//		// Vertex 1
//		DataOut.point = top;
//		gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//		if (clippingPlaneEnabled) {
//			gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//			if (crossSectionEnabled)
//				gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//		}
//		gOutColor = vOutColor[1];
//		EmitVertex();
//
//		// Vertex 2
//		DataOut.point = (base + left - up);
//		gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//		if (clippingPlaneEnabled) {
//			gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//			if (crossSectionEnabled)
//				gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//		}
//		gOutColor = vOutColor[0];
//		EmitVertex();
//
//		// Vertex 3
//		DataOut.point = (base - left - up);
//		gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//		if (clippingPlaneEnabled) {
//			gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//			if (crossSectionEnabled)
//				gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//		}
//		gOutColor = vOutColor[0];
//		EmitVertex();
//
//		// Vertex 4
//		DataOut.point = (base + left + up);
//		gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//		if (clippingPlaneEnabled) {
//			gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//			if (crossSectionEnabled)
//				gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//		}
//		gOutColor = vOutColor[0];
//		EmitVertex();
//
//		// Vertex 5
//		DataOut.point = (base - left + up);
//		gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//		if (clippingPlaneEnabled) {
//			gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//			if (crossSectionEnabled)
//				gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//		}
//		gOutColor = vOutColor[0];
//		EmitVertex();
//	}
//	else {
//		// Vertex 1
//		DataOut.point = top;
//		gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//		if (clippingPlaneEnabled) {
//			gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//			if (crossSectionEnabled)
//				gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//		}
//		gOutColor = vOutColor[1];
//		EmitVertex();
//
//		// Liangliang: determining the minimum extend for this case is tricky, so I just use 
//		//             an enlarge factor to make sure all the entire one is covered. 
//		float factor = 1.1; // It seems 1.1 is large enough
//
//		// Vertex 2
//		DataOut.point = (base - left * factor + up);	//
//		gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//		if (clippingPlaneEnabled) {
//			gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//			if (crossSectionEnabled)
//				gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//		}
//		gOutColor = vOutColor[0];
//		EmitVertex();
//
//		// Vertex 3
//		DataOut.point = (base + left * factor + up);	// 1.2 is just 
//		gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//		if (clippingPlaneEnabled) {
//			gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//			if (crossSectionEnabled)
//				gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//		}
//		gOutColor = vOutColor[0];
//		EmitVertex();
//
//		// Vertex 4
//		DataOut.point = (base - left - up);
//		gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//		if (clippingPlaneEnabled) {
//			gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//			if (crossSectionEnabled)
//				gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//		}
//		gOutColor = vOutColor[0];
//		EmitVertex();
//
//		// Vertex 5
//		DataOut.point = (base + left - up);
//		gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//		if (clippingPlaneEnabled) {
//			gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//			if (crossSectionEnabled)
//				gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//		}
//		gOutColor = vOutColor[0];
//		EmitVertex();
//	}
//
//	EndPrimitive();
//})";
//
//
//std::string lines_cones_color_vert = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//// It uses geometry shader for vertex generation.
//// The code does not cover round caps.
//
////#extension GL_EXT_gpu_shader4 : enable
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//uniform vec3	default_color;
//uniform bool	per_vertex_color;
//
//in  vec3 vtx_position;	// point position
//in  vec3 vtx_color;		// point color
//
//out vec3 vOutColor;
//
//void main()
//{
//	gl_Position = MV * MANIP * vec4(vtx_position, 1.0);
//
//	if (per_vertex_color)
//		vOutColor = vtx_color;
//	else
//		vOutColor = default_color;
//})";
//
//
//
//
//std::string lines_cones_texture_frag = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//// It uses geometry shader for vertex generation.
//// The code does not cover round caps.
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//uniform sampler2D	textureID;
//
//uniform bool	perspective;
//
//uniform vec3	color;
//uniform float	radius;
//
//layout(std140) uniform Lighting {
//	vec3	wLightPos;
//	vec3	eLightPos;
//	vec3	wCamPos;
//	vec3	ambient;		// in [0, 1], r==g==b;
//	vec3	specular;		// in [0, 1], r==g==b;
//	float	shininess;
//};
//in Data{
//	vec3 point;// camera space
//	vec3 axis;
//	vec3 base;
//	vec3 end;
//	vec3 U;
//	vec3 V;
//} DataIn;
//
//in  vec2 gOutTexcoord;
//
//out vec4 outputF;
//
//
//vec3 ComputeLight(vec3 N, vec3 L, vec3 V, vec3 amb, vec3 spec, float sh, vec3 color) {
//	float df = max(dot(N, L), 0);
//
//	float sf = 0.0;	// specular factor
//	if (df > 0.0) {	// if the vertex is lit compute the specular color
//		vec3 half_vector = normalize(L + V);	// compute the half vector
//		sf = max(dot(half_vector, N), 0.0);
//		sf = pow(sf, sh);
//	}
//
//	return color * df + spec * sf + amb;
//}
//
//void main()
//{
//	// First of all, I need the correct point that we're pointing at
//
//	vec3 view_dir = vec3(0, 0, 1);	// this is view direction for orthographic mode in camera space
//	if (perspective) {
//		view_dir = normalize(vec3(0) - DataIn.point);	// camera pos is (0, 0, 0) in camera space
//	}
//
//	// basis = local coordinate system of cone 
//	mat3 basis = mat3(DataIn.U, DataIn.V, DataIn.axis);
//	vec3 diff = DataIn.point - DataIn.base;
//	vec3 P = diff * basis;
//
//	float radius2 = radius*radius;
//	float length2 = dot(DataIn.end - DataIn.base, DataIn.end - DataIn.base);
//	float coeff = radius2 / length2;
//	float h = dot(DataIn.end - DataIn.base, DataIn.axis);
//
//	// Now calculate distance to the intersecting point from ray origin
//	// See Ray tracing cones here: https://www.cl.cam.ac.uk/teaching/1999/AGraphHCI/SMAG/node2.html#eqn:rectray
//
//	// Direction of the ray in cone space
//	vec3 D = vec3(dot(DataIn.U, view_dir), dot(DataIn.V, view_dir), dot(DataIn.axis, view_dir));
//	// Now the intersection is between z-axis aligned cone and a ray
//	float a = D.x * D.x + D.y * D.y - D.z * D.z * coeff;
//	float b = 2.0 * (P.x * D.x + P.y * D.y - coeff * (P.z * D.z - h * D.z));
//	float c = P.x * P.x + P.y * P.y - coeff * (P.z - h) * (P.z - h);
//	// calculate a discriminant of the above quadratic equation
//	float d = b*b - 4 * a*c;
//	if (d < 0.0)		// outside of the cone
//		discard;
//
//	// point of intersection on cone surface
//	float dist = (-b + sqrt(d)) / (2 * a);
//	vec3 new_point = DataIn.point + dist * view_dir;
//
//	// now compute the normal of the new point (for computing the lighting).
//	vec3 tangent = cross(new_point - DataIn.base, new_point - DataIn.end);
//	vec3 normal = cross(new_point - DataIn.end, tangent);
//	normal = normalize(normal);
//
//	// to calculate caps, simply check the angle between the following two vectors:
//	//		- (point of intersection) - (the cone's base center)
//	//		- cap plane normal (which is the cone axis)
//	// if the angle < 0, the point is outside of cone
//	float cap_test = dot((new_point - DataIn.base), DataIn.axis);
//
//
//	// Now I need to determine if the projection of the cone's peak (onto the base plane) is within the base disc
//	float tt = dot(DataIn.base - DataIn.end, -DataIn.axis) / dot(view_dir, -DataIn.axis);
//	vec3 projection = DataIn.end + view_dir * tt;
//	bool within_base = dot(projection - DataIn.base, projection - DataIn.base) < radius2;
//
//
//	if (cap_test < 0.0 || (within_base && dot(view_dir, DataIn.axis) < 0.0))
//	{
//		normal = -DataIn.axis;
//
//		// ray-plane intersection
//		float t = dot(DataIn.base - DataIn.point, normal) / dot(view_dir, normal);
//		new_point = DataIn.point + view_dir * t;
//
//		// within the cap radius?
//		if (dot(new_point - DataIn.base, new_point - DataIn.base) > radius2)
//			discard;
//	}
//
//	// compute the depth. I can do it easier
//	//vec4 pos = PROJ * vec4(new_point, 1.0);
//	//float depth = 0.5*(pos.z / pos.w) + 0.5;
//	vec2 clipZW = new_point.z * PROJ[2].zw + PROJ[3].zw;
//	float depth = 0.5 + 0.5 * clipZW.x / clipZW.y;
//
//	// this is a workaround necessary for Mac, otherwise the modified fragment won't clip properly
//	if (depth <= 0.0 || depth >= 1.0)
//		discard;
//
//	gl_FragDepth = depth;
//
//	vec3 light_dir = normalize(eLightPos);
//
//	vec3 color = texture(textureID, gOutTexcoord).rgb;
//	vec3 final_color = ComputeLight(normal, light_dir, view_dir, ambient, specular, shininess, color);
//	outputF = vec4(final_color, 1.0);
//})";
//
//
//std::string lines_cones_texture_geom = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//// It uses geometry shader for vertex generation.
//// The code does not cover round caps.
//
////#extension GL_EXT_geometry_shader4 : enable
//
//layout(lines) in;
//layout(triangle_strip, max_vertices = 5) out;
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//uniform bool	perspective;
//uniform float	radius;
//
//in  vec2 vOutTexcoord[];
//
//out vec2 gOutTexcoord;
//
//out Data{
//	vec3 point;	// camera space
//	vec3 axis;
//	vec3 base;
//	vec3 end;	// the peak of the cone
//	vec3 U;
//	vec3 V;
//} DataOut;
//
//
//void main()
//{
//	vec3  base = gl_in[0].gl_Position.xyz;  // in camera space
//	vec3  top = gl_in[1].gl_Position.xyz;   // in camera space
//	vec3  len_dir = top - base;
//	vec3  view_dir = vec3(0,0,1);	// this is view direction for orthographic mode in camera space
//	if (perspective) 
//		view_dir = normalize(vec3(0) - base);	// camera pos is (0, 0, 0) in camera space
//
//	vec3 left = cross(view_dir, len_dir);
//	vec3 up = cross(left, len_dir);
//	DataOut.axis = normalize(len_dir);
//	DataOut.U = normalize(up);
//	DataOut.V = normalize(left);
//	left = radius * DataOut.V;
//	up = radius * DataOut.U;
//
//	DataOut.base = base;
//	DataOut.end = top;
//
//	float b = dot(view_dir, len_dir);
//	if (b < 0) {
//		// Vertex 1
//		DataOut.point = top;
//		gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//		if (clippingPlaneEnabled) {
//			gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//			if (crossSectionEnabled)
//				gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//		}
//		gOutTexcoord = vOutTexcoord[1];
//		EmitVertex();
//
//		// Vertex 2
//		DataOut.point = (base + left - up);
//		gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//		if (clippingPlaneEnabled) {
//			gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//			if (crossSectionEnabled)
//				gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//		}
//		gOutTexcoord = vOutTexcoord[0];
//		EmitVertex();
//
//		// Vertex 3
//		DataOut.point = (base - left - up);
//		gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//		if (clippingPlaneEnabled) {
//			gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//			if (crossSectionEnabled)
//				gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//		}
//		gOutTexcoord = vOutTexcoord[0];
//		EmitVertex();
//
//		// Vertex 4
//		DataOut.point = (base + left + up);
//		gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//		if (clippingPlaneEnabled) {
//			gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//			if (crossSectionEnabled)
//				gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//		}
//		gOutTexcoord = vOutTexcoord[0];
//		EmitVertex();
//
//		// Vertex 5
//		DataOut.point = (base - left + up);
//		gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//		if (clippingPlaneEnabled) {
//			gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//			if (crossSectionEnabled)
//				gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//		}
//		gOutTexcoord = vOutTexcoord[0];
//		EmitVertex();
//	}
//	else {
//		// Vertex 1
//		DataOut.point = top;
//		gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//		if (clippingPlaneEnabled) {
//			gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//			if (crossSectionEnabled)
//				gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//		}
//		gOutTexcoord = vOutTexcoord[1];
//		EmitVertex();
//
//		// Liangliang: determining the minimum extend for this case is tricky, so I just use 
//		//             an enlarge factor to make sure all the entire one is covered. 
//		float factor = 1.1; // It seems 1.1 is large enough
//
//		// Vertex 2
//		DataOut.point = (base - left * factor + up);	//
//		gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//		if (clippingPlaneEnabled) {
//			gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//			if (crossSectionEnabled)
//				gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//		}
//		gOutTexcoord = vOutTexcoord[0];
//		EmitVertex();
//
//		// Vertex 3
//		DataOut.point = (base + left * factor + up);	// 1.2 is just 
//		gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//		if (clippingPlaneEnabled) {
//			gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//			if (crossSectionEnabled)
//				gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//		}
//		gOutTexcoord = vOutTexcoord[0];
//		EmitVertex();
//
//		// Vertex 4
//		DataOut.point = (base - left - up);
//		gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//		if (clippingPlaneEnabled) {
//			gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//			if (crossSectionEnabled)
//				gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//		}
//		gOutTexcoord = vOutTexcoord[0];
//		EmitVertex();
//
//		// Vertex 5
//		DataOut.point = (base + left - up);
//		gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//		if (clippingPlaneEnabled) {
//			gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//			if (crossSectionEnabled)
//				gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//		}
//		gOutTexcoord = vOutTexcoord[0];
//		EmitVertex();
//	}
//
//	EndPrimitive();
//})";
//
//
//std::string lines_cones_texture_vert = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//// It uses geometry shader for vertex generation.
//// The code does not cover round caps.
//
////#extension GL_EXT_gpu_shader4 : enable
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//in  vec3 vtx_position;	// point position
//in  vec2 vtx_texcoord;
//
//out vec2 vOutTexcoord;
//
//void main()
//{
//	gl_Position = MV * MANIP * vec4(vtx_position, 1.0);
//
//	vOutTexcoord = vtx_texcoord;
//})";
//
//
//
//
//std::string lines_cylinders_color_frag = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//// It uses geometry shader for vertex generation.
//// The code does not cover round caps.
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//uniform bool	perspective;
//
//uniform vec3	color;
//uniform float	radius;
//
//layout(std140) uniform Lighting {
//	vec3	wLightPos;
//	vec3	eLightPos;
//	vec3	wCamPos;
//	vec3	ambient;		// in [0, 1], r==g==b;
//	vec3	specular;		// in [0, 1], r==g==b;
//	float	shininess;
//};
//
//in Data{
//	vec3 point;// camera space
//	vec3 axis;
//	vec3 base;
//	vec3 end;
//	vec3 U;
//	vec3 V;
//} DataIn;
//
//in  vec3 gOutColor;
//
//out vec4 outputF;
//
//
//vec3 ComputeLight(vec3 N, vec3 L, vec3 V, vec3 amb, vec3 spec, float sh, vec3 color){
//	float df = max(dot(N, L), 0);
//
//	float sf = 0.0;	// specular factor
//	if (df > 0.0) {	// if the vertex is lit compute the specular color
//		vec3 half_vector = normalize(L + V);	// compute the half vector
//		sf = max(dot(half_vector, N), 0.0);
//		sf = pow(sf, sh);
//	}
//
//	return color * df + spec * sf + amb;
//}
//
//void main()
//{
//	// First of all, I need the correct point that we're pointing at
//
//	vec3 view_dir = vec3(0, 0, 1);	// this is view direction for orthographic mode in camera space
//	if (perspective) {
//		view_dir = normalize(vec3(0) - DataIn.point);	// camera pos is (0, 0, 0) in camera space
//	}
//
//	// basis = local coordinate system of cylinder
//	mat3 basis = mat3(DataIn.U, DataIn.V, DataIn.axis);
//	vec3 diff = DataIn.point - DataIn.base;
//	vec3 P = diff * basis;
//
//	float radius2 = radius*radius;
//
//	// Now calculate distance to the cylinder from ray origin
//	// See Ray tracing cylinders here: https://www.cl.cam.ac.uk/teaching/1999/AGraphHCI/SMAG/node2.html#eqn:rectray
//
//	// Direction of the ray in cylinder space
//	vec3 D = vec3(dot(DataIn.U, view_dir), dot(DataIn.V, view_dir), dot(DataIn.axis, view_dir));
//	// Now the intersection is between z-axis aligned cylinder and a ray
//	float a = D.x * D.x + D.y * D.y;
//	float b = 2.0 * (P.x * D.x + P.y * D.y);
//	float c = P.x * P.x + P.y * P.y - radius2;
//	// calculate a discriminant of the above quadratic equation
//	float d = b*b - 4*a*c;
//	if (d < 0.0)		// outside of the cylinder
//		discard;
//
//	float dist = (-b + sqrt(d)) / (2 * a);
//
//	// point of intersection on cylinder surface
//	vec3 new_point = DataIn.point + dist * view_dir;
//
//	// now compute the normal of the new point (for computing the lighting).
//	vec3 tmp_point = new_point - DataIn.base;
//	vec3 normal = normalize(tmp_point - DataIn.axis * dot(tmp_point, DataIn.axis));
//
//	// to calculate caps, simply check the angle between the following two vectors:
//	//		- (point of intersection) - (one of the cylinder's end points)
//	//		- cap plane normal (which is the cylinder axis)
//	// if the angle < 0, the point is outside of cylinder
//	float cap_test = dot((new_point - DataIn.base), DataIn.axis);
//
//	// test front cap
//	if (cap_test < 0.0) {
//		normal = -DataIn.axis;
//
//		// ray-plane intersection
//		float t = dot(DataIn.base - DataIn.point, normal) / dot(view_dir, normal);
//		new_point = DataIn.point + view_dir * t;
//
//		// within the cap radius?
//		if (dot(new_point - DataIn.base, new_point - DataIn.base) > radius2)
//			discard;
//	}
//
//	// test end cap
//	cap_test = dot((new_point - DataIn.end), DataIn.axis);
//	if (cap_test > 0.0) {
//		normal = DataIn.axis;
//
//		// ray-plane intersection
//		float t = dot(DataIn.end - DataIn.point, normal) / dot(view_dir, normal);
//		new_point = DataIn.point + view_dir * t;
//
//		// within the cap radius?
//		if (dot(new_point - DataIn.end, new_point - DataIn.end) > radius2)
//			discard;
//	}
//
//	// compute the depth. I can do it easier
//	//vec4 pos = PROJ * vec4(new_point, 1.0);
//	//float depth = 0.5*(pos.z / pos.w) + 0.5;
//	vec2 clipZW = new_point.z * PROJ[2].zw + PROJ[3].zw;
//	float depth = 0.5 + 0.5 * clipZW.x / clipZW.y;
//
//	// this is a workaround necessary for Mac, otherwise the modified fragment won't clip properly
//	if (depth <= 0.0 || depth >= 1.0)
//		discard;
//
//	gl_FragDepth = depth;
//
//	vec3 light_dir = normalize(eLightPos);
//	vec3 final_color = ComputeLight(normal, light_dir, view_dir, ambient, specular, shininess, gOutColor);
//	outputF = vec4(final_color, 1.0);
//})";
//
//
//std::string lines_cylinders_color_geom = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//// It uses geometry shader for vertex generation.
//// The code does not cover round caps.
//
////#extension GL_EXT_geometry_shader4 : enable
//
//layout(lines) in;
//layout(triangle_strip, max_vertices = 6) out;
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//uniform bool	perspective;
//uniform float	radius;
//
//in  vec3 vOutcolor[];
//
//out vec3 gOutColor;
//
//out Data{
//	vec3 point;// camera space
//	vec3 axis;
//	vec3 base;
//	vec3 end;
//	vec3 U;
//	vec3 V;
//} DataOut;
//
//
//
//void main()
//{
//	vec3  base = gl_in[0].gl_Position.xyz;  // in camera space
//	vec3  top = gl_in[1].gl_Position.xyz;   // in camera space
//	vec3  len_dir = top - base;
//	vec3  view_dir = vec3(0,0,1);	// this is view direction for orthographic mode in camera space
//	if (perspective) {
//		//view_dir = normalize(vec3(0) - 0.5*(base + top));	// camera pos is (0, 0, 0) in camera space
//		view_dir = normalize(vec3(0) - base);	// camera pos is (0, 0, 0) in camera space
//	}
//
//	vec3 base_color = vOutcolor[0];
//	vec3 top_color = vOutcolor[1];
//
//	float b = dot(view_dir, len_dir);
//	if (b < 0.0) {// direction vector looks away, so flip
//		len_dir = -len_dir;
//		//swap(base, top);
//		vec3 tmp = base;
//		base = top;
//		top = tmp;
//		
//		base_color = vOutcolor[1];
//		top_color = vOutcolor[0];
//	}
//
//	vec3 left = cross(view_dir, len_dir);
//	vec3 up = cross(left, len_dir);
//	DataOut.axis = normalize(len_dir);
//	DataOut.U = normalize(up);
//	DataOut.V = normalize(left);
//	left = radius * DataOut.V;
//	up = radius * DataOut.U;
//
//	DataOut.base = base;
//	DataOut.end = top;
//
//	// Vertex 1
//	DataOut.point = (base + left - up);
//	gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//	}
//	gOutColor = base_color;
//	EmitVertex();
//
//	// Vertex 2
//	DataOut.point = (base - left - up);
//	gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//	}
//	gOutColor = base_color;
//	EmitVertex();
//
//	// Vertex 3
//	DataOut.point = (top + left - up);
//	gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//	}
//	gOutColor = top_color;
//	EmitVertex();
//
//	// Vertex 4
//	DataOut.point = (top - left - up);
//	gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//	}
//	gOutColor = top_color;
//	EmitVertex();
//
//	// Vertex 5
//	DataOut.point = (top + left + up);
//	gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//	}
//	gOutColor = top_color;
//	EmitVertex();
//
//	// Vertex 6
//	DataOut.point = (top - left + up);
//	gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//	}
//	gOutColor = top_color;
//	EmitVertex();
//
//	EndPrimitive();
//})";
//
//
//std::string lines_cylinders_color_vert = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//// It uses geometry shader for vertex generation.
//// The code does not cover round caps.
//
////#extension GL_EXT_gpu_shader4 : enable
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//uniform vec3	default_color;
//uniform bool	per_vertex_color;
//
//in  vec3 vtx_position;	// point position
//in  vec3 vtx_color;		// point color
//
//out vec3 vOutcolor;
//
//void main()
//{
//	gl_Position = MV * MANIP * vec4(vtx_position, 1.0);
//
//	if (per_vertex_color)
//		vOutcolor = vtx_color;
//	else
//		vOutcolor = default_color;
//})";
//
//
//
//
//std::string lines_cylinders_texture_frag = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//// It uses geometry shader for vertex generation.
//// The code does not cover round caps.
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//uniform sampler2D	textureID;
//
//uniform bool	perspective;
//
//uniform vec3	color;
//uniform float	radius;
//
//layout(std140) uniform Lighting {
//	vec3	wLightPos;
//	vec3	eLightPos;
//	vec3	wCamPos;
//	vec3	ambient;		// in [0, 1], r==g==b;
//	vec3	specular;		// in [0, 1], r==g==b;
//	float	shininess;
//};
//
//in Data{
//	vec3 point;// camera space
//	vec3 axis;
//	vec3 base;
//	vec3 end;
//	vec3 U;
//	vec3 V;
//} DataIn;
//
//in  vec2 gOutTexcoord;
//
//out vec4 outputF;
//
//
//vec3 ComputeLight(vec3 N, vec3 L, vec3 V, vec3 amb, vec3 spec, float sh, vec3 color){
//	float df = max(dot(N, L), 0);
//
//	float sf = 0.0;	// specular factor
//	if (df > 0.0) {	// if the vertex is lit compute the specular color
//		vec3 half_vector = normalize(L + V);	// compute the half vector
//		sf = max(dot(half_vector, N), 0.0);
//		sf = pow(sf, sh);
//	}
//
//	return color * df + spec * sf + amb;
//}
//
//void main()
//{
//	// First of all, I need the correct point that we're pointing at
//
//	vec3 view_dir = vec3(0, 0, 1);	// this is view direction for orthographic mode in camera space
//	if (perspective) {
//		view_dir = normalize(vec3(0) - DataIn.point);	// camera pos is (0, 0, 0) in camera space
//	}
//
//	// basis = local coordinate system of cylinder
//	mat3 basis = mat3(DataIn.U, DataIn.V, DataIn.axis);
//	vec3 diff = DataIn.point - DataIn.base;
//	vec3 P = diff * basis;
//
//	float radius2 = radius*radius;
//
//	// Now calculate distance to the cylinder from ray origin
//	// See Ray tracing cylinders here: https://www.cl.cam.ac.uk/teaching/1999/AGraphHCI/SMAG/node2.html#eqn:rectray
//
//	// Direction of the ray in cylinder space
//	vec3 D = vec3(dot(DataIn.U, view_dir), dot(DataIn.V, view_dir), dot(DataIn.axis, view_dir));
//	// Now the intersection is between z-axis aligned cylinder and a ray
//	float a = D.x * D.x + D.y * D.y;
//	float b = 2.0 * (P.x * D.x + P.y * D.y);
//	float c = P.x * P.x + P.y * P.y - radius2;
//	// calculate a discriminant of the above quadratic equation
//	float d = b*b - 4*a*c;
//	if (d < 0.0)		// outside of the cylinder
//		discard;
//
//	float dist = (-b + sqrt(d)) / (2 * a);
//
//	// point of intersection on cylinder surface
//	vec3 new_point = DataIn.point + dist * view_dir;
//
//	// now compute the normal of the new point (for computing the lighting).
//	vec3 tmp_point = new_point - DataIn.base;
//	vec3 normal = normalize(tmp_point - DataIn.axis * dot(tmp_point, DataIn.axis));
//
//	// to calculate caps, simply check the angle between the following two vectors:
//	//		- (point of intersection) - (one of the cylinder's end points)
//	//		- cap plane normal (which is the cylinder axis)
//	// if the angle < 0, the point is outside of cylinder
//	float cap_test = dot((new_point - DataIn.base), DataIn.axis);
//
//	// test front cap
//	if (cap_test < 0.0) {
//		normal = -DataIn.axis;
//
//		// ray-plane intersection
//		float t = dot(DataIn.base - DataIn.point, normal) / dot(view_dir, normal);
//		new_point = DataIn.point + view_dir * t;
//
//		// within the cap radius?
//		if (dot(new_point - DataIn.base, new_point - DataIn.base) > radius2)
//			discard;
//	}
//
//	// test end cap
//	cap_test = dot((new_point - DataIn.end), DataIn.axis);
//	if (cap_test > 0.0) {
//		normal = DataIn.axis;
//
//		// ray-plane intersection
//		float t = dot(DataIn.end - DataIn.point, normal) / dot(view_dir, normal);
//		new_point = DataIn.point + view_dir * t;
//
//		// within the cap radius?
//		if (dot(new_point - DataIn.end, new_point - DataIn.end) > radius2)
//			discard;
//	}
//
//	// compute the depth. I can do it easier
//	//vec4 pos = PROJ * vec4(new_point, 1.0);
//	//float depth = 0.5*(pos.z / pos.w) + 0.5;
//	vec2 clipZW = new_point.z * PROJ[2].zw + PROJ[3].zw;
//	float depth = 0.5 + 0.5 * clipZW.x / clipZW.y;
//
//	// this is a workaround necessary for Mac, otherwise the modified fragment won't clip properly
//	if (depth <= 0.0 || depth >= 1.0)
//		discard;
//
//	gl_FragDepth = depth;
//
//	vec3 light_dir = normalize(eLightPos);
//
//	vec3 color = texture(textureID, gOutTexcoord).rgb;
//	vec3 final_color = ComputeLight(normal, light_dir, view_dir, ambient, specular, shininess, color);
//	outputF = vec4(final_color, 1.0);
//})";
//
//
//std::string lines_cylinders_texture_geom = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//// It uses geometry shader for vertex generation.
//// The code does not cover round caps.
//
////#extension GL_EXT_geometry_shader4 : enable
//
//layout(lines) in;
//layout(triangle_strip, max_vertices = 6) out;
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//uniform bool	perspective;
//uniform float	radius;
//
//in  vec2 vOutTexcoord[];
//
//out vec2 gOutTexcoord;
//
//out Data{
//	vec3 point;// camera space
//	vec3 axis;
//	vec3 base;
//	vec3 end;
//	vec3 U;
//	vec3 V;
//} DataOut;
//
//
//
//void main()
//{
//	vec3  base = gl_in[0].gl_Position.xyz;  // in camera space
//	vec3  top = gl_in[1].gl_Position.xyz;   // in camera space
//	vec3  len_dir = top - base;
//	vec3  view_dir = vec3(0,0,1);	// this is view direction for orthographic mode in camera space
//	if (perspective) {
//		//view_dir = normalize(vec3(0) - 0.5*(base + top));	// camera pos is (0, 0, 0) in camera space
//		view_dir = normalize(vec3(0) - base);	// camera pos is (0, 0, 0) in camera space
//	}
//
//	vec2 base_texcoord = vOutTexcoord[0];
//	vec2 top_texcoord = vOutTexcoord[1];
//
//	float b = dot(view_dir, len_dir);
//	if (b < 0.0) {// direction vector looks away, so flip
//		len_dir = -len_dir;
//		//swap(base, top);
//		vec3 tmp = base;
//		base = top;
//		top = tmp;
//		
//		base_texcoord = vOutTexcoord[1];
//		top_texcoord = vOutTexcoord[0];
//	}
//
//	vec3 left = cross(view_dir, len_dir);
//	vec3 up = cross(left, len_dir);
//	DataOut.axis = normalize(len_dir);
//	DataOut.U = normalize(up);
//	DataOut.V = normalize(left);
//	left = radius * DataOut.V;
//	up = radius * DataOut.U;
//
//	DataOut.base = base;
//	DataOut.end = top;
//
//	// Vertex 1
//	DataOut.point = (base + left - up);
//	gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//	}
//	gOutTexcoord = base_texcoord;
//	EmitVertex();
//
//	// Vertex 2
//	DataOut.point = (base - left - up);
//	gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//	}
//	gOutTexcoord = base_texcoord;
//	EmitVertex();
//
//	// Vertex 3
//	DataOut.point = (top + left - up);
//	gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//	}
//	gOutTexcoord = top_texcoord;
//	EmitVertex();
//
//	// Vertex 4
//	DataOut.point = (top - left - up);
//	gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//	}
//	gOutTexcoord = top_texcoord;
//	EmitVertex();
//
//	// Vertex 5
//	DataOut.point = (top + left + up);
//	gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//	}
//	gOutTexcoord = top_texcoord;
//	EmitVertex();
//
//	// Vertex 6
//	DataOut.point = (top - left + up);
//	gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//	}
//	gOutTexcoord = top_texcoord;
//	EmitVertex();
//
//	EndPrimitive();
//})";
//
//
//std::string lines_cylinders_texture_vert = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//// It uses geometry shader for vertex generation.
//// The code does not cover round caps.
//
////#extension GL_EXT_gpu_shader4 : enable
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//in  vec3 vtx_position;	// point position
//in  vec2 vtx_texcoord;
//
//out vec2 vOutTexcoord;
//
//void main()
//{
//	gl_Position = MV * MANIP * vec4(vtx_position, 1.0);
//
//	vOutTexcoord = vtx_texcoord;
//})";
//
//
//
//
//
//std::string lines_plain_texture_frag = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//uniform sampler2D	textureID;
//
//in  vec2 vOutTexcoord;
//
//out vec4 outputF;
//
//void main()
//{
//	outputF = texture(textureID, vOutTexcoord);
//
//	// the following line is required for fxaa (will not work with blending!)
//	//outputF.a = dot(outputF.rgb, vec3(0.299, 0.587, 0.114));
//})";
//
//
//std::string lines_plain_texture_vert = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//in  vec3 vtx_position;	// point position
//in  vec2 vtx_texcoord;
//
//out vec2 vOutTexcoord;
//
//void main()
//{
//	vec4 new_position = MANIP * vec4(vtx_position, 1.0);
//
//	gl_Position = MVP * new_position;
//
//	vOutTexcoord = vtx_texcoord;
//
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(new_position, clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(new_position, clippingPlane1);
//	}
//})";
//
//
//
//
//std::string mesh_effect_glass_absorption_frag = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//
//uniform sampler2D textureID;
//uniform vec3 color;
//
//in vec2 tcoordVC;
//
//out vec4 FragColor;
//
//void main()
//{
//	float thickness = abs(texture2D(textureID, tcoordVC).r);
//    if (thickness <= 0.0)
//        discard;
//
//    float sigma = 10.0;
//	float fresnel = 1.0 - texture2D(textureID, tcoordVC).g;
//    float intensity = fresnel * exp(-sigma * thickness);
//	FragColor = vec4(intensity * color, 1.0);
//})";
//
//
//std::string mesh_effect_glass_absorption_vert = R"(
//#version 150
//
//in	vec3	vertexMC;
//in	vec2	tcoordMC;
//
//out	vec2	tcoordVC;
//
//void main()
//{
//	tcoordVC = tcoordMC;
//	gl_Position = vec4(vertexMC, 1.0);
//}
//)";
//
//
//
//
//std::string mesh_effect_glass_depth_frag = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//uniform float DepthScale;
//
//in vec3 vNormal;
//in vec3 vPosition;
//
//out vec4 FragColor;
//
//void main()
//{
//    vec3 N = normalize(vNormal);
//    vec3 P = vPosition;
//    vec3 I = normalize(P);
//    float cosTheta = abs(dot(I, N));
//    float fresnel = pow(1.0 - cosTheta, 4.0);
//
//    float depth = DepthScale * gl_FragCoord.z;
//
//    FragColor = vec4(depth, fresnel, 0, 0);
//})";
//
//
//std::string mesh_effect_glass_depth_vert = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//in vec3 vtx_position;
//in vec3 vtx_normal;
//
//uniform mat4 MV;
//uniform mat4 MVP;
//uniform mat3 NORMAL;
//
//out vec3 vNormal;
//out vec3 vPosition;
//
//
//void main()
//{
//	vPosition = (MV * vec4(vtx_position, 1.0)).xyz;
//	vNormal = NORMAL * vtx_normal;
//	gl_Position = MVP * vec4(vtx_position, 1.0);
//}
//
//)";
//
//
//
//
//std::string mesh_effect_toon_frag = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//
//layout(std140) uniform Lighting {
//	vec3	wLightPos;
//	vec3	eLightPos;
//	vec3	wCamPos;
//	vec3	ambient;		// in [0, 1], r==g==b;
//	vec3	specular;		// in [0, 1], r==g==b;
//	float	shininess;
//};
//
//uniform vec3 color;
//uniform bool two_sides_lighting;
//
//in Data{
//	vec3 normal;
//	vec3 position;
//} DataIn;
//
//
//out vec4 outputF;
//
//
//float stepmix(float edge0, float edge1, float E, float x)
//{
//    float T = clamp(0.5 * (x - edge0 + E) / E, 0.0, 1.0);
//    return mix(edge0, edge1, T);
//}
//
//void main()
//{
//	vec3 normal = normalize(DataIn.normal);
//
//	vec3 view_dir = normalize(wCamPos - DataIn.position);
//	vec3 light_dir = normalize(wLightPos);
//
//	float df = 0.0;	// diffuse factor
//	if (two_sides_lighting)
//		df = abs(dot(light_dir, normal));
//	else
//		df = max(dot(light_dir, normal), 0);
//
//	float sf = 0.0;	// specular factor
//	if (df > 0.0) {	// if the vertex is lit compute the specular color
//		vec3 half_vector = normalize(light_dir + view_dir);	// compute the half vector
//
//		if (two_sides_lighting)
//			sf = abs(dot(half_vector, normal));
//		else
//			sf = max(dot(half_vector, normal), 0.0);
//
//		sf = pow(sf, shininess);
//	}
//
//    const float A = 0.1;
//    const float B = 0.3;
//    const float C = 0.6;
//    const float D = 1.0;
//    float E = fwidth(df);
//
//    if      (df > A - E && df < A + E) df = stepmix(A, B, E, df);
//    else if (df > B - E && df < B + E) df = stepmix(B, C, E, df);
//    else if (df > C - E && df < C + E) df = stepmix(C, D, E, df);
//    else if (df < A) df = 0.0;
//    else if (df < B) df = B;
//    else if (df < C) df = C;
//    else df = D;
//
//    E = fwidth(sf);
//    if (sf > 0.5 - E && sf < 0.5 + E) {
//		// Hermite interpolation between two values
//        sf = smoothstep(0.5 - E, 0.5 + E, sf);
//    }
//    else
//        sf = step(0.5, sf);
//
//	outputF = vec4(df * color + sf * specular + ambient, 1.0);
//})";
//
//
//std::string mesh_effect_toon_vert = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//in vec3 vtx_position;
//in vec3 vtx_normal;
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//// the data to be sent to the fragment shader
//out Data{
//	vec3 normal;
//	vec3 position;
//} DataOut;
//
//
//void main()
//{
//	DataOut.normal = vtx_normal;
//	DataOut.position = vec4(vtx_position, 1.0).xyz;
//
//	gl_Position = MVP * vec4(vtx_position, 1.0);
//})";
//
//
//
//
//std::string pointcloud_group_frag = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//in	vec3 vsColor;
//out vec4 outputF;
//
//
//void main(void) {
//	outputF = vec4(vsColor, 1.0f);
//})";
//
//
//std::string pointcloud_group_vert = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//in  vec3 vtx_position;	// point position
//in  vec3 vtx_color;		// point color
//in	vec3 vtx_normal;	// point normal
//
//uniform bool lighting;		// true if lighting is on
//uniform bool has_normal;	// true if data has information
//	
//layout(std140) uniform Matrices{
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//};
//
//uniform bool    clippingPlaneEnabled;
//uniform bool    crossSectionEnabled;
//uniform vec4	clippingPlane0;
//uniform vec4	clippingPlane1;
//
//layout(std140) uniform Lighting{
//	vec3	light_pos;		// model space
//	vec3	cam_pos;		// model space
//	vec3	ambient;		// in [0, 1], r==g==b;
//	vec3	specular;		// in [0, 1], r==g==b;
//	float	shininess;
//};
//
//uniform bool two_sides_lighting;
//
//// the data to be sent to the fragment shader
//out vec3 vsColor;
//
//void main(void) {
//
//	vec4 new_position = MANIP * vec4(vtx_position, 1.0);
//
//	if (lighting && has_normal) {
//		vec3 normal = normalize(NORMAL * vtx_normal);
//		vec3 light_dir = normalize(light_pos);
//		float df = 0.0;	// diffuse factor
//		if (two_sides_lighting)
//			df = abs(dot(light_dir, normal));
//		else
//			df = max(dot(light_dir, normal), 0);
//
//		float sf = 0.0;	// specular factor
//		if (df > 0.0) {	// if the vertex is lit compute the specular color
//			vec3 view_dir = normalize(cam_pos - new_position.xyz);	// compute view direction and normalize it 
//			vec3 half_vector = normalize(light_dir + view_dir);				// compute the half vector
//
//			if (two_sides_lighting)
//				sf = abs(dot(half_vector, normal));
//			else
//				sf = max(dot(half_vector, normal), 0.0);
//
//			sf = pow(sf, shininess);
//		}
//
//		vsColor = vtx_color * df + specular * sf + ambient;
//	}
//	else {
//		vsColor = vtx_color;
//	}
//
//	gl_Position = MVP * new_position;
//
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(new_position, clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(new_position, clippingPlane1);
//	}
//})";
//
//
//
//
//std::string points_plain_texture_frag = R"(
//#version 430
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//uniform sampler2D	textureID;	
//uniform bool lighting;	// true if lighting is on
//
//layout(std140) uniform Lighting {
//	vec3	wLightPos;
//	vec3	eLightPos;
//	vec3	wCamPos;
//	vec3	ambient;		// in [0, 1], r==g==b;
//	vec3	specular;		// in [0, 1], r==g==b;
//	float	shininess;
//};
//
//uniform bool two_sides_lighting;
//
//uniform int	 hightlight_id;
//uniform bool selected;
//
//uniform bool distinct_back_color;
//uniform vec3 back_color;
//
//in Data{
//	vec3 position;
//	vec2 texcoord;
//	vec3 normal;
//} DataIn;
//
//layout(std430, binding = 1) buffer selection_t {
//	uint data[];
//} selection;
//
//
//out vec4 outputF;	// Ouput data
//
//
//void main() 
//{
//	vec3 color = texture(textureID, DataIn.texcoord).rgb;
//
//	if (lighting) {
//		vec3 normal = normalize(DataIn.normal);
//		vec3 light_dir = normalize(wLightPos);
//		vec3 view_dir = normalize(wCamPos - DataIn.position);	// compute view direction and normalize it 
//
//		if (distinct_back_color) {
//			if (dot(normal, view_dir) < 0)
//				color = back_color;
//		}
//
//		float df = 0.0;	// diffuse factor
//		if (two_sides_lighting)
//			df = abs(dot(light_dir, normal));
//		else
//			df = max(dot(light_dir, normal), 0);
//
//		float sf = 0.0;	// specular factor
//		if (df > 0.0) {	// if the vertex is lit compute the specular color
//			vec3 half_vector = normalize(light_dir + view_dir);	// compute the half vector
//
//			if (two_sides_lighting)
//				sf = abs(dot(half_vector, normal));
//			else
//				sf = max(dot(half_vector, normal), 0.0);
//
//			sf = pow(sf, shininess);
//		}
//
//		color = color * df + specular * sf + ambient;
//	}
//
//	//uint addr = gl_PrimitiveID / 32;
//	//uint offs = gl_PrimitiveID % 32;
//	uint addr = gl_PrimitiveID >> 5;
//	uint offs = gl_PrimitiveID & 31;
//	if ((selection.data[addr] & (1 << offs)) != 0)
//		color = mix(color, vec3(1.0, 0.0, 0.0), 0.8);
//	else if (gl_PrimitiveID == hightlight_id)
//		color = vec3(1.0, 0.0, 0.0);
//
//	if (selected) {
//		color = vec3(1.0, 0.0, 0.0);
//	}
//
//	outputF = vec4(color, 1.0);
//})";
//
//
//std::string points_plain_texture_vert = R"(
//#version 430
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//in vec3 vtx_position;
//in vec2 vtx_texcoord;
//in vec3 vtx_normal;
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//// the data to be sent to the fragment shader
//out Data{
//	vec3 position;
//	vec2 texcoord;
//	vec3 normal;
//} DataOut;
//
//
//void main() {
//
//	vec4 new_position = MANIP * vec4(vtx_position, 1.0);
//
//	DataOut.texcoord = vtx_texcoord;
//	DataOut.normal = NORMAL * vtx_normal;
//
//	// Output position of the vertex, in clip space : MVP * position
//	gl_Position = MVP * new_position;
//
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(new_position, clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(new_position, clippingPlane1);
//	}
//}
//)";
//
//
//
//
//std::string points_spheres_geometry_color_frag = R"(
//#version 430 core
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//uniform bool	perspective;
//
//layout(std140) uniform Lighting {
//	vec3	wLightPos;
//	vec3	eLightPos;
//	vec3	wCamPos;
//	vec3	ambient;		// in [0, 1], r==g==b;
//	vec3	specular;		// in [0, 1], r==g==b;
//	float	shininess;
//};
//
//uniform float	sphere_radius;
//
//in Data{
//	flat	vec3	sphere_color;
//	smooth	vec2	tex;
//	flat	vec4	position;
//	//flat	float	sphere_radius;
//} DataIn;
//
//out vec4 outputF;
//
//
//void main()  
//{    
//	/*  with perspective correction
//	*   Ref: Learning Modern 3D Graphics Programming, by Jason L. McKesson
//	*	http://alfonse.bitbucket.org/oldtut/Illumination/Tut13%20Correct%20Chicanery.html
//	**/
//	if (perspective) {
//		//vec2 tex = DataIn.tex;
//		vec2 tex = DataIn.tex * 1.5; // 1.5 times larger ensuring the quad is big enought in perspective view
//
//		vec3 planePos = vec3(tex * sphere_radius, 0.0) + DataIn.position.xyz;
//		vec3 view_dir = normalize(planePos);
//		float B = 2.0 * dot(view_dir, -DataIn.position.xyz);
//		float C = dot(DataIn.position.xyz, DataIn.position.xyz) - (sphere_radius * sphere_radius);
//		float det = (B * B) - (4 * C);
//		if (det < 0.0)
//			discard;
//
//		float sqrtDet = sqrt(det);
//		float posT = (-B + sqrtDet) / 2;
//		float negT = (-B - sqrtDet) / 2;
//		float intersectT = min(posT, negT);
//		vec3 pos = view_dir * intersectT;
//		vec3 normal = normalize(pos - DataIn.position.xyz);
//
//		// compute the depth. I can do it easier
//		//vec4 pos4 = PROJ * vec4(pos, 1.0);
//		//gl_FragDepth = 0.5*(pos4.z / pos4.w) + 0.5;
//		vec2 clipZW = pos.z * PROJ[2].zw + PROJ[3].zw;
//		gl_FragDepth = 0.5 * clipZW.x / clipZW.y + 0.5;
//
//		// camera pos is (0, 0, 0) in the camera coordinate system
//		vec3 light_dir = normalize(eLightPos);
//		float df = max(dot(normal, light_dir), 0);
//
//		float sf = 0.0;	// specular factor
//		if (df > 0.0) {	// if the vertex is lit compute the specular color
//			view_dir = -view_dir; // '-' because I used the reversed direction 
//			vec3 half_vector = normalize(light_dir + view_dir);	// compute the half vector
//			sf = max(dot(half_vector, normal), 0.0);
//			sf = pow(sf, shininess);
//		}
//
//		outputF = vec4(DataIn.sphere_color * df + specular * sf + ambient, 1.0);
//	}
//
//	// without perspective correction
//	else {
//		// r^2 = (x - x0)^2 + (y - y0)^2 + (z - z0)^2
//		float x = DataIn.tex.x;
//		float y = DataIn.tex.y;
//		float zz = 1.0 - x*x - y*y;
//
//		if (zz <= 0.0)
//			discard;
//
//		float z = sqrt(zz);
//		vec4 pos = DataIn.position;
//		pos.z += sphere_radius*z;
//
//		// compute the depth. I can do it easier
//		//vec4 pos4 = PROJ * vec4(pos, 1.0);
//		//gl_FragDepth = 0.5*(pos4.z / pos4.w) + 0.5;
//		vec2 clipZW = pos.z * PROJ[2].zw + PROJ[3].zw;
//		gl_FragDepth = 0.5 * clipZW.x / clipZW.y + 0.5;
//
//		// camera pos is (0, 0, 0) in the camera coordinate system
//		vec3 view_dir = vec3(0, 0, 1);
//		vec3 light_dir = normalize(eLightPos);
//
//		vec3 normal = vec3(x, y, z); // sure this was normalized because (z = sqrt(1.0 - x*x - y*y))
//		float df = max(dot(normal, light_dir), 0);
//
//		float sf = 0.0;	// specular factor
//		if (df > 0.0) {	// if the vertex is lit compute the specular color
//			vec3 half_vector = normalize(light_dir + view_dir);	// compute the half vector
//			sf = max(dot(half_vector, normal), 0.0);
//			sf = pow(sf, shininess);
//		}
//
//		outputF = vec4(DataIn.sphere_color * df + specular * sf + ambient, 1.0);
//	}
//}
//)";
//
//
//std::string points_spheres_geometry_color_geom = R"(
//#version 430 core
//// please send comments or report bug to: liangliang.nan@gmail.com
//
////#extension GL_EXT_geometry_shader4 : enable
//
//layout(points) in;
//layout(triangle_strip, max_vertices=4) out;
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//uniform float	sphere_radius;
//
//in vec3	sphere_color_in[];
////in float	sphere_radius_in;
//
//out Data {
//	flat	vec3	sphere_color;
//	smooth	vec2	tex;
//	flat	vec4	position;
//	//flat	float	sphere_radius;
//} DataOut;
//
//
//void main()
//{
//  // Output vertex position
//  DataOut.position = MV * gl_in[0].gl_Position; 
//  DataOut.sphere_color = sphere_color_in[0];
//  //sphere_radius = sphere_radius_in;
//    
//  // Vertex 1
//  DataOut.tex = vec2(-1.0, -1.0);
//  gl_Position = DataOut.position;
//  gl_Position.xy += vec2(-sphere_radius, -sphere_radius);
//  gl_Position = PROJ  * gl_Position;
//  if (clippingPlaneEnabled) {
//	gl_ClipDistance[0] = dot(gl_in[0].gl_Position, clippingPlane0);
//	if (crossSectionEnabled)
//		gl_ClipDistance[1] = dot(gl_in[0].gl_Position, clippingPlane1);
//  }
//  EmitVertex();
//
//  // Vertex 2
//  DataOut.tex = vec2(-1.0, 1.0);
//  gl_Position = DataOut.position;
//  gl_Position.xy += vec2(-sphere_radius, sphere_radius);
//  gl_Position = PROJ  * gl_Position;
//  if (clippingPlaneEnabled) {
//	gl_ClipDistance[0] = dot(gl_in[0].gl_Position, clippingPlane0);
//	if (crossSectionEnabled)
//		gl_ClipDistance[1] = dot(gl_in[0].gl_Position, clippingPlane1);
//  }
//  EmitVertex();
//
//  // Vertex 3
//  DataOut.tex = vec2(1.0, -1.0);
//  gl_Position = DataOut.position;
//  gl_Position.xy += vec2(sphere_radius, -sphere_radius);
//  gl_Position = PROJ  * gl_Position;
//  if (clippingPlaneEnabled) {
//	gl_ClipDistance[0] = dot(gl_in[0].gl_Position, clippingPlane0);
//	if (crossSectionEnabled)
//		gl_ClipDistance[1] = dot(gl_in[0].gl_Position, clippingPlane1);
//  }
//  EmitVertex();
//
//  // Vertex 4
//  DataOut.tex = vec2(1.0, 1.0);
//  gl_Position = DataOut.position;
//  gl_Position.xy += vec2(sphere_radius, sphere_radius);
//  gl_Position = PROJ  * gl_Position;
//  if (clippingPlaneEnabled) {
//	gl_ClipDistance[0] = dot(gl_in[0].gl_Position, clippingPlane0);
//	if (crossSectionEnabled)
//		gl_ClipDistance[1] = dot(gl_in[0].gl_Position, clippingPlane1);
//  }
//  EmitVertex();
//
//  EndPrimitive();
//})";
//
//
//std::string points_spheres_geometry_color_vert = R"(
////#version 330 core   // for geometry shader to work
//#version 430 core	  // for shader storage buffer
//// please send comments or report bug to: liangliang.nan@gmail.com
//
////#extension GL_EXT_gpu_shader4 : enable
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//in vec3  vtx_position;
//in vec3  vtx_color;
////in float sphere_radius;
//
//uniform vec3	default_color;
//uniform bool	per_vertex_color;
//uniform int		hightlight_id;
//uniform bool	selected;
//
//layout(std430, binding = 1) buffer selection_t {
//	uint data[];
//} selection;
//
//out vec3	sphere_color_in;
////out float	sphere_radius_in;
//
//void main()
//{  
//	sphere_color_in = default_color;
//	if (per_vertex_color)
//		sphere_color_in = vtx_color;
//	
//	//uint addr = gl_VertexID / 32;
//	//uint offs = gl_VertexID % 32;
//	uint addr = gl_VertexID >> 5;
//	uint offs = gl_VertexID & 31;
//	if ((selection.data[addr] & (1 << offs)) != 0)
//		sphere_color_in = mix(sphere_color_in, vec3(1.0, 0.0, 0.0), 0.8);
//	else if (gl_VertexID == hightlight_id)
//		sphere_color_in = vec3(1.0, 0.0, 0.0);
//
//	if (selected)
//		sphere_color_in = vec3(1.0, 0.0, 0.0);
//
//	//sphere_radius_in = sphere_radius;
//
//	gl_Position = MANIP * vec4(vtx_position, 1.0);
//})";
//
//
//
//
//std::string points_spheres_geometry_texture_frag = R"(
//#version 430 core
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//uniform bool	perspective;
//
//uniform sampler2D	textureID;
//
//layout(std140) uniform Lighting {
//	vec3	wLightPos;
//	vec3	eLightPos;
//	vec3	wCamPos;
//	vec3	ambient;		// in [0, 1], r==g==b;
//	vec3	specular;		// in [0, 1], r==g==b;
//	float	shininess;
//};
//
//uniform float	sphere_radius;
//
//uniform int	 hightlight_id;
//uniform bool selected;
//
//layout(std430, binding = 1) buffer selection_t {
//	uint data[];
//} selection;
//
//in Data{
//	flat	vec2	texcoord;
//	smooth	vec2	tex;
//	flat	vec4	position;
//	//flat	float	sphere_radius;
//} DataIn;
//
//out vec4 outputF;
//
//
//
//void main() 
//{     
//	/*  with perspective correction
//	*   Ref: Learning Modern 3D Graphics Programming, by Jason L. McKesson
//	*	http://alfonse.bitbucket.org/oldtut/Illumination/Tut13%20Correct%20Chicanery.html
//	**/
//	if (perspective) {
//		//vec2 tex = DataIn.tex;
//		vec2 tex = DataIn.tex * 1.5; // 1.5 times larger ensuring the quad is big enought in perspective view
//
//		vec3 planePos = vec3(tex * sphere_radius, 0.0) + DataIn.position.xyz;
//		vec3 view_dir = normalize(planePos);
//		float B = 2.0 * dot(view_dir, -DataIn.position.xyz);
//		float C = dot(DataIn.position.xyz, DataIn.position.xyz) - (sphere_radius * sphere_radius);
//		float det = (B * B) - (4 * C);
//		if (det < 0.0)
//			discard;
//
//		float sqrtDet = sqrt(det);
//		float posT = (-B + sqrtDet) / 2;
//		float negT = (-B - sqrtDet) / 2;
//		float intersectT = min(posT, negT);
//		vec3 pos = view_dir * intersectT;
//		vec3 normal = normalize(pos - DataIn.position.xyz);
//
//		// compute the depth. I can do it easier
//		//vec4 pos4 = PROJ * vec4(pos, 1.0);
//		//gl_FragDepth = 0.5*(pos4.z / pos4.w) + 0.5;
//		vec2 clipZW = pos.z * PROJ[2].zw + PROJ[3].zw;
//		gl_FragDepth = 0.5 * clipZW.x / clipZW.y + 0.5;
//
//		// camera pos is (0, 0, 0) in the camera coordinate system
//		vec3 light_dir = normalize(eLightPos);
//		float df = max(dot(normal, light_dir), 0);
//
//		float sf = 0.0;	// specular factor
//		if (df > 0.0) {	// if the vertex is lit compute the specular color
//			view_dir = -view_dir; // '-' because I used the reversed direction 
//			vec3 half_vector = normalize(light_dir + view_dir);	// compute the half vector
//			sf = max(dot(half_vector, normal), 0.0);
//			sf = pow(sf, shininess);
//		}
//
//		vec3 color = texture(textureID, DataIn.texcoord).rgb;
//
//		//uint addr = gl_PrimitiveID / 32;
//		//uint offs = gl_PrimitiveID % 32;
//		uint addr = gl_PrimitiveID >> 5;
//		uint offs = gl_PrimitiveID & 31;
//		if ((selection.data[addr] & (1 << offs)) != 0)
//			color = mix(color, vec3(1.0, 0.0, 0.0), 0.8);
//		else if (gl_PrimitiveID == hightlight_id)
//			color = vec3(1.0, 0.0, 0.0);
//
//		if (selected)
//			color = vec3(1.0, 0.0, 0.0);
//
//		outputF = vec4(color * df + specular * sf + ambient, 1.0);
//	}
//
//	// without perspective correction
//	else {
//		// r^2 = (x - x0)^2 + (y - y0)^2 + (z - z0)^2
//		float x = DataIn.tex.x;
//		float y = DataIn.tex.y;
//		float zz = 1.0 - x*x - y*y;
//
//		if (zz <= 0.0)
//			discard;
//
//		float z = sqrt(zz);
//		vec4 pos = DataIn.position;
//		pos.z += sphere_radius*z;
//		pos = PROJ * pos;
//		gl_FragDepth = 0.5*(pos.z / pos.w) + 0.5;
//
//		// camera pos is (0, 0, 0) in the camera coordinate system
//		vec3 view_dir = vec3(0, 0, 1);
//		vec3 light_dir = normalize(eLightPos);
//
//		vec3 normal = vec3(x, y, z); // sure this was normalized because (z = sqrt(1.0 - x*x - y*y))
//		float df = max(dot(normal, light_dir), 0);
//
//		float sf = 0.0;	// specular factor
//		if (df > 0.0) {	// if the vertex is lit compute the specular color
//			vec3 half_vector = normalize(light_dir + view_dir);	// compute the half vector
//			sf = max(dot(half_vector, normal), 0.0);
//			sf = pow(sf, shininess);
//		}
//
//		vec3 color = texture(textureID, DataIn.texcoord).rgb;
//
//		//uint addr = gl_PrimitiveID / 32;
//		//uint offs = gl_PrimitiveID % 32;
//		uint addr = gl_PrimitiveID >> 5;
//		uint offs = gl_PrimitiveID & 31;
//		if ((selection.data[addr] & (1 << offs)) != 0)
//			color = mix(color, vec3(1.0, 0.0, 0.0), 0.8);
//		else if (gl_PrimitiveID == hightlight_id)
//			color = vec3(1.0, 0.0, 0.0);
//
//		if (selected)
//			color = vec3(1.0, 0.0, 0.0);
//
//		outputF = vec4(color * df + specular * sf + ambient, 1.0);
//	}
//}
//)";
//
//
//std::string points_spheres_geometry_texture_geom = R"(
//#version 430 core
//// please send comments or report bug to: liangliang.nan@gmail.com
//
////#extension GL_EXT_geometry_shader4 : enable
//
//layout(points) in;
//layout(triangle_strip, max_vertices = 4) out;
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//uniform float	sphere_radius;
//
//in		vec2	texcoord[];
////in float	sphere_radius_in;
//
//out Data{
//	flat	vec2	texcoord;
//	smooth	vec2	tex;
//	flat	vec4	position;
//	//flat	float	sphere_radius;
//} DataOut;
//
//
//void main()
//{
//	// Output vertex position
//	DataOut.position = MV * gl_in[0].gl_Position;
//	DataOut.texcoord = texcoord[0];
//	//sphere_radius = sphere_radius_in;
//
//	// Vertex 1
//	DataOut.tex = vec2(-1.0, -1.0);
//	gl_Position = DataOut.position;
//	gl_Position.xy += vec2(-sphere_radius, -sphere_radius);
//	gl_Position = PROJ  * gl_Position;
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(gl_in[0].gl_Position, clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(gl_in[0].gl_Position, clippingPlane1);
//	}
//	// In the geometry language, gl_PrimitiveID is an output variable that is passed to the corresponding gl_PrimitiveID input variable in 
//	// the fragment shader.If no geomery shader is present then gl_PrimitiveID in the fragment language behaves identically as it would in 
//	// the tessellation control and evaluation languages.If a geometry shader is present but does not write to gl_PrimitiveID, the value 
//	// of gl_PrimitiveID in the fragment shader is undefined.
//	gl_PrimitiveID = gl_PrimitiveIDIn;
//	EmitVertex();
//
//	// Vertex 2
//	DataOut.tex = vec2(-1.0, 1.0);
//	gl_Position = DataOut.position;
//	gl_Position.xy += vec2(-sphere_radius, sphere_radius);
//	gl_Position = PROJ  * gl_Position;
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(gl_in[0].gl_Position, clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(gl_in[0].gl_Position, clippingPlane1);
//	}
//	// In the geometry language, gl_PrimitiveID is an output variable that is passed to the corresponding gl_PrimitiveID input variable in 
//	// the fragment shader.If no geomery shader is present then gl_PrimitiveID in the fragment language behaves identically as it would in 
//	// the tessellation control and evaluation languages.If a geometry shader is present but does not write to gl_PrimitiveID, the value 
//	// of gl_PrimitiveID in the fragment shader is undefined.
//	gl_PrimitiveID = gl_PrimitiveIDIn;
//	EmitVertex();
//
//	// Vertex 3
//	DataOut.tex = vec2(1.0, -1.0);
//	gl_Position = DataOut.position;
//	gl_Position.xy += vec2(sphere_radius, -sphere_radius);
//	gl_Position = PROJ  * gl_Position;
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(gl_in[0].gl_Position, clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(gl_in[0].gl_Position, clippingPlane1);
//	}
//	// In the geometry language, gl_PrimitiveID is an output variable that is passed to the corresponding gl_PrimitiveID input variable in 
//	// the fragment shader.If no geomery shader is present then gl_PrimitiveID in the fragment language behaves identically as it would in 
//	// the tessellation control and evaluation languages.If a geometry shader is present but does not write to gl_PrimitiveID, the value 
//	// of gl_PrimitiveID in the fragment shader is undefined.
//	gl_PrimitiveID = gl_PrimitiveIDIn;
//	EmitVertex();
//
//	// Vertex 4
//	DataOut.tex = vec2(1.0, 1.0);
//	gl_Position = DataOut.position;
//	gl_Position.xy += vec2(sphere_radius, sphere_radius);
//	gl_Position = PROJ  * gl_Position;
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(gl_in[0].gl_Position, clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(gl_in[0].gl_Position, clippingPlane1);
//	}
//	// In the geometry language, gl_PrimitiveID is an output variable that is passed to the corresponding gl_PrimitiveID input variable in 
//	// the fragment shader.If no geomery shader is present then gl_PrimitiveID in the fragment language behaves identically as it would in 
//	// the tessellation control and evaluation languages.If a geometry shader is present but does not write to gl_PrimitiveID, the value 
//	// of gl_PrimitiveID in the fragment shader is undefined.
//	gl_PrimitiveID = gl_PrimitiveIDIn;
//	EmitVertex();
//
//	EndPrimitive();
//})";
//
//
//std::string points_spheres_geometry_texture_vert = R"(
//#version 430 core
//// please send comments or report bug to: liangliang.nan@gmail.com
//
////#extension GL_EXT_gpu_shader4 : enable
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//
//in vec3  vtx_position;
//in vec2  vtx_texcoord;
////in float sphere_radius;
// 
//out		vec2	texcoord;
////out float	sphere_radius_in;
//
//void main()
//{  
//	texcoord = vtx_texcoord;
//
//	//sphere_radius_in = sphere_radius;
//
//	gl_Position = MANIP * vec4(vtx_position, 1.0);
//})";
//
//
//
//
//std::string points_spheres_sprite_color_frag = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//uniform bool	perspective;
//
//layout(std140) uniform Lighting {
//	vec3	wLightPos;
//	vec3	eLightPos;
//	vec3	wCamPos;
//	vec3	ambient;		// in [0, 1], r==g==b;
//	vec3	specular;		// in [0, 1], r==g==b;
//	float	shininess;
//};
//
//uniform float	sphere_radius;
//
//in Data{
//	vec4	position; // in eye space
//	vec3	sphere_color;
//	//float	sphere_radius;
//} DataIn;
//
//out vec4 outputF;
//
//// from https://www.opengl.org/sdk/docs/man/html/gl_PointCoord.xhtml
//// If GL_POINT_SPRITE_COORD_ORIGIN is GL_LOWER_LEFT, gl_PointCoord.t 
//// varies from 0.0 to 1.0 vertically from bottom to top. Otherwise, 
//// if GL_POINT_SPRITE_COORD_ORIGIN is GL_UPPER_LEFT then gl_PointCoord.t 
//// varies from 0.0 to 1.0 vertically from top to bottom. The default 
//// value of GL_POINT_SPRITE_COORD_ORIGIN is GL_UPPER_LEFT.
//
//
//void main() 
//{
//	/*  with perspective correction
//	*   Ref: Learning Modern 3D Graphics Programming, by Jason L. McKesson
//	*	http://alfonse.bitbucket.org/oldtut/Illumination/Tut13%20Correct%20Chicanery.html
//	**/
//	if (perspective) {
//		vec2 tex = gl_PointCoord* 2.0 - vec2(1.0);
//		tex = vec2(tex.x, -tex.y) * 1.5; // 1.5 times larger ensure the quad is big enough in perspective view
//
//		vec3 planePos = vec3(tex * sphere_radius, 0.0) + DataIn.position.xyz;
//		vec3 view_dir = normalize(planePos);
//		float B = 2.0 * dot(view_dir, -DataIn.position.xyz);
//		float C = dot(DataIn.position.xyz, DataIn.position.xyz) - (sphere_radius * sphere_radius);
//		float det = (B * B) - (4 * C);
//		if (det < 0.0)
//			discard;
//
//		float sqrtDet = sqrt(det);
//		float posT = (-B + sqrtDet) / 2;
//		float negT = (-B - sqrtDet) / 2;
//		float intersectT = min(posT, negT);
//		vec3 pos = view_dir * intersectT;
//		vec3 normal = normalize(pos - DataIn.position.xyz);
//
//		// compute the depth. I can do it easier
//		//vec4 pos4 = PROJ * vec4(pos, 1.0);
//		//gl_FragDepth = 0.5*(pos4.z / pos4.w) + 0.5;
//		vec2 clipZW = pos.z * PROJ[2].zw + PROJ[3].zw;
//		gl_FragDepth = 0.5 * clipZW.x / clipZW.y + 0.5;
//
//		// camera pos is (0, 0, 0) in the camera coordinate system
//		vec3 light_dir = normalize(eLightPos);
//		float df = max(dot(normal, light_dir), 0);
//
//		float sf = 0.0;	// specular factor
//		if (df > 0.0) {	// if the vertex is lit compute the specular color
//			view_dir = -view_dir; // '-' because I used the reversed direction 
//			vec3 half_vector = normalize(light_dir + view_dir);	// compute the half vector
//			sf = max(dot(half_vector, normal), 0.0);
//			sf = pow(sf, shininess);
//		}
//
//		outputF = vec4(DataIn.sphere_color * df + specular * sf + ambient, 1.0);
//	}
//
//	// without perspective correction
//	else {
//		// r^2 = (x - x0)^2 + (y - y0)^2 + (z - z0)^2
//		vec2 tex = gl_PointCoord* 2.0 - vec2(1.0);
//		float x = tex.x;
//		float y = -tex.y;
//		float zz = 1.0 - x*x - y*y;
//
//		if (zz < 0.0)
//			discard;
//
//		float z = sqrt(zz);
//		vec4 pos = DataIn.position;
//		pos.z += sphere_radius*z;
//
//		// compute the depth. I can do it easier
//		//pos = PROJ * pos;
//		//gl_FragDepth = 0.5*(pos.z / pos.w) + 0.5;
//		vec2 clipZW = pos.z * PROJ[2].zw + PROJ[3].zw;
//		gl_FragDepth = 0.5 * clipZW.x / clipZW.y + 0.5;
//
//		// camera pos is (0, 0, 0) in the camera coordinate system
//		vec3 view_dir = vec3(0, 0, 1);
//		vec3 light_dir = normalize(eLightPos);
//
//		vec3 normal = vec3(x, y, z); // sure this was normalized because (z = sqrt(1.0 - x*x - y*y))
//		float df = max(dot(normal, light_dir), 0);
//
//		float sf = 0.0;	// specular factor
//		if (df > 0.0) {	// if the vertex is lit compute the specular color
//			vec3 half_vector = normalize(light_dir + view_dir);	// compute the half vector
//			sf = max(dot(half_vector, normal), 0.0);
//			sf = pow(sf, shininess);
//		}
//
//		outputF = vec4(DataIn.sphere_color * df + specular * sf + ambient, 1.0);
//	}
//}
//)";
//
//
//std::string points_spheres_sprite_color_vert = R"(
////#version 150		// for point sprite to work
//#version 430 core	// for shader storage buffer
//
//// please send comments or report bug to: liangliang.nan@gmail.com
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//in vec3  vtx_position;
//in vec3  vtx_color;
////in float sphere_radius;
//
//uniform int		screen_width;   // scale to calculate size in pixels
//uniform float	sphere_radius;
//uniform vec3	default_color;
//uniform bool	per_vertex_color;
//
//uniform int		hightlight_id;
//uniform bool	selected;
//
//layout(std430, binding = 1) buffer selection_t {
//	uint data[];
//} selection;
//
//
//out Data{
//	vec4	position; // in eye space
//	vec3	sphere_color;
//	//float	sphere_radius;
//} DataOut;
//
//
//void main()
//{
//	vec4 new_position = MANIP * vec4(vtx_position, 1.0);
//
//	vec3 color = default_color;
//	if (per_vertex_color)
//		color = vtx_color;
//
//	//uint addr = gl_VertexID / 32;
//	//uint offs = gl_VertexID % 32;
//	//if ((selection.data[addr] & (1 << offs)) != 0)
//	if ((selection.data[gl_VertexID >> 5] & (1 << (gl_VertexID & 31))) != 0)
//		color = mix(color, vec3(1.0, 0.0, 0.0), 0.8);
//	else if (hightlight_id == gl_VertexID)
//		color = vec3(1.0, 0.0, 0.0);
//
//	if (selected)
//		color = vec3(1.0, 0, 0);
//
//	DataOut.sphere_color = color;
//
//	//DataOut.sphere_radius = sphere_radius;
//
//	// Output vertex position
//	DataOut.position = MV * new_position; // eye space
//
//	// http://stackoverflow.com/questions/8608844/resizing-point-sprites-based-on-distance-from-the-camera
//	vec4 projCorner = PROJ * vec4(sphere_radius, sphere_radius, DataOut.position.z, DataOut.position.w);
//	gl_PointSize = screen_width * projCorner.x / projCorner.w;
//	// you can also compute the point size using both screen width and height information.
//	// http://stackoverflow.com/questions/17397724/point-sprites-for-particle-system
//	//vec2 projSize = screen_size * projCorner.xy / projCorner.w;  // screen_size = vec2(screen_width, screen_height);
//	//gl_PointSize = 0.5 * (projSize.x + projSize.y);
//
//	gl_Position = PROJ * DataOut.position;
//
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(new_position, clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(new_position, clippingPlane1);
//	}
//})";
//
//
//
//
//std::string points_spheres_sprite_texture_frag = R"(
//#version 430
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//uniform bool	perspective;
//
//uniform sampler2D	textureID;
//
//layout(std140) uniform Lighting {
//	vec3	wLightPos;
//	vec3	eLightPos;
//	vec3	wCamPos;
//	vec3	ambient;		// in [0, 1], r==g==b;
//	vec3	specular;		// in [0, 1], r==g==b;
//	float	shininess;
//};
//
//uniform float	sphere_radius;
//
//uniform int	 hightlight_id;
//uniform bool selected;
//
//layout(std430, binding = 1) buffer selection_t {
//	uint data[];
//} selection;
//
//in Data{
//	vec4	position; // eye space
//	vec2	texcoord;
//	//float	sphere_radius;
//} DataIn;
//
//out vec4 outputF;
//
//// from https://www.opengl.org/sdk/docs/man/html/gl_PointCoord.xhtml
//// If GL_POINT_SPRITE_COORD_ORIGIN is GL_LOWER_LEFT, gl_PointCoord.t 
//// varies from 0.0 to 1.0 vertically from bottom to top. Otherwise, 
//// if GL_POINT_SPRITE_COORD_ORIGIN is GL_UPPER_LEFT then gl_PointCoord.t 
//// varies from 0.0 to 1.0 vertically from top to bottom. The default 
//// value of GL_POINT_SPRITE_COORD_ORIGIN is GL_UPPER_LEFT.
//
//
//void main() 
//{
//	/*  with perspective correction
//	*   Ref: Learning Modern 3D Graphics Programming, by Jason L. McKesson
//	*	http://alfonse.bitbucket.org/oldtut/Illumination/Tut13%20Correct%20Chicanery.html
//	**/
//	if (perspective) {
//		vec2 tex = gl_PointCoord* 2.0 - vec2(1.0);
//		tex = vec2(tex.x, -tex.y) * 1.5; // 1.5 times larger ensure the quad is big enought in perspective view
//
//		vec3 planePos = vec3(tex * sphere_radius, 0.0) + DataIn.position.xyz;
//		vec3 view_dir = normalize(planePos);
//		float B = 2.0 * dot(view_dir, -DataIn.position.xyz);
//		float C = dot(DataIn.position.xyz, DataIn.position.xyz) - (sphere_radius * sphere_radius);
//		float det = (B * B) - (4 * C);
//		if (det < 0.0)
//			discard;
//
//		float sqrtDet = sqrt(det);
//		float posT = (-B + sqrtDet) / 2;
//		float negT = (-B - sqrtDet) / 2;
//		float intersectT = min(posT, negT);
//		vec3 pos = view_dir * intersectT;
//		vec3 normal = normalize(pos - DataIn.position.xyz);
//
//		// compute the depth. I can do it easier
//		//vec4 pos4 = PROJ * vec4(pos, 1.0);
//		//gl_FragDepth = 0.5*(pos4.z / pos4.w) + 0.5;
//		vec2 clipZW = pos.z * PROJ[2].zw + PROJ[3].zw;
//		gl_FragDepth = 0.5 * clipZW.x / clipZW.y + 0.5;
//
//		// camera pos is (0, 0, 0) in the camera coordinate system
//		vec3 light_dir = normalize(eLightPos);
//		float df = max(dot(normal, light_dir), 0);
//
//		float sf = 0.0;	// specular factor
//		if (df > 0.0) {	// if the vertex is lit compute the specular color
//			view_dir = -view_dir; // '-' because I used the reversed direction 
//			vec3 half_vector = normalize(light_dir + view_dir);	// compute the half vector
//			sf = max(dot(half_vector, normal), 0.0);
//			sf = pow(sf, shininess);
//		}
//
//		vec3 color = texture(textureID, DataIn.texcoord).rgb;
//
//		//uint addr = gl_PrimitiveID / 32;
//		//uint offs = gl_PrimitiveID % 32;
//		uint addr = gl_PrimitiveID >> 5;
//		uint offs = gl_PrimitiveID & 31;
//		if ((selection.data[addr] & (1 << offs)) != 0)
//			color = mix(color, vec3(1.0, 0.0, 0.0), 0.8);
//		else if (gl_PrimitiveID == hightlight_id)
//			color = vec3(1.0, 0.0, 0.0);
//
//		if (selected)
//			color = vec3(1.0, 0.0, 0.0);
//
//		outputF = vec4(color * df + specular * sf + ambient, 1.0);
//	}
//
//	// without perspective correction
//	else {
//		// r^2 = (x - x0)^2 + (y - y0)^2 + (z - z0)^2
//		vec2 tex = gl_PointCoord* 2.0 - vec2(1.0);
//		float x = tex.x;
//		//float y = tex.y;
//		float y = -tex.y;
//		float zz = 1.0 - x*x - y*y;
//
//		if (zz < 0.0)
//			discard;
//
//		float z = sqrt(zz);
//		vec4 pos = DataIn.position;
//		pos.z += sphere_radius*z;
//
//		// compute the depth. I can do it easier
//		//pos = PROJ * pos;
//		//gl_FragDepth = 0.5*(pos.z / pos.w) + 0.5;
//		vec2 clipZW = pos.z * PROJ[2].zw + PROJ[3].zw;
//		gl_FragDepth = 0.5 * clipZW.x / clipZW.y + 0.5;
//
//		// camera pos is (0, 0, 0) in the camera coordinate system
//		vec3 view_dir = vec3(0, 0, 1);
//		vec3 light_dir = normalize(eLightPos);
//
//		vec3 normal = vec3(x, y, z); // sure this was normalized because (z = sqrt(1.0 - x*x - y*y))
//		float df = max(dot(normal, light_dir), 0);
//
//		float sf = 0.0;	// specular factor
//		if (df > 0.0) {	// if the vertex is lit compute the specular color
//			vec3 half_vector = normalize(light_dir + view_dir);	// compute the half vector
//			sf = max(dot(half_vector, normal), 0.0);
//			sf = pow(sf, shininess);
//		}
//
//		vec3 color = texture(textureID, DataIn.texcoord).rgb;
//
//		//uint addr = gl_PrimitiveID / 32;
//		//uint offs = gl_PrimitiveID % 32;
//		uint addr = gl_PrimitiveID >> 5;
//		uint offs = gl_PrimitiveID & 31;
//		if ((selection.data[addr] & (1 << offs)) != 0)
//			color = mix(color, vec3(1.0, 0.0, 0.0), 0.8);
//		else if (gl_PrimitiveID == hightlight_id)
//			color = vec3(1.0, 0.0, 0.0);
//
//		if (selected)
//			color = vec3(1.0, 0.0, 0.0);
//
//		outputF = vec4(color * df + specular * sf + ambient, 1.0);
//	}
//})";
//
//
//std::string points_spheres_sprite_texture_vert = R"(
//#version 430
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//in vec3  vtx_position;
//in vec2  vtx_texcoord;
////in float sphere_radius;
//
//uniform int		screen_width;   // scale to calculate size in pixels
//uniform float	sphere_radius;
//
//
//out Data{
//	vec4	position; // eye space
//	vec2	texcoord;
//	//float	sphere_radius;
//} DataOut;
//
//void main()
//{
//	vec4 new_position = MANIP * vec4(vtx_position, 1.0);
//
//	DataOut.texcoord = vtx_texcoord;
//
//	// Output vertex position
//	DataOut.position = MV * new_position; // eye space
//
//	//DataOut.sphere_radius = sphere_radius;
//
//	// http://stackoverflow.com/questions/8608844/resizing-point-sprites-based-on-distance-from-the-camera
//	vec4 projCorner = PROJ * vec4(sphere_radius, sphere_radius, DataOut.position.z, DataOut.position.w);
//	gl_PointSize = screen_width * projCorner.x / projCorner.w;
//	// you can also compute the point size using both screen width and height information.
//	// http://stackoverflow.com/questions/17397724/point-sprites-for-particle-system
//	//vec2 projSize = screen_size * projCorner.xy / projCorner.w;  // screen_size = vec2(screen_width, screen_height);
//	//gl_PointSize = 0.5 * (projSize.x + projSize.y);
//
//	gl_Position = PROJ * DataOut.position;
//
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(new_position, clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(new_position, clippingPlane1);
//	}
//})";
//
//
//
//
//std::string selection_model_frag = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//uniform vec4 modelColor;
//
//out  vec4	outputF;
//
//void main()
//{
//	outputF = modelColor;
//})";
//
//
//std::string selection_model_vert = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//in  vec3	vtx_position;	// vertex position
//
//uniform mat4 MVP_MANIP;
//
//void main()
//{
//	gl_Position = MVP_MANIP * vec4(vtx_position, 1.0);
//} )";
//
//
//
//
//std::string selection_model_pointcloud_sphere_sprite_frag = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//uniform bool	perspective;
//
//uniform mat4	PROJ;
//uniform float	sphere_radius;
//
//uniform vec4 modelColor;
//
//in  vec4 position; // in eye space
//
//out vec4 outputF;
//
//
//void main()
//{
//	/*  with perspective correction
//	*   Ref: Learning Modern 3D Graphics Programming, by Jason L. McKesson
//	*	http://alfonse.bitbucket.org/oldtut/Illumination/Tut13%20Correct%20Chicanery.html
//	**/
//	if (perspective) {
//		vec2 tex = gl_PointCoord* 2.0 - vec2(1.0);
//		tex = vec2(tex.x, -tex.y) * 1.5; // 1.5 times larger ensure the quad is big enought in perspective view
//
//		vec3 planePos = vec3(tex * sphere_radius, 0.0) + position.xyz;
//		vec3 view_dir = normalize(planePos);
//		float B = 2.0 * dot(view_dir, -position.xyz);
//		float C = dot(position.xyz, position.xyz) - (sphere_radius * sphere_radius);
//		float det = (B * B) - (4 * C);
//		if (det < 0.0)
//			discard;
//
//		float sqrtDet = sqrt(det);
//		float posT = (-B + sqrtDet) / 2;
//		float negT = (-B - sqrtDet) / 2;
//		float intersectT = min(posT, negT);
//		vec3 pos = view_dir * intersectT;
//
//		// compute the depth. I can do it easier
//		//vec4 pos4 = PROJ * vec4(pos, 1.0);
//		//gl_FragDepth = 0.5*(pos4.z / pos4.w) + 0.5;
//		vec2 clipZW = pos.z * PROJ[2].zw + PROJ[3].zw;
//		gl_FragDepth = 0.5 * clipZW.x / clipZW.y + 0.5;
//
//		outputF = modelColor;
//	}
//
//	// without perspective correction
//	else {
//		// r^2 = (x - x0)^2 + (y - y0)^2 + (z - z0)^2
//		vec2 tex = gl_PointCoord* 2.0 - vec2(1.0);
//		float x = tex.x;
//		float y = -tex.y;
//		float zz = 1.0 - x*x - y*y;
//
//		if (zz < 0.0)
//			discard;
//
//		float z = sqrt(zz);
//		vec4 pos = position;
//		pos.z += sphere_radius*z;
//
//		// compute the depth. I can do it easier
//		//pos = PROJ * pos;
//		//gl_FragDepth = 0.5*(pos.z / pos.w) + 0.5;
//		vec2 clipZW = pos.z * PROJ[2].zw + PROJ[3].zw;
//		gl_FragDepth = 0.5 * clipZW.x / clipZW.y + 0.5;
//
//		outputF = modelColor;
//	}
//}
//
//)";
//
//
//std::string selection_model_pointcloud_sphere_sprite_vert = R"(
//#version 150		
//
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//in vec3  vtx_position;
//
//uniform mat4	MV;
//uniform mat4	PROJ;
//uniform mat4	MANIP;
//
//uniform int		screen_width;   // scale to calculate size in pixels
//uniform float	sphere_radius;
//
//
//out vec4	position; // in eye space
//
//void main()
//{
//	position = MV * MANIP * vec4(vtx_position, 1.0);
//
//	// http://stackoverflow.com/questions/8608844/resizing-point-sprites-based-on-distance-from-the-camera
//	vec4 projCorner = PROJ * vec4(sphere_radius, sphere_radius, position.z, position.w);
//	gl_PointSize = screen_width * projCorner.x / projCorner.w;
//
//	gl_Position = PROJ * position;
//})";
//
//
//
//
//std::string selection_pointcloud_lasso_frag = R"(
//#version 430
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//
//void main()
//{
//} 
//)";
//
//
//std::string selection_pointcloud_lasso_vert = R"(
//#version 430
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//in  vec3		vtx_position;	// vertex position
//
//uniform mat4	MVP_MANIP;
//uniform ivec4	viewport;
//uniform bool	deselect;
//
//
//layout(std430, binding = 0) buffer polygon_t {
//	vec2 data[];
//} polygon;
//
//
//layout(std430, binding = 1) buffer selection_t {
//	uint data[];
//} selection;
//
//
//
//bool point_in_polygon(vec2 p)
//{
//	bool inside = false;
//	uint n = polygon.data.length();
//	for (uint i = 0, j = n - 1; i < n; j = i, ++i) {
//		vec2 u0 = polygon.data[i];
//		vec2 u1 = polygon.data[j];  // current edge
//
//		if (((u0.y <= p.y) && (p.y < u1.y)) ||  // U1 is above the ray, U0 is on or below the ray
//			((u1.y <= p.y) && (p.y < u0.y)))    // U0 is above the ray, U1 is on or below the ray
//		{
//			// find x-intersection of current edge with the ray. 
//			// Only consider edge crossings on the ray to the right of P.
//			float x = u0.x + (p.y - u0.y) * (u1.x - u0.x) / (u1.y - u0.y);
//			if (x > p.x)
//				inside = !inside;
//		}
//	}
//
//	return inside;
//}
//
//
//void main()
//{
//	vec4 p = MVP_MANIP * vec4(vtx_position, 1.0);
//	float x = p.x / p.w * 0.5 + 0.5;
//	float y = p.y / p.w * 0.5 + 0.5;
//	x = x * viewport[2] + viewport[0];
//	y = y * viewport[3] + viewport[1];
//
//	vec2 v = vec2(x, viewport[3] - 1 - y);
//	if (point_in_polygon(v)) {
//		//int addr = gl_VertexID / 32;
//		//int offs = gl_VertexID % 32;
//		uint addr = gl_VertexID >> 5;
//		uint offs = gl_VertexID & 31;
//		if (deselect) {
//			atomicAnd(selection.data[addr], ~(1 << offs));
//			//selection.data[addr] &= ~(1 << offs);
//		}
//		else {
//			atomicOr(selection.data[addr], (1 << offs));
//			//selection.data[addr] |= (1 << offs);
//		}
//	}
//
//	gl_Position = p;
//}
//)";
//
//
//
//
//std::string selection_pointcloud_rect_frag = R"(
//#version 430
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//
//void main()
//{
//} 
//)";
//
//
//std::string selection_pointcloud_rect_vert = R"(
//#version 430
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//in  vec3		vtx_position;	// vertex position
//
//uniform mat4	MVP_MANIP;		// model-view-projection matrix
//uniform ivec4	viewport;		// view port
//uniform vec4	rect;			// the rectangle region
//uniform bool	deselect;		// select or deselect
//
//layout(std430, binding = 1) buffer selection_t {
//	uint data[];
//} selection;
//
//
//void main()
//{
//	vec4 p = MVP_MANIP * vec4(vtx_position, 1.0);
//	float x = p.x / p.w * 0.5 + 0.5;
//	float y = p.y / p.w * 0.5 + 0.5;
//	x = x * viewport[2] + viewport[0];
//	y = y * viewport[3] + viewport[1];
//	y = viewport[3] - 1 - y;
//	if (x >= rect[0] && x <= rect[1] && y >= rect[2] && y <= rect[3]) 
//	{
//		//int addr = gl_VertexID / 32;
//		//int offs = gl_VertexID % 32;
//		uint addr = gl_VertexID >> 5;
//		uint offs = gl_VertexID & 31;
//		if (deselect) {
//			atomicAnd(selection.data[addr], ~(1 << offs));
//			//selection.data[addr] &= ~(1 << offs);
//		}
//		else {
//			atomicOr(selection.data[addr], (1 << offs));
//			//selection.data[addr] |= (1 << offs);
//		}
//	}
//
//	gl_Position = p;
//}
//)";
//
//
//
//
//std::string selection_point_sphere_sprite_frag = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//uniform bool	perspective;
//
//uniform mat4	PROJ;
//uniform float	sphere_radius;
//
//in  vec4 position; // in eye space
//
//out vec4 outputF;
//
//
//void main()
//{
//	/*  with perspective correction
//	*   Ref: Learning Modern 3D Graphics Programming, by Jason L. McKesson
//	*	http://alfonse.bitbucket.org/oldtut/Illumination/Tut13%20Correct%20Chicanery.html
//	**/
//	if (perspective) {
//		vec2 tex = gl_PointCoord* 2.0 - vec2(1.0);
//		tex = vec2(tex.x, -tex.y) * 1.5; // 1.5 times larger ensure the quad is big enought in perspective view
//
//		vec3 planePos = vec3(tex * sphere_radius, 0.0) + position.xyz;
//		vec3 view_dir = normalize(planePos);
//		float B = 2.0 * dot(view_dir, -position.xyz);
//		float C = dot(position.xyz, position.xyz) - (sphere_radius * sphere_radius);
//		float det = (B * B) - (4 * C);
//		if (det < 0.0)
//			discard;
//
//		float sqrtDet = sqrt(det);
//		float posT = (-B + sqrtDet) / 2;
//		float negT = (-B - sqrtDet) / 2;
//		float intersectT = min(posT, negT);
//		vec3 pos = view_dir * intersectT;
//
//		// compute the depth. I can do it easier
//		//vec4 pos4 = PROJ * vec4(pos, 1.0);
//		//gl_FragDepth = 0.5*(pos4.z / pos4.w) + 0.5;
//		vec2 clipZW = pos.z * PROJ[2].zw + PROJ[3].zw;
//		gl_FragDepth = 0.5 * clipZW.x / clipZW.y + 0.5;
//
//
//		// NOTE: For points, the gl_PrimitiveID represent the ID of the input points. 
//		//		 For polygonal models, the gl_PrimitiveID represent the ID of triangles (the GPU assembles 
//		//		 all primitive type, e.g., triangle fan, polygon, triangle strips as triangles).
//		//		 Since we're selecting a single point, the id can also be computed in vertex shader using gl_VertexID.
//		int id = gl_PrimitiveID;
//		outputF.r = ((id >> 16) & 0xff) / 255.0;
//		outputF.g = ((id >> 8) & 0xff) / 255.0;
//		outputF.b = (id & 0xff) / 255.0;
//		outputF.a = (id >> 24) / 255.0;
//	}
//
//	// without perspective correction
//	else {
//		// r^2 = (x - x0)^2 + (y - y0)^2 + (z - z0)^2
//		vec2 tex = gl_PointCoord* 2.0 - vec2(1.0);
//		float x = tex.x;
//		float y = -tex.y;
//		float zz = 1.0 - x*x - y*y;
//
//		if (zz < 0.0)
//			discard;
//
//		float z = sqrt(zz);
//		vec4 pos = position;
//		pos.z += sphere_radius*z;
//
//		// compute the depth. I can do it easier
//		//pos = PROJ * pos;
//		//gl_FragDepth = 0.5*(pos.z / pos.w) + 0.5;
//		vec2 clipZW = pos.z * PROJ[2].zw + PROJ[3].zw;
//		gl_FragDepth = 0.5 * clipZW.x / clipZW.y + 0.5;
//
//		// NOTE: For points, the gl_PrimitiveID represent the ID of the input points. 
//		//		 For polygonal models, the gl_PrimitiveID represent the ID of triangles (the GPU assembles 
//		//		 all primitive type, e.g., triangle fan, polygon, triangle strips as triangles).
//		//		 Since we're selecting a single point, the id can also be computed in vertex shader using gl_VertexID.
//		int id = gl_PrimitiveID;
//		outputF.r = ((id >> 16) & 0xff) / 255.0;
//		outputF.g = ((id >> 8) & 0xff) / 255.0;
//		outputF.b = (id & 0xff) / 255.0;
//		outputF.a = (id >> 24) / 255.0;
//	}
//}
//
//)";
//
//
//std::string selection_point_sphere_sprite_vert = R"(
//#version 150		
//
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//in vec3  vtx_position;
//
//uniform mat4	MV;
//uniform mat4	PROJ;
//uniform mat4	MANIP;
//
//uniform int		screen_width;   // scale to calculate size in pixels
//uniform float	sphere_radius;
//
//
//out vec4	position; // in eye space
//
//void main()
//{
//	position = MV * MANIP * vec4(vtx_position, 1.0);
//
//	// http://stackoverflow.com/questions/8608844/resizing-point-sprites-based-on-distance-from-the-camera
//	vec4 projCorner = PROJ * vec4(sphere_radius, sphere_radius, position.z, position.w);
//	gl_PointSize = screen_width * projCorner.x / projCorner.w;
//
//	gl_Position = PROJ * position;
//})";
//
//
//
//
//std::string selection_single_primitive_frag = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//out  vec4	outputF;
//
//void main()
//{
//	// NOTE: For points, the gl_PrimitiveID represent the ID of the input points. 
//	//		 For polygonal models, the gl_PrimitiveID represent the ID of triangles (the GPU assembles 
//	//		 all primitive type, e.g., triangle fan, polygon, triangle strips as triangles).
//	//		 If you're selecting a single point, the id can also be computed in vertex shader using gl_VertexID.
//	int id = gl_PrimitiveID; 
//	outputF.r = ((id >> 16) & 0xff) / 255.0;
//	outputF.g = ((id >> 8) & 0xff) / 255.0;
//	outputF.b = (id & 0xff) / 255.0;
//	outputF.a = (id >> 24) / 255.0;
//} )";
//
//
//std::string selection_single_primitive_vert = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//in  vec3	vtx_position;	// vertex position
//
//uniform mat4 MVP_MANIP;
//
//void main()
//{
//	gl_Position = MVP_MANIP * vec4(vtx_position, 1.0);
//} )";
//
//
//
//
//std::string shadow_lines_cones_frag = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//// It uses geometry shader for vertex generation.
//// The code does not cover round caps.
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//uniform bool	generateShadowMap;
//
//uniform float	radius;
//
//layout(std140) uniform Lighting {
//	vec3	wLightPos;
//	vec3	eLightPos;
//	vec3	wCamPos;
//	vec3	ambient;		// in [0, 1], r==g==b;
//	vec3	specular;		// in [0, 1], r==g==b;
//	float	shininess;
//};
//
//in Data{
//	vec3 point;// camera space
//	vec3 axis;
//	vec3 base;
//	vec3 end;
//	vec3 U;
//	vec3 V;
//} DataIn;
//
//
//out vec4 FragColor;	// Ouput data
//
//void main()
//{
//	if (generateShadowMap) {
//
//		// First of all, I need the correct point that we're pointing at
//
//		vec3 view_dir = vec3(0, 0, 1);	// this is view direction for orthographic mode in camera space
//
//		// basis = local coordinate system of cone 
//		mat3 basis = mat3(DataIn.U, DataIn.V, DataIn.axis);
//		vec3 diff = DataIn.point - DataIn.base;
//		vec3 P = diff * basis;
//
//		float radius2 = radius*radius;
//		float length2 = dot(DataIn.end - DataIn.base, DataIn.end - DataIn.base);
//		float coeff = radius2 / length2;
//		float h = dot(DataIn.end - DataIn.base, DataIn.axis);
//
//		// Now calculate distance to the intersecting point from ray origin
//		// See Ray tracing cones here: https://www.cl.cam.ac.uk/teaching/1999/AGraphHCI/SMAG/node2.html#eqn:rectray
//
//		// Direction of the ray in cone space
//		vec3 D = vec3(dot(DataIn.U, view_dir), dot(DataIn.V, view_dir), dot(DataIn.axis, view_dir));
//		// Now the intersection is between z-axis aligned cone and a ray
//		float a = D.x * D.x + D.y * D.y - D.z * D.z * coeff;
//		float b = 2.0 * (P.x * D.x + P.y * D.y - coeff * (P.z * D.z - h * D.z));
//		float c = P.x * P.x + P.y * P.y - coeff * (P.z - h) * (P.z - h);
//		// calculate a discriminant of the above quadratic equation
//		float d = b*b - 4 * a*c;
//		if (d < 0.0)		// outside of the cone
//			discard;
//
//		// point of intersection on cone surface
//		float dist = (-b + sqrt(d)) / (2 * a);
//		vec3 new_point = DataIn.point + dist * view_dir;
//
//		// now compute the normal of the new point (for computing the lighting).
//		vec3 tangent = cross(new_point - DataIn.base, new_point - DataIn.end);
//		vec3 normal = cross(new_point - DataIn.end, tangent);
//		normal = normalize(normal);
//
//		// to calculate caps, simply check the angle between the following two vectors:
//		//		- (point of intersection) - (the cone's base center)
//		//		- cap plane normal (which is the cone axis)
//		// if the angle < 0, the point is outside of cone
//		float cap_test = dot((new_point - DataIn.base), DataIn.axis);
//
//
//		// Now I need to determine if the projection of the cone's peak (onto the base plane) is within the base disc
//		float tt = dot(DataIn.base - DataIn.end, -DataIn.axis) / dot(view_dir, -DataIn.axis);
//		vec3 projection = DataIn.end + view_dir * tt;
//		bool within_base = dot(projection - DataIn.base, projection - DataIn.base) < radius2;
//
//
//		if (cap_test < 0.0 || (within_base && dot(view_dir, DataIn.axis) < 0.0))
//		{
//			normal = -DataIn.axis;
//
//			// ray-plane intersection
//			float t = dot(DataIn.base - DataIn.point, normal) / dot(view_dir, normal);
//			new_point = DataIn.point + view_dir * t;
//
//			// within the cap radius?
//			if (dot(new_point - DataIn.base, new_point - DataIn.base) > radius2)
//				discard;
//		}
//
//		// compute the depth. I can do it easier
//		//vec4 pos = PROJ * vec4(new_point, 1.0);
//		//float depth = 0.5*(pos.z / pos.w) + 0.5;
//		vec2 clipZW = new_point.z * PROJ[2].zw + PROJ[3].zw;
//		float depth = 0.5 + 0.5 * clipZW.x / clipZW.y;
//
//		// this is a workaround necessary for Mac, otherwise the modified fragment won't clip properly
//		if (depth <= 0.0 || depth >= 1.0)
//			discard;
//
//		gl_FragDepth = depth;
//		FragColor = vec4(depth, 0, 0, 1.0);
//	}
//})";
//
//
//std::string shadow_lines_cones_geom = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//// It uses geometry shader for vertex generation.
//// The code does not cover round caps.
//
////#extension GL_EXT_geometry_shader4 : enable
//
//layout(lines) in;
//layout(triangle_strip, max_vertices = 5) out;
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//uniform float	radius;
//
//out Data{
//	vec3 point;	// camera space
//	vec3 axis;
//	vec3 base;
//	vec3 end;	// the peak of the cone
//	vec3 U;
//	vec3 V;
//} DataOut;
//
//
//void main()
//{
//	vec3  base = gl_in[0].gl_Position.xyz;  // in camera space
//	vec3  top = gl_in[1].gl_Position.xyz;   // in camera space
//	vec3  len_dir = top - base;
//	vec3  view_dir = vec3(0,0,1);	// this is view direction for orthographic mode in camera space
//
//	vec3 left = cross(view_dir, len_dir);
//	vec3 up = cross(left, len_dir);
//	DataOut.axis = normalize(len_dir);
//	DataOut.U = normalize(up);
//	DataOut.V = normalize(left);
//	left = radius * DataOut.V;
//	up = radius * DataOut.U;
//
//	DataOut.base = base;
//	DataOut.end = top;
//
//	float b = dot(view_dir, len_dir);
//	if (b < 0) {
//		// Vertex 1
//		DataOut.point = top;
//		gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//		if (clippingPlaneEnabled) {
//			gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//			if (crossSectionEnabled)
//				gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//		}
//		EmitVertex();
//
//		// Vertex 2
//		DataOut.point = (base + left - up);
//		gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//		if (clippingPlaneEnabled) {
//			gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//			if (crossSectionEnabled)
//				gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//		}
//		EmitVertex();
//
//		// Vertex 3
//		DataOut.point = (base - left - up);
//		gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//		if (clippingPlaneEnabled) {
//			gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//			if (crossSectionEnabled)
//				gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//		}
//		EmitVertex();
//
//		// Vertex 4
//		DataOut.point = (base + left + up);
//		gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//		if (clippingPlaneEnabled) {
//			gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//			if (crossSectionEnabled)
//				gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//		}
//		EmitVertex();
//
//		// Vertex 5
//		DataOut.point = (base - left + up);
//		gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//		if (clippingPlaneEnabled) {
//			gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//			if (crossSectionEnabled)
//				gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//		}
//		EmitVertex();
//	}
//	else {
//		// Vertex 1
//		DataOut.point = top;
//		gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//		if (clippingPlaneEnabled) {
//			gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//			if (crossSectionEnabled)
//				gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//		}
//		EmitVertex();
//
//		// Liangliang: determining the minimum extend for this case is tricky, so I just use 
//		//             an enlarge factor to make sure all the entire cone is covered. 
//		float factor = 1.1; // It seems 1.1 is large enough
//
//		// Vertex 2
//		DataOut.point = (base - left * factor + up);	//
//		gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//		if (clippingPlaneEnabled) {
//			gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//			if (crossSectionEnabled)
//				gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//		}
//		EmitVertex();
//
//		// Vertex 3
//		DataOut.point = (base + left * factor + up);	// 1.2 is just 
//		gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//		if (clippingPlaneEnabled) {
//			gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//			if (crossSectionEnabled)
//				gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//		}
//		EmitVertex();
//
//		// Vertex 4
//		DataOut.point = (base - left - up);
//		gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//		if (clippingPlaneEnabled) {
//			gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//			if (crossSectionEnabled)
//				gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//		}
//		EmitVertex();
//
//		// Vertex 5
//		DataOut.point = (base + left - up);
//		gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//		if (clippingPlaneEnabled) {
//			gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//			if (crossSectionEnabled)
//				gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//		}
//		EmitVertex();
//	}
//
//	EndPrimitive();
//})";
//
//
//std::string shadow_lines_cones_vert = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//// It uses geometry shader for vertex generation.
//// The code does not cover round caps.
//
////#extension GL_EXT_gpu_shader4 : enable
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//in  vec3 vtx_position;	// point position
//
//void main()
//{
//	gl_Position = MV * MANIP * vec4(vtx_position, 1.0);
//})";
//
//
//
//
//std::string shadow_lines_cylinders_frag = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//// It uses geometry shader for vertex generation.
//// The code does not cover round caps.
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//uniform bool	generateShadowMap;
//
//uniform float	radius;
//
//layout(std140) uniform Lighting {
//	vec3	wLightPos;
//	vec3	eLightPos;
//	vec3	wCamPos;
//	vec3	ambient;		// in [0, 1], r==g==b;
//	vec3	specular;		// in [0, 1], r==g==b;
//	float	shininess;
//};
//
//in Data{
//	vec3 point;// camera space
//	vec3 axis;
//	vec3 base;
//	vec3 end;
//	vec3 U;
//	vec3 V;
//} DataIn;
//
//
//out vec4 FragColor;	// Ouput data
//
//void main()
//{
//	if (generateShadowMap) {
//
//		// First of all, I need the correct point that we're pointing at
//
//		vec3 view_dir = vec3(0, 0, 1);	// this is view direction for orthographic mode in camera space
//
//		// basis = local coordinate system of cylinder
//		mat3 basis = mat3(DataIn.U, DataIn.V, DataIn.axis);
//		vec3 diff = DataIn.point - DataIn.base;
//		vec3 P = diff * basis;
//
//		float radius2 = radius*radius;
//
//		// Now calculate distance to the cylinder from ray origin
//		// See Ray tracing cylinders here: https://www.cl.cam.ac.uk/teaching/1999/AGraphHCI/SMAG/node2.html#eqn:rectray
//
//		// Direction of the ray in cylinder space
//		vec3 D = vec3(dot(DataIn.U, view_dir), dot(DataIn.V, view_dir), dot(DataIn.axis, view_dir));
//		// Now the intersection is between z-axis aligned cylinder and a ray
//		float a = D.x * D.x + D.y * D.y;
//		float b = 2.0 * (P.x * D.x + P.y * D.y);
//		float c = P.x * P.x + P.y * P.y - radius2;
//		// calculate a discriminant of the above quadratic equation
//		float d = b*b - 4 * a*c;
//		if (d < 0.0)		// outside of the cylinder
//			discard;
//
//		float dist = (-b + sqrt(d)) / (2 * a);
//
//		// point of intersection on cylinder surface
//		vec3 new_point = DataIn.point + dist * view_dir;
//
//		// now compute the normal of the new point (for computing the lighting).
//		vec3 tmp_point = new_point - DataIn.base;
//		vec3 normal = normalize(tmp_point - DataIn.axis * dot(tmp_point, DataIn.axis));
//
//		// to calculate caps, simply check the angle between the following two vectors:
//		//		- (point of intersection) - (one of the cylinder's end points)
//		//		- cap plane normal (which is the cylinder axis)
//		// if the angle < 0, the point is outside of cylinder
//		float cap_test = dot((new_point - DataIn.base), DataIn.axis);
//
//		// test front cap
//		if (cap_test < 0.0) {
//			normal = -DataIn.axis;
//
//			// ray-plane intersection
//			float t = dot(DataIn.base - DataIn.point, normal) / dot(view_dir, normal);
//			new_point = DataIn.point + view_dir * t;
//
//			// within the cap radius?
//			if (dot(new_point - DataIn.base, new_point - DataIn.base) > radius2)
//				discard;
//		}
//
//		// test end cap
//		cap_test = dot((new_point - DataIn.end), DataIn.axis);
//		if (cap_test > 0.0) {
//			normal = DataIn.axis;
//
//			// ray-plane intersection
//			float t = dot(DataIn.end - DataIn.point, normal) / dot(view_dir, normal);
//			new_point = DataIn.point + view_dir * t;
//
//			// within the cap radius?
//			if (dot(new_point - DataIn.end, new_point - DataIn.end) > radius2)
//				discard;
//		}
//
//		// compute the depth. I can do it easier
//		//vec4 pos = PROJ * vec4(new_point, 1.0);
//		//float depth = 0.5*(pos.z / pos.w) + 0.5;
//		vec2 clipZW = new_point.z * PROJ[2].zw + PROJ[3].zw;
//		float depth = 0.5 + 0.5 * clipZW.x / clipZW.y;
//
//		// this is a workaround necessary for Mac, otherwise the modified fragment won't clip properly
//		if (depth <= 0.0 || depth >= 1.0)
//			discard;
//
//		gl_FragDepth = depth;
//		FragColor = vec4(depth, 0, 0, 1.0);
//	}
//})";
//
//
//std::string shadow_lines_cylinders_geom = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//// It uses geometry shader for vertex generation.
//// The code does not cover round caps.
//
////#extension GL_EXT_geometry_shader4 : enable
//
//layout(lines) in;
//layout(triangle_strip, max_vertices = 6) out;
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//uniform float	radius;
//
//out Data{
//	vec3 point;// camera space
//	vec3 axis;
//	vec3 base;
//	vec3 end;
//	vec3 U;
//	vec3 V;
//} DataOut;
//
//
//void main()
//{
//	vec3  base = gl_in[0].gl_Position.xyz;  // in camera space
//	vec3  top = gl_in[1].gl_Position.xyz;   // in camera space
//	vec3  len_dir = top - base;
//	vec3  view_dir = vec3(0,0,1);	// this is view direction for orthographic mode in camera space
//
//	float b = dot(view_dir, len_dir);
//	if (b < 0.0) {// direction vector looks away, so flip
//		len_dir = -len_dir;
//		//swap(base, top);
//		vec3 tmp = base;
//		base = top;
//		top = tmp;
//			}
//
//	vec3 left = cross(view_dir, len_dir);
//	vec3 up = cross(left, len_dir);
//	DataOut.axis = normalize(len_dir);
//	DataOut.U = normalize(up);
//	DataOut.V = normalize(left);
//	left = radius * DataOut.V;
//	up = radius * DataOut.U;
//
//	DataOut.base = base;
//	DataOut.end = top;
//
//	// Vertex 1
//	DataOut.point = (base + left - up);
//	gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//	}
//	EmitVertex();
//
//	// Vertex 2
//	DataOut.point = (base - left - up);
//	gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//	}
//	EmitVertex();
//
//	// Vertex 3
//	DataOut.point = (top + left - up);
//	gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//	}
//	EmitVertex();
//
//	// Vertex 4
//	DataOut.point = (top - left - up);
//	gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//	}
//	EmitVertex();
//
//	// Vertex 5
//	DataOut.point = (top + left + up);
//	gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//	}
//	EmitVertex();
//
//	// Vertex 6
//	DataOut.point = (top - left + up);
//	gl_Position = PROJ  * vec4(DataOut.point, 1.0);
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(invMV * vec4(DataOut.point, 1.0), clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(invMV *vec4(DataOut.point, 1.0), clippingPlane1);
//	}
//	EmitVertex();
//
//	EndPrimitive();
//})";
//
//
//std::string shadow_lines_cylinders_vert = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//// It uses geometry shader for vertex generation.
//// The code does not cover round caps.
//
////#extension GL_EXT_gpu_shader4 : enable
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//in  vec3 vtx_position;	// point position
//
//void main()
//{
//	gl_Position = MV * MANIP * vec4(vtx_position, 1.0);
//})";
//
//
//
//
//std::string shadow_lines_plain_frag = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//uniform bool	generateShadowMap;
//
//out vec4 FragColor;	// Ouput data
//
//void main()
//{
//	if (generateShadowMap) {
//		// write the depth into the RED component
//		FragColor = vec4(gl_FragCoord.z, 0, 0, 1.0);
//	}
//	else {
//		// not needed, just supress the "generateShadowMap not active" warning
//		discard; 
//	}
//})";
//
//
//std::string shadow_lines_plain_vert = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//in  vec3 vtx_position;	// point position
//
//void main()
//{
//	vec4 new_position = MANIP * vec4(vtx_position, 1.0);
//
//	gl_Position = MVP * new_position;
//
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(new_position, clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(new_position, clippingPlane1);
//	}
//})";
//
//
//
//
//std::string shadow_points_plain_frag = R"(
//#version 430
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//layout(std140) uniform Lighting {
//	vec3	wLightPos;
//	vec3	eLightPos;
//	vec3	wCamPos;
//	vec3	ambient;		// in [0, 1], r==g==b;
//	vec3	specular;		// in [0, 1], r==g==b;
//	float	shininess;
//};
//
//uniform bool generateShadowMap;
//
//uniform int  samplePattern;
//
//uniform sampler2DShadow  shadowMapPCF;
//
//uniform vec2  lightRadiusUV;
//uniform float lightZNear;
//uniform float darkness;
//uniform mat4  lightViewMatrix;
//
//uniform bool  useTexture;		// otherwise use the color attribute or default color
//uniform sampler2D textureID;
//
//uniform bool lightingEnabled;
//uniform bool two_sides_lighting;
//
//uniform int  hightlight_id;
//uniform bool selected;
//
//uniform bool distinct_back_color;
//uniform vec3 back_color;
//
//layout(std430, binding = 1) buffer selection_t {
//	uint data[];
//} selection;
//
//in Data{
//	vec3 position;
//	vec3 normal;
//	vec3 color;
//	vec2 texcoord;
//	vec4 shadowCoord;
//} DataIn;
//
//
//out vec4 FragColor;	// Ouput data
//
//#define POISSON_25_25   0
//#define POISSON_32_64   1
//#define POISSON_100_100 2
//#define POISSON_64_128  3
//#define REGULAR_49_225  4
//
//const vec2 Poisson25[25] = vec2[](
//	vec2(-0.978698, -0.0884121),
//	vec2(-0.841121, 0.521165),
//	vec2(-0.71746, -0.50322),
//	vec2(-0.702933, 0.903134),
//	vec2(-0.663198, 0.15482),
//	vec2(-0.495102, -0.232887),
//	vec2(-0.364238, -0.961791),
//	vec2(-0.345866, -0.564379),
//	vec2(-0.325663, 0.64037),
//	vec2(-0.182714, 0.321329),
//	vec2(-0.142613, -0.0227363),
//	vec2(-0.0564287, -0.36729),
//	vec2(-0.0185858, 0.918882),
//	vec2(0.0381787, -0.728996),
//	vec2(0.16599, 0.093112),
//	vec2(0.253639, 0.719535),
//	vec2(0.369549, -0.655019),
//	vec2(0.423627, 0.429975),
//	vec2(0.530747, -0.364971),
//	vec2(0.566027, -0.940489),
//	vec2(0.639332, 0.0284127),
//	vec2(0.652089, 0.669668),
//	vec2(0.773797, 0.345012),
//	vec2(0.968871, 0.840449),
//	vec2(0.991882, -0.657338)
//	);
//
//const vec2 Poisson32[32] = vec2[](
//	vec2(-0.975402, -0.0711386),
//	vec2(-0.920347, -0.41142),
//	vec2(-0.883908, 0.217872),
//	vec2(-0.884518, 0.568041),
//	vec2(-0.811945, 0.90521),
//	vec2(-0.792474, -0.779962),
//	vec2(-0.614856, 0.386578),
//	vec2(-0.580859, -0.208777),
//	vec2(-0.53795, 0.716666),
//	vec2(-0.515427, 0.0899991),
//	vec2(-0.454634, -0.707938),
//	vec2(-0.420942, 0.991272),
//	vec2(-0.261147, 0.588488),
//	vec2(-0.211219, 0.114841),
//	vec2(-0.146336, -0.259194),
//	vec2(-0.139439, -0.888668),
//	vec2(0.0116886, 0.326395),
//	vec2(0.0380566, 0.625477),
//	vec2(0.0625935, -0.50853),
//	vec2(0.125584, 0.0469069),
//	vec2(0.169469, -0.997253),
//	vec2(0.320597, 0.291055),
//	vec2(0.359172, -0.633717),
//	vec2(0.435713, -0.250832),
//	vec2(0.507797, -0.916562),
//	vec2(0.545763, 0.730216),
//	vec2(0.56859, 0.11655),
//	vec2(0.743156, -0.505173),
//	vec2(0.736442, -0.189734),
//	vec2(0.843562, 0.357036),
//	vec2(0.865413, 0.763726),
//	vec2(0.872005, -0.927)
//	);
//
//const vec2 Poisson64[64] = vec2[](
//	vec2(-0.934812, 0.366741),
//	vec2(-0.918943, -0.0941496),
//	vec2(-0.873226, 0.62389),
//	vec2(-0.8352, 0.937803),
//	vec2(-0.822138, -0.281655),
//	vec2(-0.812983, 0.10416),
//	vec2(-0.786126, -0.767632),
//	vec2(-0.739494, -0.535813),
//	vec2(-0.681692, 0.284707),
//	vec2(-0.61742, -0.234535),
//	vec2(-0.601184, 0.562426),
//	vec2(-0.607105, 0.847591),
//	vec2(-0.581835, -0.00485244),
//	vec2(-0.554247, -0.771111),
//	vec2(-0.483383, -0.976928),
//	vec2(-0.476669, -0.395672),
//	vec2(-0.439802, 0.362407),
//	vec2(-0.409772, -0.175695),
//	vec2(-0.367534, 0.102451),
//	vec2(-0.35313, 0.58153),
//	vec2(-0.341594, -0.737541),
//	vec2(-0.275979, 0.981567),
//	vec2(-0.230811, 0.305094),
//	vec2(-0.221656, 0.751152),
//	vec2(-0.214393, -0.0592364),
//	vec2(-0.204932, -0.483566),
//	vec2(-0.183569, -0.266274),
//	vec2(-0.123936, -0.754448),
//	vec2(-0.0859096, 0.118625),
//	vec2(-0.0610675, 0.460555),
//	vec2(-0.0234687, -0.962523),
//	vec2(-0.00485244, -0.373394),
//	vec2(0.0213324, 0.760247),
//	vec2(0.0359813, -0.0834071),
//	vec2(0.0877407, -0.730766),
//	vec2(0.14597, 0.281045),
//	vec2(0.18186, -0.529649),
//	vec2(0.188208, -0.289529),
//	vec2(0.212928, 0.063509),
//	vec2(0.23661, 0.566027),
//	vec2(0.266579, 0.867061),
//	vec2(0.320597, -0.883358),
//	vec2(0.353557, 0.322733),
//	vec2(0.404157, -0.651479),
//	vec2(0.410443, -0.413068),
//	vec2(0.413556, 0.123325),
//	vec2(0.46556, -0.176183),
//	vec2(0.49266, 0.55388),
//	vec2(0.506333, 0.876888),
//	vec2(0.535875, -0.885556),
//	vec2(0.615894, 0.0703452),
//	vec2(0.637135, -0.637623),
//	vec2(0.677236, -0.174291),
//	vec2(0.67626, 0.7116),
//	vec2(0.686331, -0.389935),
//	vec2(0.691031, 0.330729),
//	vec2(0.715629, 0.999939),
//	vec2(0.8493, -0.0485549),
//	vec2(0.863582, -0.85229),
//	vec2(0.890622, 0.850581),
//	vec2(0.898068, 0.633778),
//	vec2(0.92053, -0.355693),
//	vec2(0.933348, -0.62981),
//	vec2(0.95294, 0.156896)
//	);
//
//const vec2 Poisson100[100] = vec2[](
//	vec2(-0.9891574, -0.1059512),
//	vec2(-0.9822294, 0.05140843),
//	vec2(-0.961332, 0.2562195),
//	vec2(-0.9149657, -0.2404464),
//	vec2(-0.8896608, -0.4183828),
//	vec2(-0.8398135, 0.3748641),
//	vec2(-0.8149028, 0.1989844),
//	vec2(-0.8046502, 0.5212684),
//	vec2(-0.7970151, -0.5834194),
//	vec2(-0.7484995, -0.3153634),
//	vec2(-0.738582, -0.09323367),
//	vec2(-0.695694, 0.08865929),
//	vec2(-0.6868832, 0.6336682),
//	vec2(-0.6751406, 0.2777427),
//	vec2(-0.666558, -0.6801786),
//	vec2(-0.631489, -0.4702293),
//	vec2(-0.5870083, 0.518836),
//	vec2(-0.5744062, -0.06333278),
//	vec2(-0.5667221, 0.1699501),
//	vec2(-0.5537653, 0.7677022),
//	vec2(-0.5337034, 0.3299558),
//	vec2(-0.5201509, -0.2033358),
//	vec2(-0.4873925, -0.8545401),
//	vec2(-0.4712743, -0.3607009),
//	vec2(-0.4524891, -0.5142469),
//	vec2(-0.4421883, -0.6830674),
//	vec2(-0.4293752, 0.6299667),
//	vec2(-0.4240644, 0.8706763),
//	vec2(-0.4139857, 0.1598689),
//	vec2(-0.3838707, 0.4078749),
//	vec2(-0.3688077, -0.0358762),
//	vec2(-0.3432877, -0.2311365),
//	vec2(-0.3256257, -0.9325441),
//	vec2(-0.2751555, 0.302412),
//	vec2(-0.2679778, -0.654425),
//	vec2(-0.2554769, -0.4441924),
//	vec2(-0.243476, -0.8034022),
//	vec2(-0.2367678, -0.108045),
//	vec2(-0.2196257, 0.8243803),
//	vec2(-0.2119443, 0.06230118),
//	vec2(-0.1708038, -0.9437978),
//	vec2(-0.1694005, 0.5692244),
//	vec2(-0.136494, 0.3937041),
//	vec2(-0.1318274, -0.2166154),
//	vec2(-0.09781472, -0.5743775),
//	vec2(-0.09480921, 0.2369129),
//	vec2(-0.07638182, -0.0571501),
//	vec2(-0.06661344, -0.7966294),
//	vec2(-0.06305461, -0.3521975),
//	vec2(-0.04525706, 0.6982157),
//	vec2(-0.04149697, 0.9666064),
//	vec2(-0.003192461, -0.9693027),
//	vec2(0.0104818, 0.5000805),
//	vec2(0.03228819, -0.1681713),
//	vec2(0.03715288, -0.673852),
//	vec2(0.08470399, -0.3922319),
//	vec2(0.09848712, -0.8374477),
//	vec2(0.09940207, 0.1117471),
//	vec2(0.1395643, 0.313647),
//	vec2(0.1565993, 0.8555924),
//	vec2(0.1772605, -0.5248074),
//	vec2(0.1899546, 0.5249656),
//	vec2(0.1952665, -0.9595091),
//	vec2(0.213078, -0.07045701),
//	vec2(0.2277649, -0.3361143),
//	vec2(0.247221, 0.7353553),
//	vec2(0.2493455, -0.6874771),
//	vec2(0.269915, 0.07673722),
//	vec2(0.3039587, 0.9087375),
//	vec2(0.3189922, 0.3008468),
//	vec2(0.3215453, -0.1954931),
//	vec2(0.3593478, 0.4527411),
//	vec2(0.3745022, -0.597945),
//	vec2(0.3879738, -0.7821383),
//	vec2(0.4522015, 0.6819367),
//	vec2(0.4591872, -0.4484442),
//	vec2(0.4626173, -0.03955235),
//	vec2(0.4751598, 0.2083394),
//	vec2(0.4894366, 0.8694122),
//	vec2(0.4896614, -0.2676601),
//	vec2(0.5070116, -0.6733028),
//	vec2(0.5525513, 0.436241),
//	vec2(0.5542312, -0.8262905),
//	vec2(0.6012187, 0.7003717),
//	vec2(0.6075609, -0.1610506),
//	vec2(0.6291932, 0.2213627),
//	vec2(0.6300695, -0.5324634),
//	vec2(0.6613995, -0.7056449),
//	vec2(0.6699739, -0.3828001),
//	vec2(0.6705787, 0.01011722),
//	vec2(0.6814164, 0.5618623),
//	vec2(0.7808329, 0.261445),
//	vec2(0.7830279, -0.1817809),
//	vec2(0.8006546, -0.5266678),
//	vec2(0.8030878, 0.4266291),
//	vec2(0.8259325, 0.08734058),
//	vec2(0.8621388, -0.3646038),
//	vec2(0.9531851, 0.3011991),
//	vec2(0.9578334, -0.1584408),
//	vec2(0.9898114, 0.1029227)
//	);
//
//const vec2 Poisson128[128] = vec2[](
//	vec2(-0.9406119, 0.2160107),
//	vec2(-0.920003, 0.03135762),
//	vec2(-0.917876, -0.2841548),
//	vec2(-0.9166079, -0.1372365),
//	vec2(-0.8978907, -0.4213504),
//	vec2(-0.8467999, 0.5201505),
//	vec2(-0.8261013, 0.3743192),
//	vec2(-0.7835162, 0.01432008),
//	vec2(-0.779963, 0.2161933),
//	vec2(-0.7719588, 0.6335353),
//	vec2(-0.7658782, -0.3316436),
//	vec2(-0.7341912, -0.5430729),
//	vec2(-0.6825727, -0.1883408),
//	vec2(-0.6777467, 0.3313724),
//	vec2(-0.662191, 0.5155144),
//	vec2(-0.6569989, -0.7000636),
//	vec2(-0.6021447, 0.7923283),
//	vec2(-0.5980815, -0.5529259),
//	vec2(-0.5867089, 0.09857152),
//	vec2(-0.5774597, -0.8154474),
//	vec2(-0.5767041, -0.2656419),
//	vec2(-0.575091, -0.4220052),
//	vec2(-0.5486979, -0.09635002),
//	vec2(-0.5235587, 0.6594529),
//	vec2(-0.5170338, -0.6636339),
//	vec2(-0.5114055, 0.4373561),
//	vec2(-0.4844725, 0.2985838),
//	vec2(-0.4803245, 0.8482798),
//	vec2(-0.4651957, -0.5392771),
//	vec2(-0.4529685, 0.09942394),
//	vec2(-0.4523471, -0.3125569),
//	vec2(-0.4268422, 0.5644538),
//	vec2(-0.4187512, -0.8636028),
//	vec2(-0.4160798, -0.0844868),
//	vec2(-0.3751733, 0.2196607),
//	vec2(-0.3656596, -0.7324334),
//	vec2(-0.3286595, -0.2012637),
//	vec2(-0.3147397, -0.0006635741),
//	vec2(-0.3135846, 0.3636878),
//	vec2(-0.3042951, -0.4983553),
//	vec2(-0.2974239, 0.7496996),
//	vec2(-0.2903037, 0.8890813),
//	vec2(-0.2878664, -0.8622097),
//	vec2(-0.2588971, -0.653879),
//	vec2(-0.2555692, 0.5041648),
//	vec2(-0.2553292, -0.3389159),
//	vec2(-0.2401368, 0.2306108),
//	vec2(-0.2124457, -0.09935001),
//	vec2(-0.1877905, 0.1098409),
//	vec2(-0.1559879, 0.3356432),
//	vec2(-0.1499449, 0.7487829),
//	vec2(-0.146661, -0.9256138),
//	vec2(-0.1342774, 0.6185387),
//	vec2(-0.1224529, -0.3887629),
//	vec2(-0.116467, 0.8827716),
//	vec2(-0.1157598, -0.539999),
//	vec2(-0.09983152, -0.2407187),
//	vec2(-0.09953719, -0.78346),
//	vec2(-0.08604223, 0.4591112),
//	vec2(-0.02128129, 0.1551989),
//	vec2(-0.01478849, 0.6969455),
//	vec2(-0.01231739, -0.6752576),
//	vec2(-0.005001599, -0.004027164),
//	vec2(0.00248426, 0.567932),
//	vec2(0.00335562, 0.3472346),
//	vec2(0.009554717, -0.4025437),
//	vec2(0.02231783, -0.1349781),
//	vec2(0.04694207, -0.8347212),
//	vec2(0.05412609, 0.9042216),
//	vec2(0.05812819, -0.9826952),
//	vec2(0.1131321, -0.619306),
//	vec2(0.1170737, 0.6799788),
//	vec2(0.1275105, 0.05326218),
//	vec2(0.1393405, -0.2149568),
//	vec2(0.1457873, 0.1991508),
//	vec2(0.1474208, 0.5443151),
//	vec2(0.1497117, -0.3899909),
//	vec2(0.1923773, 0.3683496),
//	vec2(0.2110928, -0.7888536),
//	vec2(0.2148235, 0.9586087),
//	vec2(0.2152219, -0.1084362),
//	vec2(0.2189204, -0.9644538),
//	vec2(0.2220028, -0.5058427),
//	vec2(0.2251696, 0.779461),
//	vec2(0.2585723, 0.01621339),
//	vec2(0.2612841, -0.2832426),
//	vec2(0.2665483, -0.6422054),
//	vec2(0.2939872, 0.1673226),
//	vec2(0.3235748, 0.5643662),
//	vec2(0.3269232, 0.6984669),
//	vec2(0.3425438, -0.1783788),
//	vec2(0.3672505, 0.4398117),
//	vec2(0.3755714, -0.8814359),
//	vec2(0.379463, 0.2842356),
//	vec2(0.3822978, -0.381217),
//	vec2(0.4057849, -0.5227674),
//	vec2(0.4168737, -0.6936938),
//	vec2(0.4202749, 0.8369391),
//	vec2(0.4252189, 0.03818182),
//	vec2(0.4445904, -0.09360636),
//	vec2(0.4684285, 0.5885228),
//	vec2(0.4952184, -0.2319764),
//	vec2(0.5072351, 0.3683765),
//	vec2(0.5136194, -0.3944138),
//	vec2(0.519893, 0.7157083),
//	vec2(0.5277841, 0.1486474),
//	vec2(0.5474944, -0.7618791),
//	vec2(0.5692734, 0.4852227),
//	vec2(0.582229, -0.5125455),
//	vec2(0.583022, 0.008507785),
//	vec2(0.6500257, 0.3473313),
//	vec2(0.6621304, -0.6280518),
//	vec2(0.6674218, -0.2260806),
//	vec2(0.6741871, 0.6734863),
//	vec2(0.6753459, 0.1119422),
//	vec2(0.7083091, -0.4393666),
//	vec2(0.7106963, -0.102099),
//	vec2(0.7606754, 0.5743545),
//	vec2(0.7846709, 0.2282225),
//	vec2(0.7871446, 0.3891495),
//	vec2(0.8071781, -0.5257092),
//	vec2(0.8230689, 0.002674922),
//	vec2(0.8531976, -0.3256475),
//	vec2(0.8758298, -0.1824844),
//	vec2(0.8797691, 0.1284946),
//	vec2(0.926309, 0.3576975),
//	vec2(0.9608918, -0.03495717),
//	vec2(0.972032, 0.2271516)
//	);
//
//
//
//bool isBlack(vec3 c)
//{
//	return (dot(c, c) == 0.0);
//}
//
//
//float borderPCFTexture(sampler2DShadow tex, vec3 uvz)
//{
//	return ((uvz.x <= 1.0) && (uvz.y <= 1.0) &&
//		(uvz.x >= 0.0) && (uvz.y >= 0.0)) ? texture(tex, uvz) :
//		((uvz.z <= 1.0) ? 1.0 : 0.0);
//}
//
//
//float biasedZ(float z0)
//{
//	return z0 - 0.005;
//}
//
//
//// Using similar triangles from the surface point to the area light
//vec2 searchRegionRadiusUV(float zWorld)
//{
//	return lightRadiusUV * (zWorld - lightZNear) / zWorld;
//}
//
//
//// Performs PCF filtering on the shadow map using multiple taps in the filter region.
//float pcfFilter(vec2 uv, float z0, vec2 filterRadiusUV)
//{
//	float sum = 0.0;
//	float z = biasedZ(z0);
//
//	switch (samplePattern)
//	{
//	case POISSON_25_25:
//	{
//		for (int i = 0; i < 25; ++i) {
//			vec2 offset = Poisson25[i] * filterRadiusUV;
//			sum += borderPCFTexture(shadowMapPCF, vec3(uv + offset, z));
//		}
//		return sum / 25.0;
//	}
//
//	case POISSON_32_64:
//	{
//		for (int i = 0; i < 64; ++i) {
//			vec2 offset = Poisson64[i] * filterRadiusUV;
//			sum += borderPCFTexture(shadowMapPCF, vec3(uv + offset, z));
//		}
//		return sum / 64.0;
//	}
//
//	case POISSON_100_100:
//	{
//		for (int i = 0; i < 100; ++i) {
//			vec2 offset = Poisson100[i] * filterRadiusUV;
//			sum += borderPCFTexture(shadowMapPCF, vec3(uv + offset, z));
//		}
//		return sum / 100.0;
//	}
//
//	case POISSON_64_128:
//	{
//		for (int i = 0; i < 128; ++i) {
//			vec2 offset = Poisson128[i] * filterRadiusUV;
//			sum += borderPCFTexture(shadowMapPCF, vec3(uv + offset, z));
//		}
//		return sum / 128.0;
//	}
//
//	case REGULAR_49_225:
//	{
//		vec2 stepUV = filterRadiusUV / 7.0;
//		for (int x = -7; x <= 7; ++x) {
//			for (int y = -7; y <= 7; ++y) {
//				vec2 offset = vec2(x, y) * stepUV;
//				sum += borderPCFTexture(shadowMapPCF, vec3(uv + offset, z));
//			}
//		}
//		float numSamples = 7.0 * 2.0 + 1.0;
//		return sum / (numSamples * numSamples);
//	}
//
//	default:
//		return 1.0;
//	}
//}
//
//float pcfShadow(vec2 uv, float z, float zEye)
//{
//	// ------------------------
//	// STEP 1: blocker search
//	// ------------------------
//	//float accumBlockerDepth, numBlockers, maxBlockers;
//	vec2 searchRadiusUV = searchRegionRadiusUV(zEye);
//
//	return pcfFilter(uv, z, searchRadiusUV);
//}
//
//
//vec3 shade(vec3 worldPos)
//{
//	vec3 color;
//	if (useTexture)
//		color = texture(textureID, DataIn.texcoord).rgb;
//	else
//		color = DataIn.color;
//
//	vec3 view_dir = normalize(wCamPos - worldPos);
//	vec3 normal = DataIn.normal;
//	if (lightingEnabled && distinct_back_color) {
//		if (dot(normal, view_dir) < 0)
//			color = back_color;
//	}
//
//	//uint addr = gl_PrimitiveID / 32;
//	//uint offs = gl_PrimitiveID % 32;
//	uint addr = gl_PrimitiveID >> 5;
//	uint offs = gl_PrimitiveID & 31;
//	if ((selection.data[addr] & (1 << offs)) != 0)
//		color = mix(color, vec3(1.0, 0.0, 0.0), 0.8);
//	else if (gl_PrimitiveID == hightlight_id)
//		color = vec3(1.0, 0.0, 0.0);
//
//	if (selected) {
//		color = vec3(1.0, 0.0, 0.0);
//	}
//
//	if (lightingEnabled) {
//		vec3 light_dir = normalize(wLightPos);	// directional light
//		//vec3 light_dir = normalize(wLightPos - worldPos); // point light
//
//		float df = 0.0;	// diffuse factor
//		if (two_sides_lighting)
//			df = abs(dot(light_dir, normal));
//		else
//			df = max(dot(light_dir, normal), 0);
//
//		float sf = 0.0;	// specular factor
//		if (df > 0.0) {	// if the vertex is lit compute the specular color
//			vec3 half_vector = normalize(light_dir + view_dir);	// compute the half vector
//			if (two_sides_lighting)
//				sf = abs(dot(half_vector, normal));
//			else
//				sf = max(dot(half_vector, normal), 0.0);
//			sf = pow(sf, shininess);
//		}
//	
//		color = color * df + specular * sf;
//	}
//
//	return color;
//}
//
//
//void main(void) {
//	if (generateShadowMap) {
//		// write the depth into the RED component
//		FragColor = vec4(gl_FragCoord.z, 0, 0, 1.0);
//	}
//	else {
//		vec3 color = shade(DataIn.position);
//
//		if (lightingEnabled) 
//		{
//			// transforming from clip space to NDC space
//			vec3 ProjCoords = DataIn.shadowCoord.xyz / DataIn.shadowCoord.w;
//			// It seems checking Z is sufficient (implies X and Y)?
//			if ((isBlack(color.rgb)) ||
//				//(ProjCoords.x <= 0 || ProjCoords.x >= 1) ||
//				//(ProjCoords.y <= 0 || ProjCoords.y >= 1) ||
//				(ProjCoords.z <= 0 || ProjCoords.z >= 1))
//			{
//				FragColor = vec4(color + ambient, 1.0);
//			}
//			else {
//				vec2 uv = ProjCoords.xy;
//				float z = ProjCoords.z;
//
//				// Eye-space z from the light's point of view
//				float zEye = -(lightViewMatrix * vec4(DataIn.position, 1.0)).z;
//				float shadow = pcfShadow(uv, z, zEye);
//
//				// I don't want the shadow regions to be completely dark.
//				shadow = (1.0 - darkness) + shadow * darkness;
//
//				FragColor = vec4(color * shadow + ambient, 1.0);
//			}
//		}
//		else
//			FragColor = vec4(color, 1.0);
//	}
//})";
//
//
//std::string shadow_points_plain_vert = R"(
//#version 430
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//in  vec3 vtx_position;	// vertex position
//in	vec3 vtx_normal;	// vertex normal
//in  vec3 vtx_color;		// vertex color
//in  vec2 vtx_texcoord;
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//uniform vec3 default_color;
//uniform bool per_vertex_color;
//
//// the data to be sent to the fragment shader
//out Data{
//	vec3 position;
//	vec3 normal;
//	vec3 color;
//	vec2 texcoord;
//	vec4 shadowCoord;
//} DataOut;
//
//
//void main() {
//	vec4 new_position = MANIP * vec4(vtx_position, 1.0);
//	DataOut.position = new_position.xyz;
//	DataOut.normal = NORMAL * vtx_normal;
//	DataOut.shadowCoord = SHADOW * new_position;
//
//	DataOut.texcoord = vtx_texcoord;
//	if (per_vertex_color)
//		DataOut.color = vtx_color;
//	else
//		DataOut.color = default_color;
//
//	gl_Position = MVP * new_position;
//
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(new_position, clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(new_position, clippingPlane1);
//	}
//})";
//
//
//
//
//std::string shadow_points_plain_texture_frag = R"(
//#version 430
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//
//layout(std140) uniform Lighting {
//	vec3	wLightPos;
//	vec3	eLightPos;
//	vec3	wCamPos;
//	vec3	ambient;		// in [0, 1], r==g==b;
//	vec3	specular;		// in [0, 1], r==g==b;
//	float	shininess;
//};
//
//layout(std430, binding = 1) buffer selection_t {
//	uint data[];
//} selection;
//
//
//uniform bool lighting;	// true if lighting is on
//uniform bool two_sides_lighting;
//
//uniform int	 hightlight_id;
//uniform bool selected;
//
//uniform sampler2D	textureID;
//
/////-----------------------------------------
//
//uniform bool generateShadowMap;
//
//
//#define USE_RandomSampling
//
//// Liangliang: Random Sampling dosn't produce very good shadow effect on the model, 
////			   but OK on ground plane. So I use a smaller blur radius for models.
//#define MODEL_BLUR_RADIUS_FACTOR  0.5  
//
//uniform sampler2DShadow  ShadowMap;
//
//uniform bool selfShadow;
//
//uniform bool isGroundPlane;
//uniform vec3 backgroundColor;
//
//uniform sampler3D	OffsetTex;
//uniform vec3		OffsetTexSize; // (width, height, depth)
//uniform int			BlurRadius;
//
/////-----------------------------------------
//
//in Data{
//	vec3 position;
//	vec2 texcoord;
//	vec3 normal;
//	vec4 shadowCoord;
//} DataIn;
//
//out vec4 FragColor;	// Ouput data
//
//
//// We only shade if the current fragment��s depth (in light space) is really less (by a threshold) 
//// than lightmap value. But this has the 'Peter Panning' problem, see 
//// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/
//#define EPSILON 0.01
//
//
//float ShadowFactor(vec4 LightSpacePos, float threshold, int blur_radius)
//{
//	// transforming from clip space to NDC space
//	vec3 ProjCoords = LightSpacePos.xyz / LightSpacePos.w; 
//	// Liangliang: To avoid sample values out of the light's view frustum.
//	if ((ProjCoords.x <= -1 || ProjCoords.x >= 1) || (ProjCoords.y <= -1 || ProjCoords.y >= 1) || (ProjCoords.z <= -1 || ProjCoords.z >= 1))
//		return 1.0;
//	
//	// transforming X from NDC space to texture space
//	ProjCoords.x = 0.5 * ProjCoords.x + 0.5;	
//	ProjCoords.y = 0.5 * ProjCoords.y + 0.5;
//	ProjCoords.z = 0.5 * ProjCoords.z + 0.5 - threshold;
//
//#ifdef USE_RandomSampling
//
//	ivec2 ShadowMapSize = textureSize(ShadowMap, 0);
//	float radius = float(blur_radius) / ShadowMapSize.x;
//
//	float Factor = 0.0;
//
//	ivec3 offsetCoord;
//	offsetCoord.xy = ivec2(mod(gl_FragCoord.xy, OffsetTexSize.xy));
//
//	int samplesDiv2 = int(OffsetTexSize.z);
//	vec4 sc = vec4(ProjCoords, LightSpacePos.w);
//
//	for (int i = 0; i < 4; i++) {
//		offsetCoord.z = i;
//		vec4 offsets = texelFetch(OffsetTex, offsetCoord, 0) * radius * LightSpacePos.w;
//		sc.xy = ProjCoords.xy + offsets.xy;
//		Factor += textureProj(ShadowMap, sc);
//		sc.xy = ProjCoords.xy + offsets.zw;
//		Factor += textureProj(ShadowMap, sc);
//	}
//	float shadow = Factor / 8.0;
//
//	// The first loop tests the first eight samples. If the first eight samples are all 0.0 or 
//	// 1.0, then we assume that all of the samples will be the same (the sample area is completely 
//	// in or out of the shadow). In that case, we skip the evaluation of the rest of the samples. 
//	// Otherwise, we evaluate the following samples and compute the overall average.
//	if (shadow != 1.0 && shadow != 0.0) {
//		for (int i = 4; i < samplesDiv2; i++) {
//			offsetCoord.z = i;
//			vec4 offsets = texelFetch(OffsetTex, offsetCoord, 0) * radius * LightSpacePos.w;
//			sc.xy = ProjCoords.xy + offsets.xy;
//			Factor += textureProj(ShadowMap, sc);
//			sc.xy = ProjCoords.xy + offsets.zw;
//			Factor += textureProj(ShadowMap, sc);
//		}
//		shadow = Factor / float(samplesDiv2 * 2.0);
//	}
//	//return shadow;			  // in [0, 1], too dark   
//	return 0.5 + shadow / 2.0;	  // in [0.5, 1.0], not too dark
//
//#else // Standard PCF
//
//	float Factor = 0.0;
//	float sum = (blur_radius * 2 + 1) * (blur_radius * 2 + 1);
//	for (int y = -blur_radius; y <= blur_radius; y++) {
//		for (int x = -blur_radius; x <= blur_radius; x++) {
//			Factor += textureProjOffset(ShadowMap, vec4(ProjCoords, LightSpacePos.w), ivec2(x, y));
//		}
//	}
//	//return (Factor / sum);			// in [0, 1], too dark   
//	return 0.5 + Factor / (sum * 2.0);  // in [0.5, 1.0], not too dark
//
//#endif
//}
//
//
//void main()
//{
//	if (generateShadowMap) {
//	}
//	else {
//		vec3 color = texture(textureID, DataIn.texcoord).rgb;
//
//		vec3 normal = normalize(DataIn.normal);
//		vec3 light_dir = normalize(wLightPos);
//
//		//------------------------------------------------------------
//		// compupte the scalable bias according to the slope
//		float cosTheta = clamp(dot(normal, light_dir), 0.0, 1.0);
//		float threshold = EPSILON * tan(acos(cosTheta));
//		threshold = clamp(threshold, 0.0, EPSILON * 2.0);
//		//------------------------------------------------------------
//
//		if (isGroundPlane) {
//			float shadow = ShadowFactor(DataIn.shadowCoord, threshold, BlurRadius);
//			if (shadow < 0.9999999) { // the background is in shadow
//				// Consider ambient as a portion of the backgroundColor. Otherwise an 
//				// undersired border may appear in penumbra when changing ambient.
//				//FragColor = vec4(backgroundColor * shadow + ambient, 1.0);
//				float factor = clamp(shadow + ambient.x, 0, 1);
//				FragColor = vec4(backgroundColor * factor, 1.0);
//			}
//			else { // the background is lit
//				discard;
//				//FragColor = vec4(vec3(0.9, 0.9, 0.9) * shadow, 1.0);
//			}
//		}
//		else {
//			if (lighting) {
//				float df = 0.0;	// diffuse factor
//				if (two_sides_lighting)
//					df = abs(dot(light_dir, normal));
//				else
//					df = max(dot(light_dir, normal), 0);
//
//				float sf = 0.0;	// specular factor
//				if (df > 0.0) {	// if the vertex is lit compute the specular color
//					vec3 view_dir = normalize(wCamPos - DataIn.position);	// compute view direction and normalize it 
//					vec3 half_vector = normalize(light_dir + view_dir);	// compute the half vector
//
//					if (two_sides_lighting)
//						sf = abs(dot(half_vector, normal));
//					else
//						sf = max(dot(half_vector, normal), 0.0);
//
//					sf = pow(sf, shininess);
//				}
//
//				color = color * df + specular * sf;
//			}
//
//			//uint addr = gl_PrimitiveID / 32;
//			//uint offs = gl_PrimitiveID % 32;
//			uint addr = gl_PrimitiveID >> 5;
//			uint offs = gl_PrimitiveID & 31;
//			if ((selection.data[addr] & (1 << offs)) != 0)
//				color = mix(color, vec3(1.0, 0.0, 0.0), 0.8);
//			else if (gl_PrimitiveID == hightlight_id)
//				color = vec3(1.0, 0.0, 0.0);
//
//			if (selected) {
//				color = vec3(1.0, 0.0, 0.0);
//			}
//
//			// If the fragment is in shadow, use ambient light only.
//			if (selfShadow) {
//				float shadow = ShadowFactor(DataIn.shadowCoord, threshold, int(BlurRadius * MODEL_BLUR_RADIUS_FACTOR));
//				FragColor = vec4(color * shadow + ambient, 1.0);
//			}
//			else
//				FragColor = vec4(color + ambient, 1.0);
//		}
//	}
//})";
//
//
//std::string shadow_points_plain_texture_vert = R"(
//#version 430
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//in vec3 vtx_position;
//in vec2 vtx_texcoord;
//in vec3 vtx_normal;
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool cs_enabled;
//	bool cs_thin;
//	vec4 cs_plane0;
//	vec4 cs_plane1;
//};
//
//// the data to be sent to the fragment shader
//out Data{
//	vec3 position;
//	vec2 texcoord;
//	vec3 normal;
//	vec4 shadowCoord;
//} DataOut;
//
//
//void main() {
//
//	vec4 new_position = MANIP * vec4(vtx_position, 1.0);
//
//	DataOut.texcoord = vtx_texcoord;
//	DataOut.normal = NORMAL * vtx_normal;
//	DataOut.shadowCoord = SHADOW * new_position;
//
//	// Output position of the vertex, in clip space : MVP * position
//	gl_Position = MVP * new_position;
//
//	if (cs_enabled) {
//		gl_ClipDistance[0] = dot(new_position, cs_plane0);
//		if (cs_thin)
//			gl_ClipDistance[1] = dot(new_position, cs_plane1);
//	}
//}
//)";
//
//
//
//
//std::string shadow_points_spheres_geometry_frag = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//uniform bool	generateShadowMap;
//
//uniform float	sphere_radius;
//
//in Data{
//	smooth	vec2	tex;
//	flat	vec4	position;
//} DataIn;
//
//
//out vec4 FragColor;	// Ouput data
//
//void main()
//{
//	if (generateShadowMap) {
//		// always orthographic projection
//		// r^2 = (x - x0)^2 + (y - y0)^2 + (z - z0)^2
//		float x = DataIn.tex.x;
//		float y = DataIn.tex.y;
//		float zz = 1.0 - x*x - y*y;
//
//		if (zz <= 0.0)
//			discard;
//
//		float z = sqrt(zz);
//		vec4 pos = DataIn.position;
//		pos.z += sphere_radius*z;
//
//		// compute the depth. I can do it easier
//		//vec4 pos4 = PROJ * vec4(pos, 1.0);
//		//gl_FragDepth = 0.5*(pos4.z / pos4.w) + 0.5;
//		vec2 clipZW = pos.z * PROJ[2].zw + PROJ[3].zw;
//		float depth = 0.5 * clipZW.x / clipZW.y + 0.5;
//
//		gl_FragDepth = depth;
//		FragColor = vec4(depth, 0, 0, 1.0);
//	}
//	else {
//	}
//}
//)";
//
//
//std::string shadow_points_spheres_geometry_geom = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//layout(points) in;
//layout(triangle_strip, max_vertices = 4) out;
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//uniform float	sphere_radius;
//
//out Data{
//	smooth	vec2	tex;
//	flat	vec4	position;
//} DataOut;
//
//
//void main()
//{
//	// Output vertex position
//	DataOut.position = MV * gl_in[0].gl_Position;
//
//	// Vertex 1
//	DataOut.tex = vec2(-1.0, -1.0);
//	gl_Position = DataOut.position;
//	gl_Position.xy += vec2(-sphere_radius, -sphere_radius);
//	gl_Position = PROJ  * gl_Position;
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(gl_in[0].gl_Position, clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(gl_in[0].gl_Position, clippingPlane1);
//	}
//	EmitVertex();
//
//	// Vertex 2
//	DataOut.tex = vec2(-1.0, 1.0);
//	gl_Position = DataOut.position;
//	gl_Position.xy += vec2(-sphere_radius, sphere_radius);
//	gl_Position = PROJ  * gl_Position;
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(gl_in[0].gl_Position, clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(gl_in[0].gl_Position, clippingPlane1);
//	}
//	EmitVertex();
//
//	// Vertex 3
//	DataOut.tex = vec2(1.0, -1.0);
//	gl_Position = DataOut.position;
//	gl_Position.xy += vec2(sphere_radius, -sphere_radius);
//	gl_Position = PROJ  * gl_Position;
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(gl_in[0].gl_Position, clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(gl_in[0].gl_Position, clippingPlane1);
//	}
//	EmitVertex();
//
//	// Vertex 4
//	DataOut.tex = vec2(1.0, 1.0);
//	gl_Position = DataOut.position;
//	gl_Position.xy += vec2(sphere_radius, sphere_radius);
//	gl_Position = PROJ  * gl_Position;
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(gl_in[0].gl_Position, clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(gl_in[0].gl_Position, clippingPlane1);
//	}
//	EmitVertex();
//
//	EndPrimitive();
//})";
//
//
//std::string shadow_points_spheres_geometry_vert = R"(
//#version 150	 
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//in vec3  vtx_position;
//
//void main()
//{
//	gl_Position = MANIP * vec4(vtx_position, 1.0);
//})";
//
//
//
//
//std::string shadow_surface_frag = R"(
//#version 430
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//layout(std140) uniform Lighting {
//	vec3	wLightPos;
//	vec3	eLightPos;
//	vec3	wCamPos;
//	vec3	ambient;		// in [0, 1], r==g==b;
//	vec3	specular;		// in [0, 1], r==g==b;
//	float	shininess;
//};
//
//uniform bool generateShadowMap;
//
//uniform int  samplePattern;
//
//uniform sampler2DShadow  shadowMapPCF;
//uniform sampler2D		 shadowMapDepth;
//
//uniform vec2  lightRadiusUV;
//uniform float lightZNear;
//uniform float lightZFar;
//uniform float darkness;
//uniform mat4  lightViewMatrix;
//
//uniform bool  isGroundPlane;
//uniform bool  useTexture;			// otherwise use the default color
//uniform float textureRepeat;
//uniform float textureFractionalRepeat;
//uniform sampler2D textureID;
//
//uniform bool two_sides_lighting;
//uniform bool smooth_shading;
//
//uniform bool distinct_back_color;
//uniform vec3 back_color;
//
//uniform int  hightlight_id_min;
//uniform int  hightlight_id_max;
//
//uniform bool selected;
//
//layout(std430, binding = 1) buffer selection_t {
//	uint data[];
//} selection;
//
//in Data{
//	vec3 position;
//	vec3 normal;
//	vec3 color;
//	vec2 texcoord;
//	vec4 shadowCoord;
//} DataIn;
//
//// SSAO
////------------------------------
//uniform sampler2D	ssaoTexture;
//uniform bool        ssaoEnabled;
//
//
//out vec4 FragColor;	// Ouput data
//
//#define POISSON_25_25   0
//#define POISSON_32_64   1
//#define POISSON_100_100 2
//#define POISSON_64_128  3
//#define REGULAR_49_225  4
//
//const vec2 Poisson25[25] = vec2[](
//	vec2(-0.978698, -0.0884121),
//	vec2(-0.841121, 0.521165),
//	vec2(-0.71746, -0.50322),
//	vec2(-0.702933, 0.903134),
//	vec2(-0.663198, 0.15482),
//	vec2(-0.495102, -0.232887),
//	vec2(-0.364238, -0.961791),
//	vec2(-0.345866, -0.564379),
//	vec2(-0.325663, 0.64037),
//	vec2(-0.182714, 0.321329),
//	vec2(-0.142613, -0.0227363),
//	vec2(-0.0564287, -0.36729),
//	vec2(-0.0185858, 0.918882),
//	vec2(0.0381787, -0.728996),
//	vec2(0.16599, 0.093112),
//	vec2(0.253639, 0.719535),
//	vec2(0.369549, -0.655019),
//	vec2(0.423627, 0.429975),
//	vec2(0.530747, -0.364971),
//	vec2(0.566027, -0.940489),
//	vec2(0.639332, 0.0284127),
//	vec2(0.652089, 0.669668),
//	vec2(0.773797, 0.345012),
//	vec2(0.968871, 0.840449),
//	vec2(0.991882, -0.657338)
//	);
//
//const vec2 Poisson32[32] = vec2[](
//	vec2(-0.975402, -0.0711386),
//	vec2(-0.920347, -0.41142),
//	vec2(-0.883908, 0.217872),
//	vec2(-0.884518, 0.568041),
//	vec2(-0.811945, 0.90521),
//	vec2(-0.792474, -0.779962),
//	vec2(-0.614856, 0.386578),
//	vec2(-0.580859, -0.208777),
//	vec2(-0.53795, 0.716666),
//	vec2(-0.515427, 0.0899991),
//	vec2(-0.454634, -0.707938),
//	vec2(-0.420942, 0.991272),
//	vec2(-0.261147, 0.588488),
//	vec2(-0.211219, 0.114841),
//	vec2(-0.146336, -0.259194),
//	vec2(-0.139439, -0.888668),
//	vec2(0.0116886, 0.326395),
//	vec2(0.0380566, 0.625477),
//	vec2(0.0625935, -0.50853),
//	vec2(0.125584, 0.0469069),
//	vec2(0.169469, -0.997253),
//	vec2(0.320597, 0.291055),
//	vec2(0.359172, -0.633717),
//	vec2(0.435713, -0.250832),
//	vec2(0.507797, -0.916562),
//	vec2(0.545763, 0.730216),
//	vec2(0.56859, 0.11655),
//	vec2(0.743156, -0.505173),
//	vec2(0.736442, -0.189734),
//	vec2(0.843562, 0.357036),
//	vec2(0.865413, 0.763726),
//	vec2(0.872005, -0.927)
//	);
//
//const vec2 Poisson64[64] = vec2[](
//	vec2(-0.934812, 0.366741),
//	vec2(-0.918943, -0.0941496),
//	vec2(-0.873226, 0.62389),
//	vec2(-0.8352, 0.937803),
//	vec2(-0.822138, -0.281655),
//	vec2(-0.812983, 0.10416),
//	vec2(-0.786126, -0.767632),
//	vec2(-0.739494, -0.535813),
//	vec2(-0.681692, 0.284707),
//	vec2(-0.61742, -0.234535),
//	vec2(-0.601184, 0.562426),
//	vec2(-0.607105, 0.847591),
//	vec2(-0.581835, -0.00485244),
//	vec2(-0.554247, -0.771111),
//	vec2(-0.483383, -0.976928),
//	vec2(-0.476669, -0.395672),
//	vec2(-0.439802, 0.362407),
//	vec2(-0.409772, -0.175695),
//	vec2(-0.367534, 0.102451),
//	vec2(-0.35313, 0.58153),
//	vec2(-0.341594, -0.737541),
//	vec2(-0.275979, 0.981567),
//	vec2(-0.230811, 0.305094),
//	vec2(-0.221656, 0.751152),
//	vec2(-0.214393, -0.0592364),
//	vec2(-0.204932, -0.483566),
//	vec2(-0.183569, -0.266274),
//	vec2(-0.123936, -0.754448),
//	vec2(-0.0859096, 0.118625),
//	vec2(-0.0610675, 0.460555),
//	vec2(-0.0234687, -0.962523),
//	vec2(-0.00485244, -0.373394),
//	vec2(0.0213324, 0.760247),
//	vec2(0.0359813, -0.0834071),
//	vec2(0.0877407, -0.730766),
//	vec2(0.14597, 0.281045),
//	vec2(0.18186, -0.529649),
//	vec2(0.188208, -0.289529),
//	vec2(0.212928, 0.063509),
//	vec2(0.23661, 0.566027),
//	vec2(0.266579, 0.867061),
//	vec2(0.320597, -0.883358),
//	vec2(0.353557, 0.322733),
//	vec2(0.404157, -0.651479),
//	vec2(0.410443, -0.413068),
//	vec2(0.413556, 0.123325),
//	vec2(0.46556, -0.176183),
//	vec2(0.49266, 0.55388),
//	vec2(0.506333, 0.876888),
//	vec2(0.535875, -0.885556),
//	vec2(0.615894, 0.0703452),
//	vec2(0.637135, -0.637623),
//	vec2(0.677236, -0.174291),
//	vec2(0.67626, 0.7116),
//	vec2(0.686331, -0.389935),
//	vec2(0.691031, 0.330729),
//	vec2(0.715629, 0.999939),
//	vec2(0.8493, -0.0485549),
//	vec2(0.863582, -0.85229),
//	vec2(0.890622, 0.850581),
//	vec2(0.898068, 0.633778),
//	vec2(0.92053, -0.355693),
//	vec2(0.933348, -0.62981),
//	vec2(0.95294, 0.156896)
//	);
//
//const vec2 Poisson100[100] = vec2[](
//	vec2(-0.9891574, -0.1059512),
//	vec2(-0.9822294, 0.05140843),
//	vec2(-0.961332, 0.2562195),
//	vec2(-0.9149657, -0.2404464),
//	vec2(-0.8896608, -0.4183828),
//	vec2(-0.8398135, 0.3748641),
//	vec2(-0.8149028, 0.1989844),
//	vec2(-0.8046502, 0.5212684),
//	vec2(-0.7970151, -0.5834194),
//	vec2(-0.7484995, -0.3153634),
//	vec2(-0.738582, -0.09323367),
//	vec2(-0.695694, 0.08865929),
//	vec2(-0.6868832, 0.6336682),
//	vec2(-0.6751406, 0.2777427),
//	vec2(-0.666558, -0.6801786),
//	vec2(-0.631489, -0.4702293),
//	vec2(-0.5870083, 0.518836),
//	vec2(-0.5744062, -0.06333278),
//	vec2(-0.5667221, 0.1699501),
//	vec2(-0.5537653, 0.7677022),
//	vec2(-0.5337034, 0.3299558),
//	vec2(-0.5201509, -0.2033358),
//	vec2(-0.4873925, -0.8545401),
//	vec2(-0.4712743, -0.3607009),
//	vec2(-0.4524891, -0.5142469),
//	vec2(-0.4421883, -0.6830674),
//	vec2(-0.4293752, 0.6299667),
//	vec2(-0.4240644, 0.8706763),
//	vec2(-0.4139857, 0.1598689),
//	vec2(-0.3838707, 0.4078749),
//	vec2(-0.3688077, -0.0358762),
//	vec2(-0.3432877, -0.2311365),
//	vec2(-0.3256257, -0.9325441),
//	vec2(-0.2751555, 0.302412),
//	vec2(-0.2679778, -0.654425),
//	vec2(-0.2554769, -0.4441924),
//	vec2(-0.243476, -0.8034022),
//	vec2(-0.2367678, -0.108045),
//	vec2(-0.2196257, 0.8243803),
//	vec2(-0.2119443, 0.06230118),
//	vec2(-0.1708038, -0.9437978),
//	vec2(-0.1694005, 0.5692244),
//	vec2(-0.136494, 0.3937041),
//	vec2(-0.1318274, -0.2166154),
//	vec2(-0.09781472, -0.5743775),
//	vec2(-0.09480921, 0.2369129),
//	vec2(-0.07638182, -0.0571501),
//	vec2(-0.06661344, -0.7966294),
//	vec2(-0.06305461, -0.3521975),
//	vec2(-0.04525706, 0.6982157),
//	vec2(-0.04149697, 0.9666064),
//	vec2(-0.003192461, -0.9693027),
//	vec2(0.0104818, 0.5000805),
//	vec2(0.03228819, -0.1681713),
//	vec2(0.03715288, -0.673852),
//	vec2(0.08470399, -0.3922319),
//	vec2(0.09848712, -0.8374477),
//	vec2(0.09940207, 0.1117471),
//	vec2(0.1395643, 0.313647),
//	vec2(0.1565993, 0.8555924),
//	vec2(0.1772605, -0.5248074),
//	vec2(0.1899546, 0.5249656),
//	vec2(0.1952665, -0.9595091),
//	vec2(0.213078, -0.07045701),
//	vec2(0.2277649, -0.3361143),
//	vec2(0.247221, 0.7353553),
//	vec2(0.2493455, -0.6874771),
//	vec2(0.269915, 0.07673722),
//	vec2(0.3039587, 0.9087375),
//	vec2(0.3189922, 0.3008468),
//	vec2(0.3215453, -0.1954931),
//	vec2(0.3593478, 0.4527411),
//	vec2(0.3745022, -0.597945),
//	vec2(0.3879738, -0.7821383),
//	vec2(0.4522015, 0.6819367),
//	vec2(0.4591872, -0.4484442),
//	vec2(0.4626173, -0.03955235),
//	vec2(0.4751598, 0.2083394),
//	vec2(0.4894366, 0.8694122),
//	vec2(0.4896614, -0.2676601),
//	vec2(0.5070116, -0.6733028),
//	vec2(0.5525513, 0.436241),
//	vec2(0.5542312, -0.8262905),
//	vec2(0.6012187, 0.7003717),
//	vec2(0.6075609, -0.1610506),
//	vec2(0.6291932, 0.2213627),
//	vec2(0.6300695, -0.5324634),
//	vec2(0.6613995, -0.7056449),
//	vec2(0.6699739, -0.3828001),
//	vec2(0.6705787, 0.01011722),
//	vec2(0.6814164, 0.5618623),
//	vec2(0.7808329, 0.261445),
//	vec2(0.7830279, -0.1817809),
//	vec2(0.8006546, -0.5266678),
//	vec2(0.8030878, 0.4266291),
//	vec2(0.8259325, 0.08734058),
//	vec2(0.8621388, -0.3646038),
//	vec2(0.9531851, 0.3011991),
//	vec2(0.9578334, -0.1584408),
//	vec2(0.9898114, 0.1029227)
//	);
//
//const vec2 Poisson128[128] = vec2[](
//	vec2(-0.9406119, 0.2160107),
//	vec2(-0.920003, 0.03135762),
//	vec2(-0.917876, -0.2841548),
//	vec2(-0.9166079, -0.1372365),
//	vec2(-0.8978907, -0.4213504),
//	vec2(-0.8467999, 0.5201505),
//	vec2(-0.8261013, 0.3743192),
//	vec2(-0.7835162, 0.01432008),
//	vec2(-0.779963, 0.2161933),
//	vec2(-0.7719588, 0.6335353),
//	vec2(-0.7658782, -0.3316436),
//	vec2(-0.7341912, -0.5430729),
//	vec2(-0.6825727, -0.1883408),
//	vec2(-0.6777467, 0.3313724),
//	vec2(-0.662191, 0.5155144),
//	vec2(-0.6569989, -0.7000636),
//	vec2(-0.6021447, 0.7923283),
//	vec2(-0.5980815, -0.5529259),
//	vec2(-0.5867089, 0.09857152),
//	vec2(-0.5774597, -0.8154474),
//	vec2(-0.5767041, -0.2656419),
//	vec2(-0.575091, -0.4220052),
//	vec2(-0.5486979, -0.09635002),
//	vec2(-0.5235587, 0.6594529),
//	vec2(-0.5170338, -0.6636339),
//	vec2(-0.5114055, 0.4373561),
//	vec2(-0.4844725, 0.2985838),
//	vec2(-0.4803245, 0.8482798),
//	vec2(-0.4651957, -0.5392771),
//	vec2(-0.4529685, 0.09942394),
//	vec2(-0.4523471, -0.3125569),
//	vec2(-0.4268422, 0.5644538),
//	vec2(-0.4187512, -0.8636028),
//	vec2(-0.4160798, -0.0844868),
//	vec2(-0.3751733, 0.2196607),
//	vec2(-0.3656596, -0.7324334),
//	vec2(-0.3286595, -0.2012637),
//	vec2(-0.3147397, -0.0006635741),
//	vec2(-0.3135846, 0.3636878),
//	vec2(-0.3042951, -0.4983553),
//	vec2(-0.2974239, 0.7496996),
//	vec2(-0.2903037, 0.8890813),
//	vec2(-0.2878664, -0.8622097),
//	vec2(-0.2588971, -0.653879),
//	vec2(-0.2555692, 0.5041648),
//	vec2(-0.2553292, -0.3389159),
//	vec2(-0.2401368, 0.2306108),
//	vec2(-0.2124457, -0.09935001),
//	vec2(-0.1877905, 0.1098409),
//	vec2(-0.1559879, 0.3356432),
//	vec2(-0.1499449, 0.7487829),
//	vec2(-0.146661, -0.9256138),
//	vec2(-0.1342774, 0.6185387),
//	vec2(-0.1224529, -0.3887629),
//	vec2(-0.116467, 0.8827716),
//	vec2(-0.1157598, -0.539999),
//	vec2(-0.09983152, -0.2407187),
//	vec2(-0.09953719, -0.78346),
//	vec2(-0.08604223, 0.4591112),
//	vec2(-0.02128129, 0.1551989),
//	vec2(-0.01478849, 0.6969455),
//	vec2(-0.01231739, -0.6752576),
//	vec2(-0.005001599, -0.004027164),
//	vec2(0.00248426, 0.567932),
//	vec2(0.00335562, 0.3472346),
//	vec2(0.009554717, -0.4025437),
//	vec2(0.02231783, -0.1349781),
//	vec2(0.04694207, -0.8347212),
//	vec2(0.05412609, 0.9042216),
//	vec2(0.05812819, -0.9826952),
//	vec2(0.1131321, -0.619306),
//	vec2(0.1170737, 0.6799788),
//	vec2(0.1275105, 0.05326218),
//	vec2(0.1393405, -0.2149568),
//	vec2(0.1457873, 0.1991508),
//	vec2(0.1474208, 0.5443151),
//	vec2(0.1497117, -0.3899909),
//	vec2(0.1923773, 0.3683496),
//	vec2(0.2110928, -0.7888536),
//	vec2(0.2148235, 0.9586087),
//	vec2(0.2152219, -0.1084362),
//	vec2(0.2189204, -0.9644538),
//	vec2(0.2220028, -0.5058427),
//	vec2(0.2251696, 0.779461),
//	vec2(0.2585723, 0.01621339),
//	vec2(0.2612841, -0.2832426),
//	vec2(0.2665483, -0.6422054),
//	vec2(0.2939872, 0.1673226),
//	vec2(0.3235748, 0.5643662),
//	vec2(0.3269232, 0.6984669),
//	vec2(0.3425438, -0.1783788),
//	vec2(0.3672505, 0.4398117),
//	vec2(0.3755714, -0.8814359),
//	vec2(0.379463, 0.2842356),
//	vec2(0.3822978, -0.381217),
//	vec2(0.4057849, -0.5227674),
//	vec2(0.4168737, -0.6936938),
//	vec2(0.4202749, 0.8369391),
//	vec2(0.4252189, 0.03818182),
//	vec2(0.4445904, -0.09360636),
//	vec2(0.4684285, 0.5885228),
//	vec2(0.4952184, -0.2319764),
//	vec2(0.5072351, 0.3683765),
//	vec2(0.5136194, -0.3944138),
//	vec2(0.519893, 0.7157083),
//	vec2(0.5277841, 0.1486474),
//	vec2(0.5474944, -0.7618791),
//	vec2(0.5692734, 0.4852227),
//	vec2(0.582229, -0.5125455),
//	vec2(0.583022, 0.008507785),
//	vec2(0.6500257, 0.3473313),
//	vec2(0.6621304, -0.6280518),
//	vec2(0.6674218, -0.2260806),
//	vec2(0.6741871, 0.6734863),
//	vec2(0.6753459, 0.1119422),
//	vec2(0.7083091, -0.4393666),
//	vec2(0.7106963, -0.102099),
//	vec2(0.7606754, 0.5743545),
//	vec2(0.7846709, 0.2282225),
//	vec2(0.7871446, 0.3891495),
//	vec2(0.8071781, -0.5257092),
//	vec2(0.8230689, 0.002674922),
//	vec2(0.8531976, -0.3256475),
//	vec2(0.8758298, -0.1824844),
//	vec2(0.8797691, 0.1284946),
//	vec2(0.926309, 0.3576975),
//	vec2(0.9608918, -0.03495717),
//	vec2(0.972032, 0.2271516)
//	);
//
//
//
//bool isBlack(vec3 c)
//{
//	return (dot(c, c) == 0.0);
//}
//
//float borderDepthTexture(sampler2D tex, vec2 uv)
//{
//	return ((uv.x <= 1.0) && (uv.y <= 1.0) &&
//		(uv.x >= 0.0) && (uv.y >= 0.0)) ? texture(tex, uv).x : 1.0;
//}
//
//float borderPCFTexture(sampler2DShadow tex, vec3 uvz)
//{
//	return ((uvz.x <= 1.0) && (uvz.y <= 1.0) &&
//		(uvz.x >= 0.0) && (uvz.y >= 0.0)) ? texture(tex, uvz) :
//		((uvz.z <= 1.0) ? 1.0 : 0.0);
//}
//
//
//// Using similar triangles from the surface point to the area light
//vec2 searchRegionRadiusUV(float zWorld)
//{
//	return lightRadiusUV * (zWorld - lightZNear) / zWorld;
//}
//
//// Using similar triangles between the area light, the blocking plane and the surface point
//vec2 penumbraRadiusUV(float zReceiver, float zBlocker)
//{
//	return lightRadiusUV * (zReceiver - zBlocker) / zBlocker;
//}
//
//// Project UV size to the near plane of the light
//vec2 projectToLightUV(vec2 sizeUV, float zWorld)
//{
//	return sizeUV * lightZNear / zWorld;
//}
//
//// Derivatives of light-space depth with respect to texture2D coordinates
//vec2 depthGradient(vec2 uv, float z)
//{
//	vec2 dz_duv = vec2(0.0, 0.0);
//
//	vec3 duvdist_dx = dFdx(vec3(uv, z));
//	vec3 duvdist_dy = dFdy(vec3(uv, z));
//
//	dz_duv.x = duvdist_dy.y * duvdist_dx.z;
//	dz_duv.x -= duvdist_dx.y * duvdist_dy.z;
//
//	dz_duv.y = duvdist_dx.x * duvdist_dy.z;
//	dz_duv.y -= duvdist_dy.x * duvdist_dx.z;
//
//	float det = (duvdist_dx.x * duvdist_dy.y) - (duvdist_dx.y * duvdist_dy.x);
//	dz_duv /= det;
//
//	return dz_duv;
//}
//
//float biasedZ(float z0, vec2 dz_duv, vec2 offset)
//{
//	return z0 + dot(dz_duv, offset) - 0.005;
//}
//
//float zClipToEye(float z)
//{
//	return lightZFar * lightZNear / (lightZFar - z * (lightZFar - lightZNear));
//}
//
//// Returns average blocker depth in the search region, as well as the number of found blockers.
//// Blockers are defined as shadow-map samples between the surface point and the light.
//void findBlocker(
//	out float accumBlockerDepth,
//	out float numBlockers,
//	out float maxBlockers,
//	vec2 uv,
//	float z0,
//	vec2 dz_duv,
//	vec2 searchRadiusUV)
//{
//	accumBlockerDepth = 0.0;
//	numBlockers = 0.0;
//	maxBlockers = 300.0;
//
//	switch (samplePattern)
//	{
//	case POISSON_25_25:
//	{
//		maxBlockers = 25.0;
//		for (int i = 0; i < 25; ++i) {
//			vec2 offset = Poisson25[i] * searchRadiusUV;
//			float shadowMapDepth = borderDepthTexture(shadowMapDepth, uv + offset);
//			float z = biasedZ(z0, dz_duv, offset);
//			if (shadowMapDepth < z) {
//				accumBlockerDepth += shadowMapDepth;
//				numBlockers++;
//			}
//		}
//	}
//	break;
//
//	case POISSON_32_64:
//	{
//		maxBlockers = 32.0;
//		for (int i = 0; i < 32; ++i) {
//			vec2 offset = Poisson32[i] * searchRadiusUV;
//			float shadowMapDepth = borderDepthTexture(shadowMapDepth, uv + offset);
//			float z = biasedZ(z0, dz_duv, offset);
//			if (shadowMapDepth < z) {
//				accumBlockerDepth += shadowMapDepth;
//				numBlockers++;
//			}
//		}
//	}
//	break;
//
//	case POISSON_100_100:
//	{
//		maxBlockers = 100.0;
//		for (int i = 0; i < 100; ++i) {
//			vec2 offset = Poisson100[i] * searchRadiusUV;
//			float shadowMapDepth = borderDepthTexture(shadowMapDepth, uv + offset);
//			float z = biasedZ(z0, dz_duv, offset);
//			if (shadowMapDepth < z) {
//				accumBlockerDepth += shadowMapDepth;
//				numBlockers++;
//			}
//		}
//	}
//	break;
//
//	case POISSON_64_128:
//	{
//		maxBlockers = 64.0;
//		for (int i = 0; i < 64; ++i) {
//			vec2 offset = Poisson64[i] * searchRadiusUV;
//			float shadowMapDepth = borderDepthTexture(shadowMapDepth, uv + offset);
//			float z = biasedZ(z0, dz_duv, offset);
//			if (shadowMapDepth < z) {
//				accumBlockerDepth += shadowMapDepth;
//				numBlockers++;
//			}
//		}
//	}
//	break;
//
//	case REGULAR_49_225:
//	{
//		maxBlockers = 49.0;
//		vec2 stepUV = searchRadiusUV / 3.0;
//		for (int x = -3; x <= 3; ++x) {
//			for (int y = -3; y <= 3; ++y) {
//				vec2 offset = vec2(x, y) * stepUV;
//				float shadowMapDepth = borderDepthTexture(shadowMapDepth, uv + offset);
//				float z = biasedZ(z0, dz_duv, offset);
//				if (shadowMapDepth < z) {
//					accumBlockerDepth += shadowMapDepth;
//					numBlockers++;
//				}
//			}
//		}
//	}
//	break;
//	}
//}
//
//// Performs PCF filtering on the shadow map using multiple taps in the filter region.
//float pcfFilter(vec2 uv, float z0, vec2 dz_duv, vec2 filterRadiusUV)
//{
//	float sum = 0.0;
//
//	switch (samplePattern)
//	{
//	case POISSON_25_25:
//	{
//		for (int i = 0; i < 25; ++i) {
//			vec2 offset = Poisson25[i] * filterRadiusUV;
//			float z = biasedZ(z0, dz_duv, offset);
//			sum += borderPCFTexture(shadowMapPCF, vec3(uv + offset, z));
//		}
//		return sum / 25.0;
//	}
//
//	case POISSON_32_64:
//	{
//		for (int i = 0; i < 64; ++i) {
//			vec2 offset = Poisson64[i] * filterRadiusUV;
//			float z = biasedZ(z0, dz_duv, offset);
//			sum += borderPCFTexture(shadowMapPCF, vec3(uv + offset, z));
//		}
//		return sum / 64.0;
//	}
//
//	case POISSON_100_100:
//	{
//		for (int i = 0; i < 100; ++i) {
//			vec2 offset = Poisson100[i] * filterRadiusUV;
//			float z = biasedZ(z0, dz_duv, offset);
//			sum += borderPCFTexture(shadowMapPCF, vec3(uv + offset, z));
//		}
//		return sum / 100.0;
//	}
//
//	case POISSON_64_128:
//	{
//		for (int i = 0; i < 128; ++i) {
//			vec2 offset = Poisson128[i] * filterRadiusUV;
//			float z = biasedZ(z0, dz_duv, offset);
//			sum += borderPCFTexture(shadowMapPCF, vec3(uv + offset, z));
//		}
//		return sum / 128.0;
//	}
//
//	case REGULAR_49_225:
//	{
//		vec2 stepUV = filterRadiusUV / 7.0;
//		for (int x = -7; x <= 7; ++x) {
//			for (int y = -7; y <= 7; ++y) {
//				vec2 offset = vec2(x, y) * stepUV;
//				float z = biasedZ(z0, dz_duv, offset);
//				sum += borderPCFTexture(shadowMapPCF, vec3(uv + offset, z));
//			}
//		}
//		float numSamples = 7.0 * 2.0 + 1.0;
//		return sum / (numSamples * numSamples);
//	}
//
//	default:
//		return 1.0;
//	}
//}
//
//float pcssShadow(vec2 uv, float z, vec2 dz_duv, float zEye)
//{
//	// ------------------------
//	// STEP 1: blocker search
//	// ------------------------
//	float accumBlockerDepth, numBlockers, maxBlockers;
//	vec2 searchRadiusUV = searchRegionRadiusUV(zEye);
//	findBlocker(accumBlockerDepth, numBlockers, maxBlockers, uv, z, dz_duv, searchRadiusUV);
//
//	// Early out if not in the penumbra
//	if (numBlockers == 0.0)
//		return 1.0;
//
//	// This is for dubug: visualize the percentage of occlusion
//	//return  numBlockers / maxBlockers;
//
//	// ------------------------
//	// STEP 2: penumbra size
//	// ------------------------
//	float avgBlockerDepth = accumBlockerDepth / numBlockers;
//	float avgBlockerDepthWorld = zClipToEye(avgBlockerDepth);
//	vec2 penumbraRadius = penumbraRadiusUV(zEye, avgBlockerDepthWorld);
//	vec2 filterRadius = projectToLightUV(penumbraRadius, zEye);
//
//	// ------------------------
//	// STEP 3: filtering
//	// ------------------------
//	return pcfFilter(uv, z, dz_duv, filterRadius);
//}
//
//
//vec3 shade(vec3 worldPos)
//{
//	if (isGroundPlane) {
//		// I want to make ground plane different from the model (i.e., no diffuse and specular)
//		if (useTexture) {
//			float repeat = textureRepeat + textureFractionalRepeat / 100.0f;
//			return texture(textureID, DataIn.texcoord * repeat).rgb;
//		}
//		else {
//			return DataIn.color;
//			//return vec3(0.6, 0.3, 0.6);
//		}
//	}
//	else {
//		vec3 normal;
//		if (smooth_shading)
//			normal = normalize(DataIn.normal);
//		else {
//			normal = normalize(cross(dFdx(worldPos), dFdy(worldPos)));
//			if (dot(normal, DataIn.normal) < 0)
//				normal = -normal;
//		}
//
//		vec3 view_dir = normalize(wCamPos - worldPos);
//		vec3 light_dir = normalize(wLightPos);	// directional light
//		//vec3 light_dir = normalize(wLightPos - worldPos); // point light
//
//		float df = 0.0;	// diffuse factor
//		if (two_sides_lighting)
//			df = abs(dot(light_dir, normal));
//		else
//			df = max(dot(light_dir, normal), 0);
//
//		float sf = 0.0;	// specular factor
//		if (df > 0.0) {	// if the vertex is lit compute the specular color
//			vec3 half_vector = normalize(light_dir + view_dir);	// compute the half vector
//			if (two_sides_lighting)
//				sf = abs(dot(half_vector, normal));
//			else
//				sf = max(dot(half_vector, normal), 0.0);
//			sf = pow(sf, shininess);
//		}
//
//		vec3 color;
//		if (!gl_FrontFacing && distinct_back_color)
//			color = back_color;
//		else {
//			if (useTexture) {
//				float repeat = textureRepeat + textureFractionalRepeat / 100.0f;
//				color = texture(textureID, DataIn.texcoord * repeat).rgb;
//			}
//			else {
//				color = DataIn.color;
//			}
//		}
//
//		//uint addr = gl_PrimitiveID / 32;
//		//uint offs = gl_PrimitiveID % 32;
//		uint addr = gl_PrimitiveID >> 5;
//		uint offs = gl_PrimitiveID & 31;
//		if ((selection.data[addr] & (1 << offs)) != 0)
//			color = mix(color, vec3(1.0, 0.0, 0.0), 0.8);
//		else if (gl_PrimitiveID >= hightlight_id_min && gl_PrimitiveID <= hightlight_id_max)
//			color = vec3(1.0, 0.0, 0.0);
//
//		if (selected) {
//			color = vec3(1.0, 0.0, 0.0);
//		}
//
//		color = color * df + specular * sf;
//
//		return color;
//	}
//}
//
//
//void main(void) {
//	if (generateShadowMap) {
//		// write the depth into the RED component
//		FragColor = vec4(gl_FragCoord.z, 0, 0, 1.0);
//	}
//	else {
//		vec3 color = shade(DataIn.position);
//
//		// transforming from clip space to NDC space
//		vec3 ProjCoords = DataIn.shadowCoord.xyz / DataIn.shadowCoord.w;
//		// It seems checking Z is sufficient (implies X and Y)?
//		if ((isBlack(color.rgb)) ||
//			//(ProjCoords.x <= 0 || ProjCoords.x >= 1) ||
//			//(ProjCoords.y <= 0 || ProjCoords.y >= 1) ||
//			(ProjCoords.z <= 0 || ProjCoords.z >= 1))
//		{
//			if (isGroundPlane)
//				FragColor = vec4(color, 1.0);
//			else {
//				if (ssaoEnabled) {
//					vec2 texCoord = gl_FragCoord.xy / textureSize(ssaoTexture, 0);
//					float coeff = texture(ssaoTexture, texCoord).r;
//					FragColor = vec4(vec3(color + ambient) * coeff, 1.0);
//				}
//				else
//					FragColor = vec4(color + ambient, 1.0);
//			}
//		}
//		else {
//			vec2 uv = ProjCoords.xy;
//			float z = ProjCoords.z;
//
//			// Compute gradient using ddx/ddy before any branching
//			vec2 dz_duv = depthGradient(uv, z);
//
//			// Eye-space z from the light's point of view
//			float zEye = -(lightViewMatrix * vec4(DataIn.position, 1.0)).z;
//			float shadow = pcssShadow(uv, z, dz_duv, zEye);
//
//			// I don't want the shadow regions to be completely dark.
//			shadow = (1.0 - darkness) + shadow * darkness;
//
//			if (isGroundPlane)
//				FragColor = vec4(color * shadow, 1.0);
//			else {
//				if (ssaoEnabled) {
//					vec2 texCoord = gl_FragCoord.xy / textureSize(ssaoTexture, 0);
//					float coeff = texture(ssaoTexture, texCoord).r;
//					FragColor = vec4(vec3(color * shadow + ambient) * coeff, 1.0);
//				}
//				else
//					FragColor = vec4(color * shadow + ambient, 1.0);
//			}
//		}
//	}
//})";
//
//
//std::string shadow_surface_vert = R"(
//#version 430
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//in  vec3 vtx_position;	// vertex position
//in	vec3 vtx_normal;	// vertex normal
//in  vec3 vtx_color;		// vertex color
//in  vec2 vtx_texcoord;
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//uniform vec3 default_color;
//uniform bool per_vertex_color;
//
//// the data to be sent to the fragment shader
//out Data{
//	vec3 position;
//	vec3 normal;
//	vec3 color;
//	vec2 texcoord;
//	vec4 shadowCoord;
//} DataOut;
//
//
//void main() {
//	vec4 new_position = MANIP * vec4(vtx_position, 1.0);
//	DataOut.position = new_position.xyz;
//	DataOut.normal = NORMAL * vtx_normal;
//	DataOut.shadowCoord = SHADOW * new_position;
//
//	DataOut.texcoord = vtx_texcoord;
//	if (per_vertex_color)	
//		DataOut.color = vtx_color;
//	else
//		DataOut.color = default_color;
//	
//	gl_Position = MVP * new_position;
//
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(new_position, clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(new_position, clippingPlane1);
//	}
//})";
//
//
//
//
//std::string ssao_frag = R"(
//#version 150
//
//in vec2 TexCoords;
//
//uniform sampler2D gPosition;
//uniform sampler2D gNormal;
//uniform sampler2D texNoise;
//
//uniform vec3 samples[64];
//
//uniform mat4 PROJ;
//
//// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
//int kernelSize = 64;
////float radius = 0.5;
//uniform float radius;
//float bias = 0.005;	// 0.025;
//
//out float FragColor;
//
//void main()
//{
//	// tile noise texture over screen based on screen dimensions divided by noise size
//	vec2 noiseScale = textureSize(gPosition, 0) / 4.0;
//
//    // get input for SSAO algorithm
//    vec3 fragPos = texture(gPosition, TexCoords).xyz;
//    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
//    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);
//    // create TBN change-of-basis matrix: from tangent-space to view-space
//    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
//    vec3 bitangent = cross(normal, tangent);
//    mat3 TBN = mat3(tangent, bitangent, normal);
//    // iterate over the sample kernel and calculate occlusion factor
//    float occlusion = 0.0;
//    for(int i = 0; i < kernelSize; ++i)
//    {
//        // get sample position
//        vec3 sample = TBN * samples[i]; // from tangent to view-space
//        sample = fragPos + sample * radius; 
//        
//        // project sample position (to sample texture) (to get position on screen/texture)
//        vec4 offset = vec4(sample, 1.0);
//        offset = PROJ * offset; // from view to clip-space
//        offset.xyz /= offset.w; // perspective divide
//        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
//        
//        // get sample depth
//        float sampleDepth = texture(gPosition, offset.xy).z; // get depth value of kernel sample
//        
//        // range check & accumulate
//        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
//        occlusion += (sampleDepth >= sample.z + bias ? 1.0 : 0.0) * rangeCheck;           
//    }
//    occlusion = 1.0 - (occlusion / kernelSize);
//    
//    FragColor = occlusion;
//})";
//
//
//std::string ssao_vert = R"(
//#version 150
//
//
//in	vec3	vertexMC;
//in	vec2	tcoordMC;
//
//out vec2 TexCoords;
//
//void main()
//{
//    TexCoords = tcoordMC;
//    gl_Position = vec4(vertexMC, 1.0);
//})";
//
//
//
//
//std::string ssao_blur_frag = R"(
//#version 150
//
//
//in vec2 TexCoords;
//
//uniform sampler2D ssaoInput;
//
//out float FragColor;
//
//void main() 
//{
//    vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));
//    float result = 0.0;
//    for (int x = -2; x <= 2; ++x) 
//    {
//        for (int y = -2; y <= 2; ++y) 
//        {
//            vec2 offset = vec2(float(x), float(y)) * texelSize;
//            result += texture(ssaoInput, TexCoords + offset).r;
//        }
//    }
//    FragColor = result / (5.0 * 5.0);
//}  )";
//
//
//std::string ssao_blur_vert = R"(
//#version 150
//
//
//in	vec3	vertexMC;
//in	vec2	tcoordMC;
//
//out vec2 TexCoords;
//
//void main()
//{
//	TexCoords = tcoordMC;
//	gl_Position = vec4(vertexMC, 1.0);
//})";
//
//
//
//
//std::string ssao_geometry_frag = R"(
//#version 150
//
//uniform bool smooth_shading;
//
//in vec3 FragPos;
//in vec3 Normal;
//
//out vec3 gPosition;
//out vec3 gNormal;
//
//void main()
//{    
//    // store the fragment position vector in the first gbuffer texture
//    gPosition = FragPos;
//    // also store the per-fragment normals into the gbuffer
//	if (smooth_shading)
//		gNormal = normalize(Normal);
//	else {
//		gNormal = normalize(cross(dFdx(FragPos), dFdy(FragPos)));
//		if (dot(gNormal, Normal) < 0)
//			gNormal = -gNormal;
//	}
//
//	// to handle two sided lighting
//	if (dot(gNormal, vec3(0, 0, 1)) < 0)
//		gNormal = -gNormal;
//})";
//
//
//std::string ssao_geometry_vert = R"(
//#version 150
//
//in vec3 vtx_position;
//in vec3 vtx_normal;
//
//out vec3 FragPos;
//out vec3 Normal;
//
//uniform mat4 MV;
//uniform mat4 PROJ;
//uniform mat4 MANIP;
//uniform mat3 NORMAL;
//
//uniform bool clippingPlaneEnabled;
//uniform bool crossSectionEnabled;
//uniform vec4 clippingPlane0;
//uniform vec4 clippingPlane1;
//
//void main()
//{
//	vec4 new_position = MANIP * vec4(vtx_position, 1.0);
//
//    vec4 viewPos = MV * new_position;
//    FragPos = viewPos.xyz; 
//    
//    Normal = NORMAL * vtx_normal;
//    
//    gl_Position = PROJ * viewPos;
//
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(new_position, clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(new_position, clippingPlane1);
//	}
//})";
//
//
//
//std::string surface_texture_frag = R"(
//#version 430
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//layout(std140) uniform Lighting {
//	vec3	wLightPos;
//	vec3	eLightPos;
//	vec3	wCamPos;
//	vec3	ambient;		// in [0, 1], r==g==b;
//	vec3	specular;		// in [0, 1], r==g==b;
//	float	shininess;
//};
//
//uniform bool lightingEnabled = true;
//uniform bool two_sides_lighting;
//uniform bool smooth_shading;
//
//uniform int  hightlight_id_min;
//uniform int  hightlight_id_max;
//
//uniform bool selected;
//
//layout(std430, binding = 1) buffer selection_t {
//	uint data[];
//} selection;
//
//in Data{
//	vec2 texcoord;
//	vec3 normal;
//	vec3 position;
//} DataIn;
//
//uniform sampler2D	textureID;
//
//uniform float texture_repeat;
//uniform float fractional_repeat;
//
//// SSAO
////------------------------------
//uniform sampler2D	ssaoTexture;
//uniform bool        ssaoEnabled;
//
//out vec4 outputF;	// Ouput data
//
//
////#define ENABLE_ALPHA
//
//void main() {
//	float repeat = texture_repeat + fractional_repeat / 100.0f;
//
//#ifndef ENABLE_ALPHA
//	vec3 color = texture(textureID, DataIn.texcoord * repeat).rgb;
//	float alpha = 1.0f;
//#else
//	vec4 tmp = texture(textureID, DataIn.texcoord * repeat);
//	if (tmp.a <= 0)
//		discard;
//	vec3 color = tmp.rgb;
//	float alpha = tmp.a;
//#endif
//
//	//uint addr = gl_PrimitiveID / 32;
//	//uint offs = gl_PrimitiveID % 32;
//	uint addr = gl_PrimitiveID >> 5;
//	uint offs = gl_PrimitiveID & 31;
//	if ((selection.data[addr] & (1 << offs)) != 0)
//		color = mix(color, vec3(1.0, 0.0, 0.0), 0.8);
//	else if (gl_PrimitiveID >= hightlight_id_min && gl_PrimitiveID <= hightlight_id_max)
//		color = vec3(1.0, 0.0, 0.0);
//
//	if (selected) {
//		color = vec3(1.0, 0.0, 0.0);
//	}
//
//	if (lightingEnabled) {
//		vec3 normal;
//		if (smooth_shading)
//			normal = normalize(DataIn.normal);
//		else {
//			normal = normalize(cross(dFdx(DataIn.position), dFdy(DataIn.position)));
//			if (dot(normal, DataIn.normal) < 0)
//				normal = -normal;
//		}
//
//		vec3 view_dir = normalize(wCamPos - DataIn.position);
//		vec3 light_dir = normalize(wLightPos);
//
//		float df = 0.0;	// diffuse factor
//		if (two_sides_lighting)
//			df = abs(dot(light_dir, normal));
//		else
//			df = max(dot(light_dir, normal), 0);
//
//		float sf = 0.0;	// specular factor
//		if (df > 0.0) {	// if the vertex is lit compute the specular color
//			vec3 half_vector = normalize(light_dir + view_dir);		// compute the half vector
//
//			if (two_sides_lighting)
//				sf = abs(dot(half_vector, normal));
//			else
//				sf = max(dot(half_vector, normal), 0.0);
//
//			sf = pow(sf, shininess);
//		}
//
//		if (ssaoEnabled) {
//			vec2 texCoord = gl_FragCoord.xy / textureSize(ssaoTexture, 0);
//			float coeff = texture(ssaoTexture, texCoord).r;
//			outputF = vec4(vec3(color * df + specular * sf + ambient) * coeff, alpha);
//		}
//		else
//			outputF = vec4(color * df + specular * sf + ambient, alpha);
//	}
//	else {
//		outputF = vec4(color, alpha);
//	}
//}
//)";
//
//
//std::string surface_texture_vert = R"(
//#version 430
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//in vec3 vtx_position;
//in vec2 vtx_texcoord;
//in vec3 vtx_normal;
//
//layout(std140) uniform Matrices {
//	mat4 MV;
//	mat4 invMV;
//	mat4 PROJ;
//	mat4 MVP;
//	mat4 MANIP;
//	mat3 NORMAL;
//	mat4 SHADOW;
//	bool clippingPlaneEnabled;
//	bool crossSectionEnabled;
//	vec4 clippingPlane0;
//	vec4 clippingPlane1;
//};
//
//out Data{
//	vec2 texcoord;
//	vec3 normal;
//	vec3 position;
//} DataOut;
//
//void main() {
//
//	vec4 new_position = MANIP * vec4(vtx_position, 1.0);
//
//	DataOut.texcoord = vtx_texcoord;
//	DataOut.normal = NORMAL * vtx_normal;
//	DataOut.position = new_position.xyz;
//
//	gl_Position = MVP * new_position;
//
//	if (clippingPlaneEnabled) {
//		gl_ClipDistance[0] = dot(new_position, clippingPlane0);
//		if (crossSectionEnabled)
//			gl_ClipDistance[1] = dot(new_position, clippingPlane1);
//	}
//}
//)";
//
//
//
//
//std::string textured_quad_frag = R"(
//#version 150
//// please send comments or report bug to: liangliang.nan@gmail.com
//
//in		vec2		tcoordVC;
//out		vec4		fragOutput;	// Ouput data
//
//uniform sampler2D	textureID;
//
//
//void main() {
//	vec4 color = texture(textureID, tcoordVC);
//	if (color.a <= 0)
//		discard;
//
//	fragOutput = color;
//})";
//
//
//std::string textured_quad_vert = R"(
//#version 150
//
//in	vec3	vertexMC;
//in	vec2	tcoordMC;
//
//out	vec2	tcoordVC;
//
//void main()
//{
//	tcoordVC = tcoordMC;
//	gl_Position = vec4(vertexMC, 1.0);
//}
//)";
