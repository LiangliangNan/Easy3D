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

#ifndef EASY3D_RENDERER_OPENGL_READ_PIXEL_H
#define EASY3D_RENDERER_OPENGL_READ_PIXEL_H

#include <string>
#include <vector>


namespace easy3d {

	/**
	 * \brief OpenGL-related functionalities for reading pixel/depth data from the framebuffer.
	 * \details For multisample-version functions, a normal framebuffer is created for bliting color/depth from the
	 *		multisample framebuffer object. For high-frequent queries, you'd better create a normal fbo for bliting
	 *		operations. This way, you can avoid frequently allocating and deallocating GPU memories.
	 * \namespace easy3d::opengl
	 */
	namespace opengl {

		/**
		 * \brief Read color value at pixel (x, y) from the current non-multisample framebuffer.
		 * \param rgba The color value.
		 * \param x The x-coordinate of the pixel, represented in the OpenGL coordinate system.
		 * \param y The y-coordinate of the pixel, represented in the OpenGL coordinate system.
		 * \see read_color_ms.
		 */
		void read_color(unsigned char rgba[4], int x, int y);
		/**
		 * \brief Read color value at pixel (x, y) from the current multisample framebuffer.
		 * \param rgba The color value.
		 * \param x The x-coordinate of the pixel, represented in the OpenGL coordinate system.
		 * \param y The y-coordinate of the pixel, represented in the OpenGL coordinate system.
		 */
		void read_color_ms(unsigned char rgba[4], int x, int y);

		/**
		 * \brief Read depth value at pixel (x, y) from the current non-multisample framebuffer.
		 * \param depth The depth value.
		 * \param x The x-coordinate of the pixel, represented in the OpenGL coordinate system.
		 * \param y The y-coordinate of the pixel, represented in the OpenGL coordinate system.
		 * \see read_depth_ms.
		 */
		void read_depth(float& depth, int x, int y);
		/**
		 * \brief Read depth value at pixel (x, y) from the current multisample framebuffer.
		 * \param depth The depth value.
		 * \param x The x-coordinate of the pixel, represented in the OpenGL coordinate system.
		 * \param y The y-coordinate of the pixel, represented in the OpenGL coordinate system.
		 * \see read_depth.
		 */
		void read_depth_ms(float& depth, int x, int y);

		/**
		* \brief Read color data of the current non-multisample framebuffer into a specified buffer.
		* \param buffer The buffer to store the color data.
		* \param format The format of the pixel data. Supported formats: GL_RGB, GL_BGR, GL_RGBA, and GL_BGRA.
		* \param flip_vertically Flip the image data vertically if it is true (default value). This is convenient for
		*		OpenGL applications where the first pixel in the output array is expected to be the bottom left corner
		*		of the image.
		* \see read_color_ms.
		*/
		void read_color(std::vector<unsigned char>& buffer, unsigned int format, bool flip_vertically = true);
		/**
		* \brief Read color data of the current multisample framebuffer into a specified buffer.
		* \param buffer The buffer to store the color data.
		* \param format The format of the pixel data. Supported formats: GL_RGB, GL_BGR, GL_RGBA, and GL_BGRA.
		* \param flip_vertically Flip the image data vertically if it is true (default value). This is convenient for
		*		OpenGL applications where the first pixel in the output array is expected to be the bottom left corner
		*		of the image.
		* \see read_color.
		*/
		void read_color_ms(std::vector<unsigned char>& buffer, unsigned int format, bool flip_vertically = true);

		/**
		 * \brief Read the depth data of the current non-multisample framebuffer into a specified buffer.
		 * \param buffer The buffer to store the depth data.
		 * \param flip_vertically Flip the image data vertically if it is true (default value). This is convenient for
		 *		OpenGL applications where the first pixel in the output array is expected to be the bottom left corner
		 *		of the image.
		 * \see read_depth_ms.
		 */
		void read_depth(std::vector<float>& buffer, bool flip_vertically = true);
		/**
		 * \brief Read the depth data of the current multisample framebuffer into a specified buffer.
		 * \param buffer The buffer to store the depth data.
		 * \param flip_vertically Flip the image data vertically if it is true (default value). This is convenient for
		 *		OpenGL applications where the first pixel in the output array is expected to be the bottom left corner
		 *		of the image.
		 * \see read_depth.
		 */
		void read_depth_ms(std::vector<float>& buffer, bool flip_vertically = true);

		/**
		 * \brief Snapshot the color render buffer of the current non-multisample framebuffer into an image file. This
		 *		is very useful for debugging.
		 * \param file_name The file name of the image file. Supported formats: png, jpg, bmp, tga, and ppm. File format
		 *		is determined by the extension of the file name.
		 * \see snapshot_color_ms.
		 */
        void snapshot_color(const std::string& file_name);
		/**
		 * \brief Snapshot the color render buffer of the current multisample framebuffer into an image file. This
		 *		is very useful for debugging.
		 * \param file_name The file name of the image file. Supported formats: png, jpg, bmp, tga, and ppm. File format
		 *		is determined by the extension of the file name.
		 * \see snapshot_color.
		 */
        void snapshot_color_ms(const std::string& file_name);

		/**
		 * \brief Snapshot the depth render buffer of the current non-multisample framebuffer into an image file. This
		 *		is very useful for debugging.
		 * \param file_name The file name of the image file. Supported formats: png, jpg, bmp, tga, and ppm. File format
		 *		is determined by the extension of the file name.
		 * \see snapshot_depth_ms.
		 */
        void snapshot_depth(const std::string& file_name);
		/**
		 * \brief Snapshot the depth render buffer of the current multisample framebuffer into an image file. This
		 *		is very useful for debugging.
		 * \param file_name The file name of the image file. Supported formats: png, jpg, bmp, tga, and ppm. File format
		 *		is determined by the extension of the file name.
		 * \see snapshot_depth.
		 */
        void snapshot_depth_ms(const std::string& file_name);

	}

}

#endif // EASY3D_RENDERER_OPENGL_READ_PIXEL_H


