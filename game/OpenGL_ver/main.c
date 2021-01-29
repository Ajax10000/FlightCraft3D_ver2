#include <stdio.h>           // for fclose(), fopen(), getchar(), printf()
#include <stdlib.h>          // for exit()

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <GL/glcorearb.h>

#include "globals.h"         // for the global variables
#include "airplane.h"        // for checkForPlaneCollision(), drawAirplane(), importAirplanePolyhedron(), initPoints()
#include "graphics.h"        // for clearScreen(), drawAxes(), drawLogo(), updateVirtualCameraPos()
#include "physics.h"         // for simulatePhysics()
#include "specialEffects.h"  // for addSmokeAtPoint(), launchProjectiles()
#include "terrain.h"         // for drawTerrain(), initTerrain(), loadTerrainTextures()
#include "trees.h"           // for drawTrees(), initTrees(), loadTreeTextures()

// ####################################################################################################################
// # Function prototypes                                                                                              #
// #                                                                                                                  #

int main(void);
void initSDL(void);
void initOpenGL(void);
void displayStatusInfo(int cycles, float h, float theta, float fi, float zoomFactor);
void processEvent(float *turnch, float *turncv, float *zoomFactor, float *prevZoomFactor, 
				  int *plane_up, int *plane_down, int *plane_inclleft, int *plane_inclright, float *h);
void initData(float h);

// #                                                                                                                  #
// # End function prototypes                                                                                          #
// ####################################################################################################################


// ####################################################################################################################
// the C  main() function  
// ####################################################################################################################
int main()
{
	float turnch = 0.0; // used for smooth turning of virtual camera 
	float turncv = 0.0; // used for smooth turning of virtual camera 
	int plane_up = 0;
	int plane_down = 0;
	int plane_inclleft = 0;
	int plane_inclright = 0;
	// User can zoom in by pressing 1 key, and zoom out by pressing 2 key
	float zoomFactor = 20.0;
	float prevZoomFactor;

	// 3 angoli che indicano la orientazione del sistema di riferimento della telecaera virtuale, per dire 
	// 3 angles that indicate the orientation of the reference system of the virtual camera, so to speak
	float theta = 4.2, fi = 1.2, psi = 0.0; 

	// For internal view!!! cockpit view
	float th_add = 0.1, fi_add = 0.1, psi_add = 0.0; 

	int cycles;

	// uno step di simulazione che da' un risultato non troppo disastroso
	// a simulation step that gives a not too disastrous result
	float h = 0.01; 

	double g = -9.81; // positive +++++ assumed!!

	initSDL();

	initOpenGL();

	initData(h);

	cycles = 0;
	while (cycles < 50000)
	{
		// moving camera position and airplane interactive control 
		if (turncv == 1.0 || turncv == -1.0)
		{
			// vertical camera displacement
			fi = fi - turncv * 0.02; // rotate camera's reference system's fi angle
			fi_add = fi_add - turncv * 0.01;
		}
		if (turnch == 1.0 || turnch == -1.0)
		{
			theta = theta + turnch * 0.02; // rotate camera's reference system's theta angle 
			th_add = th_add + turnch * 0.01;
		}

		// None of the parameters passed to simulatePhysics are modified
		simulatePhysics(plane_up, plane_down, plane_inclleft, plane_inclright, h, g);

		// If the plane collides with the ground, this routine will call function bounceAirplane.
		checkForPlaneCollision();

		// Cancella schermata/lavagna
		// Clear screen/board
		clearScreen(WIDTH, HEIGHT); 

		updatePQRAxes(theta, fi);

		// Update the virtual camera position x, y, z
		updateVirtualCameraPos(zoomFactor);

		// If we are not using low resolution ...
		if (gloUsingLowResolution == 0)
		{
			// Add smoke effect at plane location xp, yp, zp
			addSmokeAtPoint(xp, yp, zp, h, 2); // DEACTIVATED TO TEST
		}

		drawAxes();

		drawLogo();

		drawTerrain();

		drawTrees();

		// If we're not using low resolution ...
		if (gloUsingLowResolution == 0)
		{
			// Why are we adding smoke effect at coordinates (x, y, z) = (1, 2, 3)?
			addSmokeAtPoint(1, 2, 3, h, 0); // we must make special effect sequences go on. 
		}

		drawAirplane(h);

		// call function which displays the matrix of pixels in a true graphics window 
		sdldisplay();

		// Every ten cycles ...
		if (cycles % 10 == 0)
		{
			// Display status/debugging information
			displayStatusInfo(cycles, h, theta, fi, zoomFactor);
		}

		SDL_Delay(10);
		cycles++;

		while (SDL_PollEvent(&gloEvent))
		{ 
			processEvent(&turnch, &turncv, &zoomFactor, &prevZoomFactor, &plane_up, &plane_down, &plane_inclleft, &plane_inclright, &h);
			
			// if graphic window is closed, terminate program 
			if (gloEvent.type == SDL_QUIT)
			{ 
				printf("GRAPHICS WINDOW CLOSED: PROGRAM TERMINATED\n");
				SDL_Quit();

				exit(1); 
			} 
		} // end of continual event-check loop. 
	}

	printf("check graphics Window!\n");
	// close graphics window to avoid abnormal terminations.
	SDL_Quit();

	getchar();

	return 0;
} // end main function

