# Mapple-Qt

The Qt version of Easy3D's **Mapple** (the old Mapple). Now retired.

## History

(2026-11-16): the ImGui application `applications/Mapple` took over the Mapple
  name; the Qt app became `applications/Mapple-Qt` (target `Mapple-Qt`, OpenGL only, built
  behind the `Easy3D_ENABLE_QT` CMake gate).

## Current status

**Retired.** The ImGui app `applications/Mapple` is the supported Mapple; it runs on both the
OpenGL and Vulkan backends through Easy3D's backend-neutral overlay path. The Qt app is kept
in this tree for reference; the Easy3D build no longer carries it (the `Easy3D_ENABLE_QT` gate is
gone from the top-level and application CMake files), and it can only be built as a standalone
project with Easy3D with the OpenGL backend (`Easy3D_ENABLE_VULKAN=OFF`).

## Future

The retired Qt app is intended to move to its own repository in the future.
