/*
 * image.h
 *
 *  Created on: 6 Dec 2010
 *      Author: s0841912
 */

#ifndef IMAGE_H_
#define IMAGE_H_

//#include <string>

GLuint LoadTexture( const char * filename, int wrap )
{
    GLuint texture;
    int width, height;
    char buf[1024];
    char header[100];
    char * data;
    int index = 0;

    // open texture data
    ifstream f(filename);

    if (f == NULL) {
		cerr << "failed reading texture data file " << filename << endl;
		exit(1);
	}
    f.getline(header,100,'\n');
    f.getline(header,100,'\n');
    while(header[0]=='#' || header[0]==0)
       f.getline(header,100,'\n');
    sscanf(header, "%i %i", &width, &height);
    data = (char *)malloc( width * height * 3 );
    f.getline(header,100,'\n');
    while(header[0]=='#' || header[0]==0)
        f.getline(header,100,'\n');
    //cout << "start" << endl;
    while (!f.eof()) {
		f.getline(buf, sizeof(buf), '\n');
		if (buf[0]=='#' || buf[0]==0)
			continue;
		int c;
		sscanf(buf, "%i", &c);
		data[index]=c;
		index += 1;

		/*string bufStr = (string) buf;

		int r=-1,g=-1,b=-1;
		for (int i=0; i<3*width; i+=3)
		{
			int start = 0;
			if (r!=-1)
			{
				start = 3;
				if (r>99)
					start += 3;
				else if (r>9)
					start += 2;
				else
					start += 1;
				if (g>99)
					start += 3;
				else if (g>9)
					start += 2;
				else
					start += 1;
				if (b>99)
					start += 3;
				else if (b>9)
					start += 2;
				else
					start += 1;
				start = start*i/3;
			}
			//cout << bufStr.substr(start,bufStr.length()-1) << endl;
			sscanf(bufStr.substr(start,bufStr.length()).c_str(), "%i %i %i", &r, &g, &b);
			cout << r << ' ' << g << ' ' << b << " | ";
			data[index] = (int)r;
			data[index+1] = (int)g;
			data[index+2] = (int)b;
			index += 3;
			cout << index << endl;
		}
		cout << endl;*/
    }
    //cout << "end" << endl;
    f.close();

    // allocate a texture name
    glGenTextures( 1, &texture );

    // select current texture
    glBindTexture( GL_TEXTURE_2D, texture );

    // select modulate to mix texture with color for shading
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

    // when texture area is small, bilinear filter the closest mipmap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                     GL_LINEAR_MIPMAP_NEAREST );
    // when texture area is large, bilinear filter the first mipmap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    // if wrap is true, the texture wraps over at the edges (repeat)
    //       ... false, the texture ends at the edges (clamp)
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                     wrap ? GL_REPEAT : GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                     wrap ? GL_REPEAT : GL_CLAMP );

    // build our texture mipmaps
    gluBuild2DMipmaps( GL_TEXTURE_2D, 3, width, height,
                       GL_RGB, GL_UNSIGNED_BYTE, data );

    // free buffer
    free( data );

    return texture;
}


#endif /* IMAGE_H_ */
