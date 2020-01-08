add_subdirectory(glog)

# Build tutorials
option(EASY3D_BUILD_TINYGLOG_EXAMPLE       "Build tinyglog example"         OFF)

if(EASY3D_BUILD_TINYGLOG_EXAMPLE)
    add_subdirectory(example)
endif()
