#include <stdio.h>           // for fopen(), fclose(), fscanf(), printf()
#include <stdlib.h>          // for rand()

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <GL/glcorearb.h>

#include "globals.h"         // for the global variables
#include "graphics.h"        // for draw... functions
#include "airplane.h"        // for our prototypes
#include "physics.h"         // for bounceAirplane()
#include "specialEffects.h"  // for addExplosionAtPoint(), launchProjectiles()
#include "terrain.h"         // for getTerrainHeight()

// ####################################################################################################################
// Function initPoints
// ####################################################################################################################
void initPoints()
{
	int i;

	float tmp[NVERTEXES][3];
	for (i = 0; i < NVERTEXES; i++)
	{
		tmp[i][0] = 0.01 * gloPunti[i][0]; // from centimeters to meters (x/100)
		tmp[i][1] = 0.01 * gloPunti[i][1];
		tmp[i][2] = 0.01 * gloPunti[i][2];
	}

	for (i = 0; i < NVERTEXES; i++)
	{
		gloPunti[i][0] = tmp[i][0];
		gloPunti[i][1] = tmp[i][1];
		gloPunti[i][2] = tmp[i][2];
	} 
} // end initPoints function

// ####################################################################################################################
// Function initAirplaneColors initializes the airplane colors to random numbers.
// ####################################################################################################################
void initAirplaneColors()
{
	int i;

	// Note that when defined, col_tris is defined with 20 elements, indexed from 0 to 19
	// airplane random colors 
	for (i = 18; i < NTRIS; i++)
	{
		col_tris[i][0] = (float)0.3 * rand() / (float)RAND_MAX;
		col_tris[i][1] = (float)0.3 * rand() / (float)RAND_MAX;
		col_tris[i][2] = (float)0.9 * rand() / (float)RAND_MAX;
	}
} // end initAirplaneColors function

// ####################################################################################################################
// Function loadAirplaneModel
// ####################################################################################################################
void loadAirplaneModel()
{
	printf("TRYING TO IMPORT VERTEX LIST OF 3D MODEL\n");
	FILE *FilePtr; // pointer to input file 

	FilePtr = fopen("input/vertexes.txt", "r");
	if (FilePtr < 0)
	{
		printf("NO FILE TO IMPORT VERTEX LIST...USING DEFAULT...\n");
		fclose(FilePtr);
	}
	else
	{
		importAirplanePolyhedron();
	}
} // end loadAirplaneModel function

// ####################################################################################################################
// Function importAirplanePolyhedron
// ####################################################################################################################
void importAirplanePolyhedron(void)
{
	// read in poly definition from file
	float auxxv[3 * NVERTEXES];
	int nelem = 0, i, j;

	printf("TRYING TO IMPORT VERTEX LIST OF 3D MODEL\n");

	nelem = countNumbersInFile("input/vertexes.txt");
	getFloatsInFile("input/vertexes.txt", auxxv, nelem); // read file and values in the auxxv array.

	// feed into 'the' array used for this
	for (j = 0; j < nelem / 3; j++)
	{
		for (i = 0; i < 3; i++)
		{
			gloPunti[j][i] = 2.4 * auxxv[j * 3 + i];
		}
	}
	nvertexes = nelem / 3;

	printf("TRYING TO IMPORT TRIANGULATION OF 3D MODEL\n");

	nelem = countNumbersInFile("input/triangulation.txt");
	getFloatsInFile("input/triangulation.txt", auxxv, nelem); // read file and values into the auxxv array.

	// feed into 'the' array used for this
	for (j = 0; j < nelem / 3; j++)
	{
		for (i = 0; i < 3; i++)
		{
			tris[j][i] = (int)auxxv[j * 3 + i];
		}
	}
	ntris = nelem / 3;

	printf("TRYING TO IMPORT TRIANGULATION OF 3D MODEL\n");

	nelem = countNumbersInFile("input/facecolor.txt");
	getFloatsInFile("input/facecolor.txt", auxxv, nelem); // read file and values into the auxxv array.

	// feed into 'the' array used for this
	for (j = 0; j < nelem / 3; j++)
	{
		for (i = 0; i < 3; i++)
		{
			col_tris[j][i] = auxxv[j * 3 + i];
		}
	}
} // end importAirplanePolyhedron function

