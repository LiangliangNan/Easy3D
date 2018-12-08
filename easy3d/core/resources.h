#ifndef _EASY3D_OPENGL_SHADER_CODES_H_
#define _EASY3D_OPENGL_SHADER_CODES_H_


#include <easy3d/model/math_types.h>

#include <string>
#include <vector>

namespace easy3d {

	namespace shadercode {

		extern std::string points_color_frag;
		extern std::string points_color_vert;

		extern std::string lines_color_frag;
		extern std::string lines_color_vert;

		extern std::string surface_color_frag;
		extern std::string surface_color_vert;
	}


	namespace demodata {
		extern const std::vector<vec3> vertices;
		extern const std::vector<vec3> colors;
	}

}

#endif // _EASY3D_OPENGL_SHADER_CODES_H_
