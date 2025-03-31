## Easy3D Python Bindings
Python bindings for the Easy3D library, enabling easy integration of 3D data processing and visualization in Python applications.

For more information about Easy3D, visit:
https://github.com/LiangliangNan/Easy3D

### Obtain/Build Easy3D Python bindings

Prebuilt Python bindings are available for download on the [Releases](https://github.com/LiangliangNan/Easy3D/releases) page. 
Simply Download the appropriate wheel file for your platform and Python version, and install it using `pip`:
```bash
pip install easy3d-2.6.0-py3-none-any.whl  # <-- Use your actual wheel file name
```

#### Build from source
If prebuilt bindings are not available for your platform or specific Python version, you can build the bindings from 
the source code. Here’s how:
- Ensure you have [Python](https://www.python.org/downloads/) installed on your system.
- In your CMake configuration, enable the `Easy3D_BUILD_PYTHON_BINDINGS` option.
- Run CMake to configure the project and then build Easy3D. 
- After building, the Python bindings module, `PyEasy3D`, will be available in `YOUR_BUILD_DIRECTORY/lib/python/easy3d`.

#### Use Easy3D bindings in Python code

Once the Python bindings module is built (or installed), you can use it in your Python code. Here's a simple example:

``` python
import sys
sys.path.append("YOUR_BUILD_DIRECTORY/lib/python")  # <-- Use your actual build path. Not required if the wheel is installed

import easy3d

easy3d.initialize()
viewer = easy3d.Viewer("Test")
viewer.add_model(easy3d.resource_directory() + "/data/bunny.ply")
viewer.run()
```

**Note**: the Python version used for running your code should match the version used for building the bindings.

### (Optional) Installing Easy3D Python bindings

If you prefer not to build Easy3D from source on each machine, you can create a wheel (`.whl`) installer for easier
distribution and installation. This will allow you to install Easy3D like any other Python package (e.g., [Numpy](https://numpy.org/)).

Follow these steps to build and install the wheel file:

- Step 1: Install the `build` tool (if not already installed)

  - `pip install build`

- Step 2: Navigate to the bindings directory and build the wheel

  - `cd YOUR_BUILD_DIRECTORY/lib/python`

  - `python -m build`

- Step 3: Install the wheel

  - `pip install dist/easy3d-2.6.0-py3-none-any.whl` # <-- Use your actual wheel file name

  You can verify the installation by running the following command:

  - `pip show easy3d`