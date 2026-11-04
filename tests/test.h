// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_TEST_H
#define EASY3D_TEST_H
#include <easy3d/util/initializer.h>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
namespace easy3d { namespace test {
inline void require(bool ok,const char* message) { if(!ok) throw std::runtime_error(message); }
template<class Function> int run(const char* name,Function function) {
    try {
        initialize(false,false,false);std::srand(0);function();
        std::cout<<"PASS: "<<name<<'\n';return EXIT_SUCCESS;
    } catch(const std::exception& e) {
        std::cerr<<"FAIL: "<<name<<": "<<e.what()<<'\n';return EXIT_FAILURE;
    }
}
}}
#endif
