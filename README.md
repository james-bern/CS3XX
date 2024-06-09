Disclaimer
======================
i am going to present just one (relatively) simple setup for doing 3D graphics
- there are others!

Summary of Rasterization
========================
we place some virtual geometry (points, line segments, triangles, ...) in a virtual world 
- let's say a point is defined by one "vertex" (the point itself), a line segment by 2 (the endpoints), and a triangle by 3
we define a virtual camera to obersve the geometry
we project the geometry onto the camera's 2D "film plane"
the film plane is drawn inside your app's window on your real-world computer monitor



Coordinate Systems
==================
there are really only a few coordinate systems we need to do basic 2D and 3D graphics with OpenGL


World coordinates
-----------------
this is where a vertex is "in the world" / "in the scene"
- the y-axis pointing up


Camera-coordinates
------------------
this is where a point is in the coordinate system of the (virtual) camera
- the Camera's y-axis points up
- the Camera's z-axis pointing out the back
- the Camera's x-axis is determined by the right-hand rule
  (z := cross(x, y); => in the picture below, x_Camera is pointing out of the Screen)
- in Camera-coordinates, length is the same as in world coordinates

               y_Camera
  
               ^
               |
  
  z_Camera <- [o]<  ----> Camera is pointing this way


Pixel-coordinates
-----------------
this is where a vertex is inside your app's window (the big rectangle in the picture below)
- the Pixel-coordinates origin is in the top left corner
- the Pixel-coordinates x-axis points to the right
- the Pixel-coordinates y-axis points down
- in Pixel-coordinates, length 1 is the length of one pixel
  note: i abstracted away the macbook retina sub-pixel nonsense and will (hopefully) never think about it again

o-> x_Pixel ----------------------  -
|                                 | ^
v                                 | |
y_Pixel                           | |
                                  |  
|                                 | window_height_Pixel
|                                 |  
|                                 | |
|                                 | |
|                                 | v
 ---------------------------------  -

|<----- window_width_Pixel ------>| 
          

OpenGL-coordinates (aka Normalized Device Coordinates -- NDC)
-------------------------------------------------------------
we have to convert the vertex to this coordinate system for OpenGL to draw it the way we want
- the OpenGL-coorindates origin is in the center of the window
- the OpenGL-coorindates x-axis points to the right
- the OpenGL-coorindates x-axis points to the right
- in OpenGL-coordinates, the window width is length 2 and the window height is length 2
  the upper-right corner is ( 1,  1)
  the lower-left  corner is (-1, -1)
  in OpenGL-coordinates your image is squooshed; this is fine :)

 ---------------------------------. -
|                                 | ^
|                y_OpenGL         | |
|                ^                | |
|                |                |  
|                o--> x_OpenGL    | 2
|                                 |  
|                                 | |
|                                 | |
|                                 | v
 ---------------------------------  -

|<-------------- 2 -------------->| 


PVM
---
Consider
M is the **model matrix** M


Example
-------
when i'm not working in world-coordinates, i try to specify this in the variable name
- foo_Pixel
- foo_Camera
- foo_NDC 
