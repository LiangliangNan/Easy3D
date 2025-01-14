"""
Attention: only used/tested on Windows (though supposed to work on Windows, macOS, and Linux).


This Python script is designed to identify and copy the runtime dependencies of a specified binary to a given
destination directory. Its primary purpose is to facilitate the redistribution of binaries by ensuring all necessary
shared libraries (e.g., .dll, .dylib, .so) are included in a self-contained directory. This can be especially useful
when packaging software for deployment, testing in isolated environments, or sharing pre-built binaries with minimal
dependencies.

The script uses platform-specific tools (dumpbin on Windows, otool on macOS, and ldd on Linux) to analyze the
specified binary and identify its runtime dependencies. The script searches for these dependencies in predefined
search paths:
  - On Windows, it searches in the system PATH.
  - On macOS, it uses default system library paths like /System/Library/Frameworks.
  - On Linux, it searches paths defined in the LD_LIBRARY_PATH environment variable.
To avoid copying system libraries or irrelevant files, the script excludes paths and files based on a predefined
list (EXCLUDED_PATHS_AND_FILES). This ensures that only relevant application-level dependencies are included.

Once dependencies are identified and located, the script recursively copies them to the specified destination
directory. This process ensures all transitive dependencies are also resolved.

Limitations:
  - The script relies on tools like dumpbin, otool, and ldd, which must be installed on the target platform.
    If these tools are missing or inaccessible, the script will fail.
  - The exclusion mechanism is based on simple string matching. This may inadvertently skip necessary dependencies
    if their paths contain substrings matching the exclusion list, or include unwanted files if their paths are not
    explicitly excluded.
  - The script is designed to work on Windows, macOS, and Linux, but has only been tested on Windows. There may be
    unaddressed issues or edge cases on other platforms.
  - The script assumes standard runtime dependency resolution methods. It may not work as expected for binaries with
    unusual build configurations or custom dynamic loading mechanisms.
  - While the script attempts to handle errors gracefully, issues like missing dependencies, inaccessible paths, or
    permission errors during copying may still lead to incomplete outputs.


Usage:
    python copy_runtime_dependencies.py <target_binary> <search_path> <destination>
Arguments:
    - target_binary (str): The path to the target binary file whose dependencies need to be copied.
    - search_path (str): A user-specified directory to search for dependencies. This is useful when dependencies
        are located in a known path (e.g., ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}) different from the target binary.
    - destination (str): The directory where the target binary and its dependencies will be copied.
"""


import os
import shutil
import sys
import subprocess
from typing import Set, List

# List of system paths to exclude from the PATH variable
EXCLUDED_PATHS_AND_FILES = [
    "c:\\windows", "api-ms-", "VCRUNTIME", "MSVCP",  # Excluded paths for Windows
    "/usr/lib",      # Common exclusion for Unix-like systems
    "/System",       # macOS system paths
    # Add more paths here as needed
]

def find_dependencies(target_binary: str, platform: str) -> Set[str]:
    """
    Finds runtime dependencies of a given binary using platform-specific tools.
    """
    dependencies = set()
    try:
        if platform == 'win32':  # Windows
            result = subprocess.run(['dumpbin', '/DEPENDENTS', target_binary], capture_output=True, text=True, check=True)
            dependencies.update(line.strip() for line in result.stdout.splitlines() if line.strip().endswith(".dll"))

        elif platform == 'darwin':  # macOS
            result = subprocess.run(['otool', '-L', target_binary], capture_output=True, text=True, check=True)
            dependencies.update(line.split()[0] for line in result.stdout.splitlines()[1:])

        elif platform.startswith('linux'):  # Linux
            result = subprocess.run(['ldd', target_binary], capture_output=True, text=True, check=True)
            dependencies.update(line.split("=>")[1].split("(")[0].strip() for line in result.stdout.splitlines() if "=>" in line)

    except Exception as e:
        print(f"Error finding dependencies for {target_binary}: {e}", file=sys.stderr)

    return dependencies

def find_in_search_paths(filename: str, search_paths: List[str]) -> str:
    """
    Search for a file in the given search paths.
    """
    for directory in search_paths:
        filepath = os.path.join(directory, filename)
        if os.path.exists(filepath):
            return filepath
    return None

def is_excluded_path_or_file(filepath: str, excluded_list: List[str]) -> bool:
    """
    Checks if the given filepath contains any of the strings in the excluded list. The check is case-insensitive.
    """
    filepath_lower = filepath.lower()
    return any(excl.lower() in filepath_lower for excl in excluded_list)

def copy_dependencies(target_binary: str, target_binary_dir: str, destination: str, dependencies_file_path : str, resolved_dependencies: Set[str] = None):
    """
    Recursively copies the target binary and its transitive runtime dependencies to the destination directory,
    avoiding system libraries and excluded paths. Also writes the full paths of dependencies to a file.
    """
    if resolved_dependencies is None:
        resolved_dependencies = set()

    platform = sys.platform
    os.makedirs(destination, exist_ok=True)

    # Add directory of the target binary to search paths
    search_paths = []
    if platform == 'win32':
        search_paths = os.environ['PATH'].split(os.pathsep)
    elif platform == 'darwin':
        search_paths = ['/System/Library/Frameworks']
    elif platform.startswith('linux'):
        search_paths = os.environ.get('LD_LIBRARY_PATH', '').split(os.pathsep)

    # Add the known search path
    search_paths.append(target_binary_dir)

    # Ensure the target binary is processed only once
    if target_binary in resolved_dependencies:
        return
    resolved_dependencies.add(target_binary)

    # Copy the main target binary
    try:
        shutil.copy2(target_binary, destination)
        # Write dependency path to the file
        with open(dependencies_file_path, 'a') as dependencies_file:
            dependencies_file.write(target_binary + '\n')
    except Exception as e:
        print(f"Error copying {target_binary}: {e}", file=sys.stderr)

    # Find and process dependencies
    dependencies = find_dependencies(target_binary, platform)
    for dep in dependencies:
        if dep in resolved_dependencies:
            continue  # Skip already processed dependencies
        dep_path = find_in_search_paths(os.path.basename(dep), search_paths)
        if dep_path:
            if not is_excluded_path_or_file(dep_path, EXCLUDED_PATHS_AND_FILES):
                copy_dependencies(dep_path, target_binary_dir, destination, dependencies_file_path, resolved_dependencies)


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python copy_runtime_dependencies.py <target_binary> <destination>", file=sys.stderr)
        sys.exit(1)

    target_binary = sys.argv[1]
    destination = sys.argv[2]

    # Extract the directory of the target_binary
    target_binary_dir = os.path.dirname(target_binary)

    if not os.path.exists(target_binary):
        print(f"Error: Target binary {target_binary} does not exist.", file=sys.stderr)
        sys.exit(1)

    dependencies_file_path = os.path.join(destination, 'runtime_dependencies.txt')
    open(dependencies_file_path, 'w').close()  # This will clear or create the file

    copy_dependencies(target_binary, target_binary_dir, destination, dependencies_file_path)
