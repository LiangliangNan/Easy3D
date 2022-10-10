/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************/

#version 150


in Data{
	vec2 texcoord;
	vec3 color;
	vec3 normal;
	vec3 position;
	vec4 shadowCoord;
	float clipped;
} DataIn;


uniform vec3	wLightPos;
uniform vec3	wCamPos;
layout(std140) uniform Material {
        vec3	ambient;
        vec3	specular;
        float	shininess;
};

// smooth shading
uniform bool  smooth_shading = true;

// backside color
uniform bool  distinct_back_color = true;
uniform vec3  backside_color = vec3(0.8f, 0.4f, 0.4f);

uniform int  samplePattern;

uniform sampler2DShadow  shadowMapPCF;
uniform sampler2D	 shadowMapDepth;

uniform vec2  lightRadiusUV;
uniform float lightZNear;
uniform float lightZFar;
uniform float darkness;
uniform mat4  lightViewMatrix;

uniform bool two_sides_lighting;

uniform bool is_background = false;

uniform bool use_texture = false;
uniform sampler2D textureID;
uniform float texture_repeat = 1.0f;
uniform float fractional_repeat = 0.0f;
//#define ENABLE_ALPHA

uniform bool selected = false;
uniform vec4 	highlight_color;

out vec4 FragColor;	// Ouput data


#define POISSON_25_25   0
#define POISSON_32_64   1
#define POISSON_64_128  2
#define POISSON_100_100 3
#define REGULAR_49_225  4

const vec2 Poisson25[25] = vec2[](
	vec2(-0.978698, -0.0884121),
	vec2(-0.841121, 0.521165),
	vec2(-0.71746, -0.50322),
	vec2(-0.702933, 0.903134),
	vec2(-0.663198, 0.15482),
	vec2(-0.495102, -0.232887),
	vec2(-0.364238, -0.961791),
	vec2(-0.345866, -0.564379),
	vec2(-0.325663, 0.64037),
	vec2(-0.182714, 0.321329),
	vec2(-0.142613, -0.0227363),
	vec2(-0.0564287, -0.36729),
	vec2(-0.0185858, 0.918882),
	vec2(0.0381787, -0.728996),
	vec2(0.16599, 0.093112),
	vec2(0.253639, 0.719535),
	vec2(0.369549, -0.655019),
	vec2(0.423627, 0.429975),
	vec2(0.530747, -0.364971),
	vec2(0.566027, -0.940489),
	vec2(0.639332, 0.0284127),
	vec2(0.652089, 0.669668),
	vec2(0.773797, 0.345012),
	vec2(0.968871, 0.840449),
	vec2(0.991882, -0.657338)
	);

const vec2 Poisson32[32] = vec2[](
	vec2(-0.975402, -0.0711386),
	vec2(-0.920347, -0.41142),
	vec2(-0.883908, 0.217872),
	vec2(-0.884518, 0.568041),
	vec2(-0.811945, 0.90521),
	vec2(-0.792474, -0.779962),
	vec2(-0.614856, 0.386578),
	vec2(-0.580859, -0.208777),
	vec2(-0.53795, 0.716666),
	vec2(-0.515427, 0.0899991),
	vec2(-0.454634, -0.707938),
	vec2(-0.420942, 0.991272),
	vec2(-0.261147, 0.588488),
	vec2(-0.211219, 0.114841),
	vec2(-0.146336, -0.259194),
	vec2(-0.139439, -0.888668),
	vec2(0.0116886, 0.326395),
	vec2(0.0380566, 0.625477),
	vec2(0.0625935, -0.50853),
	vec2(0.125584, 0.0469069),
	vec2(0.169469, -0.997253),
	vec2(0.320597, 0.291055),
	vec2(0.359172, -0.633717),
	vec2(0.435713, -0.250832),
	vec2(0.507797, -0.916562),
	vec2(0.545763, 0.730216),
	vec2(0.56859, 0.11655),
	vec2(0.743156, -0.505173),
	vec2(0.736442, -0.189734),
	vec2(0.843562, 0.357036),
	vec2(0.865413, 0.763726),
	vec2(0.872005, -0.927)
	);

