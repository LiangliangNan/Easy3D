import os
import subprocess

# Path to Binder executable
# binder_executable = "/Library/Frameworks/Python.framework/Versions/3.10/bin/binder"
binder_executable = "/Users/lnan/Downloads/binder/cmake-build-release/source/binder"
easy3d_root = "/Users/lnan/Documents/Projects/Easy3D"
output_dir = "./tmp"  # Output directory

# Include directories
include_dirs = [
    "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/c++/v1",
    "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/16/include",
    "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include",
    "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include",
    "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/c++/v1",
    "/Users/lnan/Documents/Dev/CGAL-6.0.1/include",
    "/opt/homebrew/Cellar/boost/1.86.0_2/include",
    easy3d_root,
    easy3d_root + "/cmake-build-release/include",
]

# Create output directory if it doesn't exist
os.makedirs(output_dir, exist_ok=True)

# Construct the Binder command
command = [
    binder_executable,
    "--prefix", output_dir,     # Directory for generated bindings
    "--root-module", "easy3d",  # Name of the root module
    "--bind", "easy3d",         # Bind only the easy3d namespace

    # bindings for STL classes in <pybind11/stl.h> will be used instead of generating custom STL bindings.
    # Note: STL bindings may be overkill and have potential performance implications if data does not need
    #       to be copied between C++ and python. For more information, see pybind11 STL documentation:
    #       https://pybind11.readthedocs.io/en/stable/advanced/cast/stl.html
    "--include-pybind11-stl",

    # if the generated bindings codes are correct but there are some fatal errors from clang and you want to
    # get rid of them. This situation can happen when you would like to generate binding codes for a small
    # part of a huge project and the you cannot include all the required header files with -I to the command.
    # "--suppress-errors",

    # instruct Binder to add extra debug output before binding each type. This might be useful when debugging
    # generated code that produce seg-faults during python import.
    # "--trace",

    "all_includes.hpp",  # Header file
    "--",  # Separator for Binder-specific and compiler flags
    "-std=c++17",   # C++ standard
    "-stdlib=libc++",
    *[f"-I{inc}" for inc in include_dirs],  # Include directories
    "-DNDEBUG",
    "-DGLEW_NO_GLU",
    "-DEasy3D_RESOURCE_DIR=\"/Users/lnan/Documents/Projects/Easy3D/resources\""
]


print(f"Running command: {' '.join(command)}")
subprocess.run(command, check=True)


## Running the above Python code is equivalent to running the following from commandline:
# /Library/Frameworks/Python.framework/Versions/3.10/bin/binder 	\
# --root-module  easy3d  	                \
# --prefix  ./tmp 		                    \
# --bind  easy3d		                    \
# all_headers.hpp		                    \
# -- -std=c++11 		                    \
# -I/Users/lnan/Documents/Projects/Easy3D	\
# -I/usr/include						    \
# -I/usr/local/include					    \
# -I/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/16/include    \
# -I/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include                 \
# -I/System/Library/Frameworks		        \
# -I/Library/Frameworks			            \
# -DNDEBUG