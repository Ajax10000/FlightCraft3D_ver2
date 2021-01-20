#include <math.h>            // for floor(), pow(), sqrt()
#include <stdio.h>           // for printf(), sprintf()
#include <stdlib.h>          // for rand(), srand()
#include <time.h>            // for clock(), time(), CLOCKS_PER_SEC

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL.h>

#include <GL/gl.h>
#include <GL/glcorearb.h>

#include "globals.h"         // for the global variables
#include "graphics.h"        // for draw... functions
#include "terrain.h"         // for our prototypes

// ####################################################################################################################
// Function initTerrain initializes the terrain with random values.
// ####################################################################################################################
void initTerrain()
{
	int i, j, k;

	// Game terrain initial values
	// BE CAREFUL!!! says how many meters per side, if seen from above, because inclined sides obey
	// sqrt(x^2 + z^2) but this is referred to the case when it's seen **FROM ABOVE**
	gloTerrain.GPunit = 50.0; 
	gloTerrain.map_size = 300;

	// test... the pseudorandom num sequence gotten using the rand() function is the same. 
	// on the same system so... it's not random.
	srand(1234); 
	// but so it is random since the time now is a number, the time in a second is another number and 
	// except system time reset of overflow the num is always different.
	srand((long int)time(NULL));
	printf("TIME number used to generate random environment: %li\n\n", (long int)time(NULL));
	waitMs(1000.0);

	// generate random height samples 
	for (j = 0; j < TERRAIN_SIZE; j++)
	{
		for (i = 0; i < TERRAIN_SIZE; i++)
		{
			gloTerrain.shmap[j][i] = 0.2 * (((double)rand()) / ((double)RAND_MAX));

			gloTerrain.scol[j][i][0] = (float)0.1 * rand() / (float)RAND_MAX;
			gloTerrain.scol[j][i][1] = (float)0.6 * rand() / (float)RAND_MAX;
			gloTerrain.scol[j][i][2] = (float)0.0 * rand() / (float)RAND_MAX;
			gloTerrain.map_texture_indexes[j][i] = 1; // BULK TEXTURE... DEFAULT TEXTURE.
		}
	}

	int kux;

	for (kux = 0; kux < 10; kux++)
	{
		// more natural shape... 
		for (k = 0; k < 600; k++)
		{
			int x, y, s_x, s_y, siz;
			j = (int)TERRAIN_SIZE * ((float)rand() / (float)RAND_MAX);
			i = (int)TERRAIN_SIZE * ((float)rand() / (float)RAND_MAX);
			siz = (int)TERRAIN_SIZE / 4 * ((float)rand() / (float)RAND_MAX);

			s_x = 10 + (int)siz * ((float)rand() / (float)RAND_MAX);
			s_y = 10 + (int)siz * ((float)rand() / (float)RAND_MAX);

			for (y = j; y < j + s_x && y < TERRAIN_SIZE; y++)
			{
				for (x = i; x < i + s_y && x < TERRAIN_SIZE; x++)
				{
					gloTerrain.shmap[y][x] = gloTerrain.shmap[y][x] + 0.2;

					gloTerrain.scol[y][x][0] = gloTerrain.scol[y][x][0] * 0.98;
					gloTerrain.scol[y][x][1] = gloTerrain.scol[y][x][1] * 0.99;
					gloTerrain.map_texture_indexes[y][x] = 4 + k % 3;
				}
			}
		} // end for k
	} // end for kux

	// put plane, lengthy airports at random positions, with random size 
	for (k = 0; k < 50; k++)
	{ 
		// be careful... this is not rescaled
		int x, y, airport_x, airport_y;
		j = (int)TERRAIN_SIZE * ((float)rand() / (float)RAND_MAX);
		i = (int)TERRAIN_SIZE * ((float)rand() / (float)RAND_MAX);

		// x= 200 meters + 20*some  [meters]
		// y= 200 meters + 100*some [meters]
		airport_x = (200 + (int)20 * ((float)rand() / (float)RAND_MAX)) / gloTerrain.GPunit;
		airport_y = (200 + (int)500 * ((float)rand() / (float)RAND_MAX)) / gloTerrain.GPunit;

		for (y = j; y < j + airport_x && y < TERRAIN_SIZE; y++)
		{
			for (x = i; x < i + airport_y && x < TERRAIN_SIZE; x++)
			{
				gloTerrain.shmap[y][x] = gloTerrain.shmap[j][i] + 1.0;

				gloTerrain.scol[y][x][0] = 0.4;
				gloTerrain.scol[y][x][1] = 0.4;
				gloTerrain.map_texture_indexes[y][x] = 3;
			}
		}
	}

	gloTerrain.map_size = loadHeightMap("terrain_data/hmap_300x300.bmp");
} // end initTerrain function

