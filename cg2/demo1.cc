using namespace std ;

#include <stdio.h>
#include <string.h>

#include <iostream>
#include <fstream>
#include <cstdlib>

#include <GL/gl.h>
//#include <GL/glx.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glext.h>

#include "demo1.h"
#include "image.h"

#define PI 3.14159265
#define nCols 640
#define nRows 480

TriangleMesh trig;

int window;
GLuint texture[3];
GLfloat	xrot;
GLfloat	yrot;
GLfloat	zrot;
GLfloat	planeXrot = 0;
GLfloat	planeYrot = 0;
GLfloat	planeZrot = 0;
GLfloat zpos = -1000;
GLfloat xpos = 0;
GLfloat lightPos[]= { -100.0f, 10.0f, 300.0f, 1.0f };
Vector3f relativeLightPos;
bool useShadows = false;

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

void init()
{
	trig.calculateVerticesNormals();
	trig.calculatePlanes();
	//trig.generateNeighbours();

	glShadeModel(GL_SMOOTH);
	glViewport(0,0,nCols,nRows);

	//set up the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat) nCols/(GLfloat) nRows, 1.0, 5000.0);

	//switch to the model view matrix
	glMatrixMode( GL_MODELVIEW );  // Select The Model View Matrix
	glLoadIdentity();

	//set clear color
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	//set up the depth buffer
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	//textures
	glEnable( GL_TEXTURE_2D );
	texture[0] = LoadTexture("abc.ppm",1);
	texture[1] = LoadTexture("floor.ppm",1);
	texture[2] = LoadTexture("rock.ppm",1);

	//lights
	glEnable(GL_LIGHTING);
	GLfloat ambient[]= { 0.8f, 0.8f, 0.8f, 1.0f };
	GLfloat diffuse[]= { 0.8f, 0.8f, 0.8f, 1.0f };
	GLfloat specular[]= { 1, 1, 1, 1.0f };
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT1, GL_POSITION,lightPos);
	glEnable(GL_LIGHT1);

	//face culling
	glCullFace(GL_BACK);

}

void setTextureCoordinates(Vector3f normal, Vector3f vertex)
{
	/*
	 * Sets the texture coordinates based on the vertex normal
	 *
	 * Works as if the texture was projected onto the top, bottom, front and back of the object
	 */
	float texX, texY;
	if (abs(normal[1])>=0.85)
	{
		texX = vertex[0]/nRows;
		texY = vertex[2]/nCols;
	}
	else
	{
		texX = vertex[0]/nRows;
		texY = vertex[1]/nCols;
	}
	glTexCoord2f(texX, texY);
}

void drawTeapot()
{
	//bool texSwap = false;
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	GLfloat reflectance[] = {1, 1, 1, 1};
	glMaterialfv(GL_FRONT, GL_SPECULAR, reflectance);
	glMateriali(GL_FRONT, GL_SHININESS, 128);
	glBegin(GL_TRIANGLES);
	for (int i = 0; i<trig.trigNum(); i++)
	{
		//if (trig.isTriangleIlluminated(i)) {
		Vector3f v0;
		Vector3f v1;
		Vector3f v2;
		Vector3f vn0, vn1, vn2;
		trig.getTriangleVertices(i, v0, v1, v2);
		trig.getVerticesNormals(i, vn0, vn1, vn2);

		glNormal3f(vn0[0], vn0[1], vn0[2]);
		setTextureCoordinates(vn0, v0);
		//if (texSwap) glTexCoord2f(0.0f, 0.0f); else glTexCoord2f(1.0f, 1.0f);
		//glTexCoord2f(v0[0]/nRows, v0[1]/nCols);
		glVertex3f( v0[0], v0[1], v0[2]);

		glNormal3f(vn1[0], vn1[1], vn1[2]);
		setTextureCoordinates(vn1, v1);
		//if (texSwap) glTexCoord2f(0.0f, 1.0f); else glTexCoord2f(1.0f, 0.0f);
		//glTexCoord2f(v1[0]/nRows, v1[1]/nCols);
		glVertex3f( v1[0], v1[1], v1[2]);

		glNormal3f(vn2[0], vn2[1], vn2[2]);
		setTextureCoordinates(vn2, v2);
		//if (texSwap) glTexCoord2f(1.0f, 0.0f); else glTexCoord2f(0.0f, 1.0f);
		//glTexCoord2f(v2[0]/nRows, v2[1]/nCols);
		glVertex3f( v2[0], v2[1], v2[2]);

		//texSwap = !texSwap;
		//}
	}
	glEnd();
}

