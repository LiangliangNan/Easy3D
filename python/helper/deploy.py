"""
copy_runtime_dependencies.py

This script recursively copies a given shared library (or binary) and its runtime
dependencies into a specified destination directory. It resolves and rewrites
paths as needed to ensure the binary and its dependencies can run standalone
from the destination directory.

Supported platforms:
- macOS: Uses otool and install_name_tool to resolve and fix @rpath dependencies.
- Linux: Uses ldd and patchelf to rewrite rpaths.
- Windows: Uses dumpbin (requires Visual Studio tools) to find DLL dependencies.

Usage:
    python copy_runtime_dependencies.py <target_binary> <destination_directory>

Arguments:
    <target_binary>            Path to the binary or shared library to process.
    <destination_directory>    Directory where the binary and dependencies will be copied.

Note:
- On macOS, this script ensures all libraries are rewritten with `@loader_path`.
- On Linux, the rpath of copied libraries is set to `$ORIGIN`.
- On Windows, it attempts to find dependent DLLs in the PATH or next to the binary.

Dependencies:
    - macOS: install_name_tool, otool
    - Linux: ldd, patchelf
    - Windows: dumpbin (from Visual Studio command line tools)
"""

import os
import sys
import shutil
import subprocess

def get_dependencies(lib_path):
    """Returns a list of dependent shared libraries for a given library."""
    if sys.platform == "darwin":
        # macOS: use otool
        result = subprocess.run(["otool", "-L", lib_path], capture_output=True, text=True)
        lines = result.stdout.split("\n")[1:]  # Skip the first line (library itself)
        dependencies = []
        for line in lines:
            parts = line.strip().split(" ")
            if parts:
                dep_path = parts[0]
                if not dep_path.startswith("/usr/lib") and not dep_path.startswith("/System/"):
                    dependencies.append(dep_path)
        return dependencies

    elif sys.platform.startswith("linux"):
        # Linux: use ldd to list dependencies
        result = subprocess.run(["ldd", lib_path], capture_output=True, text=True)
        dependencies = []
        for line in result.stdout.splitlines():
            # Example line: "\tlibm.so.6 => /lib/x86_64-linux-gnu/libm.so.6 (0x00007f...)"
            if "=>" in line:
                parts = line.strip().split("=>")
                if len(parts) == 2:
                    dep = parts[1].split()[0]
                    # Liangliang: uncomment the line below to exclude system libraries from /lib or /usr/lib if desired
                    # if not (dep.startswith("/lib") or dep.startswith("/usr/lib")):
                    dependencies.append(dep)
        return dependencies

    elif sys.platform.startswith("win"):
        # Windows: use dumpbin to list dependent DLLs (requires Visual Studio tools)
        result = subprocess.run(["dumpbin", "/DEPENDENTS", lib_path], capture_output=True, text=True)
        dependencies = []
        for line in result.stdout.splitlines():
            line = line.strip()
            if line.lower().endswith(".dll"):
                # Exclude system DLLs that are likely in C:\Windows\System32
                if not line.lower().startswith("c:\\windows"):
                    dependencies.append(line)
        return dependencies

    else:
        print("Unsupported platform for dependency resolution.")
        return []


def resolve_rpath(lib_path, dep):
    """Resolves dependency paths for @rpath-like references or similar."""
    if sys.platform == "darwin":
        # macOS: find the real path for an @rpath dependency using otool -l.
        result = subprocess.run(["otool", "-l", lib_path], capture_output=True, text=True)
        lines = result.stdout.split("\n")
        rpaths = []

        # Parse LC_RPATH entries
        for i in range(len(lines)):
            if "LC_RPATH" in lines[i]:
                path_line = lines[i + 2].strip().split("path ")[-1].split(" (offset")[0]
                rpaths.append(path_line)

        # Try to resolve @rpath/libXXX.dylib using known rpaths
        for rpath in rpaths:
            possible_path = os.path.join(rpath, os.path.basename(dep))
            if os.path.exists(possible_path):
                return possible_path

        return None  # Not found

    elif sys.platform.startswith("linux"):
        # Linux: ldd already provides full paths, so return the dependency as-is.
        return dep

    elif sys.platform.startswith("win"):
        # Windows: try to resolve a DLL by looking in the same folder as lib_path.
        lib_dir = os.path.dirname(lib_path)
        possible_path = os.path.join(lib_dir, dep)
        if os.path.exists(possible_path):
            return possible_path
        else: # If not found, check the PATH environment variable.
            # Split PATH into individual directories
            search_paths = os.environ['PATH'].split(os.pathsep)  # List of directories in PATH
            for path in search_paths:
                possible_path = os.path.join(path, dep)
                # Exclude system DLLs that are likely in C:\Windows and C:\Windows\System32
                if (possible_path.lower().startswith("c:\\windows") or
                        possible_path.lower().startswith("c:\\windows\\system32") or
                        "api-ms-" in possible_path.lower() or
                        "vcruntime" in possible_path.lower() or
                        "msvcp" in possible_path.lower()):
                    continue  # Skip this dependency
                if os.path.exists(possible_path):
                    return possible_path
        return None

    else:
        return None


