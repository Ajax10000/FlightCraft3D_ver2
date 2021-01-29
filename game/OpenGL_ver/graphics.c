#include <math.h>            // for cos(), sin()

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <GL/glcorearb.h>

#include "globals.h"         // for the global variables
#include "graphics.h"        // for our prototypes
#include "terrain.h"         // for getTerrainHeight()

// ####################################################################################################################
// Function getPixel
// If you want to understand all color and pixel info in SDL, go to http://sdl.beuc.net/sdl.wiki/Pixel_Access
// ####################################################################################################################
Uint32 getPixel(SDL_Surface *surface, int x, int y)
{
	int bpp = surface->format->BytesPerPixel;
	// Here p is the address to the pixel we want to retrieve 
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch (bpp)
	{
	case 1:
		return *p;
		break;

	case 2:
		return *(Uint16 *)p;
		break;

	case 3: 
		if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
			return p[0] << 16 | p[1] << 8 | p[2];
		else
			return p[0] | p[1] << 8 | p[2] << 16;
		break;

	case 4:
		return *(Uint32 *)p;
		break;

	default:
		return 0; // shouldn't happen, but avoids warnings 
	}
} // end getPixel function

// ####################################################################################################################
// Function clearScreen
// ####################################################################################################################
void clearScreen(int xlimit, int ylimit)
{
	GLdouble fW, fH;
	double aspect;

	// set GL stuff
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// reset all values to 0 
	glClearColor(0.4, 0.4, 0.8, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	aspect = (double)640 / (double)480;
	fH = tan(MAG / 360 * pi) * 0.1;
	fW = fH * aspect;
	glFrustum(-fW, fW, -fH, fH, 0.1, 100000.0);

	glViewport(0, 0, xlimit, ylimit);
} // end clearScreen function

// ####################################################################################################################
// Function sdldisplay
// ####################################################################################################################
void sdldisplay() 
{
	SDL_GL_SwapWindow(gloWindow);
} // end sdldisplay function

// ####################################################################################################################
// Function drawPerspPoint draws a (perspective) point in 3D space.
//
// now we define the function which, given 1 point in 3D, calculates where it ends up on the
// virtual camera pointing toward positive z-axis and passes them to the failsafe pixel drawing function. 
// ####################################################################################################################
void drawPerspPoint(float x1, float y1, float z1, float color[3])
{
	glColor3f(color[0], color[1], color[2]);
	glPointSize(2);

	glBegin(GL_POINTS);
		glVertex3f(x1, y1, -z1);
	glEnd();

	glFlush();
} // end drawPerspPoint function

// ####################################################################################################################
// Function drawPerspLine draws a (perspective) line in 3D space.
//
// now we define the function which, given 2 points in 3D, calculates where they end up on the
// virtual camera pointing toward positive z-axis and passes them to the 2D line drawing function. 
// ####################################################################################################################
void drawPerspLine(float x1, float y1, float z1, 
				   float x2, float y2, float z2, float color[3])
{
	glColor3f(color[0], color[1], color[2]);

	glBegin(GL_LINES);
		glVertex3f(x1, y1, -z1);
		glVertex3f(x2, y2, -z2);
	glEnd();

	glFlush();
} // end drawPerspLine function

// ####################################################################################################################
// Function drawFilledPerspTriangle draws a (perspective) triangle in 3D space.
//
// now we define the function which, given 3 points in 3D, calculates where they end up on the
// virtual camera pointing toward positive z-s and passes them to the 3D line drawing function. 
// ####################################################################################################################
void drawFilledPerspTriangle(float x1, float y1, float z1,
							 float x2, float y2, float z2,
							 float x3, float y3, float z3,
							 float color[4])
{
	glColor4f(color[0], color[1], color[2], color[3]);

	glBegin(GL_TRIANGLES);
		glVertex3f(x1, y1, z1);
		glVertex3f(x2, y2, z2);
		glVertex3f(x3, y3, z3);
	glEnd();

	glFlush();
} // end drawFilledPerspTriangle function

// ####################################################################################################################
// Function drawTexturedTriangle
//
// Called only from function drawTerrain.
// ####################################################################################################################
void drawTexturedTriangle(float x1, float y1, float z1,
						  float x2, float y2, float z2,
						  float x3, float y3, float z3,
						  int texId, float texcoords[3][2],
						  float color[3])
{
	glBindTexture(GL_TEXTURE_2D, texId);
	glEnable(GL_TEXTURE_2D); 
	glColor3f(color[0], color[1], color[2]);

	glBegin(GL_TRIANGLES);
		glTexCoord2f(texcoords[0][0], texcoords[0][1]);
		glVertex3f(x1, y1, z1);

		glTexCoord2f(texcoords[1][0], texcoords[1][1]);
		glVertex3f(x2, y2, z2);

		glTexCoord2f(texcoords[2][0], texcoords[2][1]);
		glVertex3f(x3, y3, z3);
	glEnd();

	glFlush();
	glDisable(GL_TEXTURE_2D);
} // end drawTexturedTriangle function

// ####################################################################################################################
// Function drawLogo draws the logo.
// ####################################################################################################################
void drawLogo()
{
	int i, j;

	glColor3f(1.0, 1.0, 1.0);
	glPointSize(2);

	for (j = 0; j < 8; j++)
	{
		for (i = 0; i < 120; i++)
		{
			if (gloLogo[7 - j][i] > 0)
			{
				glBegin(GL_POINTS);
					glVertex3f(0.012 * i + 0.14, 0.012 * j - 1, -2.0);
				glEnd();
			}
		}
	}
} // end drawLogo function 

// ####################################################################################################################
// Function drawAxes
// ####################################################################################################################
void drawAxes()
{
	float color[4] = {0.0, 0.0, 0.0, 1.0};

	// test asse perpendiclare allo 'schermo' grafica 3D
	// test axis perpendicular to the 3D graphics 'screen'
	
	// IMPORTANT NOTE: 2 meters far from camera virtual 'lens' along it perpendiculat to it 
	// towards screen, axes are each 1 meter long. use in an intelligent way the measures... 
	// in these cases, go use unit-length references. It's the obvious choice but saying it is not bad.

	// x axis should be drawn in red
	color[0] = 1.0;
	color[1] = 0.0;
	color[2] = 0.0;

	drawPerspLine(-1.0, -1.0, 2.0,
				   0.0, -1.0, 2.0, color);

	// y axis should be drawn in green 
	color[0] = 0.0;
	color[1] = 1.0;
	color[2] = 0.0;

	drawPerspLine(-1.0, -1.0, 2.0,
				  -1.0,  0.0, 2.0, color);

	// z axis should be drawn in blue
	color[0] = 0.0;
	color[1] = 0.0;
	color[2] = 1.0;

	drawPerspLine(-1.0, -1.0, 2.0,
				  -1.0, -1.0, 3.0, color);
} // end drawAxes function

// ####################################################################################################################
// Function updatePQRAxes
// ####################################################################################################################
void updatePQRAxes(float theta, float fi)
{
	// scenario inquadrature 3D / 3D shot scenery
	if (gloView == 1)
	{
		float sinTheta = sin(theta);
		float cosTheta = cos(theta);
		float sinFi = sin(fi);
		float cosFi = cos(fi);

		P[0] = -sinTheta;
		P[1] = cosTheta;
		P[2] = 0.0;

		Q[0] = -cosTheta * cosFi;
		Q[1] = -sinTheta * cosFi;
		Q[2] = sinFi;

		R[0] = cosTheta * sinFi;
		R[1] = sinTheta * sinFi;
		R[2] = cosFi;
	}
	else if (gloView == 2)
	{
		// Above the plane, looking down at the top of the plane.
		P[0] = -Pa[0]; 
		P[1] = -Pa[1];
		P[2] = -Pa[2];

		Q[0] = Ra[0];
		Q[1] = Ra[1];
		Q[2] = Ra[2];

		R[0] = Qa[0];
		R[1] = Qa[1];
		R[2] = Qa[2];
	}
	else if (gloView == 3)
	{
		// Looking at right side of airplane
		P[0] = Pa[0]; 
		P[1] = Pa[1];
		P[2] = Pa[2];

		R[0] = Ra[0];
		R[1] = Ra[1];
		R[2] = Ra[2];

		Q[0] = Qa[0];
		Q[1] = Qa[1];
		Q[2] = Qa[2];
	}
	else if (gloView == 4)
	{ 
		// Internal view from cockpit
		float Pp[3], Qp[3], Rp[3];

		Rp[0] = -Pa[0]; // had to be mirrored sorry
		Rp[1] = -Pa[1];
		Rp[2] = -Pa[2];

		Pp[0] = Ra[0];
		Pp[1] = Ra[1];
		Pp[2] = Ra[2];

		Qp[0] = Qa[0];
		Qp[1] = Qa[1];
		Qp[2] = Qa[2];

		float sinTheta = sin(theta);
		float cosTheta = cos(theta);
		float sinFi = sin(fi);
		float cosFi = cos(fi);

		// Ri, Pi, Qi - this is most practical axis-order.
		P[0] = Rp[0] * (-sinTheta)          +  Pp[0] * (cosTheta); //       +  Qp[0] * 0.0;
		P[1] = Rp[1] * (-sinTheta)          +  Pp[1] * (cosTheta); //       +  Qp[1] * 0.0;
		P[2] = Rp[2] * (-sinTheta)          +  Pp[2] * (cosTheta); //       +  Qp[2] * 0.0;

		Q[0] = Rp[0] * (-cosTheta * cosFi)  +  Pp[0] * (-sinTheta * cosFi)  +  Qp[0] * sinFi;
		Q[1] = Rp[1] * (-cosTheta * cosFi)  +  Pp[1] * (-sinTheta * cosFi)  +  Qp[1] * sinFi;
		Q[2] = Rp[2] * (-cosTheta * cosFi)  +  Pp[2] * (-sinTheta * cosFi)  +  Qp[2] * sinFi;

		R[0] = Rp[0] * (cosTheta * sinFi)   +  Pp[0] * (sinTheta * sinFi)   +  Qp[0] * cosFi;
		R[1] = Rp[1] * (cosTheta * sinFi)   +  Pp[1] * (sinTheta * sinFi)   +  Qp[1] * cosFi;
		R[2] = Rp[2] * (cosTheta * sinFi)   +  Pp[2] * (sinTheta * sinFi)   +  Qp[2] * cosFi;
	}
} // end updatePQRAxes function

// ####################################################################################################################
// Function reorientAxes
// ####################################################################################################################
void reorientAxes()
{
    // Calculate axes in their new 'orientation', using the orientation matrix.

    gloAxis1[0] = Rm[0][0] * gloOrigAxis1[0]  +  Rm[0][1] * gloOrigAxis1[1]  +  Rm[0][2] * gloOrigAxis1[2];
    gloAxis1[1] = Rm[1][0] * gloOrigAxis1[0]  +  Rm[1][1] * gloOrigAxis1[1]  +  Rm[1][2] * gloOrigAxis1[2];
    gloAxis1[2] = Rm[2][0] * gloOrigAxis1[0]  +  Rm[2][1] * gloOrigAxis1[1]  +  Rm[2][2] * gloOrigAxis1[2];

    gloAxis2[0] = Rm[0][0] * gloOrigAxis2[0]  +  Rm[0][1] * gloOrigAxis2[1]  +  Rm[0][2] * gloOrigAxis2[2]; 
    gloAxis2[1] = Rm[1][0] * gloOrigAxis2[0]  +  Rm[1][1] * gloOrigAxis2[1]  +  Rm[1][2] * gloOrigAxis2[2];
    gloAxis2[2] = Rm[2][0] * gloOrigAxis2[0]  +  Rm[2][1] * gloOrigAxis2[1]  +  Rm[2][2] * gloOrigAxis2[2];

    gloAxis3[0] = Rm[0][0] * gloOrigAxis3[0]  +  Rm[0][1] * gloOrigAxis3[1]  +  Rm[0][2] * gloOrigAxis3[2];
    gloAxis3[1] = Rm[1][0] * gloOrigAxis3[0]  +  Rm[1][1] * gloOrigAxis3[1]  +  Rm[1][2] * gloOrigAxis3[2];
    gloAxis3[2] = Rm[2][0] * gloOrigAxis3[0]  +  Rm[2][1] * gloOrigAxis3[1]  +  Rm[2][2] * gloOrigAxis3[2];

    Pa[0] = gloAxis1[0];
    Pa[1] = gloAxis1[1];
    Pa[2] = gloAxis1[2];

    Qa[0] = gloAxis2[0];
    Qa[1] = gloAxis2[1];
    Qa[2] = gloAxis2[2];

    Ra[0] = gloAxis3[0];
    Ra[1] = gloAxis3[1];
    Ra[2] = gloAxis3[2];
} // end reorientAxes function

// ####################################################################################################################
// Function updateVirtualCameraPos updates the virtual camera position x, y, z.
// 
// Parameters:
// RR - provides a zoom factor. 
//      When the user presses 1, the user's distance to the plane becomes shorter (zoomFactor = zoomFactor - 2.0), 
//      and so the user zooms in to the plane's CM.
//      When the user presses 2, the user's distance to the plane becomes longer (zoomFactor = zoomFactor + 2.0), 
//      and so the user zooms out from the plane's CM.
// ####################################################################################################################
void updateVirtualCameraPos(float zoomFactor)
{
	// moving plane's CM 
	if (aboard == 1)
	{
		x = xp + zoomFactor * R[0];
		y = yp + zoomFactor * R[1];
		z = zp + zoomFactor * R[2];

		if (gloView == 4)
		{ 
			// Internal view from cockpit
			// now: here the camera position is very important.
			x = xp + x_cockpit_view * P[0]  +  y_cockpit_view * Q[0]  +  z_cockpit_view * R[0];
			y = yp + x_cockpit_view * P[1]  +  y_cockpit_view * Q[1]  +  z_cockpit_view * R[1];
			z = zp + x_cockpit_view * P[2]  +  y_cockpit_view * Q[2]  +  z_cockpit_view * R[2];
		}
	}
	else
	{
		x = x_pilot;
		y = y_pilot;
		z = getTerrainHeight(&gloTerrain, x_pilot, y_pilot) + 1.75;
	}
} // end updateVirtualCameraPos function