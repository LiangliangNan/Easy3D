// this is just for experiment/test on-going functions/classes

#include <easy3d/util/logging.h>
#include <easy3d/core/types.h>

#include <thread>


using namespace easy3d;


void MyFunction() {
    LOG(WARNING) << "function [" << __FUNCTION__ << "] executed";
}


int main (int argc, char *argv[])
{
    logging::initialize(argv[0]);

    //------------------------------------------------

    LOG(INFO) << "exe: " << argv[0];

    //------------------------------------------------

    int a = 1;
    int b = 2;
    LOG_IF(WARNING, a < b) << "Warning, a < b";

    //------------------------------------------------

    LOG_IF(ERROR, a < b) << "Error, a < b";

    //------------------------------------------------

    std::thread t([=]() {
        LOG(WARNING) << "Run in another thread";
    });
    std::this_thread::sleep_for(std::chrono::seconds(1));

    //------------------------------------------------

    MyFunction();

    //------------------------------------------------

    LOG(INFO) << "Now test logging STL containers:";
    std::vector<int> x;
    x.push_back(1);
    x.push_back(2);
    x.push_back(3);
    LOG(INFO) << "std::vector<int>: " << x;

    //------------------------------------------------

    std::vector<vec3> points;
    for(int i=0; i<200; ++i)
        points.push_back(vec3(i));
    LOG(INFO) << "std::vector<vec3>: " << points;

    //------------------------------------------------

    LOG(FATAL) << "You should have seen the program crashed - just a test :-)";

    return EXIT_SUCCESS;
}
