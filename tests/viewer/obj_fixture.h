// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_TEST_OBJ_FIXTURE_H
#define EASY3D_TEST_OBJ_FIXTURE_H
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/file_system.h>
#include <fstream>
#include <stdexcept>
namespace obj_fixture {
inline void write(const std::string& path,const std::string& data) {
    std::ofstream output(path); output << data;
    if (!output) throw std::runtime_error("could not write OBJ fixture");
}
inline std::string create(const std::string& dir) {
    if (!easy3d::file_system::is_directory(dir) && !easy3d::file_system::create_directory(dir))
        throw std::runtime_error("could not create OBJ fixture directory");
    if (!easy3d::ImageIO::save(dir+"/colors.png",{255,0,0,255,255,0,0,255,0,0,255,255,0,0,255,255},2,2,4,false))
        throw std::runtime_error("could not save OBJ fixture texture");
    write(dir+"/materials.mtl",
        "newmtl textured\nKd 1 0 1\nmap_Ka absent-ambient.png\nmap_Kd colors.png\n"
        "newmtl green\nKd 0 1 0\n"
        "newmtl missing\nKd 1 1 0\nmap_Kd missing.png\n");
    write(dir+"/materials.obj",
        "mtllib materials.mtl\n"
        "v -3 0 0\nv -1 0 0\nv -1 2 0\nv -3 2 0\n"
        "v 1 0 0\nv 3 0 0\nv 3 2 0\nv 1 2 0\n"
        "v -3 -3 0\nv -1 -3 0\nv -1 -1 0\nv -3 -1 0\n"
        "v 1 -3 0\nv 3 -3 0\nv 3 -1 0\nv 1 -1 0\n"
        "v 4 -3 0\nv 7 -3 0\nv 7 2 0\nv 6 2 0\nv 6 -1 0\nv 5 -1 0\nv 5 2 0\nv 4 2 0\n"
        "vt .25 .25\nvt .25 .75\n"
        "usemtl textured\nf 1/1 2/1 3/1\nf 1/2 3/2 4/2\n"
        "g other-group\nusemtl green\nf 5 6 7 8\n"
        "usemtl missing\nf 9 10 11 12\n"
        "usemtl textured\nf 13/1 14 15/1 16/1\n"
        "usemtl green\nf -8 -7 -6 -5 -4 -3 -2 -1\n");
    write(dir+"/plain.obj","v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n");
    write(dir+"/compat.mtl","newmtl ambient\nKd 1 1 1\nmap_Ka colors.png\nnewmtl specular\nKd 1 1 1\nmap_Ks colors.png\n");
    write(dir+"/compat.obj","mtllib compat.mtl\nv 0 0 0\nv 1 0 0\nv 1 1 0\nv 0 1 0\nvt .25 .75\n"
        "usemtl ambient\nf 1/1 2/1 3/1\nusemtl specular\nf 1/1 3/1 4/1\n");
    write(dir+"/invalid.obj","v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 100\n");
    write(dir+"/empty.obj","v 0 0 0\n");
    return dir+"/materials.obj";
}
}
#endif
