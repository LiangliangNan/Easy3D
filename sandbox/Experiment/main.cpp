#include <easy3d/fileio/resources.h>
#include <easy3d/util/logging.h>
#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/texture.h>
#include <easy3d/core/graph.h>
#include <easy3d/fileio/graph_io.h>
#include <easy3d/core/curve.h>
#include <easy3d/util/string.h>

#include <cstring>
#include <cstdio>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_map>

#define STB_TRUETYPE_IMPLEMENTATION
#include <3rd_party/stb/stb_truetype.h>


using namespace easy3d;

std::string get_question(int x, int y, bool addition) {
    std::ostringstream ss;
    if (x < 10)
        ss << " " << x;
    else
        ss << x;

    if (addition)
        ss << " + ";
    else
        ss << " - ";

    if (y < 10)
        ss << " " << y << " = \t\t";
    else
        ss << y << " = \t\t";
    return ss.str();
}


int main(int argc, char **argv) {
#if 1   // For Amy
    std::string name = "Amy";
    int x_max = 100;
    int y_max = 100;
    int per_line = 4;
#else   // For Jessie
    std::string name = "Jessie";
    int x_max = 20;
    int y_max = 10;
    int per_line = 3;
#endif


    std::vector<std::string> questions;
    for (int x=2; x<x_max; ++x) {
        for (int y=2; y<y_max; ++y) {
            questions.push_back(get_question(x, y, true));
            if (x >= y)
                questions.push_back(get_question(x, y, false));
        }
    }

    std::ofstream output(name + ".txt");
    int count = 0;
    do {
        int idx = rand() % (questions.size());
        output << questions[idx];

        ++count;
        if (count % per_line == 0) {
            output << std::endl << std::endl;
        }
        if (count % (per_line * 5) == 0) {
            output << std::endl;
        }

        auto pos = std::find(questions.begin(), questions.end(), questions[idx]);
        questions.erase(pos);
    } while (!questions.empty());

    return 0;
}
