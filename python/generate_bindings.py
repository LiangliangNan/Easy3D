import os
import subprocess

# Path to Binder executable
BINDER_EXECUTABLE = "/Library/Frameworks/Python.framework/Versions/3.10/bin/binder"

# Paths and configuration
source_directory = "../easy3d/util"  # Path to your header files
output_dir = "my_bindings"  # Directory for generated bindings
root_module = "util"  # Name of the root module
include_dirs = [  # Include directories
    "..",
    "../easy3d/util",  # Project include directory
    "/usr/include",  # System include directory
    "/usr/local/include",  # Local include directory
    "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/16/include",
    "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include",
    "/System/Library/Frameworks",
    "/Library/Frameworks"
]


# Construct the Binder command
command = [
    BINDER_EXECUTABLE,
    "--root-module", root_module,  # Root module name
    "--prefix", output_dir,  # Output directory
    "--bind", "easy3d",  # Bind all namespaces
    "all_bash_includes.hpp",  # Header file
    "--",  # Separator for Binder-specific and compiler flags
    "c11", "-DNDEBUG",  # C++ standard and other flags
    *[f"-I{inc}" for inc in include_dirs],  # Include directories
]

try:
    print(f"Running command: {' '.join(command)}")
    subprocess.run(command, check=True)
except subprocess.CalledProcessError as e:
    print(f"Error: Command failed")
    print("Suggestions:")
    print("- Ensure all dependencies for this header are installed.")
    print("- Verify the include paths (-I) and compiler flags are correct.")
    print("- Check if the header file includes standard C++ headers correctly.")
    print(e)
    raise




# Create the output directory
os.makedirs(output_dir, exist_ok=True)

# Find header files
headers = get_header_files(source_directory)
if not headers:
    print("No header files found!")
else:
    # Generate bindings
    generate_bindings_for_headers(headers, output_dir, root_module, include_dirs, cpp_flags)

print("Bindings generated successfully!")
