set(module video)
set(private_dependencies)
set(public_dependencies easy3d::util ${FFMPEG_LIBRARIES})

set(${module}_headers
        video_encoder.h
        )

set(${module}_sources
        video_encoder.cpp
        )

add_module(${module} "${${module}_headers}" "${${module}_sources}" "${private_dependencies}" "${public_dependencies}")
target_include_directories(easy3d_${module} PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_compile_definitions(easy3d_${module} PUBLIC HAS_FFMPEG)
install_module(${module})