const vec2 Poisson64[64] = vec2[](
	vec2(-0.934812, 0.366741),
	vec2(-0.918943, -0.0941496),
	vec2(-0.873226, 0.62389),
	vec2(-0.8352, 0.937803),
	vec2(-0.822138, -0.281655),
	vec2(-0.812983, 0.10416),
	vec2(-0.786126, -0.767632),
	vec2(-0.739494, -0.535813),
	vec2(-0.681692, 0.284707),
	vec2(-0.61742, -0.234535),
	vec2(-0.601184, 0.562426),
	vec2(-0.607105, 0.847591),
	vec2(-0.581835, -0.00485244),
	vec2(-0.554247, -0.771111),
	vec2(-0.483383, -0.976928),
	vec2(-0.476669, -0.395672),
	vec2(-0.439802, 0.362407),
	vec2(-0.409772, -0.175695),
	vec2(-0.367534, 0.102451),
	vec2(-0.35313, 0.58153),
	vec2(-0.341594, -0.737541),
	vec2(-0.275979, 0.981567),
	vec2(-0.230811, 0.305094),
	vec2(-0.221656, 0.751152),
	vec2(-0.214393, -0.0592364),
	vec2(-0.204932, -0.483566),
	vec2(-0.183569, -0.266274),
	vec2(-0.123936, -0.754448),
	vec2(-0.0859096, 0.118625),
	vec2(-0.0610675, 0.460555),
	vec2(-0.0234687, -0.962523),
	vec2(-0.00485244, -0.373394),
	vec2(0.0213324, 0.760247),
	vec2(0.0359813, -0.0834071),
	vec2(0.0877407, -0.730766),
	vec2(0.14597, 0.281045),
	vec2(0.18186, -0.529649),
	vec2(0.188208, -0.289529),
	vec2(0.212928, 0.063509),
	vec2(0.23661, 0.566027),
	vec2(0.266579, 0.867061),
	vec2(0.320597, -0.883358),
	vec2(0.353557, 0.322733),
	vec2(0.404157, -0.651479),
	vec2(0.410443, -0.413068),
	vec2(0.413556, 0.123325),
	vec2(0.46556, -0.176183),
	vec2(0.49266, 0.55388),
	vec2(0.506333, 0.876888),
	vec2(0.535875, -0.885556),
	vec2(0.615894, 0.0703452),
	vec2(0.637135, -0.637623),
	vec2(0.677236, -0.174291),
	vec2(0.67626, 0.7116),
	vec2(0.686331, -0.389935),
	vec2(0.691031, 0.330729),
	vec2(0.715629, 0.999939),
	vec2(0.8493, -0.0485549),
	vec2(0.863582, -0.85229),
	vec2(0.890622, 0.850581),
	vec2(0.898068, 0.633778),
	vec2(0.92053, -0.355693),
	vec2(0.933348, -0.62981),
	vec2(0.95294, 0.156896)
	);

