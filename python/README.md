# Easy3D Python Bindings

Python bindings for the Easy3D library.

More information about Easy3D:
https://github.com/LiangliangNan/Easy3D

### Build Easy3D bindings

Make sure you have [Python](https://www.python.org/downloads/) installed.
Then switch on the CMake option `Easy3D_BUILD_PYTHON_BINDINGS`, run CMake, and then build. After building Easy3D, you
can find the Python bindings module `PyEasy3D` (`PyEasy3D.pyd` on Windows, `PyEasy3D.so` on macOS/Linux) in
`YOUR_BUILD_DIRECTORY/lib/python/easy3d`.

### Use Easy3D bindings in Python code

After building the Python bindings, you should already be able to use the Python bindings module
generated in `YOUR_BUILD_DIRECTORY/lib/python/easy3d`. Here is an example:

``` python
import sys
sys.path.append("YOUR_BUILD_DIRECTORY/lib/python")  # Update this to point to your actual build path

import easy3d

easy3d.initialize()
viewer = easy3d.Viewer("Test")
viewer.add_model(easy3d.directory() + "/data/bunny.ply")
viewer.run()
```

**Note**: the Python version used for running the code should match the version used for building the bindings.

### (Optional) Installation

If you want to avoid specifying the path of the generated bindings in your Python code, you can create a wheel (`.whl`) 
file for the bindings and install it globally. This will make Easy3D work like other Python packages such as [Numpy](https://numpy.org/).

Below are the detailed steps for creating the wheel file and installation.

- Step 1: Install the build Tool (only if not installed)

  - `pip install build`

- Step 2: Build the Wheel

  - `cd YOUR_BUILD_DIRECTORY/lib/python`

  - `python -m build`

- Step 3: Install the Wheel

  - `pip install dist/easy3d-2.6.0-py3-none-any.whl` # Update the wheel file name accordingly

  You can verify the installation by running the following command:

  - `pip show easy3d`



### System Dependencies (typically already installed for almost all users)
Before installing `easy3d`, ensure that the following system libraries are installed:

- **OpenGL**: Required for rendering.
  - On **Ubuntu/Debian**: `sudo apt install libgl1-mesa-dev`
  - On **Windows**: Install the latest graphics drivers from your GPU manufacturer. 
  - On **macOS**: Pre-installed.
  - On **Fedora**: `sudo dnf install mesa-libGL-devel`