// ####################################################################################################################
// Function initSDL
// ####################################################################################################################
void initSDL()
{
	// Initialize SDL 
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		printf("Unable to initialize SDL: %s\n", SDL_GetError());
		getchar();
	}

	// Enable double-buffering
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	// Initialize the screen / window 
	gloWindow = SDL_CreateWindow("FlightCraft_3D (GL) - by Simon Hasur",
							  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
							  WIDTH, HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (gloWindow == NULL)
	{
		printf("Couldn't create window.");
		SDL_Quit();
		exit(2);
	}
	printf("Created window\n");

	// Create context
	gloContext = SDL_GL_CreateContext(gloWindow);
	if (gloContext == NULL)
	{
		printf("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
		SDL_Quit();
		exit(1);
	}

	gloScreen = SDL_GetWindowSurface(gloWindow);
	printf("Set global gloScreen variable by calling SDL_GetWindowSurface\n");

	if (!gloScreen)
	{
		printf("Couldn't set %dx%d GL video mode: %s\n", WIDTH,
			   HEIGHT, SDL_GetError());
		SDL_Quit();
		exit(2);
	}
	SDL_UpdateWindowSurface(gloWindow);
} // end initSDL function

// ####################################################################################################################
// Function initOpenGL
// ####################################################################################################################
void initOpenGL()
{
	glEnable(GL_DEPTH_TEST); // activate hidden_surface_removal in OpenGL.
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	// Set some OpenGL options for least precision, since this is not a graphics program
	// We need computer power for physics calculations
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST); // Fastest - less expensive - Perspective Calculations
	glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);			// Fastest - less expensive - Perspective Calculations

	printf("1 check graphics Window!\n");
} // end initOpenGL function

// ####################################################################################################################
// Function initData
// ####################################################################################################################
void initData(float h)
{
	initPoints();

	// initTerrain initializes global variable gloTerrain with random values
	initTerrain();

	// initAirplaneColors initializes global variable col_tris with random values
	initAirplaneColors();

	// load textures in
	loadTerrainTextures(); 
	gloTreeTextureIDBounds[0] = gloTexturesAvailable; // at what ID do tree textures begin (and end also...)

	// ditto but with "bitmap" image files with alpha value for transparency information on each pixel... 
	// this is mainly used for drawing trees in a quick, nice and simple way.
	loadTreeTextures(); 

	// this comes after, because so during loading we can check if no texture index superior to the number 
	// of available textures is inserted... .
	loadMapTextureIndices("terrain_data/maptex_300x300.bmp"); 

	// initTrees initializes global variable gloTrees
	initTrees();

	// If we're not using low resolution ...
	if (gloUsingLowResolution == 0)
	{
		// Add smoke effect at plane location xp, yp, zp
		addSmokeAtPoint(xp, yp, zp, h, 1); // commented out for testing
	}

	// initPhysicsVars will set global variables It_init, p, L, It_initINV, Fcm and gloTtlTorque
	initPhysicsVars();

	// loadAirplaneModel sets global variable gloPunti, 
	loadAirplaneModel();
} // end initData function

