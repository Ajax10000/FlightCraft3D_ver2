#ifndef FC_GLOBALS_H
#define FC_GLOBALS_H

#ifndef FC_SDL2_H
#define FC_SDL2_H
#include <SDL2/SDL.h>             // for SDL_Surface, SDL_Window, SDL_Event, SDL_GLContext
#endif 

#ifndef FC_SDL2_OPENGL_H
#define FC_SDL2_OPENGL_H
#include <SDL2/SDL_opengl.h>
#endif

#define WIDTH 480
#define HEIGHT 320
#define COLDEPTH 16

// the structure representing relevant quantities concerning game's terrain
#define TERRAIN_SIZE 300

struct subterrain
{
	// Set in function initTerrain twice. 
	// At the top of initTerrain, it is set to 300. 
	// At the bottom of initTerrain, it is set to the return value of a call to loadHeightMap.
	// Used (read) in functions drawTerrain and getTerrainHeight
	int map_size;

	// Set in function initTerrain to 50.
	// Used (read) in functions initTerrain, drawTerrain, launchProjectiles, getTerrainHeight
	float GPunit;

	// Set in function initTerrain to a random value.
	// Set in functions loadHeightMap and launchProjectiles.
	// Used (read) in functions drawTerrain and getTerrainHeight.
	float shmap[TERRAIN_SIZE][TERRAIN_SIZE];

	// Field scol holds color data (see function drawTerrain)
	// Set in function initTerrain and launchProjectiles
	// Used (read) in function drawTerrain
	float scol[TERRAIN_SIZE][TERRAIN_SIZE][3]; 

	// Set in functions initTerrain and loadMapTextureIndices
	// Used as an index into global array gloTexIds in function drawTerrain
	int map_texture_indexes[TERRAIN_SIZE][TERRAIN_SIZE]; 

	// Set in function getTerrainHeight (this field is often set)
	// Used (read) in function drawTerrain, after calling function getTerrainHeight
	// Used (read) in function checkForPlaneCollision, after calling function getTerrainHeight
	// Used (read) in function addSmokeAtPoint, after calling function getTerrainHeight
	// Used (read) in function addExplosionAtPoint, after calling function getTerrainHeight
	// Used (read) in function launchProjectiles, after calling function getTerrainHeight
	float auxnormal[3]; 
};

// The glo prefix to global variable names is supposed to remind you that it is a global variable.
// Though I usually use "gl" to prefix global variables, in this case it might get confused with 
// something relating to OpenGL, as most OpenGL functions start with "gl".

// gloScreen is used only in function main
extern SDL_Surface *gloScreen;

// gloEvent is used only in function main
extern SDL_Event gloEvent;		   // for real-time interactivity functions provided by SDL library 

// gloWindow is used in functions initSDL and sdldisplay
extern SDL_Window *gloWindow; // New for SDL 2

// Set in function initSDL
extern SDL_GLContext gloContext;	   // New for SDL2
extern const GLdouble pi;

// gloUsingLowResolution is used only in function main.
extern int gloUsingLowResolution;

// gloTexturesAvailable indicates how many textures are loaded or randomly-generated
// Set in function loadTerrainTextures and loadTreeTextures.
// Used (read) in functions main, loadTreeTextures, and loadMapTextureIndices.
extern int gloTexturesAvailable; 

// MAG is used in function clearScreen.
// MAG can be increased by 10 by the user at runtime by clicking on the 't' key.
extern float MAG;

// gloView can have 4 values: 1, 2, 3, 4
// 1 => external view, looking at plane from behind and right of the plane
// 2 => external view, looking down at plane from directly above
// 3 => external view, looking at right side of plane, from directly to the right side of the plane
// 4 => internal view, from inside plane looking outside of cockpit
// The user can switch through the various values of gloView by pressing the 'o' button
//
// Used (read) in functions updatePQRAxes and updateVirtualCameraPos
extern int gloView; 

extern float x_cockpit_view;
extern float y_cockpit_view;
extern float z_cockpit_view;

extern int aboard;
extern float x_pilot, y_pilot, z_pilot;
extern float prev_x_pilot, prev_y_pilot;

// autoset by general graphics procedure!! 
// posizione telecamera vituale / virtual camera position
// Updated in function updateVirtualCameraPos
extern float x, y, z;		  

// coordinates of airplane in game... external view is just needed here to make this game 
// show accidental crashes due to overflow of the float datatype variables after going far 
// more than 4300 Km from origin 
extern float xp, yp, zp; 

// versori dei 3 assi della telecamera virtuale 
// versors of the 3 axes of the virtual camera
//
// Updated in function updatePQRAxes.
extern float P[3], Q[3], R[3];

// versori dei 3 assi del sistema di riferimento rispetto cui sono dati i vertici dello aeroplano 
// versors of the 3 axes of the reference system with respect to which the vertices of the airplane are given
//
// Set/reset in function reorientAxes
// Used in functions drawTerrain, drawAirplane, and updatePQRAxes
extern float Pa[3], Qa[3], Ra[3]; 

extern struct subterrain gloTerrain;

// NTRIS indicates the maximum number of triangular facets we will support.
// NTRIS is a worst-case consideration in which each triangular face has a different texture.
#define NTRIS 610

// gloTexIds is used only to store id number to reall...  It is filled at texture load-in at first call to GLaddpoly
extern int gloTexIds[NTRIS]; 

// texture abcd side order triplet... tells how texture image is stretched on triangular face...
// so:
// 
// c_____d(1,1)
// |     |
// |     |
// |     | 
// |_____|
// a(0,0) b(1,0)
// 
// c(0,1) yes.
// see definition below of "texcoord_x[0-3]" and "texcoord_y[0-3]" 