const vec2 Poisson100[100] = vec2[](
	vec2(-0.9891574, -0.1059512),
	vec2(-0.9822294, 0.05140843),
	vec2(-0.961332, 0.2562195),
	vec2(-0.9149657, -0.2404464),
	vec2(-0.8896608, -0.4183828),
	vec2(-0.8398135, 0.3748641),
	vec2(-0.8149028, 0.1989844),
	vec2(-0.8046502, 0.5212684),
	vec2(-0.7970151, -0.5834194),
	vec2(-0.7484995, -0.3153634),
	vec2(-0.738582, -0.09323367),
	vec2(-0.695694, 0.08865929),
	vec2(-0.6868832, 0.6336682),
	vec2(-0.6751406, 0.2777427),
	vec2(-0.666558, -0.6801786),
	vec2(-0.631489, -0.4702293),
	vec2(-0.5870083, 0.518836),
	vec2(-0.5744062, -0.06333278),
	vec2(-0.5667221, 0.1699501),
	vec2(-0.5537653, 0.7677022),
	vec2(-0.5337034, 0.3299558),
	vec2(-0.5201509, -0.2033358),
	vec2(-0.4873925, -0.8545401),
	vec2(-0.4712743, -0.3607009),
	vec2(-0.4524891, -0.5142469),
	vec2(-0.4421883, -0.6830674),
	vec2(-0.4293752, 0.6299667),
	vec2(-0.4240644, 0.8706763),
	vec2(-0.4139857, 0.1598689),
	vec2(-0.3838707, 0.4078749),
	vec2(-0.3688077, -0.0358762),
	vec2(-0.3432877, -0.2311365),
	vec2(-0.3256257, -0.9325441),
	vec2(-0.2751555, 0.302412),
	vec2(-0.2679778, -0.654425),
	vec2(-0.2554769, -0.4441924),
	vec2(-0.243476, -0.8034022),
	vec2(-0.2367678, -0.108045),
	vec2(-0.2196257, 0.8243803),
	vec2(-0.2119443, 0.06230118),
	vec2(-0.1708038, -0.9437978),
	vec2(-0.1694005, 0.5692244),
	vec2(-0.136494, 0.3937041),
	vec2(-0.1318274, -0.2166154),
	vec2(-0.09781472, -0.5743775),
	vec2(-0.09480921, 0.2369129),
	vec2(-0.07638182, -0.0571501),
	vec2(-0.06661344, -0.7966294),
	vec2(-0.06305461, -0.3521975),
	vec2(-0.04525706, 0.6982157),
	vec2(-0.04149697, 0.9666064),
	vec2(-0.003192461, -0.9693027),
	vec2(0.0104818, 0.5000805),
	vec2(0.03228819, -0.1681713),
	vec2(0.03715288, -0.673852),
	vec2(0.08470399, -0.3922319),
	vec2(0.09848712, -0.8374477),
	vec2(0.09940207, 0.1117471),
	vec2(0.1395643, 0.313647),
	vec2(0.1565993, 0.8555924),
	vec2(0.1772605, -0.5248074),
	vec2(0.1899546, 0.5249656),
	vec2(0.1952665, -0.9595091),
	vec2(0.213078, -0.07045701),
	vec2(0.2277649, -0.3361143),
	vec2(0.247221, 0.7353553),
	vec2(0.2493455, -0.6874771),
	vec2(0.269915, 0.07673722),
	vec2(0.3039587, 0.9087375),
	vec2(0.3189922, 0.3008468),
	vec2(0.3215453, -0.1954931),
	vec2(0.3593478, 0.4527411),
	vec2(0.3745022, -0.597945),
	vec2(0.3879738, -0.7821383),
	vec2(0.4522015, 0.6819367),
	vec2(0.4591872, -0.4484442),
	vec2(0.4626173, -0.03955235),
	vec2(0.4751598, 0.2083394),
	vec2(0.4894366, 0.8694122),
	vec2(0.4896614, -0.2676601),
	vec2(0.5070116, -0.6733028),
	vec2(0.5525513, 0.436241),
	vec2(0.5542312, -0.8262905),
	vec2(0.6012187, 0.7003717),
	vec2(0.6075609, -0.1610506),
	vec2(0.6291932, 0.2213627),
	vec2(0.6300695, -0.5324634),
	vec2(0.6613995, -0.7056449),
	vec2(0.6699739, -0.3828001),
	vec2(0.6705787, 0.01011722),
	vec2(0.6814164, 0.5618623),
	vec2(0.7808329, 0.261445),
	vec2(0.7830279, -0.1817809),
	vec2(0.8006546, -0.5266678),
	vec2(0.8030878, 0.4266291),
	vec2(0.8259325, 0.08734058),
	vec2(0.8621388, -0.3646038),
	vec2(0.9531851, 0.3011991),
	vec2(0.9578334, -0.1584408),
	vec2(0.9898114, 0.1029227)
	);

