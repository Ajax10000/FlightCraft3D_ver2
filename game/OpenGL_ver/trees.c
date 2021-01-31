#include <math.h>            // for floor()
#include <stdio.h>           // for printf(), sprintf()
#include <stdlib.h>          // for rand()

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <GL/glcorearb.h>

#include "globals.h"         // for the global variables
#include "graphics.h"        // for getPixel()
#include "terrain.h"         // for getTerrainHeight()
#include "trees.h"           // for our prototypes

// maximal values, if possible don't exploit maximums
#define txtWidth2 32
#define txtHeight2 32
#define txtWidth3 96
#define txtHeight3 96

// ####################################################################################################################
// Function initTrees
// ####################################################################################################################
void initTrees()
{
	int j, k;
	int tree_group_n = 30;

	// trees' position and default parameters for trees.
	for (k = 0; k < NTREES - tree_group_n; k = k + tree_group_n)
	{
		// random x and y coordinates within a rectangular area
		gloTrees[k][0] = 5000 * ((double)-0 * RAND_MAX / 2 + rand()) / ((double)RAND_MAX);
		gloTrees[k][1] = 5000 * ((double)-0 * RAND_MAX / 2 + rand()) / ((double)RAND_MAX);
		gloTrees[k][2] = getTerrainHeight(&gloTerrain, gloTrees[k][0], gloTrees[k][1]);

		gloTrees[k][4] = floor(2.0 * ((double)rand()) / ((double)RAND_MAX));

		if (gloTrees[k][4] == 0)
		{
			gloTrees[k][4] = gloTreeTextureIDBounds[0];
			gloTrees[k][3] = 3;
		}
		else if (gloTrees[k][4] == 1)
		{
			gloTrees[k][4] = gloTreeTextureIDBounds[0] + 2;
			gloTrees[k][3] = 1;
		}

		for (j = 1; j < 1 + tree_group_n; j++)
		{
			// MACCHIA attorno ad un punto / STRAIN around a point...
			gloTrees[k + j][0] = gloTrees[k][0] + 100 * ((double)-0 * RAND_MAX / 2 + rand()) / ((double)RAND_MAX);
			gloTrees[k + j][1] = gloTrees[k][1] + 100 * ((double)-0 * RAND_MAX / 2 + rand()) / ((double)RAND_MAX);
			gloTrees[k + j][2] = getTerrainHeight(&gloTerrain, gloTrees[k + j][0], gloTrees[k + j][1]);

			gloTrees[k + j][4] = floor(2.0 * ((double)rand()) / ((double)RAND_MAX));

			if (gloTrees[k + j][4] == 0.0)
			{
				gloTrees[k + j][4] = gloTreeTextureIDBounds[0]; // texture id number
				gloTrees[k + j][3] = 3;
			}
			else if (gloTrees[k + j][4] == 1.0)
			{
				gloTrees[k + j][4] = gloTreeTextureIDBounds[0] + 2;
				gloTrees[k + j][3] = 1; // texture id number
			}
		}
	}
	//trees ok.
} // end initTrees function