// ####################################################################################################################
// Function countNumbersInFile
// leggi file e controlla quanti numeri ci sono / read file and check how many numbers there are
// N.B.: numeri separati da spazi o da a-capo. Con virgole o altro si blocca. 
// N.B.: numbers separated by spaces or by a-line. With commas or other it blocks.
// ####################################################################################################################
long int countNumbersInFile(char filename[])
{
	FILE *filePtr; // pointer to input file
	filePtr = fopen(filename, "r");
	long int i = 0;
	float test;

	while (fscanf(filePtr, "%f", &test) != EOF)
	{
		i++;
		printf("%f  \n", test);
	}

	fclose(filePtr); 
	return i;
} // end countNumbersInFile function

// ####################################################################################################################
// Function getFloatsInFile
// Read in numeric vector from file
// only space-separated or newline-separated numbers!! else goes error
// ####################################################################################################################
void getFloatsInFile(char filename[], float floatsRead[], long int maxsize)
{
	FILE *filePtr; // pointer to input file 
	filePtr = fopen(filename, "r");
	long int i = 0; 

	while (fscanf(filePtr, "%f", &floatsRead[i]) != EOF && i < maxsize)
	{
		i++; 
		printf("%f . \n", floatsRead[i]);
	}
	fclose(filePtr); 
	printf("\nFILE FOUND & READ IN. LENGTH LIMIT WAS FIXED TO: %li . \n", maxsize);
} // end getFloatsInFile function

// ####################################################################################################################
// Function checkForPlaneCollision checks if the airplane has crashed against the ground, and if so, 
// models its bounce off of the ground.
// ####################################################################################################################
void checkForPlaneCollision()
{
	int i;

	// Detecting and resolving collision with ground
	// simplified collision with ground
	for (i = 0; i < NVERTEXES; i++)
	{
		// AIRPLANE...
		// coordinates of plane's vertices in the "Game World"'s reference frame.
		double xw, yw, zw; 
		xw = gloPunti[i][0] * gloAxis1[0] + gloPunti[i][1] * gloAxis2[0] + gloPunti[i][2] * gloAxis3[0];
		yw = gloPunti[i][0] * gloAxis1[1] + gloPunti[i][1] * gloAxis2[1] + gloPunti[i][2] * gloAxis3[1];
		zw = gloPunti[i][0] * gloAxis1[2] + gloPunti[i][1] * gloAxis2[2] + gloPunti[i][2] * gloAxis3[2];

		double he_id = getTerrainHeight(&gloTerrain, xp + xw, yp + yw);
		if (zp + zw < he_id)
		{ 
			// just as any vertex of airplane touches ground and tries to go below 
			bounceAirplane(xw, yw, zw, gloTerrain.auxnormal[0], gloTerrain.auxnormal[1], gloTerrain.auxnormal[2], 0.06);
			printf("TOUCH GND \n");
			zp = zp + (he_id - zp - zw);
			// check is normal < 0 and eventually calculated and assigns new velocities and so.
		}
	}
} // end checkForPlaneCollision function

