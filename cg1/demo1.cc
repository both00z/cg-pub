using namespace std ;

#include <stdio.h>
#include <string.h>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include "demo1.h"

#include<GL/gl.h>
#include<GL/glx.h>
#include<GL/glu.h>
#include <GL/glut.h>

#define PI 3.14159265
#define nCols 640
#define nRows 480

//array of pixels
RGBVector pixels[nRows][nCols];
//z-buffer
float zBuffer[nRows][nCols];
//focal length
float eZ = 500;
//position of light source
Vector3f lightPos = Vector3f(-55,0,80);
//light intensity
float lightIntensity = 1;
//camera position
Vector3f camV = Vector3f(0,0,100);
//specular intensity
int specInt = 100;
//colors
int red = 128;
int green = 0;
int blue = 0;

TriangleMesh trig;

void TriangleMesh::loadFile(char * filename)
{
	ifstream f(filename);


	if (f == NULL) {
		cerr << "failed reading polygon data file " << filename << endl;
		exit(1);
	}

	char buf[1024];
	char header[100];
	float x,y,z;
	float xmax,ymax,zmax,xmin,ymin,zmin;
	int v1, v2, v3, n1, n2, n3;
	int color = 0;

	xmax =-10000; ymax =-10000; zmax =-10000;
	xmin =10000; ymin =10000; zmin =10000;
	Vector3f av;
	av[0] = av[1] = av[2] = 0.f;

	char a[1024];

	while (!f.eof()) {
		    f.getline(buf, sizeof(buf));
		    sscanf(buf, "%s", header);  

		    if (strcmp(header, "v") == 0) {
				sscanf(buf, "%s %f %f %f", header, &x, &y, &z);

			//	x *= 1000; y *= 1000; z *= 1000;

				_v.push_back(Vector3f(x,y,z));


				av[0] += x; av[1] += y; av[2] += z;

				if (x > xmax) xmax = x;
				if (y > ymax) ymax = y;
				if (z > zmax) zmax = z;

				if (x < xmin) xmin = x;
				if (y < ymin) ymin = y;
				if (z < zmin) zmin = z;
		    }
		    else if (strcmp(header, "f") == 0) {
				sscanf(buf, "%s %d %d %d", header, &v1, &v2, &v3);

				Triangle trig(v1-1, v2-1, v3-1);
				trig.set_color(color);

				_trig.push_back(trig);

		    }
		    else if (strcmp(header, "g") == 0) {
		    	sscanf(buf, "%s %s", header, a);
		    	//cout << a << " " << strncmp(a, "lf", 2)  << " " << strncmp(a, "fr", 2) << " ";
		    	if (strcmp(a, "bud2")==0)
				color = 1;
		    	else if (strncmp(a, "lf", 2)==0)
		    		color = 2;
		    	else if (strncmp(a, "fr", 2)==0)
		    		color = 3;
		    	else color = 0;
			//cout << color << "\n";
		    }
 	}

	_xmin = xmin; _ymin = ymin; _zmin = zmin;
	_xmax = xmax; _ymax = ymax; _zmax = zmax;

	float range; 
	if (xmax-xmin > ymax-ymin) range = xmax-xmin;
	else range = ymax-ymin;

	for (int j = 0; j < 3; j++) av[j] /= _v.size();

	for (int i = 0; i < _v.size(); i++) 
	{
		for (int j = 0; j < 3; j++) _v[i][j] = (_v[i][j]-av[j])/range*400;  
	}
	cout << "trig " << _trig.size() << " vertices " << _v.size() << endl;
	f.close();
};

float calculateF(float v1x, float v1y, float v2x, float v2y, float x, float y)
{
	return (v1y-v2y)*x+(v2x-v1x)*y+v1x*v2y-v2x*v1y;
}

float dotProduct(Vector3f v1, Vector3f v2)
{
	return v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2];
}

