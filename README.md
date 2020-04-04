# ECS 277 GUI Interface #

This repository provides a software infrastructure for building graphics user interfaces.


## Dependencies ##

In this code, GLFW and GLAD are used for setting up OpenGL. GLM is used as the math implementation. The Dear ImGui library is used for constructing GUI components. LodePNG and RapidJSON are also included for image and file I/O.


## How to Compile ##

The program requires OpenGL 3.3+ with 3D texture support. It also uses CMake for building.

```
mkdir build
cd build
cmake ..
make -j8
```


## Interaction ##

Use left mouse click to rotate 

Use right mouse click to zoom.


## Transfer Function ##

* Left click gray dots to adjust control color/opacity points.

* Right click on empty area to add new control points.

* Check the 'delete point' box and right click on control points to remove them.

* Left click on the colored square to change color map color.
