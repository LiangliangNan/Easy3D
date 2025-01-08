* ToDo list (or on going):
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
    - support Vulkan.
    - Python bindings.



Missing or Invalid Code Signature:
The application or one of its dependencies is not properly code-signed.

Solution: 
1)Re-sign the Application and Libraries
Re-sign the application and all its dependencies using the codesign tool. Ensure that all libraries and frameworks are properly signed.

bash
codesign --force --deep --sign - Mapple.app
--force: Overwrite any existing code signature.
--deep: Recursively sign all nested binaries and libraries.
--sign -: Use an ad-hoc signature (for development). Replace - with your code signing identity for distribution.

2) Verify Code Signatures
Use the codesign tool to verify the code signatures of the application and its dependencies:

bash
codesign --verify --deep --strict Mapple.app
If there are any issues, the tool will provide detailed error messages.
