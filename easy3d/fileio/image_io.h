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

#ifndef EASY3D_FILEIO_IMAGE_IO_H
#define EASY3D_FILEIO_IMAGE_IO_H

#include <vector>
#include <string>


namespace easy3d {


    class ImageIO
	{
	public:
        /**
         * \brief Load image data from a file. The following formats are supported
         *        JPG/JPEG, PNG, BMP, PSD, TGA, GIF, HDR, PNM (.ppm and .pgm). File format is
         *        determined by the file extension given in the file name.
         *
         * \param file_name The image file to load.
         * \param data  Outputs the image data. Data will be empty if the specified image file
         *              doesn't exist, allocation failed, or the image is corrupt or invalid.
         *              The image data returned is an 1D array of 'unsigned char' which points
         *              to the pixel data, or empty when failed. The pixel data consists of
         *              'height' scanlines of 'width' pixels, with each pixel consisting of N
         *              interleaved 8-bit channels/components; the first pixel pointed to is
         *              top-left-most in the image. There is no padding between image scanlines
         *              or between pixels, regardless of format. The number of channels N is
         *              'requested_channels' if it is non-zero, or 'channels' otherwise.
         *              An output image with N components has the following components interleaved
         *              in this order in each pixel:
         *                  N=#channels     components
         *                  1               grey
         *                  2               grey, alpha
         *                  3               red, green, blue
         *                  4               red, green, blue, alpha
         *
         * \param width Outputs image width in pixels. Unchanged if failed.
         * \param height Outputs image height in pixels. Unchanged if failed.
         * \param channels Outputs the number of 8-bit image channels per pixel. Unchanged if failed.
         * \param requested_channels User requested image channels. If non-zero, force to treat the
         *                           image as if has this number of components, e.g. if you set it 4,
         *                           you will always get RGBA output, but you can check channels to
         *                           get the actual number of channels.
         * \param flip_vertically Flip the image data vertically if it is true (default value). This
         *                        is convenient for OpenGL applications where the first pixel in the
         *                        output array is expected to be the bottom left corner of the image.
         *
         * \return Return true on success or false if failed.
         *
         * \example
         *    int width, height, channels;
         *    std::vector<unsigned char> data;
         *    bool success = ImageIO::load(data, file_name, &width, &height, &channels, 0);
         *    // ... process data if true returned ...
         */
        static bool load(
                const std::string& file_name,
                std::vector<unsigned char>& data,
                int& width,
                int& height,
                int& channels,
                int requested_channels = 0,
                bool flip_vertically = true
                );

        /**
         * \brief Write image data into a file. The following formats are supported
         *        JPG/JPEG, PNG, BMP, and TGA. File format is determined by the file
         *        extension given in the file name.
         *
         * \param file_name The file to which the image data will be save.
         * \param data  The image data. The image data storage must follow
         *                  N=#channels     components
         *                  1               grey
         *                  2               grey, alpha
         *                  3               red, green, blue
         *                  4               red, green, blue, alpha
         * \param width The width of the image, in pixels.
         * \param height The height of the image, in pixels.
         * \param channels The number of 8-bit image channels per pixel.
         * \param flip_vertically Flip the image data vertically before writting.
         *
         * \return Return true on success or false if failed.
         */
        static bool	save(
                const std::string& file_name,
                const std::vector<unsigned char>& data,
                int width,
                int height,
                int channels,
                bool flip_vertically = false
                );
	};


	namespace io {

        /**
          * Some image IO functions I wrote years ago. Just want to keep them.
          * \attention These functions assumes specific input storage.
          */


        /**
         * \attention This function assumes that each pixel has 3 channels in RGB order.
         */
        bool save_ppm(const std::string& file_name, const std::vector<unsigned char>& bits, int width, int height);

        /**
         * \attention This function assumes that each pixel has 4 channels in BGRA order.
         */
        bool save_bmp(const std::string& file_name, const std::vector<unsigned char>& bits, int width, int height);

        /**
         * \attention This function assumes that each pixel has 4 channels in BGRA order.
         */
        bool save_tga(const std::string& file_name, const std::vector<unsigned char>& bits, int width, int height);

    }


} // namespace easy3d


#endif
