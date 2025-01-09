### ToDo list (or on going):
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
    
* Long term:
    - support Vulkan



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
