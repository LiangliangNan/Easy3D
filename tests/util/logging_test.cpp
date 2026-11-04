// SPDX-License-Identifier: GPL-3.0-or-later
#include <tests/test.h>
#include <easy3d/util/logging.h>
#include <thread>
using namespace easy3d;
int main() { return test::run("logging checks and joined worker delivery",[] {
    int value=7;CHECK_EQ(value,7);CHECK_NOTNULL(&value);
    std::thread worker([]{LOG(INFO)<<"bounded worker log";});worker.join();
    LOG_IF(value==7,INFO)<<"conditional log";
    test::require(value==7,"logging changed its input");
}); }
