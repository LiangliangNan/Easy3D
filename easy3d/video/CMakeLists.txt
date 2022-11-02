set(module video)
set(dependencies easy3d_util ${FFMPEG_LIBRARIES})

set(${module}_headers
        video_encoder.h
        )

set(${module}_sources
        video_encoder.cpp
        )

add_module(${module} "${${module}_headers}" "${${module}_sources}" "${dependencies}")
target_include_directories(easy3d_${module} PRIVATE ${FFMPEG_INCLUDE_DIRS})
install_module(${module})