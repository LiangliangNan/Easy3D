

/**************************************************************/
/* media file output */

#include <iostream>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>
#include <easy3d/fileio/image_io.h>

#include "video_encoder.h"

using namespace easy3d;


int main(int argc, char **argv)
{
	easy3d::logging::initialize();

#if 1
	if (argc < 3) {
		std::cerr << "usage: \n\t"
			<< argv[0] << "  path_to_images  output_file\n" 
			<< "e.g., .\\Images2Video.exe D:\\images D:\\images\\video.mp4\n\n"
			<< "This program generates a video from a sequence of images.\n"
			<< "The output format is automatically guessed according to the file extension.\n";
		return EXIT_SUCCESS;
	}
	const std::string image_dir = argv[1];
	const std::string output_file = argv[2];
#else 
	const std::string image_dir = "D:\\tmp\\images";
	const std::string output_filename = "D:\\tmp\\test.avi";
#endif 

	if (!file_system::is_directory(image_dir)) {
		std::cerr << "Error: image directory doesn't exist: " << image_dir << std::endl;
		return EXIT_FAILURE;
	}

	std::vector<std::string> image_files;
	file_system::get_files(image_dir, image_files, false);
	if (image_files.empty()) {
		std::cerr << "Error: the specified image directory doesn't contain images" << std::endl;
		return EXIT_FAILURE;
	}

	VideoEncoder encoder;
	encoder.start(output_file, 30);
	for (std::size_t i = 0; i < image_files.size(); ++i) {
		std::cout << "encoding " << (i + 1) << "/" << image_files.size() << std::endl;
		std::vector<unsigned char> data;
		int w, h, c;
		ImageIO::load(image_dir + "\\" + image_files[i], data, w, h, c, 0, false);
		encoder.write_frame(data.data(), w, h, c);
	}
	encoder.end();

	return EXIT_SUCCESS;
}