// ####################################################################################################################
// Function drawTrees
// ####################################################################################################################
void drawTrees()
{
	float Xo[5], Yo[5], Zo[5], x_c[5], y_c[5], z_c[5];
	int i, k;

	//trees
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	for (k = 0; k < NTREES; k++)
	{
		float xctree, yctree, zctree, sctree;

		xctree = gloTrees[k][0]; // x of conventinal geometric center
		yctree = gloTrees[k][1]; // y of conventinal geometric center

		if ((xctree < x + 1000.0 && xctree > x - 1000.0) && (yctree < y + 1000.0 && yctree > y - 1000.0))
		{
			// draw only trees that are not too far away
			// z of conventinal geometric center. this was pre-calculated so that trees 
			// stay nicely on the terrain surface.
			zctree = gloTrees[k][2]; 

			// magnification value: how much to magnify original (1_unit x 1_unit) square?.
			sctree = gloTrees[k][3]; 

			Xo[0] = xctree - sctree * 0.9;
			Yo[0] = yctree;
			Zo[0] = zctree - treeR1 + sctree * 2.0;

			Xo[1] = xctree + sctree * 0.9;
			Yo[1] = yctree;
			Zo[1] = zctree - treeR1 + sctree * 2.0;

			Xo[2] = xctree + sctree * 0.9;
			Yo[2] = yctree;
			Zo[2] = zctree - treeR1;

			Xo[3] = xctree - sctree * 0.9;
			Yo[3] = yctree;
			Zo[3] = zctree - treeR1;

			Xo[4] = xctree;
			Yo[4] = yctree;
			Zo[4] = zctree + 2.0;

			glBindTexture(GL_TEXTURE_2D, gloTexIds[(int)gloTrees[k][4]]);

			// calcola lecoordinate di questi 3 punti nel sistema P-Q-R del paracadutista/pilota 
			// calculates the coordinates of these 3 points in the parachutist / pilot's P-Q-R system
			for (i = 0; i < 5; i++)
			{
				x_c[i] = P[0] * (Xo[i] - x) + P[1] * (Yo[i] - y) + P[2] * (Zo[i] - z);
				y_c[i] = Q[0] * (Xo[i] - x) + Q[1] * (Yo[i] - y) + Q[2] * (Zo[i] - z);
				z_c[i] = R[0] * (Xo[i] - x) + R[1] * (Yo[i] - y) + R[2] * (Zo[i] - z);
			}

			glBegin(GL_QUADS);
				glTexCoord2f(0.0, 0.0);
				glVertex3f(x_c[0], y_c[0], z_c[0]); // point 1

				glTexCoord2f(1.0, 0.0);
				glVertex3f(x_c[1], y_c[1], z_c[1]); // point 2

				glTexCoord2f(1.0, 1.0);
				glVertex3f(x_c[2], y_c[2], z_c[2]); // point 3

				glTexCoord2f(0.0, 1.0);
				glVertex3f(x_c[3], y_c[3], z_c[3]); // point 4
			glEnd();

			// change these coordinates:
			Xo[0] = xctree;
			Yo[0] = yctree - sctree * 0.9;
			Zo[0] = zctree - treeR1 + sctree * 2.0;

			Xo[1] = xctree;
			Yo[1] = yctree + sctree * 0.9;
			Zo[1] = zctree - treeR1 + sctree * 2.0;

			Xo[2] = xctree;
			Yo[2] = yctree + sctree * 0.9;
			Zo[2] = zctree - treeR1;

			Xo[3] = xctree;
			Yo[3] = yctree - sctree * 0.9;
			Zo[3] = zctree - treeR1;

			// again this is to obtain coordinates in the virtual camera's reference frame.
			// calcola lecoordinate di questi 3 punti nel sistema P-Q-R del paracadutista/pilota 
			// calculates the coordinates of these 3 points in the parachutist / pilot's P-Q-R system
			for (i = 0; i < 5; i++)
			{
				x_c[i] = P[0] * (Xo[i] - x) + P[1] * (Yo[i] - y) + P[2] * (Zo[i] - z);
				y_c[i] = Q[0] * (Xo[i] - x) + Q[1] * (Yo[i] - y) + Q[2] * (Zo[i] - z);
				z_c[i] = R[0] * (Xo[i] - x) + R[1] * (Yo[i] - y) + R[2] * (Zo[i] - z);
			}

			// finally, draw also this:
			glBegin(GL_QUADS);
				glTexCoord2f(0.0, 0.0);
				glVertex3f(x_c[0], y_c[0], z_c[0]); // point 1

				glTexCoord2f(1.0, 0.0);
				glVertex3f(x_c[1], y_c[1], z_c[1]); // point 2

				glTexCoord2f(1.0, 1.0);
				glVertex3f(x_c[2], y_c[2], z_c[2]); // point 3

				glTexCoord2f(0.0, 1.0);
				glVertex3f(x_c[3], y_c[3], z_c[3]); // point 4
			glEnd();
		}
	}
	glDisable(GL_TEXTURE_2D);
} // end drawTrees function

