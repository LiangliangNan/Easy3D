#### 22/12/2020
- supports polyhedral meshes.

#### 22/04/2020
  - easier rendering state management (with the new class State). 

#### 22/04/2020
  - easier OpenGL buffer management (buffers of standard model drawabls can be automatically updated when model is modified).
  - reorganize the tutorials (because I am planning to add more tutorials which needs to be better grouped).
  
#### 27/01/2020
  - now loading models from files is done through ManifoldBuilder that resolves non-manifoldness.

#### 20/12/2019
  - now all image io is managed by ImageIO.

#### 19/12/2019
  - fixed the bug where camera gets deadlock.

#### 16/12/2019
  - show/hide mesh vertices;
  - line with control for LinesDrawables (using geometry shader).

#### 14/12/2019
  - put data in the resources directory;
  - added Tutorial_406_Tessellator.

#### 11/12/2019
  - added point imposter (using point sprites, geometry shader);
  - added line imposter (using point sprites, geometry shader).

#### 10/12/2019 
  - simplified the APIs of drawables (i.e., shader management is now taken care of by the drawables, so client code only needs to call the draw() function for rendering);

#### 02/10/2019 
  - added various KD-Tree implementations and API;
  - added point cloud normal estimation and re-orientation;
  - added plane extraction from point clouds using RANSAC;
  - add Poisson surface reconstruction.

#### 19/04/2019
  - Easy3D 2.0 released. Major changes are
    - reorganized the classes/functions in modules (i.e., core, viewer, fileio, util, algo);
    - added a few rendering techniques (e.g., shadow, eye-dome lighting, ambient occlusion, transparency);
    - added a few tutorial examples.

#### 11/12/2018
  - The first release of Easy3D;
  - Tested on macOS, Linux (Ubuntu), and Windows.
