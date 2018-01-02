[teaser]: assets/Teaser.png "Semi-Automatic Classification of Tooth Dataset using 2D Transfer Function"
[figure1]: assets/Figure1.png "An example for configuring the gradient transfer function plugin" 

# Semi-Automatic Transfer Function Generation #

![alt text][teaser]
*Teaser Semi-Automatic Classification of *Tooth* Dataset using 2D Transfer Function*

## Introduction ##

In 1998, Gordon Kindlmann introduced a semi-automatic method to generate 1D or 2D transfer functions for boundary visualization using direct volume rendering. his paper took the advantages of 1st and 2nd order gradients of a volumetric data to classify the position of boundary areas. In this final project, I implemented his idea using OSPRay.
 
## Implementation ##
### 2D Transfer Function in OSPRay ###

I implemented a 2D transfer function widget inside OSPRay. Originally OSPRay considers transfer function only as one of the properties in volume, which means any transfer function must be owned by one or several volumes. During the rendering process, OSPRay queries the emission and absorption value using the sampled scalar value only. This method simplifies the implementation of a 1D transfer function, but limits the developments of multivariate transfer functions because the transfer function cannot read other volumetric information through the existing class abstraction.

To fix the problem, I changed the way of thinking. Instead of attaching a transfer function to any volume, I removed the connections between them. As a result, the renderer need to provide one volume and one sample position to the transfer function each time it needs to calculate the color. It is now the transfer function’s responsibility to sample the volume in a provided position. Of course, a transfer function can choose to sample one or more variables depends on its implementation.

![alt text][figure1]
*Figure 1 An example for configuring the gradient transfer function plugin*

With the help of this modification in core OSPRay, I easily created a gradient transfer function plugin using OSPRay API. This plugin works for all types of volumes exist in current OSPRay. Users can easily configure the new transfer function using OSPRay API calls as well. An example is provided in [Figure 1](#figure1)

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
./viewer_raycast ../data/neghip.json
```

## How to control it ##

Use left mouse click to rotate 

Use right mouse click to zoom.

## Operate ImGui ##

* Left click and drag anywhere on the widget to move the wiget position.

* Double left click the witget top bar to collapse the widget.

* Left click and drag the botton right triangle to resize the widget.

* For a Min/Max range control, left click and drag on any rectangle to change its value. 

* For a slider, left click and drag to slider to change the value.

## Operate 1D transfer function ##

* Left click drag gray dots to adjust control color/opacity points.

* Double right click gray dots to adjust control color/opacity points.

* Double left click on empty area to add new control points.

* Left click on the color squares to change color a color.

## Operate the semi-automatic transfer function widget ##

* Adjust gradient threshold and the sigma value can usually create a good image.

* If the image looks very transparent, try to increase the opacity boost value.

* Use the opacity gamma slider to adjust the gamma correction applied to the actual opacity map.

* Use the display gamma slider to adjust the gamma correction applied to the displayed opacity map (this will not affect the rendering at all).

## Rendered Images ##

See assets folder

![alt text](assets/csafe.png "csafe")

![alt text](assets/foot.png "foot")

![alt text](assets/lobster.png "lobster")

![alt text](assets/silicium.png "silicium")

![alt text](assets/skull.png "skull")