def copy_and_fix_dependencies(source_lib, dest_dir):
    """Recursively copies the library and its dependencies to dest_dir and fixes paths."""
    if sys.platform == "darwin":
        # ========================
        # macOS Implementation
        # ========================
        if not os.path.exists(dest_dir):
            os.makedirs(dest_dir)

        lib_dir = os.path.dirname(source_lib)  # Look for missing @rpath libraries here
        copied_libs = {}
        queue = [source_lib]
        processed_libs = set()  # Track already processed libraries
        copied_lib_paths = set()  # Track already copied libraries
        resolved_deps = set()  # Track already resolved dependencies

        while queue:
            lib_path = queue.pop()
            if lib_path in processed_libs or lib_path in resolved_deps or not os.path.isfile(lib_path):
                continue

            lib_name = os.path.basename(lib_path)
            dest_path = os.path.join(dest_dir, lib_name)

            if not os.path.exists(lib_path):
                # 🔹 Check if missing @rpath libraries exist in the same folder as source_lib
                alternative_path = os.path.join(lib_dir, lib_name)
                if os.path.exists(alternative_path) and os.path.isfile(alternative_path):
                    print(f"🔍 Found missing @rpath library: {lib_path} -> {alternative_path}")
                    lib_path = alternative_path
                else:
                    print(f"⚠️ Warning: {lib_path} not found. Trying to resolve @rpath.")
                    resolved_path = resolve_rpath(source_lib, lib_path)
                    if resolved_path:
                        print(f"✅ Resolved {lib_path} -> {resolved_path}")
                        lib_path = resolved_path
                    else:
                        print(f"❌ Could not resolve {lib_path}. Skipping.")
                        continue

            try:
                shutil.copy2(lib_path, dest_path)
                os.chmod(dest_path, 0o755)  # Ensure writable permissions
                copied_lib_paths.add(lib_path)
            except PermissionError:
                print(f"⚠️ Permission denied when copying {lib_path}. Try running with sudo.")
                continue

            copied_libs[lib_path] = dest_path
            processed_libs.add(lib_path)  # Mark this library as processed
            resolved_deps.add(lib_path)  # Mark this library as resolved

            # 🔹 Get dependencies and add to queue
            dependencies = get_dependencies(lib_path)
            for dep in dependencies:
                if dep.startswith("@rpath"):
                    resolved_dep = resolve_rpath(lib_path, dep)
                    if resolved_dep:
                        print(f"🔄 Resolved @rpath dependency: {dep} -> {resolved_dep}")
                        if resolved_dep not in resolved_deps and resolved_dep not in processed_libs and resolved_dep not in copied_lib_paths:
                            queue.append(resolved_dep)
                    else:
                        print(f"⚠️ Could not resolve @rpath dependency: {dep}")
                else:
                    if dep not in resolved_deps and dep not in processed_libs and dep not in copied_lib_paths:
                        queue.append(dep)

        # 🔹 FIX: Ensure all dependencies are updated to @loader_path
        for original_path, new_path in copied_libs.items():
            new_name = os.path.basename(new_path)
            subprocess.run(["install_name_tool", "-id", f"@loader_path/{new_name}", new_path])

            for dep in get_dependencies(new_path):
                dep_name = os.path.basename(dep)
                if dep in copied_libs:
                    new_dep_path = f"@loader_path/{dep_name}"
                    print(f"🔄 Updating dependency: {new_path} -> {new_dep_path}")
                    subprocess.run(["install_name_tool", "-change", dep, new_dep_path, new_path])
                # 🔹 Ensure @rpath dependencies are replaced
                elif dep.startswith("@rpath"):
                    resolved_dep = resolve_rpath(original_path, dep)
                    if resolved_dep and os.path.exists(resolved_dep) and os.path.isfile(resolved_dep):
                        print(f"🔄 Replacing @rpath: {new_path} -> {resolved_dep}")
                        subprocess.run(["install_name_tool", "-change", dep, f"@loader_path/{os.path.basename(resolved_dep)}", new_path])

        print("✅ All dependencies copied and fixed successfully.")

    elif sys.platform.startswith("linux"):
        # ========================
        # Linux Implementation
        # ========================
        if not os.path.exists(dest_dir):
            os.makedirs(dest_dir)

        lib_dir = os.path.dirname(source_lib)
        copied_libs = {}
        queue = [source_lib]
        processed_libs = set()

        while queue:
            lib_path = queue.pop()
            if lib_path in processed_libs or not os.path.isfile(lib_path):
                continue

            lib_name = os.path.basename(lib_path)
            dest_path = os.path.join(dest_dir, lib_name)

            if not os.path.exists(lib_path):
                # Check if missing libraries exist in the same folder as source_lib
                alternative_path = os.path.join(lib_dir, lib_name)
                if os.path.exists(alternative_path) and os.path.isfile(alternative_path):
                    print(f"Found missing library: {lib_path} -> {alternative_path}")
                    lib_path = alternative_path
                else:
                    print(f"Warning: {lib_path} not found. Skipping.")
                    continue

            try:
                shutil.copy2(lib_path, dest_path)
                os.chmod(dest_path, 0o755)
            except PermissionError:
                print(f"Permission denied when copying {lib_path}.")
                continue

            copied_libs[lib_path] = dest_path
            processed_libs.add(lib_path)

            dependencies = get_dependencies(lib_path)
            for dep in dependencies:
                if dep not in processed_libs:
                    # On Linux, dependencies from ldd should already be full paths.
                    queue.append(dep)

        # Update rpath of each copied library to use $ORIGIN (requires patchelf)
        for original_path, new_path in copied_libs.items():
            subprocess.run(["patchelf", "--set-rpath", "$ORIGIN", new_path])
        print("✅ All dependencies copied and fixed successfully on Linux.")

    elif sys.platform.startswith("win"):
        # ========================
        # Windows Implementation
        # ========================
        if not os.path.exists(dest_dir):
            os.makedirs(dest_dir)

        lib_dir = os.path.dirname(source_lib)
        copied_libs = {}
        queue = [source_lib]
        processed_libs = set()

        while queue:
            lib_path = queue.pop()
            if lib_path in processed_libs or not os.path.isfile(lib_path):
                continue

            lib_name = os.path.basename(lib_path)
            dest_path = os.path.join(dest_dir, lib_name)

            if not os.path.exists(lib_path):
                alternative_path = os.path.join(lib_dir, lib_name)
                if os.path.exists(alternative_path) and os.path.isfile(alternative_path):
                    print(f"Found missing DLL: {lib_path} -> {alternative_path}")
                    lib_path = alternative_path
                else:
                    print(f"Warning: {lib_path} not found. Skipping.")
                    continue

            try:
                shutil.copy2(lib_path, dest_path)
            except PermissionError:
                print(f"Permission denied when copying {lib_path}.")
                continue

            copied_libs[lib_path] = dest_path
            processed_libs.add(lib_path)

            dependencies = get_dependencies(lib_path)
            for dep in dependencies:
                resolved_dep = resolve_rpath(lib_path, dep)
                if resolved_dep and resolved_dep not in processed_libs:
                    queue.append(resolved_dep)

        # On Windows, dependency paths are typically resolved via PATH or the DLL search order.
        print("All dependencies copied successfully on Windows. You may need to add the destination directory to your PATH.")
    else:
        print("Unsupported platform.")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python copy_runtime_dependencies.py <target_binary> <destination_directory>", file=sys.stderr)
        sys.exit(1)

    target_binary = sys.argv[1]
    destination_directory = sys.argv[2]
    copy_and_fix_dependencies(target_binary, destination_directory)