void drawFloor()
{
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glBegin(GL_QUADS);
	//floor
		glNormal3f(0, 1, 0);
		glTexCoord2f(0,0);
		glVertex3f( -400, -200, -400);
		glTexCoord2f(1,0);
		glVertex3f( 400, -200, -400);
		glTexCoord2f(1,1);
		glVertex3f( 400, -200, 400);
		glTexCoord2f(0,1);
		glVertex3f( -400, -200, 400);
	glEnd();
}

void drawWall()
{
	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glBegin(GL_QUADS);
	//back wall
		glNormal3f(0, 0, 1);
		glTexCoord2f(0,0);
		glVertex3f( -400, -200, -400);
		glTexCoord2f(1,0);
		glVertex3f( 400, -200, -400);
		glTexCoord2f(1,1);
		glVertex3f( 400, 400, -400);
		glTexCoord2f(0,1);
		glVertex3f( -400, 400, -400);
	glEnd();
}

void drawCube()
{
	glBegin(GL_QUADS);
	//back wall
		glColor3f(1,1,0);
		glVertex3f( -2, -2, 0);
		glVertex3f( 2, -2, 0);
		glVertex3f( 2, 2, 0);
		glVertex3f( -2, 2, 0);

		glVertex3f( 0, -2, -2);
		glVertex3f( 0, -2, 2);
		glVertex3f( 0, 2, 2);
		glVertex3f( 0, 2, -2);

		glVertex3f( -2, 0, -2);
		glVertex3f( -2, 0, 2);
		glVertex3f( 2, 0, 2);
		glVertex3f( 2, 0, -2);
	glEnd();
}

void doShadowPass() {
	for (int i = 0; i < trig.trigNum(); i++)
	{
		if (trig.isTriangleIlluminated(i))
		{
			// go through each edge
			for (int j = 0; j < 3; j++)
			{
				// Calculate The Two Vertices In Distance
				Vector3f v1, v2, v3, v4;
				trig.getTriangleVertexByIndex(i, j, v1);
				trig.getTriangleVertexByIndex(i, (j+1)%3, v2);

				v3[0] = (v1[0] - relativeLightPos[0]) * 100;
				v3[1] = (v1[1] - relativeLightPos[1]) * 100;
				v3[2] = (v1[2] - relativeLightPos[2]) * 100;

				v4[0] = (v2[0] - relativeLightPos[0]) * 100;
				v4[1] = (v2[1] - relativeLightPos[1]) * 100;
				v4[2] = (v2[2] - relativeLightPos[2]) * 100;

				// draw the triangle strip
				glBegin(GL_TRIANGLE_STRIP);
					glVertex3f(v1[0], v1[1], v1[2]);
					glVertex3f(v1[0] + v3[0], v1[1] + v3[1], v1[2] + v3[2]);
					glVertex3f(v2[0], v2[1], v2[2]);
					glVertex3f(v2[0] + v4[0], v2[1] + v4[1], v2[2] + v4[2]);
				glEnd();
			}
		}
	}
}


