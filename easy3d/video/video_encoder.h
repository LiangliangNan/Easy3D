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

#ifndef EASY3D_VIDEO_ENCODER_H
#define EASY3D_VIDEO_ENCODER_H

#include <string>


namespace internal {
	class VideoEncoderImpl;
}


namespace easy3d {

    /**
     * @brief A class that encodes video frames (e.g., images) into a video file.
     * @details The output format is automatically guessed according to the file extension. Below is an example
     *    of usage:
     *      \code
     *          VideoEncoder encoder;
     *          encoder.start(output_file, 30, 100 * 1024 * 1024);  // 30 fps, 100 Mbit/s
     *          for (std::size_t i = 0; i < image_files.size(); ++i) {
     *              std::vector<unsigned char> data;
     *              int w, h, c;
     *              if (ImageIO::load(file_name, data, w, h, c, 0, false))
     *                  encoder.encode(data.data(), w, h, VideoEncoder::PIX_FMT_RGBA_8888);
     *          }
     *          encoder.end();
     *      \endcode
     * @class VideoEncoder easy3d/video/video_encoder.h
     */
	class VideoEncoder
	{
	public:
        /// Constructor
		VideoEncoder();
        /// Destructor
		~VideoEncoder();

        /// The supported pixel format/storage of the video frames.
        enum PixelFormat {
            PIX_FMT_RGB_888,    /// packed RGB 8:8:8, 24bpp, RGBRGB...
            PIX_FMT_BGR_888,    /// packed BGR 8:8:8, 24bpp, BGRBGR...
            PIX_FMT_RGBA_8888,  /// packed RGBA 8:8:8:8, 32bpp, RGBARGBA...
            PIX_FMT_BGRA_8888   /// packed BGRA 8:8:8:8, 32bpp, BGRABGRA...
        };

		/**
		 * \param file_name The name of the output video file, e.g., "C:/result.mp4".
		 *		  The output format is automatically guessed according to the file extension.
		 */
		bool start(const std::string& file_name, int framerate, int bitrate);

		/**
		 * Encode one frame to the video stream.
		 * \param data The input image data. It is a 1D array of 'unsigned char' which points to the pixel data.
		 *		The pixel data consists of 'height' rows of 'width' pixels, with each pixel has one of the
		 *		following structures. The correspondences between these structures and pixel/OpenGL formats are:
		 *		    RGB 8:8:8, 24bpp     <--->  PIX_FMT_RGB_888    <--->  GL_RGB
         *          BGR 8:8:8, 24bpp     <--->  PIX_FMT_BGR_888    <--->  GL_BGR
         *          RGBA 8:8:8:8, 32bpp  <--->  PIX_FMT_RGBA_8888  <--->  GL_RGBA
         *          BGRA 8:8:8:8, 32bpp  <--->  PIX_FMT_BGRA_8888  <--->  GL_BGRA
		 * \return true on successful.
		 **/
		bool encode(const unsigned char* data, int width, int height, PixelFormat pixel_format);

        /**
         * Finish encoding all the frames.
         * \note It must be called after encoding all frames.
         */
		bool end();

        /// Returns whether the image size (width, height) is acceptable.
        static bool is_size_acceptable(int width, int height) { return (width % 8) == 0 && (height % 8) == 0; }

	private:
		internal::VideoEncoderImpl* encoder_;
	};

}

#endif	// EASY3D_VIDEO_ENCODER_H