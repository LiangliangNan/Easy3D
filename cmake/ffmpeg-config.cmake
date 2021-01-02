# CMake configuration file for FFMPEG
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