void resetZBuffer()
{
	for (int x2d = 0; x2d < nRows; x2d++)
		for (int y2d = 0; y2d < nCols; y2d++)
			zBuffer[x2d][y2d] = -1000;
}

void savePicture(char* filename)
{
	ofstream imageStream(filename, ios::out);
	imageStream << "P3\n" << nCols << " " << nRows << "\n" << "255\n";
	for (int x2d = 0; x2d < nRows; x2d++)
	{
		for (int y2d = 0; y2d < nCols; y2d++)
		{
			imageStream << pixels[x2d][y2d].red() << " " << pixels[x2d][y2d].green() << " " << pixels[x2d][y2d].blue() << " ";
			pixels[x2d][y2d].set_color(0,0,0);
		}
		imageStream << "\n";
	}
}

void saveOutline()
{
	for (int i = 0; i<trig.trigNum(); i++)
	{
		Vector3f v0;
		Vector3f v1;
		Vector3f v2;
				trig.getTriangleVertices(i, v0, v1, v2);

		//projection and selection of the bounding box
		float v0x = (v0[0] / (-v0[2] / eZ + 1) + nCols / 2);
		float maxX = v0x;
		float minX = v0x;
		float v0y = (v0[1] / (-v0[2] / eZ + 1) + nRows / 2);
		float maxY = v0y;
		float minY = v0y;

		float v1x = (v1[0] / (-v1[2] / eZ + 1) + nCols / 2);
		if (v1x > maxX)
			maxX = v1x;
		if (v1x < minX)
			minX = v1x;
		float v1y = (v1[1] / (-v1[2] / eZ + 1) + nRows / 2);
		if (v1y > maxY)
			maxY = v1y;
		if (v1y < minY)
			minY = v1y;

		float v2x = (v2[0] / (-v2[2] / eZ + 1) + nCols / 2);
		if (v2x > maxX)
			maxX = v2x;
		if (v2x < minX)
			minX = v2x;
		float v2y = (v2[1] / (-v2[2] / eZ + 1) + nRows / 2);
		if (v2y > maxY)
			maxY = v2y;
		if (v2y < minY)
			minY = v2y;

		//colouring the triangles
		for (int x = minX; x < maxX; x++)
			for (int y = minY; y < maxY; y++) {

				float alpha = calculateF(v1x, v1y, v2x, v2y, x, y)
						/ calculateF(v1x, v1y, v2x, v2y, v0x, v0y);
				float beta = calculateF(v2x, v2y, v0x, v0y, x, y) / calculateF(
						v2x, v2y, v0x, v0y, v1x, v1y);
				float gamma = calculateF(v0x, v0y, v1x, v1y, x, y)
						/ calculateF(v0x, v0y, v1x, v1y, v2x, v2y);
				if (alpha > 0 && alpha < 1 && beta > 0 && beta < 1 && gamma > 0
						&& gamma < 1 && x >= 0 && y >= 0 && x <= nCols && y
						<= nRows)
				{
					float pointDepth = (alpha * v0[2] + beta * v1[2] + gamma * v2[2]);
					if (pointDepth > zBuffer[y][x]) {
						pixels[y][x].set_color(red, green, blue);
						zBuffer[y][x] = pointDepth;
					}
				}
			}
	}
	cout << "saving picture of an object outline\n";
	savePicture("outline.ppm");
}

