// this is just for experiment/test on-going functions/classes

#include <easy3d/util/logging.h>
#include <easy3d/util/file_system.h>
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

    LOG_IF(WARNING, 1 < 2) << "Indeed, 1 < 2";

    //------------------------------------------------

    LOG_IF(ERROR, !(1 > 2)) << "Error, 1 < 2";

    //------------------------------------------------

    std::thread t([=]() {
        LOG(WARNING) << "Run in another thread";
    });
    std::this_thread::sleep_for(std::chrono::seconds(1));

    //------------------------------------------------

    MyFunction();

    //------------------------------------------------

    LOG(FATAL) << "You should see that the program crashed: this is just a test";

  return EXIT_SUCCESS;
}
