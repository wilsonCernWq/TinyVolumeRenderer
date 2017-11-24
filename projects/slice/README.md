# Slice Based Volume Renderer #

## How you solved this assignment ##

I implemented the sliced based renderer by using two framebuffer objects.

For each iteration:

1. Determine the depth of the current slices we are going to render.

2. Compute vertices of the proxy geometry using a plane-box intersection.

3. Sort the vertices in counter-clockwise order to form a TRIANGLE_FAN

4. Render the triangle fan with one framebuffer as input texture, the other one
as rendering target

5. Inside the shader, the program computes the color of each fragment, and then compose
the rendered color with the accumulated color read from the texture. The composed color
will be written to the second framebuffer obejct

6. Once everything is done, the program will swap the two framebuffers and start a new iteration until we finish all slices.

## What resources you used ##

I wrote my program using GLFW. I implemented the algorithm based on the GPU Gem online.

## Any known bugs in your assignment ##

1. Although I implemented opacity correction, but I noticed that the final color will
still change if I change the number of slices to be rendered. This bug didn't showup
in the second assignment.

2. My renderer can load NEGHIP dataset correctly, but it cannot read CSAFE dataset
for some readons.

## How to Compile ##

```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j8
```

## How to Run ##

```
cd build
./viewer_slice ../data/neghip.json
```

## How to control it ##

Use left mouse click to rotate, use right mouse click to zoom.

## Rendered Images ##

See the assets folder

![alt text](assets/bonsai.png "Bonsai")
![alt text](assets/magnetic_reconnection.png "Magnetic Reconnection")
![alt text](assets/marmoset_neurons.png "Marmoset Neurons")
![alt text](assets/neghip.png "Neghip")