void saveFlat()
{
	for (int i = 0; i < trig.trigNum(); i++) {
		int r = 255;
		int g = 255;
		int b = 255;

		Vector3f v0;
		Vector3f v1;
		Vector3f v2;
		trig.getTriangleVertices(i, v0, v1, v2);

		//calculating the normals
		Vector3f v10 = v1 - v0;
		Vector3f v20 = v2 - v0;
		Vector3f faceNormal = Vector3f(v10[1] * v20[2] - v20[1] * v10[2],
				v10[2] * v20[0] - v20[2] * v10[0], v10[0] * v20[1] - v20[0]
						* v10[1]);
		faceNormal.normalise();

		Vector3f trigCentre = Vector3f((v0[0] + v1[0] + v2[0]) / 3, (v0[1]
				+ v1[1] + v2[1]) / 3, (v0[2] + v1[2] + v2[2]) / 3);

		Vector3f lightVector = Vector3f(lightPos[0] - trigCentre[0],
				lightPos[1] - trigCentre[1], lightPos[2] - trigCentre[2]);
		lightVector.normalise();

		float shade = dotProduct(faceNormal, lightVector);
		if (shade >= 0) {
			r = shade * red;
			g = shade * green;
			b = shade * blue;
		} else {
			r = 0;
			g = 0;
			b = 0;
		}

		//projection and selection of the bounding box
		float v0x = (v0[0] / (-v0[2] / eZ + 1) + nCols / 2);
		float maxX = v0x;
		float minX = v0x;
		float v0y = (v0[1] / (-v0[2] / eZ + 1) + nRows / 2);
		float maxY = v0y;
		float minY = v0y;

		float v1x = (v1[0] / (-v1[2] / eZ + 1) + nCols / 2);
		if (v1x > maxX)
			maxX = v1x;
		if (v1x < minX)
			minX = v1x;
		float v1y = (v1[1] / (-v1[2] / eZ + 1) + nRows / 2);
		if (v1y > maxY)
			maxY = v1y;
		if (v1y < minY)
			minY = v1y;

		float v2x = (v2[0] / (-v2[2] / eZ + 1) + nCols / 2);
		if (v2x > maxX)
			maxX = v2x;
		if (v2x < minX)
			minX = v2x;
		float v2y = (v2[1] / (-v2[2] / eZ + 1) + nRows / 2);
		if (v2y > maxY)
			maxY = v2y;
		if (v2y < minY)
			minY = v2y;

		//colouring the triangles
		for (int x = minX; x < maxX; x++)
			for (int y = minY; y < maxY; y++) {
				float alpha = calculateF(v1x, v1y, v2x, v2y, x, y)
						/ calculateF(v1x, v1y, v2x, v2y, v0x, v0y);
				float beta = calculateF(v2x, v2y, v0x, v0y, x, y) / calculateF(
						v2x, v2y, v0x, v0y, v1x, v1y);
				float gamma = calculateF(v0x, v0y, v1x, v1y, x, y)
						/ calculateF(v0x, v0y, v1x, v1y, v2x, v2y);
				if (alpha >= 0 && alpha <= 1
					&& beta >= 0 && beta <= 1
					&& gamma >= 0 && gamma <= 1
					&& x >= 0 && y >= 0 && x <= nCols && y <= nRows)
				{
					float pointDepth = (alpha * v0[2] + beta * v1[2] + gamma * v2[2]);
					if (pointDepth > zBuffer[y][x]) {
						pixels[y][x].set_color(r, g, b);
						zBuffer[y][x] = pointDepth;
					}
				}
			}
	}
	cout << "saving picture of a flat-shaded object\n";
	savePicture("flat.ppm");
}

