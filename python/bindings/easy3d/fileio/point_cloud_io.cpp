#include <easy3d/core/point_cloud.h>
#include <easy3d/fileio/point_cloud_io.h>
#include <easy3d/fileio/point_cloud_io_ptx.h>
#include <easy3d/fileio/point_cloud_io_vg.h>

#include <memory>

#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_fileio_point_cloud_io(pybind11::module_ &m)
{
	{ // easy3d::PointCloudIO file:easy3d/fileio/point_cloud_io.h line:43
		pybind11::class_<easy3d::PointCloudIO, std::shared_ptr<easy3d::PointCloudIO>> cl(m, "PointCloudIO", "Implementation of file input/output operations for PointCloud.\n \n\n\n     ");
		cl.def( pybind11::init( [](){ return new easy3d::PointCloudIO(); } ) );
		cl.def_static("load", (class easy3d::PointCloud * (*)(const std::string &)) &easy3d::PointCloudIO::load, "Reads a point cloud from file \n \n\n File extension determines file format (bin, xyz/bxyz, ply, las/laz, vg/bvg)\n and type (i.e. binary or ASCII).\n \n\n The pointer of the point cloud (nullptr if failed).\n\nC++: easy3d::PointCloudIO::load(const std::string &) --> class easy3d::PointCloud *", pybind11::return_value_policy::automatic, pybind11::arg("file_name"));
		cl.def_static("save", (bool (*)(const std::string &, const class easy3d::PointCloud *)) &easy3d::PointCloudIO::save, "Saves a point_cloud to a file.\n \n\n File extension determines file format (bin, xyz/bxyz, ply, las/laz, vg/bvg) and type (i.e. binary\n or ASCII).\n \n\n The file name.\n \n\n The point cloud.\n \n\n The status of the operation\n      \n\n true if succeeded\n      \n\n false if failed\n\nC++: easy3d::PointCloudIO::save(const std::string &, const class easy3d::PointCloud *) --> bool", pybind11::arg("file_name"), pybind11::arg("cloud"));
	}
	{
		// easy3d::io::load_bin(const std::string &, class easy3d::PointCloud *) file:easy3d/fileio/point_cloud_io.h line:73
		m.def("load_bin", (bool (*)(const std::string &, class easy3d::PointCloud *)) &easy3d::io::load_bin, "Reads point cloud from a  format file.\n \n\n A typical  format file contains three blocks storing points, colors (optional),\n and normals (optional).\n\nC++: easy3d::io::load_bin(const std::string &, class easy3d::PointCloud *) --> bool", pybind11::arg("file_name"), pybind11::arg("cloud"));

		// easy3d::io::save_bin(const std::string &, const class easy3d::PointCloud *) file:easy3d/fileio/point_cloud_io.h line:77
		m.def("save_bin", (bool (*)(const std::string &, const class easy3d::PointCloud *)) &easy3d::io::save_bin, "Saves a point cloud to a  format file.\n \n\n A typical  format file contains three blocks storing points, colors (optional),\n and normals (optional).\n\nC++: easy3d::io::save_bin(const std::string &, const class easy3d::PointCloud *) --> bool", pybind11::arg("file_name"), pybind11::arg("cloud"));

		// easy3d::io::load_xyz(const std::string &, class easy3d::PointCloud *) file:easy3d/fileio/point_cloud_io.h line:82
		m.def("load_xyz", (bool (*)(const std::string &, class easy3d::PointCloud *)) &easy3d::io::load_xyz, "Reads point cloud from an  format file.\n \n\n Each line of an  file contains three floating point numbers representing the   and\n  coordinates of a point.\n\nC++: easy3d::io::load_xyz(const std::string &, class easy3d::PointCloud *) --> bool", pybind11::arg("file_name"), pybind11::arg("cloud"));

		// easy3d::io::save_xyz(const std::string &, const class easy3d::PointCloud *) file:easy3d/fileio/point_cloud_io.h line:83
		m.def("save_xyz", (bool (*)(const std::string &, const class easy3d::PointCloud *)) &easy3d::io::save_xyz, "C++: easy3d::io::save_xyz(const std::string &, const class easy3d::PointCloud *) --> bool", pybind11::arg("file_name"), pybind11::arg("cloud"));

		// easy3d::io::load_bxyz(const std::string &, class easy3d::PointCloud *) file:easy3d/fileio/point_cloud_io.h line:85
		m.def("load_bxyz", (bool (*)(const std::string &, class easy3d::PointCloud *)) &easy3d::io::load_bxyz, "Reads point cloud from a binary  format file.\n\nC++: easy3d::io::load_bxyz(const std::string &, class easy3d::PointCloud *) --> bool", pybind11::arg("file_name"), pybind11::arg("cloud"));

		// easy3d::io::save_bxyz(const std::string &, const class easy3d::PointCloud *) file:easy3d/fileio/point_cloud_io.h line:87
		m.def("save_bxyz", (bool (*)(const std::string &, const class easy3d::PointCloud *)) &easy3d::io::save_bxyz, "Saves a point cloud to a binary  format file.\n\nC++: easy3d::io::save_bxyz(const std::string &, const class easy3d::PointCloud *) --> bool", pybind11::arg("file_name"), pybind11::arg("cloud"));

		// easy3d::io::load_ply(const std::string &, class easy3d::PointCloud *) file:easy3d/fileio/point_cloud_io.h line:90
		m.def("load_ply", (bool (*)(const std::string &, class easy3d::PointCloud *)) &easy3d::io::load_ply, "Reads point cloud from a  format file.\n\nC++: easy3d::io::load_ply(const std::string &, class easy3d::PointCloud *) --> bool", pybind11::arg("file_name"), pybind11::arg("cloud"));

		// easy3d::io::save_ply(const std::string &, const class easy3d::PointCloud *, bool) file:easy3d/fileio/point_cloud_io.h line:92
		m.def("save_ply", [](const std::string & a0, const class easy3d::PointCloud * a1) -> bool { return easy3d::io::save_ply(a0, a1); }, "", pybind11::arg("file_name"), pybind11::arg("cloud"));
		m.def("save_ply", (bool (*)(const std::string &, const class easy3d::PointCloud *, bool)) &easy3d::io::save_ply, "Saves a point cloud to a  format file.\n\nC++: easy3d::io::save_ply(const std::string &, const class easy3d::PointCloud *, bool) --> bool", pybind11::arg("file_name"), pybind11::arg("cloud"), pybind11::arg("binary"));

		// easy3d::io::load_las(const std::string &, class easy3d::PointCloud *) file:easy3d/fileio/point_cloud_io.h line:96
		m.def("load_las", (bool (*)(const std::string &, class easy3d::PointCloud *)) &easy3d::io::load_las, "Reads point cloud from an  format file.\n     Internally the method uses the LASlib of martin.isenburg.com. See http://rapidlasso.com\n\nC++: easy3d::io::load_las(const std::string &, class easy3d::PointCloud *) --> bool", pybind11::arg("file_name"), pybind11::arg("cloud"));

		// easy3d::io::save_las(const std::string &, const class easy3d::PointCloud *) file:easy3d/fileio/point_cloud_io.h line:99
		m.def("save_las", (bool (*)(const std::string &, const class easy3d::PointCloud *)) &easy3d::io::save_las, "Saves a point cloud to an  format file.\n \n\n Internally it uses the LASlib of martin.isenburg.com. See http://rapidlasso.com\n\nC++: easy3d::io::save_las(const std::string &, const class easy3d::PointCloud *) --> bool", pybind11::arg("file_name"), pybind11::arg("cloud"));
	}
	
	{ // easy3d::io::PointCloudIO_ptx file:easy3d/fileio/point_cloud_io_ptx.h line:125
		pybind11::class_<easy3d::io::PointCloudIO_ptx, std::shared_ptr<easy3d::io::PointCloudIO_ptx>> cl(m, "PointCloudIO_ptx", "Implementation of file input/output operations for ASCII Cyclone point cloud export format (PTX).\n \n\n\n  \n PTX is an ASCII based interchange format for point cloud data. It utilizes the concept of separate scans,\n  each with points defined in their own coordinate system and a 'Registration' of all of those point clouds\n  into a single coordinate system. The point data for each cloud is stored in its original coordinate system\n  and a matrix of transforms for each point cloud are provided as header information.\n\n  A PTX file can have one or more point cloud. Each point cloud starts with a header. Each following line\n  then describes one point in the cloud. Cyclone exports PTX with 7 columns when the cloud has RGB values\n  from the digital camera (x, y, z, intensity, red, green, blue). Red, Green, Blue have the integer range [0, 255].\n  A cloud in PTX has 4 columns (x, y, z, intensity) when the cloud does not have RGB values. PTX intensities\n  use the decimal range [0, 1]. Individual values on the same line are separated by a blank space. The coordinate\n  unit is always in meters. PTX can only be used on \"[[KB:132:gridded]]\" clouds (directly from scans), not unordered\n  or unified clouds. Each cloud is \"fully populated\", in that even missing points are represented (as a line with\n  \"0 0 0\" for the XYZ coordinate). This gives the importing software enough information to restore the original\n  scan-line ordering of the cloud (for example, to estimate normal vectors).\n\n  PTX point cloud header:\n      \n\n\n\n\n\n\n\n\n\n\n\n\n  The first four lines of three numbers each are the position and primary axes of the scanner after any\n  registration/transformation. The next four lines of four numbers each may look similar in some cases,\n  but if you have a non-identity UCS when the PTX was exported, the numbers will look different. If the\n  cloud was untransformed by a registration (or not registered), the first four lines of three numbers\n  each would be 0,0,0; 1,0,0; 0,1,0; 0,0,1. The 4x4 matrix may not be identity if there is a UCS applied.\n\n  Note that a cloud that was registered may have a 4x4 identity matrix due to a UCS that is set to that\n  scanner's registered position. For both formats, in a point cloud with color (i.e., the R G B values a\n  re present for each point in that cloud), the RGB value (0, 0, 0) is reserved to mean \"no color\". An\n  example where this might happen would be if a digital camera did not cover that point. That point then\n  has \"no color\", even though other points may have color.\n\n  Example data file:\n      \n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n  \n\n      - PTX files hold the intensities in a range 0-1\n      - PTS files are -2048 to 2047 as integers (no fractions)\n      - If you need to convert PTS to PTX intensity values you need to add 2048 and divide by 4096.\n\n  See http://w3.leica-geosystems.com/kb/?guid=5532D590-114C-43CD-A55F-FE79E5937CB2\n\n  Example usage:\n      \n\n\n\n\n\n\n\n		 ");
		cl.def( pybind11::init<const std::string &>(), pybind11::arg("file_name") );

		cl.def( pybind11::init( [](easy3d::io::PointCloudIO_ptx const &o){ return new easy3d::io::PointCloudIO_ptx(o); } ) );
		cl.def("load_next", (class easy3d::PointCloud * (easy3d::io::PointCloudIO_ptx::*)()) &easy3d::io::PointCloudIO_ptx::load_next, "Reads a single point cloud from the file.\n\nC++: easy3d::io::PointCloudIO_ptx::load_next() --> class easy3d::PointCloud *", pybind11::return_value_policy::automatic);
		cl.def("assign", (class easy3d::io::PointCloudIO_ptx & (easy3d::io::PointCloudIO_ptx::*)(const class easy3d::io::PointCloudIO_ptx &)) &easy3d::io::PointCloudIO_ptx::operator=, "C++: easy3d::io::PointCloudIO_ptx::operator=(const class easy3d::io::PointCloudIO_ptx &) --> class easy3d::io::PointCloudIO_ptx &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
	{ // easy3d::io::PointCloudIO_vg file:easy3d/fileio/point_cloud_io_vg.h line:93
		pybind11::class_<easy3d::io::PointCloudIO_vg, std::shared_ptr<easy3d::io::PointCloudIO_vg>> cl(m, "PointCloudIO_vg", "Implementation of file input/output operations for vertex group (VG) format PointCloud.\n \n\n\n \n The vertex group (VG) format stores the point cloud and its segmentation information. A segment is\n represented by a set of indices of the points in the segment.\n\n  File format specification\n      \n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n         ");
		cl.def( pybind11::init( [](){ return new easy3d::io::PointCloudIO_vg(); } ) );
		cl.def_static("load_vg", (bool (*)(const std::string &, class easy3d::PointCloud *)) &easy3d::io::PointCloudIO_vg::load_vg, "Reads a point cloud from an ASCII file.\n \n\n The file name.\n \n\n The point cloud to write to.\n \n\n The status of the operation\n      \n\n true if succeeded\n      \n\n false if failed\n\nC++: easy3d::io::PointCloudIO_vg::load_vg(const std::string &, class easy3d::PointCloud *) --> bool", pybind11::arg("file_name"), pybind11::arg("cloud"));
		cl.def_static("save_vg", (bool (*)(const std::string &, const class easy3d::PointCloud *)) &easy3d::io::PointCloudIO_vg::save_vg, "Saves a point_cloud to an ASCII file.\n \n\n The file name.\n \n\n The point cloud.\n \n\n The status of the operation\n      \n\n true if succeeded\n      \n\n false if failed\n\nC++: easy3d::io::PointCloudIO_vg::save_vg(const std::string &, const class easy3d::PointCloud *) --> bool", pybind11::arg("file_name"), pybind11::arg("cloud"));
		cl.def_static("load_bvg", (bool (*)(const std::string &, class easy3d::PointCloud *)) &easy3d::io::PointCloudIO_vg::load_bvg, "Reads a point cloud from a binary file.\n \n\n The file name.\n \n\n The point cloud to write to.\n \n\n The status of the operation\n      \n\n true if succeeded\n      \n\n false if failed\n\nC++: easy3d::io::PointCloudIO_vg::load_bvg(const std::string &, class easy3d::PointCloud *) --> bool", pybind11::arg("file_name"), pybind11::arg("cloud"));
		cl.def_static("save_bvg", (bool (*)(const std::string &, const class easy3d::PointCloud *)) &easy3d::io::PointCloudIO_vg::save_bvg, "Saves a point_cloud to a binary file.\n \n\n The file name.\n \n\n The point cloud.\n \n\n The status of the operation\n      \n\n true if succeeded\n      \n\n false if failed\n\nC++: easy3d::io::PointCloudIO_vg::save_bvg(const std::string &, const class easy3d::PointCloud *) --> bool", pybind11::arg("file_name"), pybind11::arg("cloud"));

//		 { // easy3d::io::PointCloudIO_vg::VertexGroup file:easy3d/fileio/point_cloud_io_vg.h line:137
//		 	auto & enclosing_class = cl;
//		 	pybind11::class_<easy3d::io::PointCloudIO_vg::VertexGroup, std::shared_ptr<easy3d::io::PointCloudIO_vg::VertexGroup>> cl(enclosing_class, "VertexGroup", "");
//		 	cl.def( pybind11::init( [](){ return new easy3d::io::PointCloudIO_vg::VertexGroup(); } ), "doc" );
//		 	cl.def( pybind11::init<int>(), pybind11::arg("type") );
//
//		 	cl.def( pybind11::init( [](easy3d::io::PointCloudIO_vg::VertexGroup const &o){ return new easy3d::io::PointCloudIO_vg::VertexGroup(o); } ) );
//
//		 	pybind11::enum_<easy3d::io::PointCloudIO_vg::VertexGroup::PrimType>(cl, "PrimType", pybind11::arithmetic(), "")
//		 		.value("PLANE", easy3d::io::PointCloudIO_vg::VertexGroup::PLANE)
//		 		.value("SPHERE", easy3d::io::PointCloudIO_vg::VertexGroup::SPHERE)
//		 		.value("CYLINDER", easy3d::io::PointCloudIO_vg::VertexGroup::CYLINDER)
//		 		.value("CONE", easy3d::io::PointCloudIO_vg::VertexGroup::CONE)
//		 		.value("TORUS", easy3d::io::PointCloudIO_vg::VertexGroup::TORUS)
//		 		.value("GENERAL", easy3d::io::PointCloudIO_vg::VertexGroup::GENERAL)
//		 		.value("UNKNOWN", easy3d::io::PointCloudIO_vg::VertexGroup::UNKNOWN)
//		 		.export_values();
//
//		 	cl.def_readwrite("primitive_type_", &easy3d::io::PointCloudIO_vg::VertexGroup::primitive_type_);
//		 	cl.def_readwrite("primitive_index_", &easy3d::io::PointCloudIO_vg::VertexGroup::primitive_index_);
//		 	cl.def_readwrite("label_", &easy3d::io::PointCloudIO_vg::VertexGroup::label_);
//		 	cl.def_readwrite("color_", &easy3d::io::PointCloudIO_vg::VertexGroup::color_);
//		 	cl.def_readwrite("children_", &easy3d::io::PointCloudIO_vg::VertexGroup::children_);
//		 	cl.def("assign", (class easy3d::io::PointCloudIO_vg::VertexGroup & (easy3d::io::PointCloudIO_vg::VertexGroup::*)(const class easy3d::io::PointCloudIO_vg::VertexGroup &)) &easy3d::io::PointCloudIO_vg::VertexGroup::operator=, "C++: easy3d::io::PointCloudIO_vg::VertexGroup::operator=(const class easy3d::io::PointCloudIO_vg::VertexGroup &) --> class easy3d::io::PointCloudIO_vg::VertexGroup &", pybind11::return_value_policy::automatic, pybind11::arg(""));
//		 }

	}
}