// ####################################################################################################################
// Function drawTerrain
// ####################################################################################################################
void drawTerrain() {
	float color[4] = {0.0, 0.0, 0.0, 1.0};
	int i;

	// 3 assi dello spazio cartesiano, cosi' si capisce dove stanno le cose
	// 3 axes of cartesian space, so you know where things are
	// Triangolo 1 / Triangle 1
	float Xo[5], Yo[5], Zo[5], x_c[5], y_c[5], z_c[5];
	float GPunit;
	int xi, yi, Xi, Yi;

	GPunit = gloTerrain.GPunit;

	Xo[3] = xp;
	Yo[3] = yp;
	Zo[3] = zp;

	Xo[0] = xp + 10 * Pa[0];
	Yo[0] = yp + 10 * Pa[1];
	Zo[0] = zp + 10 * Pa[2];

	Xo[1] = xp + 10 * Qa[0];
	Yo[1] = yp + 10 * Qa[1];
	Zo[1] = zp + 10 * Qa[2];

	Xo[2] = xp + 10 * Ra[0];
	Yo[2] = yp + 10 * Ra[1];
	Zo[2] = zp + 10 * Ra[2];

	// calcola lecoordinate di questi 3 punti nel sistema P-Q-R 
	// calculates the coordinates of these 3 points in the P-Q-R system
	for (i = 0; i < 4; i++)
	{
		x_c[i] = P[0] * (Xo[i] - x) + P[1] * (Yo[i] - y) + P[2] * (Zo[i] - z);
		y_c[i] = Q[0] * (Xo[i] - x) + Q[1] * (Yo[i] - y) + Q[2] * (Zo[i] - z);
		z_c[i] = R[0] * (Xo[i] - x) + R[1] * (Yo[i] - y) + R[2] * (Zo[i] - z);
	}

	for (i = 0; i < 3; i++)
	{
		color[0] = i / 3.0;
		color[1] = i / 3.0;
		color[2] = i / 3.0;

		drawPerspLine(x_c[i], y_c[i], -z_c[i], 
					   x_c[3], y_c[3], -z_c[3], 
					   color);
	}

	// DISEGNA IL TERRENO IN MODO ALGORITMICO, UNA GRIGILIA RETTANGOLARE COME AL SOLITO,
	// 'REDERING' WIREFRAME O A TRINGOLI RIPIENI 
	// DRAW THE GROUND IN AN ALGORITHMIC WAY, A RECTANGULAR GRAY AS USUAL, 
	// 'RENDERING' WIREFRAME OR FILLED TRINGLES
	// 
	// Draw only near triangles, in order to avoid overloading graphics computational heavyness 
	int lv = 6;

	Xi = floor(xp / (gloTerrain.GPunit));
	Yi = floor(yp / (gloTerrain.GPunit));

	lv = 12;

	if (gloUsingLowResolution == 1)
	{
		lv = 6;
	}

	for (xi = Xi - lv; xi < Xi + lv; xi = xi + 1)
	{
		for (yi = Yi - lv; yi < Yi + lv; yi = yi + 1)
		{
			// Triangolo 1 / Triangle 1
			Xo[0] = GPunit * xi;
			Yo[0] = GPunit * yi;
			Zo[0] = 0.0;

			Xo[1] = GPunit * (xi + 1);
			Yo[1] = GPunit * (yi);
			Zo[1] = 0.0;

			Xo[2] = GPunit * (xi);
			Yo[2] = GPunit * (yi + 1);
			Zo[2] = 0.0;

			Xo[3] = GPunit * (xi + 1);
			Yo[3] = GPunit * (yi + 1);
			Zo[3] = 0.0;

			// default color
			color[0] = 0.4;
			color[1] = 0.4;
			color[2] = 0.4;

			// failsafe variable preset 
			Xo[4] = 1;
			Yo[4] = 1;
			Zo[4] = 1;

			if (Xi + lv < gloTerrain.map_size && Yi + lv < gloTerrain.map_size)
			{
				// escamotage (trick) for a 0-terrain outside sampled limit. but sampled within, good 
				Zo[0] = GPunit * gloTerrain.shmap[xi]    [yi];     // the height sample 
				Zo[1] = GPunit * gloTerrain.shmap[xi + 1][yi];     // the height sample 
				Zo[2] = GPunit * gloTerrain.shmap[xi]    [yi + 1]; // the height sample 
				Zo[3] = GPunit * gloTerrain.shmap[xi + 1][yi + 1]; // the height sample 

				color[0] = gloTerrain.scol[xi][yi][0];
				color[1] = gloTerrain.scol[xi][yi][1];
				color[2] = gloTerrain.scol[xi][yi][2];

				// now prepare a freshly calculated normal vector and then we draw it. 
				// it's fundamental that normals are ok for rebounce 
				getTerrainHeight(&gloTerrain, Xo[0] + 0.01, Yo[0] + 0.01); // check what is the local normal 

				Xo[4] = Xo[0] + 20.0 * gloTerrain.auxnormal[0];
				Yo[4] = Yo[0] + 20.0 * gloTerrain.auxnormal[1];
				Zo[4] = Zo[0] + 20.0 * gloTerrain.auxnormal[2]; // the height sample 
			}

			// calcola lecoordinate di questi 3 punti nel sistema P-Q-R del paracadutista/pilota 
			// calculates the coordinates of these 3 points in the parachutist / pilot's P-Q-R system
			for (i = 0; i < 5; i++)
			{
				x_c[i] = P[0] * (Xo[i] - x) + P[1] * (Yo[i] - y) + P[2] * (Zo[i] - z);
				y_c[i] = Q[0] * (Xo[i] - x) + Q[1] * (Yo[i] - y) + Q[2] * (Zo[i] - z);
				z_c[i] = R[0] * (Xo[i] - x) + R[1] * (Yo[i] - y) + R[2] * (Zo[i] - z);
			}

			// This next statement will draw a line perpendicular to the terrain
			drawPerspLine(x_c[0], y_c[0], -z_c[0],
							x_c[4], y_c[4], -z_c[4], color);

			if (xi >= 0 && yi >= 0)
			{
				// Triangolo 1 / Triangle 1
				drawTexturedTriangle(x_c[0], y_c[0], z_c[0],
									 x_c[1], y_c[1], z_c[1],
									 x_c[2], y_c[2], z_c[2],
									 gloTexIds[gloTerrain.map_texture_indexes[xi][yi]], texcoords_gnd_tri1,
									 color);

				// Triangolo 2 / Triangle 2 (change color a little bit)
				color[1] = color[1] + 0.1; // draw with slightly different color... 

				drawTexturedTriangle(x_c[1], y_c[1], z_c[1],
									 x_c[2], y_c[2], z_c[2],
									 x_c[3], y_c[3], z_c[3],
									 gloTexIds[gloTerrain.map_texture_indexes[xi][yi]], texcoords_gnd_tri2,
									 color);
			}
		}
	}
} // end function drawTerrain