void saveGouraud()
{
	for (int i = 0; i < trig.trigNum(); i++) {
		int r = 255;
		int g = 255;
		int b = 255;
		Vector3f v[3];
		Vector3f vn[3];
		Vector3f vl[3];
		RGBVector vc[3];
		trig.getTriangleVertices(i, v[0], v[1], v[2]);
		trig.getVerticesNormals(i, vn[0], vn[1], vn[2]);
		//cout << vn[0] << " " << vn[1] << " " << vn[2] << "\n";

		//calculate the color at each of the vertices
		for (int j = 0; j<3; j++)
		{
			//vector from the light source to the vertex
			vl[j][0] = lightPos[0] - v[j][0];
			vl[j][1] = lightPos[1] - v[j][1];
			vl[j][2] = lightPos[2] - v[j][2];
			vl[j].normalise();

			//reflection vector
			float lDotN = 2 * dotProduct(vl[j], vn[j]);
			Vector3f dotNorm = Vector3f(lDotN * vn[j][0], lDotN
					* vn[j][1], lDotN * vn[j][2]);
			Vector3f reflection = vl[j] - dotNorm;
			reflection.normalise();

			//vector from the vertex to the camera
			Vector3f pointToCamera = camV - v[j];
			pointToCamera.normalise();

			float shade = dotProduct(vn[j], vl[j])+pow(dotProduct(reflection,pointToCamera),specInt);

			/*if (shade>1) {
				vc[j]=RGBVector(255,255,255);
			} else*/ if (shade >= 0) {
				vc[j]=RGBVector(int(shade*red),int(shade*green),int(shade*blue));
			} else {
				vc[j]=RGBVector(0,0,0);
			}
		}
		//cout << vc[0].red() << " " << vc[1].red() << " " << vc[2].red() << "\n";

		//projection and selection of the bounding box
		float v0x = (v[0][0] / (-v[0][2] / eZ + 1) + nCols / 2);
		float maxX = v0x;
		float minX = v0x;
		float v0y = (v[0][1] / (-v[0][2] / eZ + 1) + nRows / 2);
		float maxY = v0y;
		float minY = v0y;

		float v1x = (v[1][0] / (-v[1][2] / eZ + 1) + nCols / 2);
		if (v1x > maxX)
			maxX = v1x;
		if (v1x < minX)
			minX = v1x;
		float v1y = (v[1][1] / (-v[1][2] / eZ + 1) + nRows / 2);
		if (v1y > maxY)
			maxY = v1y;
		if (v1y < minY)
			minY = v1y;

		float v2x = (v[2][0] / (-v[2][2] / eZ + 1) + nCols / 2);
		if (v2x > maxX)
			maxX = v2x;
		if (v2x < minX)
			minX = v2x;
		float v2y = (v[2][1] / (-v[2][2] / eZ + 1) + nRows / 2);
		if (v2y > maxY)
			maxY = v2y;
		if (v2y < minY)
			minY = v2y;

		//colouring the triangles
		for (int x = minX; x < maxX; x++)
			for (int y = minY; y < maxY; y++) {
				float alpha = calculateF(v1x, v1y, v2x, v2y, x, y)
						/ calculateF(v1x, v1y, v2x, v2y, v0x, v0y);
				float beta = calculateF(v2x, v2y, v0x, v0y, x, y) / calculateF(
						v2x, v2y, v0x, v0y, v1x, v1y);
				float gamma = calculateF(v0x, v0y, v1x, v1y, x, y)
						/ calculateF(v0x, v0y, v1x, v1y, v2x, v2y);
				if (alpha >= 0 && alpha <= 1
						&& beta >= 0 && beta <= 1
						&& gamma >= 0 && gamma <= 1
						&& x >= 0 && y >= 0 && x <= nCols && y <= nRows)
				{
					float pointDepth = (alpha * v[0][2] + beta * v[1][2] + gamma * v[2][2]);
					if (pointDepth > zBuffer[y][x]) {
						r = (int)(alpha * vc[0].red() + beta*vc[1].red() + gamma*vc[2].red());
						g = (int)(alpha * vc[0].green() + beta*vc[1].green() + gamma*vc[2].green());
						b = (int)(alpha * vc[0].blue() + beta*vc[1].blue() + gamma*vc[2].blue());
						pixels[y][x].set_color(r, g, b);
						zBuffer[y][x] = pointDepth;
					}
				}
			}
	}
	cout << "saving picture of a Gouraud-shaded object\n";
	savePicture("gouraud.ppm");
}

