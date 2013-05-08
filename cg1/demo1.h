#ifndef _rt_H
#define _rt_H



#include <cmath>
#include <vector>

//
// Sample code for physics simulation
//


// Implements cloth simulation


class Vector3f;
class Triangle;
class TriangleMesh;


class Vector3f {

	float _item[4];

	public:

	float & operator [] (int i) {
		return _item[i];
    	}

	Vector3f(float x, float y, float z) 
	{  _item[0] = x ; _item[1] = y ; _item[2] = z; _item[3]=1;};

	Vector3f() {};


	Vector3f & operator = (Vector3f & obj) 
	{
		_item[0] = obj[0];
		_item[1] = obj[1];
		_item[2] = obj[2];

		return *this;
	};

	Vector3f operator - (Vector3f & obj)
	{
		return Vector3f(_item[0]-obj[0],_item[1]-obj[1],_item[2]-obj[2]);
	};

	Vector3f & operator += (Vector3f & obj) 
	{
		_item[0] += obj[0];
		_item[1] += obj[1];
		_item[2] += obj[2];

		return *this;
	};

	Vector3f & operator -= (Vector3f & obj)
	{
		_item[0] -= obj[0];
		_item[1] -= obj[1];
		_item[2] -= obj[2];

		return *this;
	};

	void multiplyMatrixByVector(float matrix[4][4])
	{
		float x0 = _item[0]*matrix[0][0]+_item[1]*matrix[1][0]+_item[2]*matrix[2][0]+_item[3]*matrix[3][0];
		float x1 = _item[0]*matrix[0][1]+_item[1]*matrix[1][1]+_item[2]*matrix[2][1]+_item[3]*matrix[3][1];
		float x2 = _item[0]*matrix[0][2]+_item[1]*matrix[1][2]+_item[2]*matrix[2][2]+_item[3]*matrix[3][2];
		float x3 = _item[0]*matrix[0][3]+_item[1]*matrix[1][3]+_item[2]*matrix[2][3]+_item[3]*matrix[3][3];
		_item[0] = x0; _item[1] = x1; _item[2] = x2; _item[3] = x3;
	}

	void rotateY(float theta)
	{
		float rotationMatrix[4][4] = {{cos(theta), 0,sin(theta),0},
									  {0,          1,0,         0},
									  {-sin(theta),0,cos(theta),0},
									  {0,          0,0,         1}};
		multiplyMatrixByVector(rotationMatrix);
	}

	void rotateX(float theta)
	{
		float rotationMatrix[4][4] = {{1,0,0,0},
									  {0,cos(theta),-sin(theta),0},
									  {0,sin(theta),cos(theta),0},
									  {0,0,0,1}};
		multiplyMatrixByVector(rotationMatrix);
	}

	void rotateZ(float theta)
	{
		float rotationMatrix[4][4] = {{cos(theta),-sin(theta),0,0},
									  {sin(theta),cos(theta),0,0},
									  {0,0,1,0},
									  {0,0,0,1}};
		multiplyMatrixByVector(rotationMatrix);
	}

	void normalise()
	{
		float crossLen = sqrt(pow(_item[0],2)+pow(_item[1],2)+pow(_item[2],2));
		_item[0] = _item[0] / crossLen;
		_item[1] = _item[1] / crossLen;
		_item[2] = _item[2] / crossLen;
	}
};

ostream & operator << (ostream & stream, Vector3f & obj) 
{
	stream << obj[0] << ' ' << obj[1] << ' ' << obj[2] << ' ' << obj[3] << ' ';
};

class Triangle {
friend class TriangleMesh;

	int _vertex[3];
	int color;
public:

	Triangle(int v1, int v2, int v3) 
	{
		_vertex[0] = v1;  _vertex[1] = v2;  _vertex[2] = v3;  
		
	};

	void set_color(int i)
	{
		color = i;
	}
};

float fmax(float f1,float f2, float f3) {
	float f = f1;

	if (f < f2) f = f2;
	if (f < f3) f = f3;

	return f;
};

float fmin(float f1,float f2, float f3) {
	float f = f1;

	if (f > f2) f = f2;
	if (f > f3) f = f3;

	return f;
};


class TriangleMesh 
{
	vector <Vector3f> _v;
	vector <Triangle> _trig;
	vector <Vector3f> _normals;
	float _xmax, _xmin, _ymax, _ymin, _zmin, _zmax;

public: 
	TriangleMesh(char * filename) { loadFile(filename) ;};
	TriangleMesh() {};
	void loadFile(char * filename);

	int trigNum() { return _trig.size() ;};

	Vector3f getVertex(int i)
	{
		return _v[i];
	}

	void setVertex(int i, Vector3f v)
	{
		_v[i] = v;
	}

	vector<Vector3f>& getVertices()
	{
		return _v;
	}

	void getTriangleVertices(int i, Vector3f & v1, Vector3f & v2, Vector3f & v3)
	{
		v1 = _v[_trig[i]._vertex[0]]; 
		v2 = _v[_trig[i]._vertex[1]]; 
		v3 = _v[_trig[i]._vertex[2]]; 
	}

	int getTriangleColor(int i)
	{
		return _trig[i].color;
	}

	void getVerticesNormals(int i, Vector3f & v1, Vector3f & v2, Vector3f & v3)
	{
		v1 = _normals[_trig[i]._vertex[0]];
		v2 = _normals[_trig[i]._vertex[1]];
		v3 = _normals[_trig[i]._vertex[2]];
	}

	void calculateVerticesNormals()
	{
		for (int i=0; i<_v.size(); i++)
		{
			Vector3f normal = Vector3f(0,0,0);
			//int facesCount = 0;
			for (int j = 0; j < _trig.size(); j++)
				for (int k = 0; k < 3; k++)
					if (_trig[j]._vertex[k] == i) {
						//facesCount++;
						Vector3f v10 = _v[_trig[j]._vertex[1]] - _v[_trig[j]._vertex[0]];
						Vector3f v20 = _v[_trig[j]._vertex[2]] - _v[_trig[j]._vertex[0]];
						Vector3f faceNormal = Vector3f(v10[1] * v20[2] - v20[1]
								* v10[2], v10[2] * v20[0] - v20[2] * v10[0],
								v10[0] * v20[1] - v20[0] * v10[1]);
						faceNormal.normalise();
						normal+=faceNormal;
					}
			normal.normalise();
			_normals.push_back(normal);
		}
	}
			
};

class RGBVector
{
	int _value[3];

public:
	RGBVector()
	{
		_value[0] = 0;
		_value[1] = 0;
		_value[2] = 0;
	}

	RGBVector(int r, int g, int b)
	{
		if (r > 255)
			r = 255;
		if (g > 255)
			g = 255;
		if (b > 255)
			b = 255;
		_value[0] = r;
		_value[1] = g;
		_value[2] = b;
	}

	int red()
	{
		return _value[0];
	}

	int green()
	{
		return _value[1];
	}

	int blue()
	{
		return _value[2];
	}

	int set_color(int r, int g, int b)
	{
		if (r>255)
			r = 255;
		if (g > 255)
			g = 255;
		if (b > 255)
			b = 255;
		_value[0] = r;
		_value[1] = g;
		_value[2] = b;
	}
};

#endif //_rt_H
