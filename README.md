# Raycasting Volume Renderer #

## How you solved this assignment ##

I implemented this assignment using one rendering pass.

For each iteration:

1. I use the bounding box of the volume as the geometry and render normally

2. I also pass the camera position in screen coordinate as a uniform constant into the shader

3. In the fragment shader I calculate the ray direction and origin based on the vertex
coordinate and the camera position.

4. I do normal ray-box intersection to find the starting and ending position.

5. I do front-to-back ray composition for each ray using a for loop

6. I write the output color into the screen.

## What resources you used ##

I used GLM and GLFW internally, I used their sample program to setup 3D textures in
OpenGL. I also used the method I learned in ray-tracing class inside this assignment.

## Any known bugs in your assignment ##

My renderer can load [NEGHIP](data/neghip.json) dataset correctly, 
but it cannot read [CSAFE](data/csafe_heptane.json) dataset for some readons.

## How to Compile ##

The program requires OpenGL 4+ with 3D texture support. I don't recommend running it on Mac. I have tested it on Linux (Ubuntu and CentOS 7) machines. [Need a Linux ?](HELPME.md)

```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j8
```

## How to Run ##

```
cd build
./viewer_raycast ../data/neghip.json -sr <sampling-rate>
```

## How to control it ##

Use left mouse click to rotate 

Use right mouse click to zoom.

## how to change the transfer function ##

* Left click gray dots to adjust control color/opacity points.

* Right click on empty area to add new control points.

* Check the 'delete point' box and right click on control points to remove them.

* Left click on the colored square to change color map color.

## Rendered Images ##

See assets folder

![alt text](projects/raycast/assets/neghip.png "Neghip")
