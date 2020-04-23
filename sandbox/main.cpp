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

#include <easy3d/viewer/viewer.h>
#include <easy3d/fileio/resources.h>
#include <easy3d/util/logging.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/gui/picker_model.h>

using namespace easy3d;


// This example shows how to
//		- override the file loading function of the default easy3d viewer to visualize textured meshes;




int main(int argc, char **argv) {
    // Initialize logging.
    logging::initialize();

    const float w_a1 = 1.0/3.0;
    const float w_a2 = 1.0/3.0;
    const float w_a3 = 1.0/3.0;
    const float w_assignments = 0.4;
    const float w_exam = 0.6;

    struct Student {
        Student(const std::string& _name, float _a1, float _a2, float _a3, float _exam) : name(_name), a1(_a1), a2(_a2), a3(_a3), exam(_exam) {}
        std::string name;
        float a1;
        float a2;
        float a3;
        float exam;
    };

    std::vector<Student> students = {
            Student("Manos Papageorgiou",       80,    80,    85,   68.5),
            Student("Jialun Wu",                75,    70,    85,   69.5),
            Student("Nur An Nisa Milyana",      75,    75,    85,   60.5),

            Student("Ellie Roy",                75,    80,    70,		70.5),
            Student("Constantijn Dinklo",       80,    75,    70,		69),
            Student("Laurens van Rijssel",      80,    80,    65,		56.5),

            Student("Mels Smit",                75,    85,    85,		68),
            Student("Robin Hurkmans",           75,    80,    80,		70.5),
            Student("Mihai-Alexandru Erbașu",   75,    80,    80,		58.5),

            Student("Xiaoai Li",                85,    85,    85,		76.5),
            Student("Zhaiyu Chen",              85,    85,    85,		88),
            Student("Qian Bai",                 85,    85,    85,		73),

            Student("Lisa Keurentjes",          90,    80,    80,		61.5),
            Student("Maarten de Jong",          85,    75,    80,		32.5),
            Student("Vera Stevers",             85,    75,    75,		75),

            Student("Kristof Kenesei",          65,    75,    80,		76),
            Student("Jos Feenstra",             65,    75,    80,		65),
            Student("Max van Schendel",         65,    70,    80,		62),

            Student("Doan Truc Quynh",          85,         60,    75,72),
            Student("Yustisi Ardhitasari Lumban Gaol", 85,  65,    80,69),
            Student("Nadine Hobeika",           85,         65,    80,74),
            
            Student("Camille Morlighem",        80,    70,   70,		75),
            Student("Charalampos Chatzidiakos", 75,    65,   70,		28),
            Student("Rohit Ramlakhan",          80,    65,   70,		71.5)
    };

    for (int i=0; i<students.size(); ++i) {
        if (i%3 == 0)
            std::cout << std::endl;
        const Student& s = students[i];
        float grade = (s.a1 * w_a1 + s.a2 * w_a2 + s.a3 * w_a3) * w_assignments + s.exam * w_exam;
        std::cout << s.name << ": " << grade << std::endl;
    }

    return 0;






    try {
        const std::vector<std::string> files = {
//                resource::directory() + "/data/repair/non_manifold/complex_edges_1.off",
                resource::directory() + "/data/repair/non_manifold/complex_vertices.off",
                resource::directory() + "/data/repair/non_manifold/3_umbrellas.off",
        };

        // Create the viewer.
        Viewer viewer;
        for (const auto& name : files) {
            if (!viewer.add_model(name, true))
                LOG(FATAL) << "Error: failed to load model. Please make sure the file exists and format is correct.";
        }

        // Run the viewer
        viewer.run();
    }
    catch (const std::runtime_error &e) {
        LOG(ERROR) << "caught a fatal error: " + std::string(e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