const vec2 Poisson128[128] = vec2[](
	vec2(-0.9406119, 0.2160107),
	vec2(-0.920003, 0.03135762),
	vec2(-0.917876, -0.2841548),
	vec2(-0.9166079, -0.1372365),
	vec2(-0.8978907, -0.4213504),
	vec2(-0.8467999, 0.5201505),
	vec2(-0.8261013, 0.3743192),
	vec2(-0.7835162, 0.01432008),
	vec2(-0.779963, 0.2161933),
	vec2(-0.7719588, 0.6335353),
	vec2(-0.7658782, -0.3316436),
	vec2(-0.7341912, -0.5430729),
	vec2(-0.6825727, -0.1883408),
	vec2(-0.6777467, 0.3313724),
	vec2(-0.662191, 0.5155144),
	vec2(-0.6569989, -0.7000636),
	vec2(-0.6021447, 0.7923283),
	vec2(-0.5980815, -0.5529259),
	vec2(-0.5867089, 0.09857152),
	vec2(-0.5774597, -0.8154474),
	vec2(-0.5767041, -0.2656419),
	vec2(-0.575091, -0.4220052),
	vec2(-0.5486979, -0.09635002),
	vec2(-0.5235587, 0.6594529),
	vec2(-0.5170338, -0.6636339),
	vec2(-0.5114055, 0.4373561),
	vec2(-0.4844725, 0.2985838),
	vec2(-0.4803245, 0.8482798),
	vec2(-0.4651957, -0.5392771),
	vec2(-0.4529685, 0.09942394),
	vec2(-0.4523471, -0.3125569),
	vec2(-0.4268422, 0.5644538),
	vec2(-0.4187512, -0.8636028),
	vec2(-0.4160798, -0.0844868),
	vec2(-0.3751733, 0.2196607),
	vec2(-0.3656596, -0.7324334),
	vec2(-0.3286595, -0.2012637),
	vec2(-0.3147397, -0.0006635741),
	vec2(-0.3135846, 0.3636878),
	vec2(-0.3042951, -0.4983553),
	vec2(-0.2974239, 0.7496996),
	vec2(-0.2903037, 0.8890813),
	vec2(-0.2878664, -0.8622097),
	vec2(-0.2588971, -0.653879),
	vec2(-0.2555692, 0.5041648),
	vec2(-0.2553292, -0.3389159),
	vec2(-0.2401368, 0.2306108),
	vec2(-0.2124457, -0.09935001),
	vec2(-0.1877905, 0.1098409),
	vec2(-0.1559879, 0.3356432),
	vec2(-0.1499449, 0.7487829),
	vec2(-0.146661, -0.9256138),
	vec2(-0.1342774, 0.6185387),
	vec2(-0.1224529, -0.3887629),
	vec2(-0.116467, 0.8827716),
	vec2(-0.1157598, -0.539999),
	vec2(-0.09983152, -0.2407187),
	vec2(-0.09953719, -0.78346),
	vec2(-0.08604223, 0.4591112),
	vec2(-0.02128129, 0.1551989),
	vec2(-0.01478849, 0.6969455),
	vec2(-0.01231739, -0.6752576),
	vec2(-0.005001599, -0.004027164),
	vec2(0.00248426, 0.567932),
	vec2(0.00335562, 0.3472346),
	vec2(0.009554717, -0.4025437),
	vec2(0.02231783, -0.1349781),
	vec2(0.04694207, -0.8347212),
	vec2(0.05412609, 0.9042216),
	vec2(0.05812819, -0.9826952),
	vec2(0.1131321, -0.619306),
	vec2(0.1170737, 0.6799788),
	vec2(0.1275105, 0.05326218),
	vec2(0.1393405, -0.2149568),
	vec2(0.1457873, 0.1991508),
	vec2(0.1474208, 0.5443151),
	vec2(0.1497117, -0.3899909),
	vec2(0.1923773, 0.3683496),
	vec2(0.2110928, -0.7888536),
	vec2(0.2148235, 0.9586087),
	vec2(0.2152219, -0.1084362),
	vec2(0.2189204, -0.9644538),
	vec2(0.2220028, -0.5058427),
	vec2(0.2251696, 0.779461),
	vec2(0.2585723, 0.01621339),
	vec2(0.2612841, -0.2832426),
	vec2(0.2665483, -0.6422054),
	vec2(0.2939872, 0.1673226),
	vec2(0.3235748, 0.5643662),
	vec2(0.3269232, 0.6984669),
	vec2(0.3425438, -0.1783788),
	vec2(0.3672505, 0.4398117),
	vec2(0.3755714, -0.8814359),
	vec2(0.379463, 0.2842356),
	vec2(0.3822978, -0.381217),
	vec2(0.4057849, -0.5227674),
	vec2(0.4168737, -0.6936938),
	vec2(0.4202749, 0.8369391),
	vec2(0.4252189, 0.03818182),
	vec2(0.4445904, -0.09360636),
	vec2(0.4684285, 0.5885228),
	vec2(0.4952184, -0.2319764),
	vec2(0.5072351, 0.3683765),
	vec2(0.5136194, -0.3944138),
	vec2(0.519893, 0.7157083),
	vec2(0.5277841, 0.1486474),
	vec2(0.5474944, -0.7618791),
	vec2(0.5692734, 0.4852227),
	vec2(0.582229, -0.5125455),
	vec2(0.583022, 0.008507785),
	vec2(0.6500257, 0.3473313),
	vec2(0.6621304, -0.6280518),
	vec2(0.6674218, -0.2260806),
	vec2(0.6741871, 0.6734863),
	vec2(0.6753459, 0.1119422),
	vec2(0.7083091, -0.4393666),
	vec2(0.7106963, -0.102099),
	vec2(0.7606754, 0.5743545),
	vec2(0.7846709, 0.2282225),
	vec2(0.7871446, 0.3891495),
	vec2(0.8071781, -0.5257092),
	vec2(0.8230689, 0.002674922),
	vec2(0.8531976, -0.3256475),
	vec2(0.8758298, -0.1824844),
	vec2(0.8797691, 0.1284946),
	vec2(0.926309, 0.3576975),
	vec2(0.9608918, -0.03495717),
	vec2(0.972032, 0.2271516)
	);