// ####################################################################################################################
// Function feedIntoTxt1
// ####################################################################################################################
static void feedIntoTxt1(SDL_Surface *image, GLubyte txt1[txtHeight3][txtWidth3][4]) 
{
	int i, j;
	Uint8 red, green, blue, alpha;
	Uint32 color_to_convert;
	int txtRES2 = 96;

	for (j = 0; j < txtRES2; j++)
	{ // vertical
		for (i = 0; i < txtRES2; i++)
		{ // horizontal
			color_to_convert = getPixel(image, i, j);
			SDL_GetRGBA(color_to_convert, image->format, &red, &green, &blue, &alpha);

			txt1[j][i][0] = (GLubyte)red;
			txt1[j][i][1] = (GLubyte)green;
			txt1[j][i][2] = (GLubyte)blue;
			txt1[j][i][3] = (GLubyte)alpha;

			if (txt1[j][i][0] == 0 && txt1[j][i][1] == 0 && txt1[j][i][2] == 0)
			{
				// Add transparency value artificially according to some convention like black => transparent
				txt1[j][i][3] = 0; // make this pixel totally transparent (alpha = 0) ; opaque is alpha = 255. .
			}

			if (alpha < 255)
			{
				printf("pixel : [%d,%d,%d ,alpha_value: %d]\n", red, green, blue, alpha);
			}
		}
	}
} // end feedIntoTxt1 function

// ####################################################################################################################
// Function loadTreeTextures
// ####################################################################################################################
void loadTreeTextures()
{
	static int texture_generated = 0; // at first call of this function, a 32x32 texture sample will be generated 
	static GLubyte txt1[txtHeight3][txtWidth3][4];
	static GLuint texName;
	int txtRES2 = 96; // A reasonable texture resolution
	int j, i, k;

	if (texture_generated == 0)
	{
		//	Create texture
		texture_generated = 1;

		for (j = 0; j < txtHeight2; j++)
		{
			for (i = 0; i < txtWidth2; i++)
			{
				for (k = 0; k < 4; k++)
				{
					txt1[j][i][k] = (GLubyte)0.5 * 255.0 + (double)255.0 * rand() / (double)RAND_MAX;
				}
			}
		}

		//=================TREE TEXTURE PERSONALISED...=====================
		int texn = 1; // > 0 ABSOLUTELY!!!

		while (texn > 0)
		{
			char filename[100];
			SDL_Surface *image; // This pointer will reference our bitmap.
			int imhe;

			sprintf(filename, "textures/semitransparent/texture_%i.bmp", texn);
			printf("TRYING TO OPEN FILE: %s", filename);

			image = SDL_LoadBMP(filename);

			imhe = 96;
			if (image != NULL)
			{
				printf("bitmap found: %s\n", filename);

				imhe = image->h;
				txtRES2 = imhe; // set texture resolution, txt must be square!!!
				printf("bitmap RES: %i\n", imhe);
				SDL_Delay(5);

				// feed into txt1 array
				feedIntoTxt1(image, txt1);

				// Release the surface
				SDL_FreeSurface(image);

				texName = gloTexturesAvailable + texn - 1; // VERY CAREFUL!!! NOT OVERWRITE ALREADY OCCUPIED TEXTURES!!

				// Pass the data on to OpenGL

				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

				glGenTextures(1, &texName);

				gloTexIds[gloTexturesAvailable + texn - 1] = texName; // [texn-1] because started from 1, be careful

				glBindTexture(GL_TEXTURE_2D, gloTexIds[gloTexturesAvailable + texn - 1]); // [texn-1] because started from 1, be careful

				// Indicate what OpenGL should do when texture is magnified (GL_NEAREST => non-smoothed texture; GL_LINEAR => smoothed)
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); 

				// Indicate what OpenGL should do when texture is miniaturized because far (GL_NEAREST => non-smoothed texture)
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);	

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, txtRES2, txtRES2, 0, GL_RGBA, GL_UNSIGNED_BYTE, txt1);
				glGenerateMipmap(GL_TEXTURE_2D);

				glEnable(GL_TEXTURE_2D);
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL); // the GL_DECAL option draws texture as is: no color mixing thigs. GL_MODULATE lets mixing.

				glBindTexture(GL_TEXTURE_2D, texName);

				texn++;	// augment count... next texture
				gloTexturesAvailable++; // ditto but on an extern variable... .
			}
			else
			{
				printf("File opening error ocurred. Using random generated texture.\n");
				printf("SDL_GetError() notify: %s\n", SDL_GetError());
				texn = -1; // cause exiting from while loop.
			}
		} // end while
	}
} // end loadTreeTextures function