void savePhong()
{
	//trig.calculateVerticesNormals();
	for (int i = 0; i < trig.trigNum(); i++) {
		Vector3f v[3];
		Vector3f vn[3];

		trig.getTriangleVertices(i, v[0], v[1], v[2]);
		trig.getVerticesNormals(i, vn[0], vn[1], vn[2]);

		//projection and selection of the bounding box
		float v0x = (v[0][0] / (-v[0][2] / eZ + 1) + nCols / 2);
		float maxX = v0x;
		float minX = v0x;
		float v0y = (v[0][1] / (-v[0][2] / eZ + 1) + nRows / 2);
		float maxY = v0y;
		float minY = v0y;

		float v1x = (v[1][0] / (-v[1][2] / eZ + 1) + nCols / 2);
		if (v1x > maxX)
			maxX = v1x;
		if (v1x < minX)
			minX = v1x;
		float v1y = (v[1][1] / (-v[1][2] / eZ + 1) + nRows / 2);
		if (v1y > maxY)
			maxY = v1y;
		if (v1y < minY)
			minY = v1y;

		float v2x = (v[2][0] / (-v[2][2] / eZ + 1) + nCols / 2);
		if (v2x > maxX)
			maxX = v2x;
		if (v2x < minX)
			minX = v2x;
		float v2y = (v[2][1] / (-v[2][2] / eZ + 1) + nRows / 2);
		if (v2y > maxY)
			maxY = v2y;
		if (v2y < minY)
			minY = v2y;

		int color = trig.getTriangleColor(i);

		//colouring the triangles
		for (int x = minX; x < maxX; x++)
			for (int y = minY; y < maxY; y++) {
				float alpha = calculateF(v1x, v1y, v2x, v2y, x, y)
						/ calculateF(v1x, v1y, v2x, v2y, v0x, v0y);
				float beta = calculateF(v2x, v2y, v0x, v0y, x, y) / calculateF(
						v2x, v2y, v0x, v0y, v1x, v1y);
				float gamma = calculateF(v0x, v0y, v1x, v1y, x, y)
						/ calculateF(v0x, v0y, v1x, v1y, v2x, v2y);
				if (alpha >= 0 && alpha <= 1
						&& beta >= 0 && beta <= 1
						&& gamma >= 0 && gamma <= 1
						&& x >= 0 && y >= 0 && x <= nCols && y <= nRows)
				{
					if ((alpha * v[0][2] + beta * v[1][2] + gamma * v[2][2])
							> zBuffer[y][x]) {
						//interpolate a normal of the point
						Vector3f pointNormal = Vector3f(
								alpha * vn[0][0] + beta*vn[1][0] + gamma*vn[2][0],
								alpha * vn[0][1] + beta*vn[1][1] + gamma*vn[2][1],
								alpha * vn[0][2] + beta*vn[1][2] + gamma*vn[2][2]
							);

						//vector from light source to the point
						Vector3f lightVector = lightPos-pointNormal;
						lightVector.normalise();

						//reflection vector
						float lDotN = 2 * dotProduct(lightVector, pointNormal);
						Vector3f dotNorm =  Vector3f(lDotN*pointNormal[0],
								lDotN*pointNormal[1],
								lDotN*pointNormal[2]);
						Vector3f reflection = lightVector - dotNorm;
						reflection.normalise();

						//position of the point
						Vector3f pointVec = Vector3f(
								alpha * v[0][0] + beta*v[1][0] + gamma*v[2][0],
								alpha * v[0][1] + beta*v[1][1] + gamma*v[2][1],
								alpha * v[0][2] + beta*v[1][2] + gamma*v[2][2]
							);
						//vector from point to the camera
						Vector3f pointToCamera = camV - pointVec;
						pointToCamera.normalise();

						float shade = lightIntensity*dotProduct(pointNormal, lightVector);
						//if (shade>=0)
						//	shade+=lightIntensity*pow(dotProduct(reflection,pointToCamera),specInt);
						/*if (shade > 1) {
							pixels[y][x].set_color(255, 255, 255);
						}
						else*/ if (shade >= 0)
						{
							int r, g, b;
							if (color == 0)
							{
								r = shade*red;
								g = shade*green;
								b = shade*blue;
							}
							else if (color == 1)
							{
								r = shade * 236;
								g = shade * 216;
								b = shade * 20;
							}
							else if (color == 2) {
								r = shade * 0;
								g = shade * 108;
								b = shade * 0;
							}
							else if (color == 3) {
								r = shade * 156;
								g = shade * 56;
								b = shade * 8;
							}
							pixels[y][x].set_color(r, g, b);
						}
						else
							pixels[y][x].set_color(0, 0, 0);
						zBuffer[y][x] = alpha * v[0][2] + beta * v[1][2] + gamma
								* v[2][2];
					}
				}
			}
	}
	cout << "saving picture of a Phong-shaded object\n";
	savePicture("phong.ppm");
}


