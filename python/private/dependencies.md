To properly install the bindings module, the dependencies must be handled.
Currently, PyEasy3D.pyd (Windows) or PyEasy3D.so (macOS or Linux) depends on:
    - python3xx.dll ("3xx" is Python version; extension is "dylib" on macOS and "so" on Linux)
    - easy3d_algo.dll
    - easy3d_core.dll
    - easy3d_fileio.dll
    - easy3d_kdtree.dll
    - easy3d_renderer.dll
    - easy3d_util.dll
    - easy3d_viewer.dll
If you expose "easy3d_algo_ext" in the bindings, then the dependencies should also include:
    - easy3d_algo_ext.dll
        - libgmp-10.dll
        - libmpfr-4.dll
If you expose "easy3d_video" in the bindings, then the dependencies should also include:
    - easy3d_video.dll
        - avcodec-58.dll
        - avformat-58.dll
        - avutil-56.dll
        - swresample-3.dll
        - swscale-5.dll
