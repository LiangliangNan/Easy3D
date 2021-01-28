# CMake configuration file for FFMPEG
if(WIN32 OR MSVC)

    message(WARNING "FFMPEG supported on Windows not implemented yet. Sorry!")

elseif(APPLE)

    set(ffmpeg_path "/usr/local/Cellar/ffmpeg/4.3.1_4")

    set(prefix "${ffmpeg_path}")
    set(exec_prefix "${prefix}")
    set(libdir "${exec_prefix}/lib")
    set(FFMPEG_PREFIX "${ffmpeg_path}")
    set(FFMPEG_EXEC_PREFIX "${ffmpeg_path}")
    set(FFMPEG_LIBDIR "${exec_prefix}/lib")
    set(FFMPEG_INCLUDE_DIRS "${prefix}/include")
    set(FFMPEG_LIBRARIES "-L${FFMPEG_LIBDIR} -lavcodec -lavformat -lpostproc -lavdevice -lavresample -lswresample -lavfilter -lavutil")
    string(STRIP "${FFMPEG_LIBRARIES}" FFMPEG_LIBRARIES)

else()  # Linux

#    sudo apt-get install libavcodec-dev
#    sudo apt-get install libavformat-dev
#    sudo apt-get install libswscale-dev

    set(FFMPEG_LIBDIR "/usr/lib/x86_64-linux-gnu")
    set(FFMPEG_INCLUDE_DIRS "/usr/include/x86_64-linux-gnu")
    set(FFMPEG_LIBRARIES
            "${FFMPEG_LIBDIR}/libavdevice.so.57"
            "${FFMPEG_LIBDIR}/libavresample.so.3"
            "${FFMPEG_LIBDIR}/libavfilter.so.6"
            "${FFMPEG_LIBDIR}/libpostproc.so.54"
            "-L${FFMPEG_LIBDIR} -lavcodec -lavformat -lswresample -lavutil")
    string(STRIP "${FFMPEG_LIBRARIES}" FFMPEG_LIBRARIES)

endif()