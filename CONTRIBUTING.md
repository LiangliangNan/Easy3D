# Contribution Guide

## Contents

- [Asking a question](#asking-a-question)
- [Requesting a feature](#requesting-a-feature)
- [Reporting a bug](#reporting-a-bug)
    - [Reporting a runtime bug](#reporting-a-runtime-bug)
    - [Reporting a compile or link bug](#reporting-a-compile-or-link-bug)
    - [Reporting a context creation bug](#reporting-a-context-creation-bug)
    - [Reporting a documentation bug](#reporting-a-documentation-bug)
- [Contributing to Easy3D](#contributing-to-easy3d)
    - [Contributing a bug fix](#contributing-a-bug-fix)
    - [Contributing a feature](#contributing-a-feature)


# Asking a question

Please have a look at the [Easy3D documentation](https://3d.bk.tudelft.nl/liangliang/software/easy3d_doc/html/index.html) and 
quickly go through the issues to see if the same question has already been answered before you ask a question.

Questions about how to use Easy3D, the design or implementation of Easy3D, or about plans should be asked by opening 
an issue with a title starting with "[Question]". For example 
```
  [Question] Does Easy3D support incremental Delaunay triangulation?
```




# Requesting a feature

Please explain why you need the feature and how you intend to use it.  If you
have a specific API design in mind, please add that as well.  If you have or are
planning to write code for the feature, see the [Contributing a feature](#contributing-a-feature) section below.

If there already is a request for the feature you need, add your specific use
case unless it is already mentioned.  If it is, give it a :+1:.





# Reporting a bug

## Reporting a runtime bug

If Easy3D crashes, triggering asserts, or behaves unexpectedly at run-time, please enable logging to file (which is disabled by default) by replacing
the line of Easy3D initialization (in your `main()` function), e.g., 

```c++
    easy3d::initialize();
```
    
with 

```c++
    easy3d::initialize(true);
```

This makes sure that most events will be logged into a log file.
The log file will often tell you the cause of an error or problem. If it doesn't, that might be a separate bug. Please report 
the bug by creating an issue and provide the following:
 - a brief description of the issue;
 - operating system name and version (e.g. `Windows 10 64-bit` or `Ubuntu 18.10`);
 - Easy3D release version (e.g., `2.4.3`) if you are using an official release of Easy3D, Otherwise, the Easy3D commit ID (e.g.  
 `06d1dfe55a5eec2d4b650041cb529f5efdeadd31`) from Git;
 - any error/warning message you have observed;
 - snapshot(s) of the unexpected result (if available);
 - are you running Easy3D from a remote desktop? If so, does the issue occur when running it natively?
 - does the issue occur for both Debug and Release modes or only one of them?
 - does the issue occur for other test data or particular data only? Provide a small data that can trigger the issue;
 - if you made changes to the source code of Easy3D, a short and self-contained code example that can produce the issue. 
 Just put it inline in the body text. If the bug is reproducible with one of the test programs or examples that come with Easy3D, simply mention that instead;
 - the full log file. By default, it is located next to the executable program (unless you have modified the default path);
 - the sequence of operations that triggers the issue. For example 
```
    1: click menu "Surface Mesh -> Polyhedralization"; 
    2: press shortcut key 'O'; 
    3: click on the model using the left button of the mouse.
```

__Please provide as much information as possible__.  Unimportant information can be abbreviated or removed, but missing information can stall bug fixing,
especially when your schedule doesn't align with that of the maintainer.

__Please provide text as text, not as images__.  This includes code, error messages, and any other text.  Text in images cannot be found by other users
searching for the same problem and may have to be re-typed by maintainers when debugging.

You don't need to manually indent your code or other text to quote it with GitHub Markdown; just surround it with triple backticks:

    ```
    Some quoted text.
    ```

You can also add syntax highlighting by appending the common file extension:

    ```c
    int five() {
        return 5;
    }
    ```

## Reporting a compile or link bug

Please report the bug by creating an issue and provide the following:

 - a brief description of the issue;
 - operating system name and version (e.g. `Windows 10 64-bit` or `Ubuntu 18.10`);
 - compiler name and version (e.g., `Visual C++ 2015 Update 2`);
 - Easy3D release version (e.g., `2.4.3`) if you are using an official release of Easy3D. Otherwise, the Easy3D commit ID (e.g.  
 `06d1dfe55a5eec2d4b650041cb529f5efdeadd31`) from Git;
 - a short and self-contained code example that can produce the issue. Just put it inline in the body text. If the bug is
reproducible with one of the test programs or examples that come with Easy3D, simply mention that instead;
 - complete build log from your compiler and linker, even if it's long (it can always be shortened later, if necessary).

   

## Reporting a context creation bug

If Easy3D says that your machine lacks support for OpenGL, it very likely does.
Install drivers from the computer manufacturer or graphics card manufacturer
([Nvidia](https://www.geforce.com/drivers),
[AMD](https://www.amd.com/en/support),
[Intel](https://www-ssl.intel.com/content/www/us/en/support/detect.html)) to
fix this.

If it doesn't help, please report it by creating an issue and provide the following:

 - a brief description of the issue;
 - operating system name and version (e.g. `Windows 10 64-bit` or `Ubuntu 18.10`);
 - GPU and driver version;
 - Easy3D release version (e.g., `2.4.3`) if you are using an official release of Easy3D, Otherwise, the Easy3D commit ID (e.g.  
 `06d1dfe55a5eec2d4b650041cb529f5efdeadd31`) from Git;
 - a short and self-contained code example that can produce the issue. Just put it inline in the body text. If the bug is
reproducible with one of the test programs or examples that come with Easy3D, simply mention that instead;
 - the full log file.




## Reporting a documentation bug

If you found a bug in the documentation (including this file and the ReadMe file of the repository), it's fine to
just link to that web page or mention that source file. Then, simply quote the concerned text and point out what is wrong.



# Contributing to Easy3D

## Contributing a bug fix

Bug fixes are highly appreciated. __There is no preferred patch size__.  A one-character fix is just as welcome as a thousand line one if that is the appropriate size for the fix.

It is recommended to create a pull request for a bug fix. A complete bug fix includes:

- description of the incorrect behavior;
- change of the code;
- credits entries for all authors of the bug fix.


## Contributing a feature

__Note:__ If you haven't already implemented the feature, check first if there
already is an open issue for it and if it's already being developed in a
[development branch](https://github.com/LiangliangNan/Easy3D/branches/all). 

__Note:__ You must have all necessary rights to any code you contribute.  If you
did not write the code yourself, you must explain where it came from and under
what license.  Even code using the same license as Easy3D may not be copied
without attribution.

A complete feature includes:

- the code (i.e., implementation of the new feature);
- a change log including a description of the new feature;
- reference documentation (like other existing features);
- a minimal example;
- cross-references and mentions in relevant places (if needed);
- credits entries for all authors of the new feature.

If the feature requires platform-specific code, at minimum stubs must be added
for the new platform function to all supported and experimental platforms.
