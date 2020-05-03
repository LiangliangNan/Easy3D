# libglutess #

This is a rip of the glu tesselator into a standalone library.  It is a high quality tesselator with an implementation from over 20 years ago.  It is still relevant today.

## Features ##

The features of the tesselator are covered elsewhere.  This documentation simply covers this release.

 * tested on Linux and Mac with GCC
 * tested on Windows with Visual Studio 2012
 * scons-based build system
 * generates a static library 
 * supports debug and release builds on all platforms
 
## Compilation ##
 
     scons arch=[x86,x86-64]  debug=[0,1]
     
The output library's name has `_d` attached if it is a debug build, and `32` or `64` attached on non-fat binary platforms.
 
## Other Changes, Gotchas ##

 * `glu.h` was renamed to `glutess.h` in order to be unambiguous in case you have a `glu.h` already in your include search path.
 
 * Superficial code changes were made to `tessellate.c` in order to provide Visual C compatibility.
 
  * `glutess.h` contains all of the glu functions, including ones that were pruned out.  If you call one, it is a link error.  `gluErrorString` may be one of these.  If you go to the trouble of adding it back in, send a patch!

Michael Labbe