// texcoord_x and texcoord_y are defined but never used.
// this never changes... it's a fixed convention... of course you can change it in the 
// source cnde here and recompile for having a different flavour of editor 
// ==vertexes of tex square:===a=b=c=d===========
// const int texcoord_x[4] = {0, 1, 0, 1}; //...x
// const int texcoord_y[4] = {0, 0, 1, 1}; //...y

// NTREES is the maximum number of trees we will support
#define NTREES 10000

// 4 data define a tree: 
// x, y,z coordinates of where its convetional geometric center is;
// the other is a parameter saying how much the 96x96 or 128x128 (or more) 
// 'slice' images must be magnified with respect to the standard 1 unit-per-side case. 
// the 5-th parameter says which type of tree, chosen from a pre-defined set. This means, 
// in our case, the id of the texture used to do the 2 'slices'.
//
// NOTE: this is only a basic support for trees of course. The most extreme 
// realism would be achieved by requiring a complete polyhedral definition for each type of tree.
// After all, this way trees would be polyherda as any generic polyhedra with also semitransparent 
// faces for the 'slices' where desired. 
//
// Set in function main. Drawn in function drawTrees.
extern float gloTrees[NTREES][5];

// gloTreeTextureIDBounds[0] is set in function main.
// gloTreeTextureIDBounds[0] is read in function initTrees.
extern int gloTreeTextureIDBounds[2]; // texture_ID -related thing

// treeR1 is used in function drawTrees.
// Not modified, so this can be declared a constant.
extern float treeR1;

// auxilliary for giving right texture coordinates... the right way of 'splattering' the 
// quadrangular texture image / color-matrix onto some triangles into which all 3D objects 
// in the simplest case are divided.
//
// Used in function drawTerrain, where it is used as a parameter to drawTexturedTriangle.
// It would be better to declare this variable as a local variable in drawTerrain.
extern float texcoords_gnd_tri1[3][2];

// Used in function drawTerrain, where it is used as a parameter to drawTexturedTriangle.
// It would be better to declare this variable as a local variable in drawTerrain.
extern float texcoords_gnd_tri2[3][2];

#define NLINES 20
#define NVERTEXES 200

// Aereo: Definizione dei vertici
// Plane: Definition of the vertexes
//
// Used in functions main and makeInertiaTensor.
// Set in function importAirplanePolyhedron.
extern float gloPunti[NVERTEXES][3];

// chi sono i 2 punti estremi delle linee da disegnare? / who are the 2 extreme points of the lines to draw?
// NOTA: estremi[ i-esima linea][quale punto e' l'estremo ] / NOTE: extremes [i-th line] [which point is the extreme]
// Initialized below with 61 elements, indexed from 0 to 60.
// 
// Used in function drawAirplane as indices into matrix gloPunti.
extern int tris[NTRIS][3];

// ovvio a cosa serve... / obvious what it's for ...
// col_tris holds the airplane's colors
// Initialized with random colors in initAirplaneColors
// Loaded with data from file input/facecolor.txt in function importAirplanePolyhedron
extern float col_tris[NTRIS][3];

// nvertexes is set in function importAirplanePolyhedron to the number of rows set in array gloPunti 
// with data read from file input/vertexes.txt.
extern int nvertexes;

// ntris is used in function drawAirplane.
// It is set in function importAirplanePolyhedron to the number of rows set in array tris 
// with data read from file input/triangulation.txt.
extern int ntris;

// quantities used for simulation / realistic motion and rebounce from ground
// v = velocity
extern float v[3];

// p = momentum
extern float p[3];

extern double Rm[3][3];

extern double w[3];

// L = angular momentum
extern double L[3];

// constants which characterize dynamically the body
extern float MASS; // total mass (linear motion) 

extern double initInaTsr[3][3];
// sort of "rotational mass" (angular motion) 

// we build also the inverse matrix of R_3x3 matrix 
extern double initInaTsrInv[3][3];

// it's updated according to orientation
extern double currInaTsr[3][3]; 

extern double currInaTsrInv[3][3];

// (DON'T CARE; info: Google --> "moment of inertia tensor" ) 
// influences from outside: force vectors
extern float Fcm[3];			// total force on center-of-mass "CM" 
extern double gloTtlTorque[3]; // total torque-force on rigid body

// costants specific to simplest **AIRPLANE** game. 
// some constant deriving from viscosity of air 
// air friction due to wing and backwing total area... *parp-fall resistance*
extern double k_visc; 

// some constant deriving from viscosity of air **side-fall resistance**
extern double k_visc2; 

// some constant deriving from viscosity of air 
extern double k_visc3; 

// some constant vaguely related to viscosity of air **CODINO alignment**?? 
extern double k_visc_rot3; 

// some constant deriving form viscosity of air around main axis... 
// around the axis passing through the wings, you know... *???* 
extern double k_visc_rot_STABILIZE; 

// Force of the propeller driven by the motor... a propulsion force. 
extern double Pforce; 

extern double gloResultMatrix[3][3];
extern double gloTempMatrix[3][3];

extern double R_T[3][3];

extern double Id[3][3];

extern double dR[3][3];

extern double gloOrigAxis1[3];
extern double gloOrigAxis2[3];
extern double gloOrigAxis3[3];

// according to present orientation: 3 principal axes of inertia.
// Used in function checkForPlaneCollision.
extern double gloAxis1[3], gloAxis2[3], gloAxis3[3]; 

// auxillaries:
extern double SD[3][3], SD2[3][3], u1, u2, u3, w_abs, dAng;

// ==END OF SIMULATION physical QUANTITIES DECLARATIONS========================

// Drawn in function drawLogo
extern int gloLogo[8][128];
#endif