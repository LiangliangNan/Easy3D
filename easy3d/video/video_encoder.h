/** \cond LICENSE
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
 \endcond */

#ifndef EASY3D_VIDEO_ENCODER_H
#define EASY3D_VIDEO_ENCODER_H

#include <string>


namespace internal {
	class VideoEncoderImpl;
}


namespace easy3d {

	/**
	 * \brief Encodes same-sized image frames into a video file using FFmpeg.
	 * \details The output format is selected from the filename extension when supported by the linked FFmpeg build.
	 *          Encoding starts on the first frame; that frame fixes the stream dimensions. Frames are encoded
	 *          synchronously by encode() and the encoded stream is finalized when the object is destroyed.
	 *          Frame dimensions must be multiples of eight and must remain the same for every frame.
	 *          Input pixels are tightly packed rows in the selected channel order.
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
		 * \brief Configure the output video stream.
		 * \param file_name Output filename. Its extension selects the muxer if supported; if FFmpeg cannot infer
		 *                  a format, the implementation attempts to use MP4.
		 * \param framerate Frames per second; defaults to 30.
		 * \param bitrate Target bitrate in bits per second; defaults to 8 * 1024 * 1024.
         */
		explicit VideoEncoder(const std::string& file_name, int framerate = 30, int bitrate = 8 * 1024 * 1024);

        /// \brief Finalize the stream and release encoder resources.
		~VideoEncoder();

        /// \brief The supported pixel formats for the video frames.
        enum PixelFormat {
        	PIX_FMT_RGB_888,    ///< Packed RGB 8:8:8, 24bpp, RGBRGB...
			PIX_FMT_BGR_888,    ///< Packed BGR 8:8:8, 24bpp, BGRBGR...
			PIX_FMT_RGBA_8888,  ///< Packed RGBA 8:8:8:8, 32bpp, RGBARGBA...
			PIX_FMT_BGRA_8888   ///< Packed BGRA 8:8:8:8, 32bpp, BGRABGRA...
        };

		/**
		 * \brief Encode one frame into the output stream.
		 * \param image_data Pointer to tightly packed pixel data. The buffer must contain at least
		 *                   \p width * \p height pixels in the selected format and remain valid for this call.
		 * \param width Frame width in pixels; must be a multiple of eight and match the first frame.
		 * \param height Frame height in pixels; must be a multiple of eight and match the first frame.
		 * \param pixel_format Channel layout of the input pixels:
		 *     - RGB 8:8:8, 24bpp     <--->  PIX_FMT_RGB_888    <--->  GL_RGB
		 *     - BGR 8:8:8, 24bpp     <--->  PIX_FMT_BGR_888    <--->  GL_BGR
		 *     - RGBA 8:8:8:8, 32bpp  <--->  PIX_FMT_RGBA_8888  <--->  GL_RGBA
		 *     - BGRA 8:8:8:8, 32bpp  <--->  PIX_FMT_BGRA_8888  <--->  GL_BGRA
		 * \return True if the frame was accepted and encoded; false if initialization, dimensions, or encoding fails.
		 */
		bool encode(const unsigned char* image_data, int width, int height, PixelFormat pixel_format);

		/**
		 * \brief Check whether the dimensions are divisible by eight.
		 * \param width Frame width in pixels.
		 * \param height Frame height in pixels.
		 * \return True if both dimensions are multiples of eight. This check does not verify positivity or codec support.
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