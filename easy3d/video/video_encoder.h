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
	 * \brief A class that encodes video frames (e.g., images) into a video file.
	 * \details The output format is automatically guessed according to the file extension. Below is an example:
     *      \code
     *          VideoEncoder encoder(image_file, 30, 8 * 1024 * 1024); // framereate: 30 fps. bitrate: 8 Mbits/sec.
     *          for (std::size_t i = 0; i < image_files.size(); ++i) {
     *              std::vector<unsigned char> data;
     *              int w, h, c;
     *              if (ImageIO::load(file_name, data, w, h, c, 0, false))
     *                  encoder.encode(data.data(), w, h, c == 3 ? VideoEncoder::PIX_FMT_RGB_888 : VideoEncoder::PIX_FMT_RGBA_8888);
     *          }
     *      \endcode
     * \class VideoEncoder easy3d/video/video_encoder.h
     */
	class VideoEncoder
	{
	public:
		/**
		 * \brief Constructor to initialize the video encoder.
		 * \param file_name The name of the output video file, e.g., "D:/result.mp4".
		 *      The output format is automatically guessed according to the file extension, which can be:
		 *          - mp4: MPEG-4 Part 14;
		 *          - mpeg: MPEG-1 Systems / MPEG program stream;
		 *          - avi: Audio Video Interleaved;
		 *          - mov: QuickTime / MOV;
		 *      If it can't be guessed this way then "mp4" is used by default.
		 * \param framerate The frame rate (normally between 25 and 60, default is 30 fps).
		 * \param bitrate The bit rate (default is 8 Mbits/sec).
         */
		explicit VideoEncoder(const std::string& file_name, int framerate = 30, int bitrate = 8 * 1024 * 1024);

        /// \brief Destructor to clean up resources.
		~VideoEncoder();

        /// \brief The supported pixel formats for the video frames.
        enum PixelFormat {
        	PIX_FMT_RGB_888,    ///< Packed RGB 8:8:8, 24bpp, RGBRGB...
			PIX_FMT_BGR_888,    ///< Packed BGR 8:8:8, 24bpp, BGRBGR...
			PIX_FMT_RGBA_8888,  ///< Packed RGBA 8:8:8:8, 32bpp, RGBARGBA...
			PIX_FMT_BGRA_8888   ///< Packed BGRA 8:8:8:8, 32bpp, BGRABGRA...
        };

		/**
		 * \brief Encodes one frame to the video stream.
		 * \param image_data The input image data as a 1D array of 'unsigned char' pointing to the pixel data.
		 *		The pixel data consists of 'height' rows of 'width' pixels, with each pixel has one of the
		 *		following structures.
		 * \param width The video width (must be a multiple of 8).
		 * \param height The video height (must be a multiple of 8).
		 * \param pixel_format The pixel format. Correspondences between image structures and pixel/OpenGL formats are:
		 *     - RGB 8:8:8, 24bpp     <--->  PIX_FMT_RGB_888    <--->  GL_RGB
		 *     - BGR 8:8:8, 24bpp     <--->  PIX_FMT_BGR_888    <--->  GL_BGR
		 *     - RGBA 8:8:8:8, 32bpp  <--->  PIX_FMT_RGBA_8888  <--->  GL_RGBA
		 *     - BGRA 8:8:8:8, 32bpp  <--->  PIX_FMT_BGRA_8888  <--->  GL_BGRA
		 * \return True on success, false otherwise.
		 */
		bool encode(const unsigned char* image_data, int width, int height, PixelFormat pixel_format);

		/**
		 * \brief Checks if the image size (width, height) is acceptable.
		 * \param width The width of the image.
		 * \param height The height of the image.
		 * \return True if the size is acceptable, false otherwise.
		 */
        static bool is_size_acceptable(int width, int height) { return (width % 8) == 0 && (height % 8) == 0; }

	private:
		internal::VideoEncoderImpl* encoder_;

        const std::string file_name_;
        const int framerate_;
        const int bitrate_;
	};

}

#endif	// EASY3D_VIDEO_ENCODER_H