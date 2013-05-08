To compile:
make
Usage:
./demo1 -qwenxyzrgbXYZ <filename>
Arguments:
	Rotation:
	-q <float> - rotate the object around the X axis by <float>*PI. For example -q 0.25 will rotate the object PI/4 radians around the X axis
	-w <float> - rotate the object around the Y axis by <float>*PI
	-e <float> - rotate the object around the Z axis by <float>*PI
	The order in which these arguments have been passed to the application matters. They can also be used more than once.

	Specular Intensity:
	-n <int> - sets the specular intensity of the object. Default: 100

	Camera position:
	-x <float> - x coordinate of the camera
	-y <float> - y coordinate of the camera
	-z <float> - z coordinate of the camera

	Light colour:
	-r <int> - value of red. Default: 128
	-g <int> - value of green. Default: 0
	-b <int> - value of blue. Default: 0

	Light source position:
	-X <float> - X coordinate of the light source
	-Y <float> - Y coordinate of the light source
	-Z <float> - Z coordinate of the light source

Additional features implemented:
* Perspective projection
* Changing the viewpoint
* Rotating the object

* Hardcoded support for .obj groups from file magnolia.obj. For best results use:
./demo1 -q 0.33 -w 0.25 magnolia.obj
Phong-shaded image will be different