// ####################################################################################################################
// Function displayStatusInfo
// ####################################################################################################################
void displayStatusInfo(int cycles, float h, float theta, float fi, float zoomFactor)
{
	int i, j;

	// Display status/debugging information
	printf("GAME TIME [sec] =  %f \n", cycles * h);
	printf("GOING ON...game cycle %i : theta = %3.3f, fi = %f3.3\n", cycles, theta, fi);
	printf("Plane position: xp = %f, yp = %f, zp = %f\n", xp, yp, zp);
	printf("Virtual camera position: x = %f, y = %f, z = %f\n", x, y , z);
    printf("ZoomFactor = %f\n", zoomFactor);
	printf("View = %d\n", gloView);
	printf("x_cockpit_view, y_cockpit_view, z_cockpit_view: %f, %f, %f\n", x_cockpit_view, y_cockpit_view, z_cockpit_view);

	for (j = 0; j < 10; j++)
	{
		for (i = 0; i < 10; i++)
		{
			printf("%3i|", gloTexIds[gloTerrain.map_texture_indexes[j][i]]);
		}
		printf("\n");
	}
} // end displayStatusInfo function

// ####################################################################################################################
// Function processEvent
// ####################################################################################################################
void processEvent(float *turnch, float *turncv, float *zoomFactor, float *prevZoomFactor,
				  int *plane_up, int *plane_down, int *plane_inclleft, int *plane_inclright, float *h)
{
	// Loop until there are no events left on the queue 
	if (gloEvent.type == SDL_KEYDOWN)
	{ 
		// condition: keypress event detected 
		if (gloEvent.key.keysym.sym == SDLK_ESCAPE)
		{
			printf("ESC KEY PRESSED: PROGRAM TERMINATED\n");
			SDL_Quit();
			exit(1); 
		}

		if (gloEvent.key.keysym.sym == SDLK_c)
		{
			*turnch = -1.0; 
		}
		if (gloEvent.key.keysym.sym == SDLK_v)
		{
			*turnch = 1.0; 
		}

		if (gloEvent.key.keysym.sym == SDLK_r)
		{
			*turncv = 1.0; 
		}
		if (gloEvent.key.keysym.sym == SDLK_f)
		{
			*turncv = -1.0; 
		}

		if (gloEvent.key.keysym.sym == SDLK_9)
		{
			Pforce = Pforce + 5000.0; 
		}
		if (gloEvent.key.keysym.sym == SDLK_8)
		{
			Pforce = Pforce - 4000.0; 
			if (Pforce <= 0.0)
			{
				Pforce = 0.0;
			}
		}

		if (gloEvent.key.keysym.sym == SDLK_1)
		{
			if (*zoomFactor == 0) 
			{
				printf("Cannot zoom any more, at maximum zoom.\n");
			} else 
			{
				*zoomFactor = *zoomFactor - 2.0; 
			}
		}
		if (gloEvent.key.keysym.sym == SDLK_2)
		{
			*zoomFactor = *zoomFactor + 2.0;
		}

		// If we're in view mode 4 (i.e., looking from within the cockpit)
		if (gloView == 4)
		{
			// If user is pressing shift-x, increase x_cockpit_view
			if ((gloEvent.key.keysym.mod & KMOD_SHIFT) && (gloEvent.key.keysym.sym == SDLK_x))
			{
				x_cockpit_view = x_cockpit_view + 0.1;
				if (x_cockpit_view > 2.0)
				{
					x_cockpit_view = -2.0;
				}
			} 
			// if user is just pressing x, decrease x_cockpit_view
			else if (gloEvent.key.keysym.sym == SDLK_x)
			{
				x_cockpit_view = x_cockpit_view - 0.1;
				if (x_cockpit_view < -2.0)
				{
					x_cockpit_view = 2.0;
				}
			}

			// If user is pressing shift-y, increase y_cockpit_view
			if ((gloEvent.key.keysym.mod & KMOD_SHIFT) && (gloEvent.key.keysym.sym == SDLK_y))
			{
				y_cockpit_view = y_cockpit_view + 0.1;
				if (y_cockpit_view > 2.0)
				{
					y_cockpit_view = -2.0;
				}
			} 
			// if user is just pressing y, decrease y_cockpit_view
			else if (gloEvent.key.keysym.sym == SDLK_y)
			{
				y_cockpit_view = y_cockpit_view - 0.1;
				if (y_cockpit_view < -2.0)
				{
					y_cockpit_view = 2.0;
				}
			}

			// If user is pressing shift-z, increase z_cockpit_view
			if ((gloEvent.key.keysym.mod & KMOD_SHIFT) && (gloEvent.key.keysym.sym == SDLK_z))
			{
				z_cockpit_view = z_cockpit_view + 0.1;
				if (z_cockpit_view > 2.0)
				{
					z_cockpit_view = -2.0;
				}
			}
			// if user is just pressing z, decrease z_cockpit_view
			else if (gloEvent.key.keysym.sym == SDLK_z)
			{
				z_cockpit_view = z_cockpit_view - 0.1;
				if (z_cockpit_view < -2.0)
				{
					z_cockpit_view = 2.0;
				}
			}
		}

		if (gloEvent.key.keysym.sym == SDLK_t)
		{
			MAG = MAG + 10.0; 
		}
		if (gloEvent.key.keysym.sym == SDLK_i)
		{
			FILE *FilePtr; // pointer to input file 

			printf("TRYING TO IMPORT VERTEX LIST OF 3D MODEL\n");

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
		}

		if (gloEvent.key.keysym.sym == SDLK_e)
		{
			addExplosionAtPoint(20, 20, 20, *h, 1);
		}

		if (gloEvent.key.keysym.sym == SDLK_m)
		{
			gloUsingLowResolution = 1; // LOW GRAPHICS MODE for slow computers 
		}

		if (gloEvent.key.keysym.sym == SDLK_o)
		{
			gloView = gloView + 1; // change view
			if (gloView == 5)
			{
				gloView = 1; // restore to view 1 (normal external)
			}
		}

		if (gloEvent.key.keysym.sym == SDLK_p)
		{
			if (aboard == 1) {
				aboard = -1 * aboard;
				prev_x_pilot = x_pilot;
				prev_y_pilot = y_pilot;
				*prevZoomFactor = *zoomFactor;
				x_pilot = xp;
				y_pilot = yp;
				*zoomFactor = 1.5;
			} 
			else 
			{
				aboard = -1 * aboard;
				x_pilot = prev_x_pilot;
				y_pilot = prev_y_pilot;
				*zoomFactor = *prevZoomFactor;
			}
		}

		if (gloEvent.key.keysym.sym == SDLK_5)
		{
			*h = *h - 0.002;
		}
		if (gloEvent.key.keysym.sym == SDLK_6)
		{
			*h = *h + 0.001;
		}

		if (gloEvent.key.keysym.sym == SDLK_s)
		{
			// velocity of plane's CM + velocity of projectile... rotation ignored.
			launchProjectiles(xp, yp, zp, v[0] + 100.0 * Pa[0], v[1] + 100.0 * Pa[1], v[2] + 100.0 * Pa[2], *h, 1);
		}

		if (gloEvent.key.keysym.sym == SDLK_UP || gloEvent.key.keysym.sym == SDLK_w)
		{
			x_pilot = x_pilot - 2.0 * R[0];
			y_pilot = y_pilot - 2.0 * R[1];

			*plane_down = 1;
		}
		if (gloEvent.key.keysym.sym == SDLK_DOWN || gloEvent.key.keysym.sym == SDLK_z)
		{
			*plane_up = 1;
		}
		if (gloEvent.key.keysym.sym == SDLK_LEFT || gloEvent.key.keysym.sym == SDLK_a)
		{
			*plane_inclleft = 1;
		}
		if (gloEvent.key.keysym.sym == SDLK_RIGHT || gloEvent.key.keysym.sym == SDLK_d)
		{
			*plane_inclright = 1;
		}
	} // end condition of if-keypress-is-detected 

	if (gloEvent.type == SDL_KEYUP)
	{ 
		// condition: key-RELEASE event detected 
		if (gloEvent.key.keysym.sym == SDLK_c)
		{
			*turnch = 0.0; 
		}
		if (gloEvent.key.keysym.sym == SDLK_v)
		{
			*turnch = 0.0; 
		}

		if (gloEvent.key.keysym.sym == SDLK_r)
		{
			*turncv = 0.0; 
		}
		if (gloEvent.key.keysym.sym == SDLK_f)
		{
			*turncv = 0.0; 
		}

		if (gloEvent.key.keysym.sym == SDLK_DOWN || gloEvent.key.keysym.sym == SDLK_z)
		{
			*plane_up = 0;
		}
		if (gloEvent.key.keysym.sym == SDLK_UP || gloEvent.key.keysym.sym == SDLK_w)
		{
			*plane_down = 0;
		}

		if (gloEvent.key.keysym.sym == SDLK_LEFT || gloEvent.key.keysym.sym == SDLK_a)
		{
			*plane_inclleft = 0;
		}
		if (gloEvent.key.keysym.sym == SDLK_RIGHT || gloEvent.key.keysym.sym == SDLK_d)
		{
			*plane_inclright = 0;
		}
	}
} // end processEvent function