bool isBlack(vec3 c)
{
	return (dot(c, c) == 0.0);
}

float borderDepthTexture(sampler2D tex, vec2 uv)
{
	return ((uv.x <= 1.0) && (uv.y <= 1.0) &&
		(uv.x >= 0.0) && (uv.y >= 0.0)) ? texture(tex, uv).x : 1.0;
}

float borderPCFTexture(sampler2DShadow tex, vec3 uvz)
{
	return ((uvz.x <= 1.0) && (uvz.y <= 1.0) &&
		(uvz.x >= 0.0) && (uvz.y >= 0.0)) ? texture(tex, uvz) :
		((uvz.z <= 1.0) ? 1.0 : 0.0);
}


// Using similar triangles from the surface point to the area light
vec2 searchRegionRadiusUV(float zWorld)
{
	return lightRadiusUV * (zWorld - lightZNear) / zWorld;
}

// Using similar triangles between the area light, the blocking plane and the surface point
vec2 penumbraRadiusUV(float zReceiver, float zBlocker)
{
	return lightRadiusUV * (zReceiver - zBlocker) / zBlocker;
}

// Project UV size to the near plane of the light
vec2 projectToLightUV(vec2 sizeUV, float zWorld)
{
	return sizeUV * lightZNear / zWorld;
}

