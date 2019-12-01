imgui
================================
A small multiplatform immediate mode graphical user interface with OpenGL3.2 backend.

Imported from: https://github.com/deltaluca/imgui

About
-------------------------
Most of the code is pulled from the recast library : http://code.google.com/p/recastnavigation/

TrueType loading and rendering is done using stb_truetype : http://nothings.org/stb/stb_truetype.h

The OpenGL backend was ported from OpenGL immediate mode to OpenGL 3.2 mainly to be compatible with Mac OS X core profile.

![Alt text](http://adrien.io/img/imgui/imgui.png)

Build
-------------------------
The only depedency is OpenGL.

Usage
----------------------------

Consult [sample.cpp](https://github.com/deltaluca/imgui/blob/master/samples/sample.cpp) for a detailed usage example. (Requires glfw3 and glew)
