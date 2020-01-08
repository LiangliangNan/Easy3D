#include <easy3d/util/logging.h>

// Add headers for threads
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

static pthread_t newTid;

void *ThreadRunnable(void *arg) {
    (void)arg;
  LOG(WARNING) << "Run in another thread";
  return nullptr;
}

void TestStopWatch() {
  int *ptr = new int[10];
  CHECK_NOTNULL(ptr);
  DLOG(INFO) << "of [" << __func__ << "]";
}

int main() {
  LOG(INFO) << "Dump log test";

  // CHECK Operation
  CHECK_NE(1, 2) << ": The world must be ending!";
  // Check if it is euqual
  CHECK_EQ(std::string("abc")[1], 'b');

  int x = 2;
  int y = 1;
  LOG_IF(ERROR, x > y) << "2 > 1. This should be also OK";

  // Test dump log in different thread
  int err = pthread_create(&newTid, nullptr, ThreadRunnable, nullptr);
  if (err != 0) {
    LOG(FATAL) << "Unable to create a thread";
    return 1;
  }
  sleep(3);
  TestStopWatch();
  return 0;
}