// Derivatives of light-space depth with respect to texture2D coordinates
vec2 depthGradient(vec2 uv, float z)
{
	vec2 dz_duv = vec2(0.0, 0.0);

	vec3 duvdist_dx = dFdx(vec3(uv, z));
	vec3 duvdist_dy = dFdy(vec3(uv, z));

	dz_duv.x = duvdist_dy.y * duvdist_dx.z;
	dz_duv.x -= duvdist_dx.y * duvdist_dy.z;

	dz_duv.y = duvdist_dx.x * duvdist_dy.z;
	dz_duv.y -= duvdist_dy.x * duvdist_dx.z;

	float det = (duvdist_dx.x * duvdist_dy.y) - (duvdist_dx.y * duvdist_dy.x);
	dz_duv /= det;

	return dz_duv;
}

float biasedZ(float z0, vec2 dz_duv, vec2 offset)
{
	return z0 + dot(dz_duv, offset) - 0.005;
}

float zClipToEye(float z)
{
	return lightZFar * lightZNear / (lightZFar - z * (lightZFar - lightZNear));
}

// Returns average blocker depth in the search region, as well as the number of found blockers.
// Blockers are defined as shadow-map samples between the surface point and the light.
void findBlocker(
	out float accumBlockerDepth,
	out float numBlockers,
	out float maxBlockers,
	vec2 uv,
	float z0,
	vec2 dz_duv,
	vec2 searchRadiusUV)
{
	accumBlockerDepth = 0.0;
	numBlockers = 0.0;
	maxBlockers = 300.0;

	switch (samplePattern)
	{
	case POISSON_25_25:
	{
		maxBlockers = 25.0;
		for (int i = 0; i < 25; ++i) {
			vec2 offset = Poisson25[i] * searchRadiusUV;
			float shadowMapDepth = borderDepthTexture(shadowMapDepth, uv + offset);
			float z = biasedZ(z0, dz_duv, offset);
			if (shadowMapDepth < z) {
				accumBlockerDepth += shadowMapDepth;
				numBlockers++;
			}
		}
	}
	break;

	case POISSON_32_64:
	{
		maxBlockers = 32.0;
		for (int i = 0; i < 32; ++i) {
			vec2 offset = Poisson32[i] * searchRadiusUV;
			float shadowMapDepth = borderDepthTexture(shadowMapDepth, uv + offset);
			float z = biasedZ(z0, dz_duv, offset);
			if (shadowMapDepth < z) {
				accumBlockerDepth += shadowMapDepth;
				numBlockers++;
			}
		}
	}
	break;

	case POISSON_100_100:
	{
		maxBlockers = 100.0;
		for (int i = 0; i < 100; ++i) {
			vec2 offset = Poisson100[i] * searchRadiusUV;
			float shadowMapDepth = borderDepthTexture(shadowMapDepth, uv + offset);
			float z = biasedZ(z0, dz_duv, offset);
			if (shadowMapDepth < z) {
				accumBlockerDepth += shadowMapDepth;
				numBlockers++;
			}
		}
	}
	break;

	case POISSON_64_128:
	{
		maxBlockers = 64.0;
		for (int i = 0; i < 64; ++i) {
			vec2 offset = Poisson64[i] * searchRadiusUV;
			float shadowMapDepth = borderDepthTexture(shadowMapDepth, uv + offset);
			float z = biasedZ(z0, dz_duv, offset);
			if (shadowMapDepth < z) {
				accumBlockerDepth += shadowMapDepth;
				numBlockers++;
			}
		}
	}
	break;

	case REGULAR_49_225:
	{
		maxBlockers = 49.0;
		vec2 stepUV = searchRadiusUV / 3.0;
		for (int x = -3; x <= 3; ++x) {
			for (int y = -3; y <= 3; ++y) {
				vec2 offset = vec2(x, y) * stepUV;
				float shadowMapDepth = borderDepthTexture(shadowMapDepth, uv + offset);
				float z = biasedZ(z0, dz_duv, offset);
				if (shadowMapDepth < z) {
					accumBlockerDepth += shadowMapDepth;
					numBlockers++;
				}
			}
		}
	}
	break;
	}
}

