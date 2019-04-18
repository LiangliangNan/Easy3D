/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work, 
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan. 
*           Easy3D: a lightweight, easy-to-use, and efficient C++ 
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef EASY3D_OPENGL_READ_PIXEL_H
#define EASY3D_OPENGL_READ_PIXEL_H

#include <string>
#include <vector>


namespace easy3d {

	namespace opengl {

		// NOTE: For the multisample-version functions, a normal fbo will be created for bliting color/depth from the 
		//       multisample framebuffer object. So for high-frequent queries, you'd better create a normal fbo for 
		//		 bliting operations. This way, you can avoid frequently allocating and deallocating GPU memories. But, 
		//		 do remember to do the cleaning when all queries are done.

		// read color value at pixel (x, y) from current fbo.
		// (x, y): the pixel coordinates, which are in the OpenGL coordinate system. 
		void read_color(unsigned char rgba[4], int x, int y);
		void read_color_ms(int index, unsigned char rgba[4], int x, int y);	// multisample framebuffer object

		// read depth value at pixel (x, y) from current fbo.
		// (x, y): the pixel coordinates, which are in the OpenGL coordinate system. 
		void read_depth(float& depth, int x, int y);
		void read_depth_ms(float& depth, int x, int y);		// multisample framebuffer object

		// read the color data of the framebuffer into a specified buffer.
		// format: format of the pixel data. The following formats are accepted: GL_RGB, GL_BGR, GL_RGBA, and GL_BGRA.
		void read_color(int index, std::vector<unsigned char>& buffer, unsigned int format, bool flip_vertically = true);
		void read_color_ms(int index, std::vector<unsigned char>& buffer, unsigned int format, bool flip_vertically = true); // multisample framebuffer object

		// read the depth data of the framebuffer into a specified buffer.
		void read_depth(std::vector<float>& buffer, bool flip_vertically = true);
		void read_depth_ms(std::vector<float>& buffer, bool flip_vertically = true); // multisample framebuffer object

		//////////////////////////////////////////////////////////////////////////

		// snapshot the color data of the framebuffer as a PPM image (RGB, no alpha). This is very useful for debugging.
		void snapshot_color_ppm(const std::string& file_name);
		void snapshot_color_ppm_ms(int index, const std::string& file_name);	// multisample framebuffer object

		// snapshot the color data of the framebuffer as a tga image (BGRA, with alpha). This is very useful for debugging.
		void snapshot_color_tga(int index, const std::string& file_name);
		void snapshot_color_tga_ms(int index, const std::string& file_name);	// multisample framebuffer object

		// snapshot the depth data of the framebuffer as a ppm image. This is very useful for debugging.
		void snapshot_depth_ppm(const std::string& file_name);
		void snapshot_depth_ppm_ms(const std::string& file_name);	// multisample framebuffer object

	}

}

#endif // EASY3D_OPENGL_READ_PIXEL_H


