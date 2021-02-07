# CMake configuration file for Easy3D
include(CMakeFindDependencyMacro)
find_dependency(Threads)

set(EASY3D_SRC_DIR      "/Users/lnan/Projects/Easy3D")
set(EASY3D_BUILD_DIR    "/Users/lnan/Projects/Easy3D/cmake-build-debug")

set(EASY3D_3RD_PARTY_DIR  ${EASY3D_SRC_DIR}/3rd_party  ${EASY3D_BUILD_DIR}/3rd_party)

set(EASY3D_PREFIX       ${EASY3D_BUILD_DIR})
set(EASY3D_BINARY_DIR   ${EASY3D_PREFIX}/bin)
set(EASY3D_LIB_DIR      ${EASY3D_PREFIX}/lib)

set(EASY3D_INCLUDE_DIR
        ${EASY3D_SRC_DIR}
        ${EASY3D_BUILD_DIR}
        )

set(EASY3D_3RDPARTY_LIB_DIR    ${EASY3D_LIB_DIR})

set(EASY3D_3RDPARTY_LIBRARIES  -L${EASY3D_LIB_DIR} -lglew -limgui -lLAStools -llibtess -lpoisson_recon-9.0.1 -lRANSAC-1.1 -lrply -ltriangle -ltetgen)

# glfw
if (BUILD_SHARED_LIBS AND UNIX)
    set(EASY3D_3RDPARTY_LIBRARIES ${EASY3D_3RDPARTY_LIBRARIES} -lglfw)
else()
    set(EASY3D_3RDPARTY_LIBRARIES ${EASY3D_3RDPARTY_LIBRARIES} -lglfw3)
endif()


set(ADDITIONAL_LIBRARIES "")
if (APPLE)
    list(APPEND ADDITIONAL_LIBRARIES
            "-framework Cocoa"
            "-framework IOKit"
            "-framework CoreFoundation")
endif ()


set (OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
message(STATUS "OpenGL libraries: ${OPENGL_LIBRARIES}")


set(EASY3D_LIBRARIES    -L${EASY3D_LIB_DIR} -lcore -lfileio -lkdtree -lrenderer -lgui -lalgo -lutil -lviewer)
set(EASY3D_LIBRARIES  ${ADDITIONAL_LIBRARIES} ${OPENGL_LIBRARIES} ${EASY3D_3RDPARTY_LIBRARIES} ${EASY3D_LIBRARIES})
string(STRIP "${EASY3D_LIBRARIES}" EASY3D_LIBRARIES)