void castShadow()
{
	glClearStencil(1);
	glClear(GL_STENCIL_BUFFER_BIT);

	// Determine Which Faces Are Visible By The Light.
	for ( int i = 0; i < trig.trigNum(); i++ )
	{
		Vector3f plane = trig.getPlane(i);

		GLfloat side = plane[0]*relativeLightPos[0]+
			plane[1]*relativeLightPos[1]+
			plane[2]*relativeLightPos[2]+
			plane[3];

		if ( side > 0 )
			trig.setTriangleIlluminated(i, true);
		else
			trig.setTriangleIlluminated(i, false);
	}

	//save all the current attributes and disable depth buffer and lights
	glPushAttrib( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_POLYGON_BIT | GL_STENCIL_BUFFER_BIT );
	glDisable( GL_LIGHTING );
	glDepthMask( GL_FALSE );
	glDepthFunc( GL_LEQUAL );

	glEnable( GL_STENCIL_TEST );
	glColorMask( 0, 0, 0, 0 );
	glStencilFunc( GL_ALWAYS, 1, 10 );

	glEnable(GL_CULL_FACE);
	// first pass
	glFrontFace( GL_CCW );
	glStencilOp( GL_KEEP, GL_KEEP, GL_INCR );
	doShadowPass();
	// second pass
	glFrontFace( GL_CW );
	glStencilOp( GL_KEEP, GL_KEEP, GL_DECR );
	doShadowPass();
	glFrontFace( GL_CCW );
	glDisable(GL_CULL_FACE);

	glColorMask( 1, 1, 1, 1 );	// Enable Rendering To Colour Buffer For All Components

	// Draw A Shadowing Rectangle Covering The Entire Screen
	glColor4f( 0.0f, 0.0f, 0.0f, 0.5f );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glStencilFunc( GL_NOTEQUAL, 1, 10 );
	glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
	glPushMatrix();
	glLoadIdentity();
	glBegin( GL_QUADS );
		glVertex3f(-100, -100,-2);
		glVertex3f( 100, -100,-2);
		glVertex3f( 100, 100,-2);
		glVertex3f(-100, 100,-2);
	glEnd();

	glDisable(GL_STENCIL_TEST);
	glPopMatrix();
	glPopAttrib();


}