// ####################################################################################################################
// Function loadTerrainTextures
// ####################################################################################################################
void loadTerrainTextures()
{
	static int texture_generated = 0; // at first call of this function, a 32x32 texture sample will be generated 

	//	Create texture
// maximal values, if possible don't exploit maximums
#define txtWidth 128
#define txtHeight 128

	int txtRES = 128; // A reasonable texture resolution

	static GLubyte txt1[txtHeight][txtWidth][3];

#ifdef GL_VERSION_1_1
	static GLuint texName;
#endif

	int j, i, k;

	if (texture_generated == 0)
	{
		texture_generated = 1;

		for (j = 0; j < txtHeight; j++)
		{
			for (i = 0; i < txtWidth; i++)
			{
				for (k = 0; k < 3; k++)
				{
					txt1[j][i][k] = (GLubyte)0.5 * 255.0 + (double)255.0 * rand() / (double)RAND_MAX;
				}
			}
		}

		//=================GROUND TEXTURE PERSONALISED...=====================
		int texn = 1;

		while (texn > 0)
		{
			char filename[100];
			SDL_Surface *image; // This pointer will reference our bitmap.
			int bytes_per_color, imhe;
			Uint8 red, green, blue;
			Uint32 color_to_convert;

			bytes_per_color = COLDEPTH / 8;

			sprintf(filename, "textures/terrain_texture_%i.bmp", texn);
			printf("TRYING TO OPEN FILE: %s", filename);

			image = SDL_LoadBMP(filename);

			imhe = 128;
			if (image != NULL)
			{
				printf("bitmap found: %s\n", filename);

				imhe = image->h;
				txtRES = imhe; // set texture resolution, txt must be square!!!
				SDL_Delay(5);

				// feed into 'the' array used for this
				for (j = 0; j < txtRES; j++)
				{ // vertical
					for (i = 0; i < txtRES; i++)
					{ // horizontal
						color_to_convert = getPixel(image, i, txtRES - 1 - j);
						SDL_GetRGB(color_to_convert, image->format, &red, &green, &blue);

						txt1[j][i][0] = (GLubyte)red;
						txt1[j][i][1] = (GLubyte)green;
						txt1[j][i][2] = (GLubyte)blue;
					}
				}

				// Release the surface
				SDL_FreeSurface(image);

				texName = texn;

				//---------| TEXTURE PROCESSING |-----THIS PART MUST BE EXECUTED ONLY ONCE!!!
				// Otherwise it silently overloads memory at each call

				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

				glGenTextures(1, &texName);

				gloTexIds[texn - 1] = (int)texName; // [texn-1] because started fron 1, be careful

				printf("teName %i\n", gloTexIds[texn - 1]);

				glBindTexture(GL_TEXTURE_2D, gloTexIds[texn - 1]); // [texn-1] because startd fron 1, be careful

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // what OpgnGL should do when texture is magnified GL_NEAREST: non-smoothed texture | GL_LINEAR: smoothed
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);	// ...when texture is miniaturized because far; GL_NEAREST: non-smoothed tecture

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB4, txtRES, txtRES, 0, GL_RGB, GL_UNSIGNED_BYTE, txt1);
				glGenerateMipmap(GL_TEXTURE_2D);

				glEnable(GL_TEXTURE_2D);
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL); // the GL_DECAL option draws texture as is: no color mixing thigs. GL_MODULATE lets mixing.

				glBindTexture(GL_TEXTURE_2D, texName);
				//--------------------------| END OF TEXTURE LOAD PROCESSING |-------------------------------

				texn++;	// augment count... next texture
				gloTexturesAvailable = texn; // at left is extern... you know... .
			}
			else
			{
				printf("File opening error ocurred. Using random generated texture.\n");
				printf("SDL_GetError() notify: %s\n", SDL_GetError());

				texn = -1; // cause exiting from while loop.
			}
		}
	}
} // end loadTerrainTextures function

