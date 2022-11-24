This is a step by step "How To" for those who have no experience with C/C++ programming to build the software.

NOTE: There are usually many options to set up a development environment on each operating system. What proposed here is not the only option, but I try to minimize your effort. Besides, you may skip some steps if they have been properly set up already.

 
**Step #1. Install a C/C++ compiler and necessary tools.** Some compilers can be installed separately, but some compilers do come along with an IDE (Integrated Development Environment). We install the commonly used IDE if available on your operating system.

 - For Windows user, install [Microsoft Visual Studio](https://visualstudio.microsoft.com/) (NOT Visual Studio Code).
 
 - For Mac users, install XCode from the Apple's App Store.

 - For Linux users, install C/C++ compiler and related tools through a few commands
     - If you are using Fedora, RedHat, CentOS, or Scientific Linux, etc., use the following yum command:
       - `sudo yum groupinstall 'Development Tools'`
       
     - If you are using Debian, Ubuntu, and their variants such as Mint, run the following `apt-get` commands one by one:
       - `sudo apt-get update`
       - `sudo apt-get install build-essential cmake-gui libxmu-dev libxi-dev libgl-dev`
       
     - If you are using FreeBSD, run the following `pkg install` command:
       - `pkg install xorg lang/gcc git cmake gmake bash python perl5`
       
**Step #2. Build the software.** Choose **ONE** of the following (or whatever you are familiar with):

 - Option 1: Use any IDE that can directly handle CMakeLists files to open the `CMakeLists.txt` in the root directory of the software. Then you should have obtained a usable project and just build. I recommend using [CLion](https://www.jetbrains.com/clion/) or [QtCreator](https://www.qt.io/product).

 - Option 2: Use CMake to generate project files for your IDE. Then load the project to your IDE and build. 
  
 - Option 3: Use CMake to generate Makefiles and then `make` (on Linux/macOS) or `nmake`(on Windows with Microsoft Visual Studio).
 
For more details, see [How to Build a CMake-Based Project](https://preshing.com/20170511/how-to-build-a-cmake-based-project/).