// Performs PCF filtering on the shadow map using multiple taps in the filter region.
float pcfFilter(vec2 uv, float z0, vec2 dz_duv, vec2 filterRadiusUV)
{
	float sum = 0.0;

	switch (samplePattern)
	{
	case POISSON_25_25:
	{
		for (int i = 0; i < 25; ++i) {
			vec2 offset = Poisson25[i] * filterRadiusUV;
			float z = biasedZ(z0, dz_duv, offset);
			sum += borderPCFTexture(shadowMapPCF, vec3(uv + offset, z));
		}
		return sum / 25.0;
	}

	case POISSON_32_64:
	{
		for (int i = 0; i < 64; ++i) {
			vec2 offset = Poisson64[i] * filterRadiusUV;
			float z = biasedZ(z0, dz_duv, offset);
			sum += borderPCFTexture(shadowMapPCF, vec3(uv + offset, z));
		}
		return sum / 64.0;
	}

	case POISSON_100_100:
	{
		for (int i = 0; i < 100; ++i) {
			vec2 offset = Poisson100[i] * filterRadiusUV;
			float z = biasedZ(z0, dz_duv, offset);
			sum += borderPCFTexture(shadowMapPCF, vec3(uv + offset, z));
		}
		return sum / 100.0;
	}

	case POISSON_64_128:
	{
		for (int i = 0; i < 128; ++i) {
			vec2 offset = Poisson128[i] * filterRadiusUV;
			float z = biasedZ(z0, dz_duv, offset);
			sum += borderPCFTexture(shadowMapPCF, vec3(uv + offset, z));
		}
		return sum / 128.0;
	}

	case REGULAR_49_225:
	{
		vec2 stepUV = filterRadiusUV / 7.0;
		for (int x = -7; x <= 7; ++x) {
			for (int y = -7; y <= 7; ++y) {
				vec2 offset = vec2(x, y) * stepUV;
				float z = biasedZ(z0, dz_duv, offset);
				sum += borderPCFTexture(shadowMapPCF, vec3(uv + offset, z));
			}
		}
		float numSamples = 7.0 * 2.0 + 1.0;
		return sum / (numSamples * numSamples);
	}

	default:
		return 1.0;
	}
}

float pcssShadow(vec2 uv, float z, vec2 dz_duv, float zEye)
{
	// ------------------------
	// STEP 1: blocker search
	// ------------------------
	float accumBlockerDepth, numBlockers, maxBlockers;
	vec2 searchRadiusUV = searchRegionRadiusUV(zEye);
	findBlocker(accumBlockerDepth, numBlockers, maxBlockers, uv, z, dz_duv, searchRadiusUV);

	// Early out if not in the penumbra
	if (numBlockers == 0.0)
		return 1.0;

	// This is for dubug: visualize the percentage of occlusion
	//return  numBlockers / maxBlockers;

	// ------------------------
	// STEP 2: penumbra size
	// ------------------------
	float avgBlockerDepth = accumBlockerDepth / numBlockers;
	float avgBlockerDepthWorld = zClipToEye(avgBlockerDepth);
	vec2 penumbraRadius = penumbraRadiusUV(zEye, avgBlockerDepthWorld);
	vec2 filterRadius = projectToLightUV(penumbraRadius, zEye);

	// ------------------------
	// STEP 3: filtering
	// ------------------------
	return pcfFilter(uv, z, dz_duv, filterRadius);
}


