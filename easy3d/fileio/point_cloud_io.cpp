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


#include <easy3d/fileio/point_cloud_io.h>

#include <clocale>
#include <fstream>

#include <easy3d/core/point_cloud.h>
#include <easy3d/util/file.h>
#include <easy3d/util/stop_watch.h>


// TODO: use Memory map of a file
//		 see how this is used in the QSplat source code: 
//		 http://graphics.stanford.edu/software/qsplat/
/*
// Memory map a file, returning pointers to the start of the allocated
// memory region and the start of the map
// This returns true if we were able to do the mmap
bool QSplat_Model::MapFile(HANDLE fd, off_t len,
			   unsigned char **mem_start,
			   unsigned char **map_start)
{
#ifdef WIN32
	static HANDLE fdMapping;
	char mapName[16];
	int mapNum = 0;
	UINT error;
	do {
		if (fdMapping)
			CloseHandle(fdMapping);
		sprintf(mapName, "QSplat%d", mapNum++);
		fdMapping = CreateFileMapping(fd, nullptr, PAGE_READONLY,
						  0, len, mapName);
	} while ((error = GetLastError()) == ERROR_ALREADY_EXISTS);

	if (!fdMapping || error != NO_ERROR) {
		char fdError[32];
		sprintf(fdError, "Mapping Error: %d", error);
		MessageBox(nullptr, fdError, nullptr, MB_OK);
		return false;
	}

	*map_start = (unsigned char *) MapViewOfFile(fdMapping, FILE_MAP_READ,
							 0, 0, len);
	if (*map_start)
		return true;

	char errorstr[32];
	sprintf(errorstr, "Can't Allocate Memory! (%d)\n", GetLastError());
	MessageBox(nullptr, errorstr, nullptr, MB_OK | MB_ICONHAND);
	return false;

#else

	// First, we just try the mmap
	*map_start = (unsigned char *)
			mmap(0, len, PROT_READ, MAP_SHARED, fd, 0);

	// If that worked, we're done
	if (*map_start != MAP_FAILED)
		return true;


	// OK, that didn't work.  However, on certain operating systems
	// (cough, cough, IRIX, cough, cough), it is sometimes the case
	// that for some bizarre reason the mmap doesn't work right off
	// even though we officially do have a large-enough virtual address
	// space to map in everything.  To try to work around this, we'll
	// try to allocate a large-enough chunk of memory, and mmap there

	// Figure out how much memory to allocate
#ifdef sgi
	int align = sysconf(_SC_MMAP_FIXED_ALIGNMENT);
#else
	int align = sysconf(_SC_PAGE_SIZE);
#endif

	// To work around yet another bug, we temporarily allocate some memory
	// here, else sometimes we don't get to use the heap (!) after the mmap
	unsigned char *ugly_hack = new unsigned char[16*1024*1024];

	// Allocate memory and align the pointer correctly
	*mem_start = new unsigned char[len + align];
	long tmp = long(*mem_start);
	tmp = align * long(ceil((double) tmp / align));
	*map_start = (unsigned char *)tmp;


	// Do the mmap
	*map_start = (unsigned char *)
			mmap( (char *)(*map_start), len,
				  PROT_READ, MAP_SHARED | MAP_FIXED,
				  fd, 0 );
	delete [] ugly_hack;
	if (*map_start == MAP_FAILED) {
		// Give up
		perror("mmap");
		delete [] (*mem_start);
		*mem_start = *map_start = 0;
		return false;
	}

	return true;
#endif
}


// Destructor - unmap the file
QSplat_Model::~QSplat_Model()
{
	fragments.clear();
#ifdef WIN32
	UnmapViewOfFile(map_start);
#else
	munmap((char *)map_start, len);
#endif
	if (mem_start)
		delete [] mem_start;
	CloseHandle(fd);
}

*/


namespace easy3d {


	PointCloud* PointCloudIO::load(const std::string& file_name)
	{
		std::setlocale(LC_NUMERIC, "C");

        const std::string& ext = file::extension(file_name, true);

		PointCloud* cloud = new PointCloud;
		cloud->set_name(file_name);

		StopWatch w;

		bool success = false;
		if (ext == "ply")
			success = io::load_ply(file_name, cloud);
		else if (ext == "bin")
			success = io::load_bin(file_name, cloud);
		else if (ext == "xyz")
			success = io::load_xyz(file_name, cloud);
		else if (ext == "bxyz")
			success = io::load_bxyz(file_name, cloud);
		else if (ext == "las" || ext == "laz")
			success = io::load_las(file_name, cloud);
		//	else if (ext == "vg")
		//		PointSetSerializer_vg::load_vg(file_name, cloud);
		//	else if (ext == "bvg")
		//		PointSetSerializer_vg::load_bvg(file_name, cloud);

		else {
			std::cerr << "unknown file format: " << ext << std::endl;
			success = false;
		}

		if (!success || cloud->n_vertices() == 0) {
			delete cloud;
			return nullptr;
		}

#ifndef NDEBUG
		std::cout << "vertex properties on point cloud " << file::base_name(file_name) << std::endl;
		const auto& vnames = cloud->vertex_properties();
		for (const auto& n : vnames)
			std::cout << "\t" << n << std::endl;
#endif

		std::cout << "load model done. time: " << w.time_string() << std::endl;

		return cloud;
	}


	bool PointCloudIO::save(const std::string& file_name, const PointCloud* cloud) {
		if (!cloud) {
			std::cerr << "Point cloud is null" << std::endl;
			return false;
		}

		std::string ext = file::extension(file_name, true);
		bool success = false;
		if (ext == "ply")
			success = io::save_ply(file_name, cloud, true);
		else if (ext == "bin")
			success = io::save_bin(file_name, cloud);
		else if (ext == "xyz")
			success = io::save_xyz(file_name, cloud);
		else if (ext == "bxyz")
			success = io::save_bxyz(file_name, cloud);
		else if (ext == "las" || ext == "laz")
			success = io::save_las(file_name, cloud);
		//	else if (ext == "vg")
		//        PointSetSerializer_vg::save_vg(file_name, cloud);
		//	else if (ext == "bvg")
		//        PointSetSerializer_vg::save_bvg(file_name, cloud);
		else {
			std::cerr << "unknown file format: " << ext << std::endl;
			success = false;
		}

		return success;
	}

} // namespace easy3d