// ####################################################################################################################
// Function loadHeightMap populates field shmap of structure gloTerrain.
// ####################################################################################################################
int loadHeightMap(char *filename)
{
	int i, j, isz;
	SDL_Surface *image; // This pointer will reference our bitmap.
	Uint8 red, green, blue;
	Uint32 color_to_convert;

	isz = TERRAIN_SIZE;

	// generate default map... so if no hmap image file, one can start editing from 0 and save a map later.
	for (j = 0; j < TERRAIN_SIZE; j++)
	{
		for (i = 0; i < TERRAIN_SIZE; i++)
		{
			gloTerrain.shmap[j][i] = (float)rand() / (float)RAND_MAX;
		}
	}

	printf("TRYING TO OPEN FILE: %s\n", filename);

	image = SDL_LoadBMP(filename);

	if (image != 0)
	{
		isz = image->h;

		for (j = 0; j < TERRAIN_SIZE; j++)
		{
			for (i = 0; i < TERRAIN_SIZE; i++)
			{
				color_to_convert = getPixel(image, i, TERRAIN_SIZE - 1 - j);
				SDL_GetRGB(color_to_convert, image->format, &red, &green, &blue);

				gloTerrain.shmap[i][j] = ((float)red + 256.0 * ((float)green)) / (256.0); // simplified way
			}
		}
		printf("HEIGHTMAP LOADED FROM FILE: %s\n", filename);
	}

	// Release the surface
	SDL_FreeSurface(image);

	return isz;
} // end loadHeightMap function