vec4 shade(vec3 worldPos)
{
    if (is_background)
        return vec4(DataIn.color, 1.0f);

    else {
        vec3 normal;
        if (smooth_shading)
            normal = normalize(DataIn.normal);
        else {
#if 0	// Workaround for Adreno GPUs not able to do dFdx( vViewPosition )
			vec3 fdx = vec3( dFdx( DataIn.position.x ), dFdx( DataIn.position.y ), dFdx( DataIn.position.z ) );
			vec3 fdy = vec3( dFdy( DataIn.position.x ), dFdy( DataIn.position.y ), dFdy( DataIn.position.z ) );
			normal = normalize(cross(fdx, fdy));
#else
			normal = normalize(cross(dFdx(DataIn.position), dFdy(DataIn.position)));
#endif
//          // Instead of using the vertex normal to verify the normal direction, we can use gl_FrontFacing.
//          if (dot(normal, DataIn.normal) < 0)
			if ((gl_FrontFacing == false) && (two_sides_lighting == false))
                normal = -normal;
        }

        vec3 view_dir = normalize(wCamPos - worldPos);
        vec3 light_dir = normalize(wLightPos);	// directional light
        //vec3 light_dir = normalize(wLightPos - worldPos); // point light

        float df = 0.0;	// diffuse factor
        if (two_sides_lighting)
                df = abs(dot(light_dir, normal));
        else
                df = max(dot(light_dir, normal), 0);

        float sf = 0.0;	// specular factor
        if (df > 0.0) {	// if the vertex is lit compute the specular color
                vec3 half_vector = normalize(light_dir + view_dir);	// compute the half vector
                if (two_sides_lighting)
                        sf = abs(dot(half_vector, normal));
                else
                        sf = max(dot(half_vector, normal), 0.0);
                sf = pow(sf, shininess);
        }

		vec4 color;
		if (use_texture) {
			float repeat = texture_repeat + fractional_repeat / 100.0f;
			color = texture(textureID, DataIn.texcoord * repeat);

#ifndef ENABLE_ALPHA
			color.a = 1.0f;
#else
			if (color.a <= 0)
				discard;
#endif
		}
		else {
			color = vec4(DataIn.color, 1.0f);
		}

        if ((gl_FrontFacing == false) && distinct_back_color)
			color = vec4(backside_color, color.a);

        color = color * df + vec4(specular, 1.0) * sf;
        return color;
    }
}


void main(void) {
	if (DataIn.clipped > 0.0)
		discard;

    vec4 color = shade(DataIn.position);

	if (selected && !is_background)
		color = mix(color, highlight_color, 0.6);

    // transforming from clip space to NDC space
    vec3 ProjCoords = DataIn.shadowCoord.xyz / DataIn.shadowCoord.w;
    // It seems checking Z is sufficient (implies X and Y)?
    if ((isBlack(color.rgb)) ||
            //(ProjCoords.x <= 0 || ProjCoords.x >= 1) ||
            //(ProjCoords.y <= 0 || ProjCoords.y >= 1) ||
            (ProjCoords.z <= 0 || ProjCoords.z >= 1))
    {
        if (is_background)
            FragColor = color;
        else
            FragColor = color + vec4(ambient, 1.0f);
    }
    else {
            vec2 uv = ProjCoords.xy;
            float z = ProjCoords.z;

            // Compute gradient using ddx/ddy before any branching
            vec2 dz_duv = depthGradient(uv, z);

            // Eye-space z from the light's point of view
            float zEye = -(lightViewMatrix * vec4(DataIn.position, 1.0)).z;
            float shadow = pcssShadow(uv, z, dz_duv, zEye);

            // I don't want the shadow regions to be completely dark.
            shadow = (1.0 - darkness) + shadow * darkness;

            if (is_background)
                FragColor = color * shadow;
            else
                FragColor = color * shadow + vec4(ambient, 1.0f);
    }
}
