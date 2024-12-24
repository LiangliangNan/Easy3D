#include <easy3d/fileio/image_io.h>

#include <memory>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_fileio_image_io(pybind11::module_& m)
{
	{ // easy3d::ImageIO file:easy3d/fileio/image_io.h line:39
		pybind11::class_<easy3d::ImageIO, std::shared_ptr<easy3d::ImageIO>> cl(m, "ImageIO", "Implementation of file input/output operations for images.\n \n");
		cl.def( pybind11::init( [](){ return new easy3d::ImageIO(); } ) );
		cl.def_static("load", [](const std::string & a0, class std::vector<unsigned char> & a1, int & a2, int & a3, int & a4) -> bool { return easy3d::ImageIO::load(a0, a1, a2, a3, a4); }, "", pybind11::arg("file_name"), pybind11::arg("data"), pybind11::arg("width"), pybind11::arg("height"), pybind11::arg("channels"));
		cl.def_static("load", [](const std::string & a0, class std::vector<unsigned char> & a1, int & a2, int & a3, int & a4, int const & a5) -> bool { return easy3d::ImageIO::load(a0, a1, a2, a3, a4, a5); }, "", pybind11::arg("file_name"), pybind11::arg("data"), pybind11::arg("width"), pybind11::arg("height"), pybind11::arg("channels"), pybind11::arg("requested_channels"));
		cl.def_static("load", (bool (*)(const std::string &, class std::vector<unsigned char> &, int &, int &, int &, int, bool)) &easy3d::ImageIO::load, "Load image data from a file. The following formats are supported\n        JPG/JPEG, PNG, BMP, PSD, TGA, GIF, HDR, PNM (.ppm and .pgm). File format is\n        determined by the file extension given in the file name.\n\n \n The image file to load.\n \n\n  Outputs the image data. Data will be empty if the specified image file\n              doesn't exist, allocation failed, or the image is corrupt or invalid.\n              The image data returned is a 1D array of 'unsigned char' which points\n              to the pixel data, or empty when failed. The pixel data consists of\n              'height' scanlines of 'width' pixels, with each pixel consisting of N\n              interleaved 8-bit channels/components; the first pixel pointed to is\n              top-left-most in the image. There is no padding between image scanlines\n              or between pixels, regardless of format. The number of channels N is\n              'requested_channels' if it is non-zero, or 'channels' otherwise.\n              An output image with N components has the following components interleaved\n              in this order in each pixel:\n                  N=#channels     components\n                  1               grey\n                  2               grey, alpha\n                  3               red, green, blue\n                  4               red, green, blue, alpha\n\n \n Outputs image width in pixels. Unchanged if failed.\n \n\n Outputs image height in pixels. Unchanged if failed.\n \n\n Outputs the number of 8-bit image channels per pixel. Unchanged if failed.\n \n\n User requested image channels. If non-zero, force to treat the\n                           image as if has this number of components, e.g. if you set it 4,\n                           you will always get RGBA output, but you can check channels to\n                           get the actual number of channels.\n \n\n Flip the image data vertically if it is true (default value). This\n                        is convenient for OpenGL applications where the first pixel in the\n                        output array is expected to be the bottom left corner of the image.\n\n \n true on success or false if failed.\n\n Example usage:\n      \n\n\n\n\n\n\n         \n\nC++: easy3d::ImageIO::load(const std::string &, class std::vector<unsigned char> &, int &, int &, int &, int, bool) --> bool", pybind11::arg("file_name"), pybind11::arg("data"), pybind11::arg("width"), pybind11::arg("height"), pybind11::arg("channels"), pybind11::arg("requested_channels"), pybind11::arg("flip_vertically"));
		cl.def_static("save", [](const std::string & a0, const class std::vector<unsigned char> & a1, int const & a2, int const & a3, int const & a4) -> bool { return easy3d::ImageIO::save(a0, a1, a2, a3, a4); }, "", pybind11::arg("file_name"), pybind11::arg("data"), pybind11::arg("width"), pybind11::arg("height"), pybind11::arg("channels"));
		cl.def_static("save", (bool (*)(const std::string &, const class std::vector<unsigned char> &, int, int, int, bool)) &easy3d::ImageIO::save, "Write image data into a file. The following formats are supported\n        JPG/JPEG, PNG, BMP, and TGA. File format is determined by the file\n        extension given in the file name.\n\n \n The file to which the image data will be save.\n \n\n  The image data. The image data storage must follow\n                  N=#channels     components\n                  1               grey\n                  2               grey, alpha\n                  3               red, green, blue\n                  4               red, green, blue, alpha\n \n\n The width of the image, in pixels.\n \n\n The height of the image, in pixels.\n \n\n The number of 8-bit image channels per pixel.\n \n\n Flip the image data vertically before writing.\n\n \n Return true on success or false if failed.\n\nC++: easy3d::ImageIO::save(const std::string &, const class std::vector<unsigned char> &, int, int, int, bool) --> bool", pybind11::arg("file_name"), pybind11::arg("data"), pybind11::arg("width"), pybind11::arg("height"), pybind11::arg("channels"), pybind11::arg("flip_vertically"));
	}

    // easy3d::io::save_ppm(const std::string &, const class std::vector<unsigned char> &, int, int) file:easy3d/fileio/image_io.h line:138
    m.def("save_ppm", (bool (*)(const std::string &, const class std::vector<unsigned char> &, int, int)) &easy3d::io::save_ppm, "This function assumes that each pixel has 3 channels in RGB order.\n\nC++: easy3d::io::save_ppm(const std::string &, const class std::vector<unsigned char> &, int, int) --> bool", pybind11::arg("file_name"), pybind11::arg("bits"), pybind11::arg("width"), pybind11::arg("height"));

    // easy3d::io::save_bmp(const std::string &, const class std::vector<unsigned char> &, int, int) file:easy3d/fileio/image_io.h line:143
    m.def("save_bmp", (bool (*)(const std::string &, const class std::vector<unsigned char> &, int, int)) &easy3d::io::save_bmp, "This function assumes that each pixel has 4 channels in BGRA order.\n\nC++: easy3d::io::save_bmp(const std::string &, const class std::vector<unsigned char> &, int, int) --> bool", pybind11::arg("file_name"), pybind11::arg("bits"), pybind11::arg("width"), pybind11::arg("height"));

    // easy3d::io::save_tga(const std::string &, const class std::vector<unsigned char> &, int, int) file:easy3d/fileio/image_io.h line:148
    m.def("save_tga", (bool (*)(const std::string &, const class std::vector<unsigned char> &, int, int)) &easy3d::io::save_tga, "This function assumes that each pixel has 4 channels in BGRA order.\n\nC++: easy3d::io::save_tga(const std::string &, const class std::vector<unsigned char> &, int, int) --> bool", pybind11::arg("file_name"), pybind11::arg("bits"), pybind11::arg("width"), pybind11::arg("height"));
}
