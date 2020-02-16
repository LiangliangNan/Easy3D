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

#include <easy3d/fileio/image_io.h>

#include <iostream>

#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>

#define STB_IMAGE_IMPLEMENTATION
#include <3rd_party/stb/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <3rd_party/stb/stb_image_write.h>


namespace easy3d {

    bool ImageIO::load(
            const std::string& file_name,
            std::vector<unsigned char>& data,
            int& width,
            int& height,
            int& channels,
            int requested_channels,
            bool flip_vertically)
    {
        data.clear();

        // flag is non-zero to flip data vertically (so the first pixel in the output array is the bottom left)
        stbi_set_flip_vertically_on_load(flip_vertically);

        unsigned char* pixels = stbi_load(file_name.c_str(), &width, &height, &channels, requested_channels);
        if (pixels) {
            data.assign(pixels, pixels + height * width * channels);
            stbi_image_free(pixels);
            return true;
        }
        else {
            LOG(ERROR) << "failed load image file: " << file_name << ". " << stbi_failure_reason();
            return false;
        }
    }


    bool ImageIO::save(
            const std::string& file_name,
            const std::vector<unsigned char>& data,
            int width,
            int height,
            int channels,
            bool flip_vertically)
    {
        if (data.empty()) {
            LOG(ERROR) << "image data is empty";
            return false;
        }

        // flag is non-zero to flip data vertically
        stbi_flip_vertically_on_write(flip_vertically);

        std::string final_name = file_name;
        const std::string& ext = file_system::extension(file_name, true);

        if (ext == "png" || ext.empty()) {
            if (ext.empty()) {
                LOG(ERROR) << "No extension specified. Default to png";
                final_name = final_name + ".png";
            }

            // PNG allows you to set the deflate compression level by setting the global
            // variable 'stbi_write_png_compression_level' (it defaults to 8).
            stbi_write_png_compression_level = 8; //
            return ::stbi_write_png(final_name.c_str(), width, height, channels, data.data(), width * channels);
        }
        else if (ext == "jpg") {
            // quality is between 1 and 100. Higher quality looks better but results in a bigger image.
            return ::stbi_write_jpg(final_name.c_str(), width, height, channels, data.data(), 100);
        }
        else if (ext == "bmp")
            return ::stbi_write_bmp(final_name.c_str(), width, height, channels, data.data());
        else if (ext == "tga")
            return ::stbi_write_tga(final_name.c_str(), width, height, channels, data.data());
        else {
            LOG(ERROR) << "unsupported file format: " << ext;
            return false;
        }

    }


    namespace io {

        // NOTE: assumes that each pixel has 3 chanels in RGB order
        bool save_ppm(
                const std::string& file_name,
                const std::vector<unsigned char>& bits,
                int width,
                int height)
        {
            FILE* fptr = fopen(file_name.c_str(), "wb");
            if (!fptr) {
                LOG(ERROR) << "could not open file: " << file_name;
                return false;
            }

            char tmp[256];
            sprintf(tmp, "P6 %i %i %i\n", width, height, 255);
            fwrite(tmp, strlen(tmp), 1, fptr);
            fwrite(bits.data(), 1, bits.size(), fptr);
            fclose(fptr);

            return true;
        }


        // NOTE: assumes that each pixel has 4 chanels in BGRA order
        bool save_bmp(
                const std::string& file_name,
                const std::vector<unsigned char>& bits,
                int width,
                int height)
        {
            #pragma pack(push, 1)
            struct {
                unsigned short  bfType;
                unsigned int    bfSize;
                unsigned int    bfReserved;
                unsigned int    bfOffBits;
                unsigned int    biSize;
                signed   int    biWidth;
                signed   int    biHeight;
                unsigned short  biPlanes;
                unsigned short  biBitCount;
                unsigned int    biCompression;
                unsigned int    biSizeImage;
                signed   int    biXPelsPerMeter;
                signed   int    biYPelsPerMeter;
                unsigned int    biClrUsed;
                unsigned int    biClrImportant;
            } bmpinfo;
            #pragma pack(pop)

            bmpinfo.bfType = 19778;
            bmpinfo.bfSize = sizeof(bmpinfo) + width * height * 4 * sizeof(unsigned char);
            bmpinfo.bfReserved = 0;
            bmpinfo.bfOffBits = 54;

            bmpinfo.biSize = 40;
            bmpinfo.biWidth = width;
            bmpinfo.biHeight = height;
            bmpinfo.biPlanes = 1;
            bmpinfo.biBitCount = 32;
            bmpinfo.biCompression = 0;
            bmpinfo.biSizeImage = 0;
            bmpinfo.biXPelsPerMeter = 0;
            bmpinfo.biYPelsPerMeter = 0;
            bmpinfo.biClrUsed = 0;
            bmpinfo.biClrImportant = 0;

            FILE* bmpfile = fopen(file_name.c_str(), "wb");
            fwrite(&bmpinfo, sizeof(bmpinfo), 1, bmpfile);
            fwrite(bits.data(), sizeof(char), width * height * 4 * sizeof(unsigned char), bmpfile);
            fclose(bmpfile);

            return true;
        }


        // NOTE: assumes that each pixel has 4 chanels in BGRA order
        bool save_tga(
                const std::string& file_name,
                const std::vector<unsigned char>& bits,
                int width,
                int height)
        {
            FILE* fptr = fopen(file_name.c_str(), "wb");
            if (!fptr) {
                LOG(ERROR) << "could not open file: " << file_name;
                return false;
            }

            fputc(0, fptr); /* ID */
            fputc(0, fptr); /* Color map */
            fputc(2, fptr); /* Image type */
            fputc(0, fptr); fputc(0, fptr); /* First entry of color map (unused) */
            fputc(0, fptr); fputc(0, fptr); /* Length of color map (unused) */
            fputc(0, fptr); /* Color map entry size (unused) */
            fputc(0, fptr); fputc(0, fptr);  /* X offset */
            fputc(0, fptr); fputc(0, fptr);  /* Y offset */
            fputc(width % 256, fptr); /* Width */
            fputc(width / 256, fptr); /* continued */
            fputc(height % 256, fptr); /* Height */
            fputc(height / 256, fptr); /* continued */
            fputc(32, fptr);   /* Bits per pixel */
            fputc(0x20, fptr); /* Scan from top left */
            fwrite(bits.data(), bits.size(), 1, fptr);
            fclose(fptr);

            return true;
        }

    }


} // namespace easy3d
