/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */

#include <easy3d/viewer/read_pixel.h>

#include <iostream>
#include <cstring> // for strlen()

#include <easy3d/viewer/opengl_error.h>
#include <easy3d/viewer/opengl.h>
#include <easy3d/util/file_system.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/logging.h>


namespace easy3d {

	namespace opengl {

		// read color value at pixel (x, y) from current FBO.
		// NOTE: (x, y) - the pixel coordinates, which are in the OpenGL coordinate system. 
        void read_color(unsigned char rgba[4], int x, int y)
		{
			glFinish();

			// We need to configure how glReadPixels will behave with respect to memory alignment. 
			// See http://www.opengl-tutorial.org/miscellaneous/clicking-on-objects/picking-with-an-opengl-hack/
			// and: https://www.khronos.org/opengl/wiki/Common_Mistakes
			glPixelStorei(GL_PACK_ALIGNMENT, 1);				easy3d_debug_log_gl_error;

			glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, rgba);	easy3d_debug_log_gl_error;
		}


		// read color value at pixel (x, y) from current FBO.
		// NOTE: (x, y) - the pixel coordinates, which are in the OpenGL coordinate system. 
		// see http://stackoverflow.com/questions/765434/glreadpixels-from-fbo-fails-with-multisampling?rq=1
		void read_color_ms(int index, unsigned char rgba[4], int x, int y)
		{
			glFinish();

			GLint current_fbo;
			glGetIntegerv(GL_FRAMEBUFFER_BINDING, &current_fbo); easy3d_debug_log_gl_error;
			if (!current_fbo) {
				LOG(ERROR) << "no framebuffer is bound. You might missed makeCurrent()?";
				return;
			}

			GLuint fbo = 0;
            GLuint color_buffer = 0;

			glGenFramebuffers(1, &fbo);					easy3d_debug_log_gl_error;
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);		easy3d_debug_log_gl_error;

