#include <easy3d/video/video_encoder.h>

#include <memory>

#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif


void bind_easy3d_video_video_encoder(pybind11::module_& m)
{
	{ // easy3d::VideoEncoder file:easy3d/video/video_encoder.h line:55
		pybind11::class_<easy3d::VideoEncoder, std::shared_ptr<easy3d::VideoEncoder>> cl(m, "VideoEncoder", "A class that encodes video frames (e.g., images) into a video file.\n \n\n The output format is automatically guessed according to the file extension. Below is an example\n    of usage:\n      \n\n\n\n\n\n\n\n\n\n \n\n     ");
		cl.def( pybind11::init( [](const std::string & a0){ return new easy3d::VideoEncoder(a0); } ), "doc" , pybind11::arg("file_name"));
		cl.def( pybind11::init( [](const std::string & a0, int const & a1){ return new easy3d::VideoEncoder(a0, a1); } ), "doc" , pybind11::arg("file_name"), pybind11::arg("framerate"));
		cl.def( pybind11::init<const std::string &, int, int>(), pybind11::arg("file_name"), pybind11::arg("framerate"), pybind11::arg("bitrate") );

		cl.def( pybind11::init( [](easy3d::VideoEncoder const &o){ return new easy3d::VideoEncoder(o); } ) );

		pybind11::enum_<easy3d::VideoEncoder::PixelFormat>(cl, "PixelFormat", pybind11::arithmetic(), "The supported pixel format/storage of the video frames.")
			.value("PIX_FMT_RGB_888", easy3d::VideoEncoder::PIX_FMT_RGB_888)
			.value("PIX_FMT_BGR_888", easy3d::VideoEncoder::PIX_FMT_BGR_888)
			.value("PIX_FMT_RGBA_8888", easy3d::VideoEncoder::PIX_FMT_RGBA_8888)
			.value("PIX_FMT_BGRA_8888", easy3d::VideoEncoder::PIX_FMT_BGRA_8888)
			.export_values();

		cl.def("encode", (bool (easy3d::VideoEncoder::*)(const unsigned char *, int, int, enum easy3d::VideoEncoder::PixelFormat)) &easy3d::VideoEncoder::encode, "Encode one frame to the video stream.\n \n\n The input image data. It is a 1D array of 'unsigned char' which points to the pixel data.\n		The pixel data consists of 'height' rows of 'width' pixels, with each pixel has one of the\n		following structures.\n \n\n video width (must be a multiple of 8)\n \n\n video height (must be a multiple of 8)\n \n\n pixel format. The correspondences between the image structures and pixel/OpenGL formats are:\n		    RGB 8:8:8, 24bpp     <--->  PIX_FMT_RGB_888    <--->  GL_RGB\n          BGR 8:8:8, 24bpp     <--->  PIX_FMT_BGR_888    <--->  GL_BGR\n          RGBA 8:8:8:8, 32bpp  <--->  PIX_FMT_RGBA_8888  <--->  GL_RGBA\n          BGRA 8:8:8:8, 32bpp  <--->  PIX_FMT_BGRA_8888  <--->  GL_BGRA\n \n\n true on successful.\n\nC++: easy3d::VideoEncoder::encode(const unsigned char *, int, int, enum easy3d::VideoEncoder::PixelFormat) --> bool", pybind11::arg("image_data"), pybind11::arg("width"), pybind11::arg("height"), pybind11::arg("pixel_format"));
		cl.def_static("is_size_acceptable", (bool (*)(int, int)) &easy3d::VideoEncoder::is_size_acceptable, "Returns whether the image size (width, height) is acceptable.\n\nC++: easy3d::VideoEncoder::is_size_acceptable(int, int) --> bool", pybind11::arg("width"), pybind11::arg("height"));
	}
}
