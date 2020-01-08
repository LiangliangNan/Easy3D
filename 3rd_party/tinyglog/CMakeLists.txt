# option to build either a shared or a static library. default: shared
option(EASY3D_BUILD_SHARED_GLOG_LIBRARY    "Build glog as a shared library" ON)

# Build tutorials
option(EASY3D_BUILD_GLOG_EXAMPLE            "Build glob example"            OFF)

add_subdirectory(glog)

if(EASY3D_BUILD_GLOG_EXAMPLE)
    add_subdirectory(example)
endif()