			// create a render buffer object to store color info.
			glGenRenderbuffers(1, &color_buffer);								easy3d_debug_log_gl_error;
			glBindRenderbuffer(GL_RENDERBUFFER, color_buffer);					easy3d_debug_log_gl_error;
			glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 1, 1);				easy3d_debug_log_gl_error;

			// attach a render buffer to color attachment point
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_RENDERBUFFER, color_buffer);		easy3d_debug_log_frame_buffer_error;

			//--------------------------------------------------------------------------

			// Bind the multisample FBO for reading
			glBindFramebuffer(GL_READ_FRAMEBUFFER, current_fbo);	easy3d_debug_log_gl_error;

			//--------------------------------------------------------------------------

			// Bind the normal FBO for drawing
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);			easy3d_debug_log_gl_error;

			//--------------------------------------------------------------------------

			// Blit the multisample FBO to the normal FBO

			// we only need a single pixel (x, y)
			glBlitFramebuffer(x, y, x + 1, y + 1, 0, 0, 1, 1, GL_COLOR_BUFFER_BIT, GL_NEAREST);		easy3d_debug_log_gl_error;

			//--------------------------------------------------------------------------

			// Bind the normal FBO for reading
			glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);		easy3d_debug_log_gl_error;

			//--------------------------------------------------------------------------

			// we've blitted the data to [0, 0] or the normal buffer.
			read_color(rgba, 0, 0);

			// clean
			glDeleteRenderbuffers(1, &color_buffer);	easy3d_debug_log_gl_error;
			glDeleteFramebuffers(1, &fbo);				easy3d_debug_log_gl_error;

			glBindFramebuffer(GL_FRAMEBUFFER, current_fbo);				easy3d_debug_log_gl_error;
		}


		// read depth value at pixel (x, y) from current FBO.
		// NOTE: (x, y) - the pixel coordinates, which are in the OpenGL coordinate system. 
		// see http://stackoverflow.com/questions/765434/glreadpixels-from-fbo-fails-with-multisampling?rq=1
		void read_depth(float& depth, int x, int y)
		{
			glFinish();

			// We need to configure how glReadPixels will behave with respect to memory alignment. 
			// See http://www.opengl-tutorial.org/miscellaneous/clicking-on-objects/picking-with-an-opengl-hack/
			// and: https://www.khronos.org/opengl/wiki/Common_Mistakes
			glPixelStorei(GL_PACK_ALIGNMENT, 1);				easy3d_debug_log_gl_error;

			// The performance could be improved if you read for many pixels.
			// See here: https://www.opengl.org/wiki/Common_Mistakes#Depth_Buffer_Precision
			glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);		easy3d_debug_log_gl_error;
		}


		// read depth value at pixel (x, y) from current FBO.
		// NOTE: (x, y) - the pixel coordinates, which are in the OpenGL coordinate system. 
		// see http://stackoverflow.com/questions/765434/glreadpixels-from-fbo-fails-with-multisampling?rq=1
		void read_depth_ms(float& depth, int x, int y)
		{
			glFinish();

			GLint current_fbo;
			glGetIntegerv(GL_FRAMEBUFFER_BINDING, &current_fbo); easy3d_debug_log_gl_error;
			if (!current_fbo) {
				LOG(ERROR) << "no framebuffer is bound. You might missed makeCurrent()?";
				return;
			}

			GLuint fbo = 0;
			GLuint depth_buffer = 0;

			glGenFramebuffers(1, &fbo);					easy3d_debug_log_gl_error;
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);		easy3d_debug_log_gl_error;

			// create a render buffer object to store depth info.
			glGenRenderbuffers(1, &depth_buffer);								easy3d_debug_log_gl_error;
			glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);					easy3d_debug_log_gl_error;

			// Should the internal format of the blit FBO be exactly the same
			// as the current FBO? I have to check the specifications.
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1, 1);	easy3d_debug_log_gl_error;

			// attach a render buffer to depth attachment point
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);		easy3d_debug_log_frame_buffer_error;

			//--------------------------------------------------------------------------

			// Bind the multisample FBO for reading
			glBindFramebuffer(GL_READ_FRAMEBUFFER, current_fbo);	easy3d_debug_log_gl_error;

			//--------------------------------------------------------------------------

			// Bind the normal FBO for drawing
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);			easy3d_debug_log_gl_error;

			//--------------------------------------------------------------------------

			// Blit the multisample FBO to the normal FBO

			// we only need a single pixel (x, y)
			glBlitFramebuffer(x, y, x + 1, y + 1, 0, 0, 1, 1, GL_DEPTH_BUFFER_BIT, GL_NEAREST);		easy3d_debug_log_gl_error;

			//--------------------------------------------------------------------------

			// Bind the normal FBO for reading
			glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);		easy3d_debug_log_gl_error;

			//--------------------------------------------------------------------------

			// we've blitted the data to [0, 0] of the normal buffer.
			read_depth(depth, 0, 0);

			// clean
			glDeleteRenderbuffers(1, &depth_buffer);	easy3d_debug_log_gl_error;
			glDeleteFramebuffers(1, &fbo);				easy3d_debug_log_gl_error;

			glBindFramebuffer(GL_FRAMEBUFFER, current_fbo);						easy3d_debug_log_gl_error;
		}


		// read the color data of the framebuffer into a specified buffer.
		void read_color(std::vector<unsigned char>& buffer, unsigned int format, bool flip_vertically /* = true */)
		{
			glFinish();

			unsigned int bytes_per_pixel = 0;
			if (format == GL_RGB || format == GL_BGR)
				bytes_per_pixel = 3;
			else if (format == GL_RGBA || format == GL_BGRA)
				bytes_per_pixel = 4;
			else {
				LOG(ERROR) << "to read color buffer, the format must be one of GL_RGB, GL_BGR, GL_RGBA, and GL_BGRA.";
				return;
			}

			int viewport[4];
			glGetIntegerv(GL_VIEWPORT, viewport);
			int w = viewport[2];
			int h = viewport[3];

			// We need to configure how glReadPixels will behave with respect to memory alignment. 
			// See http://www.opengl-tutorial.org/miscellaneous/clicking-on-objects/picking-with-an-opengl-hack/
			// and: https://www.khronos.org/opengl/wiki/Common_Mistakes
			glPixelStorei(GL_PACK_ALIGNMENT, 1);				easy3d_debug_log_gl_error;
			// Bind to 0 for conventional pixel operation. See http://www.songho.ca/opengl/gl_pbo.html
			// I don't need it because no PBO is used.
			//glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);				easy3d_debug_log_gl_error;

			buffer.resize(bytes_per_pixel * w * h);
			glReadPixels(0, 0, w, h, format, GL_UNSIGNED_BYTE, buffer.data());	easy3d_debug_log_gl_error;

			// --------------------------------------------------------------------------------------

			// flip vertically
			if (flip_vertically) {
				unsigned int row_size = w * bytes_per_pixel;
				unsigned int half_height = h / 2;
#if 0
				for (int j = 0; j < half_height; ++j) {
					// get a pointer to the two lines we will swap
					unsigned char* row1 = &(buffer[0]) + j * row_size;
					unsigned char* row2 = &(buffer[0]) + (h - 1 - j) * row_size;
					// for each point on line, swap all the channels
					for (int i = 0; i < w; ++i) {
						for (int k = 0; k < bytes_per_pixel; k++) {
							std::swap(row1[bytes_per_pixel*i + k], row2[bytes_per_pixel*i + k]);
						}
					}
				}
#else
				unsigned char *line = new unsigned char[row_size];
				for (unsigned int i = 0, j = h - 1; i < half_height; ++i) {
					memcpy(line, &(buffer[0]) + i * row_size, row_size);
					memcpy(&(buffer[0]) + i * row_size, &(buffer[0]) + j * row_size, row_size);
					memcpy(&(buffer[0]) + j * row_size, line, row_size);
					j--;
				}
				delete[] line;
#endif
			}
		}


		// read the color data of the framebuffer into a specified buffer.
		void read_color_ms(int index, std::vector<unsigned char>& buffer, unsigned int format, bool flip_vertically /* = true */)
		{
			glFinish();

			GLint current_fbo;
			glGetIntegerv(GL_FRAMEBUFFER_BINDING, &current_fbo); easy3d_debug_log_gl_error;
			if (!current_fbo) {
				LOG(ERROR) << "no framebuffer is bound. You might missed makeCurrent()?";
				return;
			}

			int viewport[4];
			glGetIntegerv(GL_VIEWPORT, viewport);
			int w = viewport[2];
			int h = viewport[3];

			GLuint fbo = 0;
			GLuint color_buffer = 0;

			glGenFramebuffers(1, &fbo);					easy3d_debug_log_gl_error;
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);		easy3d_debug_log_gl_error;

			// create a render buffer object to store color info.
			glGenRenderbuffers(1, &color_buffer);								easy3d_debug_log_gl_error;
			glBindRenderbuffer(GL_RENDERBUFFER, color_buffer);					easy3d_debug_log_gl_error;
			glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, w, h);				easy3d_debug_log_gl_error;

			// attach a render buffer to color attachment point
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_RENDERBUFFER, color_buffer);		easy3d_debug_log_frame_buffer_error;

			//--------------------------------------------------------------------------

			// Bind the multisample FBO for reading
			glBindFramebuffer(GL_READ_FRAMEBUFFER, current_fbo);	easy3d_debug_log_gl_error;

			//--------------------------------------------------------------------------

			// Bind the normal FBO for drawing
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);			easy3d_debug_log_gl_error;

			//--------------------------------------------------------------------------

			// Blit the multisample FBO to the normal FBO

			glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);		easy3d_debug_log_gl_error;

			//--------------------------------------------------------------------------

			// Bind the normal FBO for reading
			glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);		easy3d_debug_log_gl_error;

			//--------------------------------------------------------------------------

			read_color(buffer, format, flip_vertically);

			// clean
			glDeleteRenderbuffers(1, &color_buffer);	easy3d_debug_log_gl_error;
			glDeleteFramebuffers(1, &fbo);				easy3d_debug_log_gl_error;

			// bind the current fbo
			glBindFramebuffer(GL_FRAMEBUFFER, current_fbo);				easy3d_debug_log_gl_error;
		}


        void snapshot_color(const std::string& file_name)
		{
            int viewport[4];
            glGetIntegerv(GL_VIEWPORT, viewport);
            int w = viewport[2];
            int h = viewport[3];

            // I can acutally alway ask for RGBA format and use my ImageIO for saving.
            // But for ppm, bmp and tga formats, this causes extra internal
            // formatting (in stb_image_write).

            std::vector<unsigned char> bits;

            const std::string& ext = file_system::extension(file_name, true);
            if (ext == "png" || ext == "jpg") {
                read_color(bits, GL_RGBA, true);
                ImageIO::save(file_name, bits, w, h, 4);
            }
            else if (ext == "ppm") {
                read_color(bits, GL_RGB, true);
                io::save_ppm(file_name, bits, w, h);
            }
            else if (ext == "bmp") {
                read_color(bits, GL_BGRA, false); // bmp is alwasy flipped?
                io::save_bmp(file_name, bits, w, h);
            }
            else if (ext == "tga") {
                read_color(bits, GL_BGRA, true);
                io::save_tga(file_name, bits, w, h);
            }
            else
                LOG(ERROR) << "unknown file format: " << ext;
		}


        void snapshot_color_ms(int index, const std::string& file_name)
		{
            int viewport[4];
            glGetIntegerv(GL_VIEWPORT, viewport);
            int w = viewport[2];
            int h = viewport[3];

            // I can acutally alway ask for RGBA format and use my ImageIO for saving.
            // But for ppm, bmp and tga formats, this causes extra internal
            // formatting (in stb_image_write).

            std::vector<unsigned char> bits;

            const std::string& ext = file_system::extension(file_name, true);
            if (ext == "png" || ext == "jpg") {
                read_color_ms(index, bits, GL_RGBA, true);
                ImageIO::save(file_name, bits, w, h, 4);
            }
            else if (ext == "ppm") {
                read_color_ms(index, bits, GL_RGB, true);
                io::save_ppm(file_name, bits, w, h);
            }
            else if (ext == "bmp") {
                read_color_ms(index, bits, GL_BGRA, true);
                io::save_bmp(file_name, bits, w, h);
            }
            else if (ext == "tga") {
                read_color_ms(index, bits, GL_BGRA, true);
                io::save_tga(file_name, bits, w, h);
            }
            else
                LOG(ERROR) << "unknown file format: " << ext;
		}


		// read the depth data of the framebuffer into a specified buffer.
		void read_depth(std::vector<float>& buffer, bool flip_vertically /* = true */)
		{
			int viewport[4];
			glGetIntegerv(GL_VIEWPORT, viewport);
			int w = viewport[2];
			int h = viewport[3];

			// We need to configure how glReadPixels will behave with respect to memory alignment. 
			// See http://www.opengl-tutorial.org/miscellaneous/clicking-on-objects/picking-with-an-opengl-hack/
			// and: https://www.khronos.org/opengl/wiki/Common_Mistakes
			glPixelStorei(GL_PACK_ALIGNMENT, 1);				easy3d_debug_log_gl_error;

			buffer.resize(w * h);
			glReadPixels(0, 0, w, h, GL_DEPTH_COMPONENT, GL_FLOAT, buffer.data());	easy3d_debug_log_gl_error;

			// --------------------------------------------------------------------------------------

			// flip vertically
			if (flip_vertically) {
				for (int j = 0; j < h / 2; j++) {
					// get a pointer to the two lines we will swap
					float* row1 = &(buffer[0]) + j * w;
					float* row2 = &(buffer[0]) + (h - 1 - j) * w;
					// swap for each point on line
					for (int i = 0; i < w; i++) {
						std::swap(row1[i], row2[i]);
					}
				}
			}
		}


		// read the depth data of the framebuffer into a specified buffer.
		void read_depth_ms(std::vector<float>& buffer, bool flip_vertically /* = true */)
		{
			glFinish();

			GLint current_fbo;
			glGetIntegerv(GL_FRAMEBUFFER_BINDING, &current_fbo); easy3d_debug_log_gl_error;
			if (!current_fbo) {
				LOG(ERROR) << "no framebuffer is bound. You might missed makeCurrent()?";
				return;
			}

			int viewport[4];
			glGetIntegerv(GL_VIEWPORT, viewport);
			int w = viewport[2];
			int h = viewport[3];

			GLuint fbo = 0;
			GLuint depth_buffer = 0;

			glGenFramebuffers(1, &fbo);					easy3d_debug_log_gl_error;
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);		easy3d_debug_log_gl_error;

			// create a render buffer object to store depth info.
			glGenRenderbuffers(1, &depth_buffer);								easy3d_debug_log_gl_error;
			glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);					easy3d_debug_log_gl_error;

			// Should the internal format of the blit FBO be exactly the same
			// as the current FBO? I have to check the specifications.
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);	easy3d_debug_log_gl_error;

			// attach a render buffer to depth attachment point
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);		easy3d_debug_log_frame_buffer_error;

			//--------------------------------------------------------------------------

			// Bind the multisample FBO for reading
			glBindFramebuffer(GL_READ_FRAMEBUFFER, current_fbo);	easy3d_debug_log_gl_error;

			//--------------------------------------------------------------------------

			// Bind the normal FBO for drawing
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);			easy3d_debug_log_gl_error;

			//--------------------------------------------------------------------------

			// Blit the multisample FBO to the normal FBO

			glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_DEPTH_BUFFER_BIT, GL_NEAREST);		easy3d_debug_log_gl_error;

			//--------------------------------------------------------------------------

			// Bind the normal FBO for reading
			glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);		easy3d_debug_log_gl_error;

			//--------------------------------------------------------------------------

			read_depth(buffer, flip_vertically);

			// clean
			glDeleteRenderbuffers(1, &depth_buffer);	easy3d_debug_log_gl_error;
			glDeleteFramebuffers(1, &fbo);				easy3d_debug_log_gl_error;

			// bind the current fbo
			glBindFramebuffer(GL_FRAMEBUFFER, current_fbo);				easy3d_debug_log_gl_error;
		}


        void snapshot_depth(const std::string& file_name)
		{
            int viewport[4];
            glGetIntegerv(GL_VIEWPORT, viewport);
            int w = viewport[2];
            int h = viewport[3];

            std::vector<float> depths;
            read_depth(depths, true);

            // convert the depth values to unsigned char RGB values
            std::vector<unsigned char> bits(depths.size() * 3);
            for (std::size_t i = 0; i < depths.size(); ++i) {
                bits[i * 3] = bits[i * 3 + 1] = bits[i * 3 + 2] = static_cast<unsigned char>(depths[i] * 255);
            }

            const std::string& ext = file_system::extension(file_name, true);
            if (ext == "ppm")
                io::save_ppm(file_name, bits, w, h);
            else
                ImageIO::save(file_name, bits, w, h, 3);
		}


        void snapshot_depth_ms(const std::string& file_name)
		{
            int viewport[4];
            glGetIntegerv(GL_VIEWPORT, viewport);
            int w = viewport[2];
            int h = viewport[3];

            std::vector<float> depths;
            read_depth_ms(depths, true);

            // convert the depth values to unsigned char RGB values
            std::vector<unsigned char> bits(depths.size() * 3);
            for (std::size_t i = 0; i < depths.size(); ++i) {
                bits[i * 3] = bits[i * 3 + 1] = bits[i * 3 + 2] = static_cast<unsigned char>(depths[i] * 255);
            }

            const std::string& ext = file_system::extension(file_name, true);
            if (ext == "ppm")
                io::save_ppm(file_name, bits, w, h);
            else
                ImageIO::save(file_name, bits, w, h, 3);
		}

	}

}
