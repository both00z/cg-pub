To compile:
make

Usage:
./demo1 <filename>

Yellow square marks the position of a light source
Supported keyboard functions:
	1 (one) - turn shadow on/off (decreases performance)
	w,s,a,d - rotate the teapot around the X and Y axes
	r,t,y,f,g,h - change the light source position (t,f,g,h change the X/Y position, r,y change the Z position)
	i,j,k,l - rotate the entire scene around X and Y axes
	arrow keys - change the camera position in X and Z planes

Additional features implemented:
* texture maps - ppm files supported
* shadows - using shadow volume and stencil buffer
* mirrors - reflection of the teapot and the wall in the water (floor), using stencil buffer

Tutorials at http://nehe.gamedev.net/ were used, but no code was copied directly.
