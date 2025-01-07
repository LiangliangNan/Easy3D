# Dependencies for PyEasy3D Bindings

To properly install and use the `PyEasy3D` bindings module, all dependencies must be handled correctly. Below is a detailed list of dependencies for each platform.

---

## **Core Dependencies**
The `PyEasy3D` module (`PyEasy3D.pyd` on Windows, `PyEasy3D.so` on macOS/Linux) depends on the following libraries:

### **1. Python Runtime**
- **Windows**: `python3xx.dll` (where `3xx` corresponds to the Python version, e.g., `python310.dll` for Python 3.10).
- **macOS**: `libpython3.x.dylib` (where `x` corresponds to the Python version, e.g., `libpython3.10.dylib` for Python 3.10).
- **Linux**: `libpython3.x.so` (where `x` corresponds to the Python version, e.g., `libpython3.10.so` for Python 3.10).

### **2. Easy3D Libraries**
The `PyEasy3D` module depends on the following Easy3D libraries:
- `easy3d_algo.dll` (or `.so`/`.dylib`)
- `easy3d_core.dll` (or `.so`/`.dylib`)
- `easy3d_fileio.dll` (or `.so`/`.dylib`)
- `easy3d_gui.dll` (or `.so`/`.dylib`)
- `easy3d_kdtree.dll` (or `.so`/`.dylib`)
- `easy3d_renderer.dll` (or `.so`/`.dylib`)
- `easy3d_util.dll` (or `.so`/`.dylib`)
- `easy3d_viewer.dll` (or `.so`/`.dylib`)

---

## **Optional Dependencies**
If additional features are exposed in the bindings, the following dependencies must also be included:

### **1. `easy3d_algo_ext` (CGAL Dependencies)**
If the bindings expose `easy3d_algo_ext`, the following dependencies are required:
- `easy3d_algo_ext.dll` (or `.so`/`.dylib`)
    - **CGAL Dependencies**:
        - `libgmp-10.dll` (or `.so`/`.dylib`)
        - `libmpfr-4.dll` (or `.so`/`.dylib`)

### **2. `easy3d_video` (FFMPEG Dependencies)**
If the bindings expose `easy3d_video`, the following dependencies are required:
- `easy3d_video.dll` (or `.so`/`.dylib`)
    - **FFMPEG Dependencies**:
        - `avcodec-58.dll` (or `.so`/`.dylib`)
        - `avformat-58.dll` (or `.so`/`.dylib`)
        - `avutil-56.dll` (or `.so`/`.dylib`)
        - `swresample-3.dll` (or `.so`/`.dylib`)
        - `swscale-5.dll` (or `.so`/`.dylib`)

---

## **Handling Dependencies**
To ensure the `PyEasy3D` module works correctly, follow these steps:

### **1. Place Dependencies in the Same Directory**
- Place all `.dll` (Windows), `.so` (Linux), or `.dylib` (macOS) files in the same directory as the `PyEasy3D` module (`PyEasy3D.pyd`, `PyEasy3D.so`, or `PyEasy3D.dylib`).

### **2. Add Directory to `PATH` (Windows) or `LD_LIBRARY_PATH` (Linux/macOS)**
- **Windows**: Add the directory containing the `.dll` files to the `PATH` environment variable.
  ```python
  import os
  os.environ['PATH'] += os.pathsep + r"F:\3_code\Easy3D-build\lib\release"
  
- **Linux/macOS**: Add the directory containing the `.so` or `.dylib` files to the `LD_LIBRARY_PATH` environment variable.
  ```bash
  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/path/to/dependencies