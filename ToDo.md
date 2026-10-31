### ToDo list (or on going):
- Vulkan support (experimental)
  - [x] Remove "easy3d/graphics" dir (content consolidated into "easy3d/renderer" and "easy3d/viewer")
  - Unify APIs for the classes in "easy3d/renderer", "easy3d/viewer", and "easy3d/gui" for Vulkan and OpenGL. Vulkan and 
    OpenGL related calls should remain in the backends.
    - [x] Unified OffScreen scene/file API and initialization-availability fallback
    - [x] Optional runtime Vulkan loader and missing/invalid-loader OffScreen fallback
    - [x] Shared GLFW lifecycle and surviving OpenGL/Vulkan viewer resources
    - [x] Common interactive frame driving, bounded execution and callback-error contract
    - [x] Runtime factory for built-in viewers with explicit backends and availability fallback
    - [x] Portable input/frame callbacks with logical coordinates and gesture capture
    - [x] Portable projection/unprojection and logical-coordinate visible-surface queries
    - [x] Concrete viewer/offscreen implementations in matching backend directories
    - [ ] Unified Viewer constructors/subclass storage, capture/picking/selection, GUI and native-only extensions
    - [ ] Make the following changes:
      - [] Remove runtime selection of backends. Try to simplify the code and reduce unnecessary functions and files.
      - [] The easy3d/viewer folder contains the implementation of the following classes:
          - OpenGLViewer: renamed from original OpenGL based Viewer (VulkanViewer' counterpart). opengl_viewer.h, opengl_viewer.cpp
          - VulkanViewer: Vulkan-based Viewer's counterpart. vulkan_viewer.h, vulkan_viewer.cpp
          - OpenGLMultiViewer: renamed from original OpenGL based MultiViewer. opengl_multi_viewer.h, opengl_multi_viewer.cpp
          - VulkanMultiViewer: Vulkan-based MultiViewer's counterpart. vulkan_multi_viewer.h, vulkan_multi_viewer.cpp
          - OpenGLOffScreen: renamed from original OpenGL based OffScreen. opengl_offscreen.h, opengl_offscreen.cpp
          - VulkanOffScreen: Vulkan-based OffScreen's counterpart. vulkan_offscreen.h, vulkan_offscreen.cpp
      - For all these classes, try to simplify the code, reduce the number of private and protected functions, avoid introducing new files/functions as much as possible.
        - [] create a viewer.h file, which contains:
              #if defined(EASY3D_NO_OPENGL) || defined(EASY3D_TUTORIAL_VULKAN)
              #define EASY3D_TUTORIAL_USES_VULKAN 1
              #include <easy3d/viewer/vulkan_viewer.h>
              #include <easy3d/viewer/vulkan_multi_viewer.h>
              #include <easy3d/viewer/vulkan_offscreen.h>
              namespace easy3d {
                  using Viewer = VulkanViewer;
                  using MultiViewer = VulkanMultiViewer;
                  using OffScreen = VulkanOffScreen;
              }
              #else
              #define EASY3D_TUTORIAL_USES_VULKAN 0
              namespace easy3d {
                  using Viewer = OpenGLViewer;
                  using MultiViewer = OpenGLMultiViewer;
                  using OffScreen = OpenGLOffScreen;
              }
              #endif
          With these changes, all my original tutorials almost don't need to make many changes (at least their header files).
          Also for tutorials where the viewer need to inherit either OpenGLViewer or VulkanViewer, this can be simply implemented
          using something like "class TutorialXXX : public easy3d::Viewer {...}"
        
- Send the computed value of "MVP * MANIP" to shaders (avoiding computing the matrix product for each vertex/fragment)
- Walkthrough and animation; allow to modify the cameras interactively
- Implement potree for huge point cloud visualization (https://github.com/potree/potree)
- Add a measuring tool
- Add contents and brief info for each model in WidgetModelList.
- Add tutorials for algorithms:
    - Delaunay2D,
    - simplification
    - subdivision
    - smoothing
    - hole filling
    - curvature
    - parameterization
    - CSG operations
- Check if clipper can be used to handle overlapping faces.
- Generate LODs using [meshoptimizer](https://github.com/zeux/meshoptimizer).
- Transparency on macOS with AMD graphics has artifact along the edges (an issue with dFdx/dFdy in the fragment shader). 
  A workaround is to provide a per-face normal (instead of using the normal computed from dFdx/dFdy);
- Previous timer events may interrupt the current one when visualizing pivot points;



### About deployment on mac:
Missing or Invalid Code Signature:
The application or one of its dependencies is not properly code-signed.

Solution: 
- Re-sign the application and all its dependencies using the codesign tool. This will ensure that all libraries and frameworks are properly signed.
    `codesign --force --deep --sign - Mapple.app`
  - `--force`: Overwrite any existing code signature.
  - `--deep`: Recursively sign all nested binaries and libraries.
  - `--sign -`: Use an ad-hoc signature (for development). Replace - with your code signing identity for distribution.
- Use the codesign tool to verify the code signatures of the application and its dependencies:
  `codesign --verify --deep --strict Mapple.app`
  If there are any issues, the tool will provide detailed error messages.