void drawScene()
{
	glClearStencil(0);
	// Clear The Screen And The Depth Buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	//reset the model view matrix
	glLoadIdentity();

	glTranslatef(xpos,0.0f,zpos);
	glRotatef(planeXrot,1.0f,0.0f,0.0f);
	glRotatef(planeYrot,0.0f,1.0f,0.0f);
	glRotatef(planeZrot,0.0f,0.0f,1.0f);

	//stencil buffer (for reflection on the floor)
	GLdouble planeClip[] = {0.0f,-1.0f, 0.0f, 200.0f}; //clip anything above -200
	glColorMask(0,0,0,0);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 1, 1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glDisable(GL_DEPTH_TEST);
	drawFloor();//mark the floor as a stencil
	glEnable(GL_DEPTH_TEST);
	glColorMask(1,1,1,1);
	glStencilFunc(GL_EQUAL, 1, 1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glEnable(GL_CLIP_PLANE0);
	glClipPlane(GL_CLIP_PLANE0, planeClip);
	glPushMatrix();
		glScalef(1.0f, -1.0f, 1.0f);
		glLightfv(GL_LIGHT1, GL_POSITION, lightPos);
		glTranslatef(0.0f,400.0f,0.0f);
		drawWall();
		glTranslatef(0.0f,10.0f,0.0f);
		glRotatef(xrot,1.0f,0.0f,0.0f);
		glRotatef(yrot,0.0f,1.0f,0.0f);
		glRotatef(zrot,0.0f,0.0f,1.0f);
		drawTeapot();
	glPopMatrix();
	glDisable(GL_CLIP_PLANE0);
	glDisable(GL_STENCIL_TEST);

	//"real" objects
	glLightfv(GL_LIGHT1, GL_POSITION, lightPos);

	glDisable(GL_LIGHTING);
	glPushMatrix();
	glTranslatef(lightPos[0], lightPos[1], lightPos[2]);
	drawCube();
	glPopMatrix();
	glEnable(GL_LIGHTING);

	drawWall();

	//floor with texture blending to obscure the
	//inverted teapot and generate an impression
	//that it's a reflection
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	drawFloor();
	glEnable(GL_LIGHTING);
	glDisable(GL_BLEND);

	glTranslatef(0.0f,10.0f,0.0f);
	glRotatef(xrot,1.0f,0.0f,0.0f);
	glRotatef(yrot,0.0f,1.0f,0.0f);
	glRotatef(zrot,0.0f,0.0f,1.0f);

	drawTeapot();

	//calculate a light vector relative to the teapot using the modelview matrix
	glPushMatrix();
	glLoadIdentity();
	//inverse the translations
	glRotatef(-xrot,1.0f,0.0f,0.0f);
	glRotatef(-yrot,0.0f,1.0f,0.0f);
	glRotatef(-zrot,0.0f,0.0f,1.0f);
	float viewMatrix[16];
	glGetFloatv(GL_MODELVIEW_MATRIX,viewMatrix);
	relativeLightPos[0] = lightPos[0];
	relativeLightPos[1] = lightPos[1];
	relativeLightPos[2] = lightPos[2];
	relativeLightPos[3] = lightPos[3];
	//relative rotation
	relativeLightPos.multiplyMatrixByVector16(viewMatrix);

	Vector3f temp;
	temp[0] = 0;
	temp[1] = 0;
	temp[2] = 0;
	temp[3] = 1;
	//inverse the translation
	glTranslatef(-xpos,0.0f,-zpos);
	glGetFloatv(GL_MODELVIEW_MATRIX,viewMatrix);
	//relative position
	temp.multiplyMatrixByVector16(viewMatrix);
	relativeLightPos += temp;
	glPopMatrix();

	if (useShadows)
		castShadow();

	glutSwapBuffers();
}

void keyPressed(unsigned char key, int x, int y)
{
    if (key == 'w')
    	xrot-=15.0f;
    if (key == 's')
        xrot+=15.0f;
    if (key == 'a')
        yrot+=15.0f;
    if (key == 'd')
        yrot-=15.0f;

    if (key == 'i' && planeXrot<180)
		planeXrot+=15.0f;
	if (key == 'k' && planeXrot>0)
		planeXrot-=15.0f;
	if (key == 'j')
		planeYrot+=15.0f;
	if (key == 'l')
		planeYrot-=15.0f;

	if (key == 't')
		lightPos[1]+=15.0f;
	if (key == 'g')
		lightPos[1]-=15.0f;
	if (key == 'f')
		lightPos[0]-=15.0f;
	if (key == 'h')
		lightPos[0]+=15.0f;
	if (key == 'r')
		lightPos[2]-=15.0f;
	if (key == 'y')
		lightPos[2]+=15.0f;

	if (key == '1')
		useShadows = !useShadows;

}

void specialKeyPressed(int key, int x, int y)
{
	//cout << key << endl;
	if (key==101)
		zpos+=10;
	else if (key==103)
		zpos-=10;
	else if (key==100)
		xpos+=10;
	else if (key==102)
		xpos-=10;
}


int main(int argc, char **argv)
{
	if (argc >  1)  {
		char* filename = argv[argc - 1];
		trig.loadFile(filename);

		glutInit(&argc, argv);
		glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_STENCIL | GLUT_DEPTH);
		glutInitWindowSize (nCols, nRows);
		glutInitWindowPosition (100, 100);
		window = glutCreateWindow ("CG Practical #2 - 0841912");

		init();

		glutDisplayFunc(&drawScene);
		glutIdleFunc(&drawScene);
		glutKeyboardFunc(&keyPressed);
		glutSpecialFunc(&specialKeyPressed);

		glutMainLoop();

		return 0;
	}
	else {
		cerr << argv[0] << " <filename> " << endl;
		exit(1);
	}
}