// ####################################################################################################################
// Function drawAirplane
// ####################################################################################################################
void drawAirplane(float h) 
{
	int i;
	// Le coordinate dei punti della pista, nel sistema di riferimento del paracadutista
	// The coordinates of the runway points, in the parachutist's reference system
	float x1, y1, z1, x2, y2, z2; 
	float x1a, y1a, z1a;
	float x2a, y2a, z2a;
	float x3a, y3a, z3a;
	float x3, y3, z3;
	float color[4] = {0.0, 0.0, 0.0, 1.0};

	// Disegna solo lo aereo / Draw only the plane
	color[0] = 1.0;
	color[1] = 1.0;
	color[2] = 1.0;

	// For each triangular facet defined ...
	for (i = 0; i < ntris; i++)
	{ // AIRPLANE...
		// Set x1a, y1a, z1a to the first point in the triangular facet
		x1a = Pa[0] * gloPunti[tris[i][0]][0] + Qa[0] * gloPunti[tris[i][0]][1] + Ra[0] * gloPunti[tris[i][0]][2];
		y1a = Pa[1] * gloPunti[tris[i][0]][0] + Qa[1] * gloPunti[tris[i][0]][1] + Ra[1] * gloPunti[tris[i][0]][2];
		z1a = Pa[2] * gloPunti[tris[i][0]][0] + Qa[2] * gloPunti[tris[i][0]][1] + Ra[2] * gloPunti[tris[i][0]][2];

		// Set x2a, y2a, z2a to the second point in the triangular facet
		x2a = Pa[0] * gloPunti[tris[i][1]][0] + Qa[0] * gloPunti[tris[i][1]][1] + Ra[0] * gloPunti[tris[i][1]][2];
		y2a = Pa[1] * gloPunti[tris[i][1]][0] + Qa[1] * gloPunti[tris[i][1]][1] + Ra[1] * gloPunti[tris[i][1]][2];
		z2a = Pa[2] * gloPunti[tris[i][1]][0] + Qa[2] * gloPunti[tris[i][1]][1] + Ra[2] * gloPunti[tris[i][1]][2];

		// Set x3a, y3a, z3a to the third point in the triangular facet
		x3a = Pa[0] * gloPunti[tris[i][2]][0] + Qa[0] * gloPunti[tris[i][2]][1] + Ra[0] * gloPunti[tris[i][2]][2];
		y3a = Pa[1] * gloPunti[tris[i][2]][0] + Qa[1] * gloPunti[tris[i][2]][1] + Ra[1] * gloPunti[tris[i][2]][2];
		z3a = Pa[2] * gloPunti[tris[i][2]][0] + Qa[2] * gloPunti[tris[i][2]][1] + Ra[2] * gloPunti[tris[i][2]][2];

		// estremo 1 / extreme 1
		x1 = P[0] * (x1a + xp - x) + P[1] * (y1a + yp - y) + P[2] * (z1a + zp - z);
		y1 = Q[0] * (x1a + xp - x) + Q[1] * (y1a + yp - y) + Q[2] * (z1a + zp - z);
		z1 = R[0] * (x1a + xp - x) + R[1] * (y1a + yp - y) + R[2] * (z1a + zp - z);

		// estremo 2 / extreme 2
		x2 = P[0] * (x2a + xp - x) + P[1] * (y2a + yp - y) + P[2] * (z2a + zp - z);
		y2 = Q[0] * (x2a + xp - x) + Q[1] * (y2a + yp - y) + Q[2] * (z2a + zp - z);
		z2 = R[0] * (x2a + xp - x) + R[1] * (y2a + yp - y) + R[2] * (z2a + zp - z);

		// vertex 3 
		x3 = P[0] * (x3a + xp - x) + P[1] * (y3a + yp - y) + P[2] * (z3a + zp - z);
		y3 = Q[0] * (x3a + xp - x) + Q[1] * (y3a + yp - y) + Q[2] * (z3a + zp - z);
		z3 = R[0] * (x3a + xp - x) + R[1] * (y3a + yp - y) + R[2] * (z3a + zp - z);

		// col_tris holds the airplane's colors
		color[0] = col_tris[i][0];
		color[1] = col_tris[i][1];
		color[2] = col_tris[i][2];

		// If we are not using low resolution ...
		if (gloUsingLowResolution == 0)
		{
			// Draw a perspective, filled triangle using the three points in our triangular facet
			drawFilledPerspTriangle(x1, y1, -z1,
								x2, y2, -z2,
								x3, y3, -z3, color);
		}
		else
		{
			// Draw a perspective, filled triangle using the three points in our triangular facet
			drawFilledPerspTriangle(x1, y1, -z1,
								x2, y2, -z2,
								x3, y3, -z3, color);
		}
	}

	if (gloUsingLowResolution == 0)
	{
		addExplosionAtPoint(x1, y1, z1, h, 0);	// we must make special effect sequences go on. 
		launchProjectiles(10, 10, 10, 20, 10, -0.1, h, 0); // idem / ditto
	}
} // end drawAirplane function