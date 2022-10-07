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

#ifndef EASY3D_IMAGE_ENCODER_H
#define EASY3D_IMAGE_ENCODER_H

#include <string>


namespace internal {
	class VideoEncoderImpl;
}


namespace easy3d {

	class VideoEncoder
	{
	public:
		VideoEncoder();
		~VideoEncoder();

		/**
		 * \param file_name The name of the output video file, e.g., "C:/result.mp4".
		 *		  The output format is automatically guessed according to the file extension.
		 */
		bool start(const std::string& file_name, int framerate, int bitrate);

		/**
		 * Encode one frame and write it to the video stream.
		 * \param data The input image data. It is a 1D array of 'unsigned char' which points to the pixel data.
		 *		The pixel data consists of 'height' scanlines of 'width' pixels, with each pixel consisting of 'channels'
		 *		interleaved 8-bit components. The first pixel pointed to its top-left-most in the image.
		 * \return true on successful.
		 * todo  When using QImage, I am confused when choosing the proper pixel format. The following seems to work, but why?
         *   AV_PIX_FMT_RGBA works for QImage::Format_RGBA8888 and  QImage::Format_RGBA8888_Premultiplied.
         *   AV_PIX_FMT_BGRA workd for QImage::Format_RGB32, QImage::Format_ARGB32, and QImage::Format_ARGB32_Premultiplied.
		 **/
		bool encode_frame(const unsigned char* data, int width, int height, int channels);

		bool end();

	private:
		internal::VideoEncoderImpl* encoder_;
	};

}

#endif	// EASY3D_IMAGE_ENCODER_H