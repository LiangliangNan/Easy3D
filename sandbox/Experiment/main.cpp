#if 0

#define ELPP_FEATURE_ALL
#define ELPP_FEATURE_CRASH_LOG
//#define ELPP_EXPERIMENTAL_ASYNC

#include <easy3d/util/easylogging++.h>


INITIALIZE_EASYLOGGINGPP

void myCrashHandler(int sig) {
    LOG(ERROR) << "Woops! Crashed!";
    // FOLLOWING LINE IS OPTIONAL
    el::Helpers::logCrashReason(sig, true);
    // FOLLOWING LINE IS ABSOLUTELY NEEDED AT THE END IN ORDER TO ABORT APPLICATION
    el::Helpers::crashAbort(sig);
}
int main(void) {
    el::Helpers::setCrashHandler(myCrashHandler);

    LOG(INFO) << "My crash handler!";

    std::cout << std::endl << "    ======= Backtrace: =========" << std::endl << el::base::debug::StackTrace();

    int* i;
    *i = 0; // Crash!

    return 0;
}

#else

#include <easy3d/util/stack_tracer.h>
#include <easy3d/util/logging.h>

#include <iostream>

using namespace easy3d;



int print(int * v) {
     std::cout << *v << std::endl;
    LOG_IF(FATAL, v== nullptr) << "v== nullptr";

}

int main(void) {

    LOG(INFO) << "crash test";
    LOG(ERROR) << "error!";


    StackTracer st;

    //print(0);

    LOG(FATAL) << "bad";

    int* i;
    *i = 0; // Crash!

    return 0;
}

#endif