int main(int argc, char **argv)
{

	if (argc >  1)  {
		char* fliename = argv[argc - 1];
		trig.loadFile(fliename);
		vector<Vector3f> &vertices = trig.getVertices();
		for (int i = 0; i < vertices.size(); i++)
		{
			//vertices[i]-=camV;
			vertices[i].rotateZ(PI);
		}
		for (int i = 1; i < argc-1; i++)
		{
			if (argv[i][0]=='-')
				if (argv[i][1]=='q')
					for (int j = 0; j < vertices.size(); j++)
						vertices[j].rotateX(atof(argv[i+1])*PI);
				else if (argv[i][1]=='w')
					for (int j = 0; j < vertices.size(); j++)
						vertices[j].rotateY(atof(argv[i+1])*PI);
				else if (argv[i][1]=='e')
					for (int j = 0; j < vertices.size(); j++)
						vertices[j].rotateZ(atof(argv[i+1])*PI);
				else if (argv[i][1]=='n')
					specInt = atoi(argv[i+1]);
				else if (argv[i][1]=='x')
					camV[0] = atof(argv[i+1]);
				else if (argv[i][1]=='y')
					camV[1] = atof(argv[i+1]);
				else if (argv[i][1]=='z')
					camV[2] = atof(argv[i+1]);
				else if (argv[i][1] == 'r')
					red = atoi(argv[i + 1]);
				else if (argv[i][1] == 'g')
					green = atoi(argv[i + 1]);
				else if (argv[i][1] == 'b')
					blue = atoi(argv[i + 1]);
				else if (argv[i][1] == 'X')
					lightPos[0] = atof(argv[i + 1]);
				else if (argv[i][1] == 'Y')
					lightPos[1] = atof(argv[i + 1]);
				else if (argv[i][1] == 'Z')
					lightPos[2] = atof(argv[i + 1]);
		}
		for (int i = 0; i < vertices.size(); i++) vertices[i]-=camV;

		for (int x2d = 0; x2d < nRows; x2d++)
		{
			for (int y2d = 0; y2d < nCols; y2d++)
			{
				pixels[x2d][y2d] = RGBVector(0,0,0);
				zBuffer[x2d][y2d] = -1000;
			}
		}

		//rotation vectors
		//float thetaX = 0;//-PI/4;
		//float thetaY = 0;
		//float thetaZ = PI;

		//translate vertices
		for (int i = 0; i < vertices.size(); i++)
		{
			//vertices[i]-=camV;
			//vertices[i].rotateX(thetaX);
			//vertices[i].rotateY(thetaY);
			//vertices[i].rotateZ(thetaZ);

			//draw vertices as pixels
			int bX = (int) (vertices[i][0]/(-vertices[i][2]/eZ+1)+nCols/2);
			int bY = (int) (vertices[i][1]/(-vertices[i][2]/eZ+1)+nRows/2);
			if (bX >= 0 && bY >= 0 && bX<=nCols && bY<=nRows)
				pixels[bY][bX].set_color(red, green, blue);
		}
		cout << "saving picture of all vertices projected as pixels\n";
		savePicture("pixels.ppm");

		saveOutline();

		resetZBuffer();

		saveFlat();

		trig.calculateVerticesNormals();

		resetZBuffer();

		saveGouraud();

		resetZBuffer();

		savePhong();


	}
	else {
		cerr << argv[0] << " <filename> " << endl;
		exit(1);
	}
}
