# Little Vulkan Engine

**(Prior version of repo has now been moved to the [tut0-22 branch](https://github.com/blurrypiano/littleVulkanEngine/tree/tut0-22))**

A video tutorial series introducing computer graphics for [Vulkan®](https://www.khronos.org/vulkan/), the new generation graphics and compute API from Khronos. The focus of this tutorial is to be approachable to newcomers to computer graphics and graphics APIs, explaining not just the Vulkan API but also computer graphics theory, mathematics and engine architecture.

## Table of Contents

- [Building](#Building)
  - [Building for Unix](#UnixBuild)
  - [Building for MacOS](#MacOSBuild)
  - [Building for Windows](#WindowsBuild)
- [Tutorials](#Tutorials)
  - [Basics](#Basics)
  - [Point Lights](#PointLights)
- [Official Khronos Vulkan Samples](#Khronossamples)
- [Credits and Attributions](#CreditsAttributions)

## <a name="Building"></a> Building

### <a name="UnixBuild"></a> Unix Build Instructions

- Install the dependencies: cmake, glm, vulkan and glfw

- For example
  ```
    sudo apt install vulkan-tools
    sudo apt install libvulkan-dev
    sudo apt install vulkan-validationlayers-dev spirv-tools
    sudo apt install libglfw3-dev
    sudo apt install libglm-dev
    sudo apt install cmake
  ```
- To Build
  ```
   cd LittleVulkanEngine
   ./unixBuild.sh
  ```

### <a name="MacOSBuild"></a> MacOS Build Instructions

#### Install Dependencies

- [Download and install MacOS Vulkan sdk](https://vulkan.lunarg.com/)
- [Download and install Homebrew](https://brew.sh/)

- Then in a terminal window

  ```
    brew install cmake
    brew install glfw
    brew install glm
  ```

- To Build
  ```
   cd littleVulkanEngine
   ./unixBuild.sh
  ```

### <a name="WindowsBuild"></a> Windows build instructions

- [Download and install Windows Vulkan sdk](https://vulkan.lunarg.com/)
- [Download and install Windows cmake x64 installer](https://cmake.org/download/)
  - Make sure to select "Add cmake to the system Path for all users"
- [Download GLFW](https://www.glfw.org/download.html) (64-bit precompiled binary)
- [Download GLM](https://github.com/g-truc/glm/releases)
- Download and open the project and rename "envWindowsExample.cmake" to ".env.cmake"
- Update the filepath variables in .env.cmake to your installation locations

#### Building for Visual Studio 2019

- In windows powershell

```
 cd littleVulkanEngine
 mkdir build
 cmake -S . -B .\build\
```

- If cmake finished successfully, it will create a LveEngine.sln file in the build directory that can be opened with visual studio. In visual studio right click the Shaders project -> build, to build the shaders. Right click the LveEngine project -> set as startup project. Change from debug to release, and then build and start without debugging.

#### Building for minGW

- [Download and install MinGW-w64](https://www.mingw-w64.org/downloads/), you probably want MingW-W64-builds/
- Make sure MinGW has been added to your Path
- Also set MINGW_PATH variable in the project's .env.cmake
- In windows powershell

```
 cd littleVulkanEngine
 ./mingwBuild.bat
```

- This will build the project to build/LveEngine.exe, double click in file explorer to open and run

## <a name="Tutorials"></a> Tutorials

### [Vulkan Basics - Tutorials 0 to 22](https://github.com/blurrypiano/littleVulkanEngine/tree/tut0-22)

This branch is a legacy branch that holds all the older tutorials in the series, starting from the beginning and going up until tutorial 22.

### <a name="PointLights"></a> Point Lights

Find the [Preliminary Completed Project Here](https://github.com/blurrypiano/littleVulkanEngine/tree/pointLights)

#### [21 - Intro to Point Lights](https://github.com/blurrypiano/littleVulkanEngine/tree/master/littleVulkanEngine/tutorial21)

In this tutorial we add a point light object to the global UBO and update the vertex shader to make use of this new lighting technique. ([Video](https://youtu.be/Z1lLwAEMt4M))

#### [22 - Vertex vs Fragment Lighting](https://github.com/blurrypiano/littleVulkanEngine/tree/master/littleVulkanEngine/tutorial22)

In this tutorial we explore the differences in per-fragment versus per-vertex lighting ([Video](https://youtu.be/YnMyKHfrgU4))

#### [23 - Project Restructure and cmake](https://github.com/blurrypiano/littleVulkanEngine/tree/tut23)

In this tutorial I change the project to use cmake rather than a simple makefile to make building on multiple platforms simpler and more straightforward. ([Video](https://youtu.be/ZuHK_5cJ6B8))

#### [24 - Billboards](https://github.com/blurrypiano/littleVulkanEngine/tree/tut24)

In this tutorial we implement a second rendering system that uses the billboard technique to render a spherical point light.
([Video](https://youtu.be/91-89b3wlSo))

#### [25 - Multiple Point Lights](https://github.com/blurrypiano/littleVulkanEngine/tree/tut25)

In this tutorial we add support for multiple point light objects in the scene. Lights will still be stored in the GlobalUbo, however for rendering the light objects we will use push constants.

([Video](https://youtu.be/1olS6ayckKM))

#### [26 - Specular Lighting](https://github.com/blurrypiano/littleVulkanEngine/tree/tut26)

In this tutorial we add specular lighting to our simple fragment shader.

([Video](https://youtu.be/8CTr0SKQ21U))

#### [27 - Alpha Blending and Transparency](https://github.com/blurrypiano/littleVulkanEngine/tree/tut27)

In this tutorial we add a limited blending capability to our point light system, allowing them to be rendered with a nicer appearance. 

([Video](https://youtu.be/uZqxj6tLDY4))

## <a name="Khronossamples"></a> Official Khronos Vulkan Samples

Khronos made an official Vulkan Samples repository available to the public ([press release](https://www.khronos.org/blog/vulkan-releases-unified-samples-repository?utm_source=Khronos%20Blog&utm_medium=Twitter&utm_campaign=Vulkan%20Repository)).

You can find this repository at https://github.com/KhronosGroup/Vulkan-Samples

## <a name="CreditsAttributions"></a> Credits

Thanks to the authors of these libraries :

- [OpenGL Mathematics (GLM)](https://github.com/g-truc/glm)

Thanks to [LunarG](http://www.lunarg.com)

Thanks to the wonderful opensource examples by [Sascha Willems](https://github.com/SaschaWillems/Vulkan)

Thanks to [ThinMatrix](https://www.youtube.com/user/ThinMatrix/featured) and his wonderful OpenGL game tutorial series which was a huge inspiration for this series and how I first started learning computer graphics

Thanks to [Sean Plott](https://day9.tv/) and the #DK30 challenge, for providing the motivating kick to give this a shot

## Attributions / Licenses

- Vulkan and the Vulkan logo are trademarks of the [Khronos Group Inc.](http://www.khronos.org)