// ####################################################################################################################
// Function loadMapTextureIndices
// load texture ID map from a bitmap deviced by an editor (or with a graphics editor program, but 
// that would be RATHER UNPRACTICAL... )
// ####################################################################################################################
int loadMapTextureIndices(char *filename)
{
	Uint8 red, green, blue;
	Uint32 color_to_convert;
	SDL_Surface *sdl_image;
	int j, i;

	sdl_image = SDL_LoadBMP(filename);

	if (sdl_image != NULL)
	{
		printf("file %s found\n", filename);
		printf("ROUTINE CHECK: this MUST be 300... correct? ==> %i\n", sdl_image->h);

		SDL_Delay(600);

		// feed into 'the' array used for this
		for (j = 0; j < 300; j++)
		{ // vertical
			for (i = 0; i < 300; i++)
			{ // horizontal
				color_to_convert = getPixel(sdl_image, i, j);
				SDL_GetRGB(color_to_convert, sdl_image->format, &red, &green, &blue);

				gloTerrain.map_texture_indexes[i][299 - j] = -0 + (int)red + ((int)green) * 256; 
				if (gloTerrain.map_texture_indexes[i][299 - j] >= gloTexturesAvailable)
				{
					// if index is superior to the number of total loaded textures, put it to some 
					// default number within the number of available textures.
					gloTerrain.map_texture_indexes[i][299 - j] = 0; // default.
				}

				if (i < 14 && i < 22)
				{
					printf("%2i|", gloTerrain.map_texture_indexes[i][299 - j]);
				}
			}
			printf("\n");
		}

		// Release the surface
		SDL_FreeSurface(sdl_image);
	}
	else
	{
		printf("file 'terrain_data/maptex_300x300.bmp' was not found... USING RANDOM-GENERATED TEXTURE IDS... ALL WILL LOOK AWFUL...\n\n");
		SDL_Delay(500);
	}

	return 1;
} // end loadMapTextureIndices function

