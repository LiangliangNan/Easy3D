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


#ifndef EASY3D_FILEIO_IMAGE_IO_H
#define EASY3D_FILEIO_IMAGE_IO_H

#include <vector>
#include <string>


namespace easy3d {


    class ImageIO
	{
	public:
        // Basic usage:
        //    int w, h, comp;
        //    std::vector<unsigned char> data;
        //    bool success = ImageIO::load(data, file_name, &w, &h, &comp, 0);
        //    // ... process data if true returned ...
        //
        // Parameters:
        //    data:     outputs the image data. Data will beempty if the specified image file
        //              doesn't exist, allocation failed, or the image is corrupt or invalid.
        //    w:        outputs image width in pixels; unchanged if failed.
        //    h:        outputs image height in pixels; unchanged if failed.
        //    comp:     outputs the number of 8-bit image components per pixel in the image file;
        //              unchanged if failed.
        //    req_comp: if non-zero, force to treat the image as if has this number of components,
        //              e.g. if you set req_comp to 4, you will always get RGBA output, but you
        //              can check comp to see the actual number of channels.
        //
        // The image data returned is an 1D array of 'unsigned char' which points to the pixel
        // data, or empty when failed. The pixel data consists of h scanlines of w pixels,
        // with each pixel consisting of N interleaved 8-bit components; the first pixel
        // pointed to is top-left-most in the image. There is no padding between image
        // scanlines or between pixels, regardless of format. The number of components N is
        // 'req_comp' if req_comp is non-zero, or comp otherwise.
        // An output image with N components has the following components interleaved in this
        // order in each pixel:
        //
        //     N=#comp     components
        //       1           grey
        //       2           grey, alpha
        //       3           red, green, blue
        //       4           red, green, blue, alpha

        static bool load(std::vector<unsigned char>& data, const std::string& file_name, int& w, int& h, int& comp, int req_comp = 0);

        // save the image data to a file. Only png, jpg, bmp, and tga formats are supported.
        // File format is determined by the file extension given in the file name.
        // The image data storage must follow
        //     N=#comp     components
        //       1           grey
        //       2           grey, alpha
        //       3           red, green, blue
        //       4           red, green, blue, alpha
        // Return false if failed.
        static bool	save(const std::vector<unsigned char>& data, const std::string& file_name, int w, int h, int comp);
	};


	namespace io {

        // some functions I wrote years ago. This function assumes specific input formats

        // NOTE: assumes that each pixel has 3 chanels in RGB order
        bool save_ppm(const std::string& file_name, const std::vector<unsigned char>& bits, int width, int height);

        // NOTE: assumes that each pixel has 4 chanels in BGRA order
        bool save_bmp(const std::string& file_name, const std::vector<unsigned char>& bits, int width, int height);

        // NOTE: assumes that each pixel has 4 chanels in BGRA order
        bool save_tga(const std::string& file_name, const std::vector<unsigned char>& bits, int width, int height);

    }


} // namespace easy3d


#endif
