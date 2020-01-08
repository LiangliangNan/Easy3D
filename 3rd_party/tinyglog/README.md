# miniglog

[![Build Status](https://travis-ci.org/tzutalin/miniglog.png)](https://travis-ci.org/tzutalin/miniglog)

A logging C++ library based on the miniglog library found in ceres-solver.

Let people include and build c++ logging quickly

I update the original miniglog because it won't show pid/tid/timestamp.

### Benefits
* Very small (less than 1000 LOC)
* Easy to use
* No 3rd-party dependencies
* Cross-platform
* Same interface as glog
* Marcos to measure time taken by a function
### Grab the source
```
    $ git clone https://github.com/tzutalin/miniglog.git
```
### Build examples
```
    Need to setup cmake and android ndk first
    $ cd miniglog

    Build for desktop
    $ python build.py

    Build for Android
    $ python build.py --ndk ${ANDROID_NDK} --abi arm64-v8a

    Add --ninja to build with ninja
    $ python build.py --ninja
    $ python build.py --ninja --ndk ${ANDROID_NDK} --abi x86
```

### Usage
- Incllude #include <glog/logging.h> to your files
- LOG Level: FATAL ERROR WARNING INFO

- Logging Macro
```
    LOG(INFO) << "msg";
    LOG(ERROR) << "error msg";
    LOG(FATAL) << "fatal msg";

    // Debug only version
    DLOG(INFO) << "msg";
    DLOG(ERROR) << "error msg";
    DLOG(FATAL) << "fatal msg";
```
- CHECK_OP macros
```
    CHECK(condition)
    CHECK_EQ(val1, val2) CHECK_OP(val1, val2, ==)
    CHECK_NE(val1, val2) CHECK_OP(val1, val2, !=)
    CHECK_LE(val1, val2) CHECK_OP(val1, val2, <=)
    CHECK_LT(val1, val2) CHECK_OP(val1, val2, <)
    CHECK_GE(val1, val2) CHECK_OP(val1, val2, >=)
    CHECK_GT(val1, val2) CHECK_OP(val1, val2, >)
    CHECK_NEAR(val1, val2, margin)

    // Debug only versions of CHECK_OP macros.
    DCHECK(condition)
    DCHECK_EQ(val1, val2)
    DCHECK_NE(val1, val2)
    DCHECK_LE(val1, val2)
    DCHECK_LT(val1, val2)
    DCHECK_GE(val1, val2)
    DCHECK_GT(val1, val2)
    DCHECK_NEAR(val1, val2, margin)
```
- CHECK_NOTNULL macros
```
    CHECK_NOTNULL(val);
    // Debug version only
    DCHECK_NOTNULL(val);
```

### Sample code
```
pthread_t newTid;

void *ThreadRunnable(void *arg) {
  LOG(WARNING) << "Run in another thread";
  return ((void *)0);
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
  int err = pthread_create(&newTid, NULL, ThreadRunnable, NULL);
  if (err != 0) {
    LOG(FATAL) << "Unable to create a thread";
    return 1;
  }
  sleep(1);
  TestStopWatch();
  return 0;
}
```
Output:
![](demo.png)

### Import for Android
You can import it quickly to your Android.mk. Android example and Android.mk, you can refer to [./example/jni](./example/jni)
```
    #MINIGLOG_LIB_TYPE:=SHARED
    MINIGLOG_LIB_TYPE:=STATIC

    ifeq ($(MINIGLOG_LIB_TYPE),SHARED)
        LOCAL_SHARED_LIBRARIES += miniglog
    else
        LOCAL_STATIC_LIBRARIES += miniglog
    endif

    include [Miniglog folder]/Android.mk
```

[More info about the usage of glog](http://rpg.ifi.uzh.ch/docs/glog.html)