// ####################################################################################################################
// Function getTerrainHeight
// ####################################################################################################################
double getTerrainHeight(struct subterrain *ite, double x, double z)
{
	double apl, bpl, cpl, dpl, Xtri, Ytri, Ztri, dist_fp1, dist_fp2, Xf, Zf;
	int Xi, Yi, col;
	double y;
	float vector0[3], vector1[3], s_nloc[3], lenght;

	// index of which square's region it is within.
	Xi = floor(x / ite[0].GPunit); // x axis (in/out-screen)
	Yi = floor(z / ite[0].GPunit); // z axis (right/left-of-screen)

	Xf = x / ite[0].GPunit;
	Zf = z / ite[0].GPunit;

	// a cautional correction to avoid SegmentationFault: SECURE.
	// if Xi accidentally goes out of range ...
	if (Xi < 0 || Xi > ite[0].map_size)
	{  
		// set it to zero, but under that there is no terrain!! 
		Xi = 0;
	}
	// if Yi accidentally goes out of range ...
	if (Yi < 0 || Yi > ite[0].map_size)
	{ 
		// set it to zero, but under that there is no terrain!! 
		Yi = 0;
	}

	dist_fp1 = sqrt(pow(Xf - Xi, 2) + pow(Zf - Yi, 2));
	dist_fp2 = sqrt(pow(Xf - (Xi + 1), 2) + pow(Zf - (Yi + 1), 2));
	// reusing Xi and Yi...:
	if (dist_fp1 < dist_fp2)
	{			 // careful if negative or positive the Z axis... in fact.
		col = 0; // lower triangle region.
	}
	else
	{
		col = 1;
	}

	// THIS IS TRIANGLE 1 , BUT WE MUST ALSO IMPLEMENT THAT IT SEES IF TRI_1 OR TRI_2
	// vect1.component-by component.
	if (col == 0)
	{
		vector0[0] = 1.0; // pick from the on-purpose triangle storer....
		vector0[1] = ite[0].shmap[Xi + 1][Yi] - ite[0].shmap[Xi][Yi];
		vector0[2] = 0.0;

		vector1[0] = 0.0; // pick from the on-purpose triangle storer....
		vector1[1] = ite[0].shmap[Xi][Yi + 1] - ite[0].shmap[Xi][Yi];
		vector1[2] = 1.0;
	}
	else if (col == 1)
	{
		vector0[0] = -1.0; // pick from the on-purpose triangle storer....
		vector0[1] = ite[0].shmap[Xi][Yi + 1] - ite[0].shmap[Xi + 1][Yi + 1];
		vector0[2] = 0.0;

		vector1[0] = 0.0; // pick from the on-purpose triangle storer....
		vector1[1] = ite[0].shmap[Xi + 1][Yi] - ite[0].shmap[Xi + 1][Yi + 1];
		vector1[2] = -1.0;
	}

	// we do directly the cross product 
	s_nloc[0] = vector0[1] * vector1[2] - vector0[2] * vector1[1]; // IMPLEMENT IT WARNING
	s_nloc[1] = vector0[2] * vector1[0] - vector0[0] * vector1[2]; //              WARNING
	s_nloc[2] = vector0[0] * vector1[1] - vector0[1] * vector1[0]; //              WARNING

	// ------------calculate apl, bpl and cpl on the fly---------------------- 
	// use cross_product method. 
	// put equation parameters.
	apl = s_nloc[0];
	bpl = s_nloc[1];
	cpl = s_nloc[2];
	dpl = 0; // this is still to calculate... it's not = 1  usually.

	// now be very careful because one should use a point which is common to both 
	// TRI 1 (col == 0 ) and TRI 2: Xi,Yi IS NOT such a point
	Xtri = (double)(Xi + 1) * ite[0].GPunit;
	Ytri = (double)ite[0].GPunit * ite[0].shmap[Xi + 1][Yi];
	Ztri = (double)Yi * ite[0].GPunit;

	// now finally calculate dpl , the 'd' of the   ax + by + cz + d = 0   plane equation.
	dpl = -apl * Xtri - bpl * Ytri - cpl * Ztri;

	// now set height and that's it. 
	y = -(		   //is negative! check equation members always!!
		(apl * x + //(ax +
		 cpl * z + // cx +
		 dpl) /
		bpl // d )/b
	);

	// additional stuff( NOT part of previous procedures) 
	lenght = sqrt(pow(s_nloc[0], 2) + pow(s_nloc[1], 2) + pow(s_nloc[2], 2));

	// normalize: this vector must be unit-length
	s_nloc[0] = s_nloc[0] / lenght;
	s_nloc[1] = s_nloc[1] / lenght;
	s_nloc[2] = s_nloc[2] / lenght;

	// look if verse is good... not always; must check if the y of the normal is 
	// positive... easy. if negative, invert vector.
	if (s_nloc[1] < 0.0)
	{ 
		s_nloc[0] = -s_nloc[0];
		s_nloc[1] = -s_nloc[1];
		s_nloc[2] = -s_nloc[2];
	}

	// copy here... very very useful to have such an auxiliary variable 
	// WARNING standard notation... z points 'upwards' 
	ite[0].auxnormal[0] = s_nloc[0];
	ite[0].auxnormal[1] = s_nloc[2];
	ite[0].auxnormal[2] = s_nloc[1];

	return y;
} // end getTerrainHeight function

// ####################################################################################################################
// Function waitMs is a timer function.
// A timer function to pause to regulate FPS is a good utility to have...
// ####################################################################################################################
int waitMs(double tt_ms)
{
	// declare variables used specifically to measure time and a normal double 
	clock_t time1, time2;
	double dt_ms = 0;

	// set the variables according to the time-measurement process 
	time1 = clock(); // request processor time, or local time, no problem which

	// WAIT: holds the program execution here until tt_sec passes. 
	while (dt_ms < tt_ms)
	{ 
		time2 = clock();
		dt_ms = (time2 - time1) / (CLOCKS_PER_SEC / 1000.0);
	}
	return 1;
} // end waitMs function 