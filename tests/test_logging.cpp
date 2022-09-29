/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
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
 ********************************************************************/

#include "easy3d/util/logging.h"


#include <thread>
#include <mutex>

// This examples shows how to use the logging functions.

using namespace easy3d;


std::mutex mutex;


struct vec3 {
    vec3(int x_, int y_, int z_) : x(x_), y(y_), z(z_) {}
    int x, y, z;
};

std::ostream& operator<<(std::ostream& os, const vec3& v) {
    os << v.x << " " << v.y << " " << v.z;
    return os;
};

void run_many_threads() {
    for (int i=0; i<8; ++i) {
        std::thread t([=]() {
            std::lock_guard<std::mutex> guard(mutex);
            LOG(WARNING) << "Run in another thread ---------: " << i;
            LOG(WARNING) << "Run in another thread *********: " << i;
        });
        t.detach();
    }
}

void DoNothingFunc() {
    LOG(WARNING) << "function " << __FUNCTION__ << "() executed";
}

void run_conditional_ccasional_logging() {
    for (int i = 0; i < 20; ++i) {
        LOG_N_TIMES(4, INFO) << "Log first 4 INFO, iteration " << i << ", " << COUNTER;
        LOG_N_TIMES(5, ERROR) << "Log first 5 ERROR, iteration " << i << ", " << COUNTER;
    }

    LOG(INFO) << " \n ------------------------ \n";

    for (int i = 0; i < 20; ++i) {
        LOG_EVERY_N(4, WARNING) << "Log every 4 WARNING, iteration " << i << ", " << COUNTER;
        LOG_EVERY_N(5, ERROR) << "Log every 5 ERROR, iteration " << i << ", " << COUNTER;
    }

    LOG(INFO) << " \n ------------------------ \n";

    for (int i = 0; i < 40; ++i) {
        LOG_IF_EVERY_N(5, i < 20, WARNING) << "Log if (i < 10) for every 5, i = " << i << ", " << COUNTER;
        LOG_IF_EVERY_N(5, i >= 20, ERROR) << "Log if (i < 10) for every 5, i = " << i << ", " << COUNTER;
    }
}


int test_logging() {
    // CHECK Operation
    CHECK_NE(1, 2) << ": The world must be ending!";
    // Check if it is euqual
    CHECK_EQ(std::string("abc")[1], 'b');

    int a = 1;
    int b = 2;
    int c = 2;

    DCHECK(b == c) << ": The world must be ending!";
    DCHECK(a != b) << ": The world must be ending!";

    CHECK_EQ(std::string("abc")[1], 'b');

    LOG_IF(a < b, WARNING) << "Warning, a < b";

    //------------------------------------------------

    std::thread t([=]() {
        std::lock_guard<std::mutex> guard(mutex);
        LOG(WARNING) << "Run in another thread";
    });
    t.detach();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    run_many_threads();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // ---------------------------------

    int *ptr = new int[10];
    CHECK_NOTNULL(ptr);
    DLOG(INFO) << "of " << __func__ << "()";
    delete[] ptr;


    //------------------------------------------------

    DoNothingFunc();

    //------------------------------------------------

    LOG(INFO) << "Now test logging STL containers:";
    std::vector<int> x;
    x.push_back(1);
    x.push_back(2);
    x.push_back(3);
    LOG(INFO) << "std::vector<int>: " << x;

    //------------------------------------------------

    std::vector<vec3> points;
    for (int i = 0; i < 3; ++i)
        points.push_back(vec3(i, i, i));
    LOG(INFO) << "std::vector<vec3>: " << points;

    //------------------------------------------------

    run_conditional_ccasional_logging();

    //------------------------------------------------

    LOG(INFO) << "---------- TEST has succeeded!!!!!!!!!!!!!!!!! ----------";
    LOG(FATAL) << "You should have seen the program crashed - just a test :-)";

    return EXIT_SUCCESS;
}
