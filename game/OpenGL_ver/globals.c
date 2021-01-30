#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "globals.h"

#define WIDTH 480
#define HEIGHT 320
#define COLDEPTH 16

// the structure representing relevant quantities concerning game's terrain
#define TERRAIN_SIZE 300

// The glo prefix to global variable names is supposed to remind you that it is a global variable.
// Though I usually use "gl" to prefix global variables, in this case it might get confused with 
// something relating to OpenGL, as most OpenGL functions start with "gl".

// gloScreen is used only in function main
SDL_Surface *gloScreen;

// gloEvent is used only in function main
SDL_Event gloEvent;		   // for real-time interactivity functions provided by SDL library 

// gloWindow is used in functions initSDL in main.c and sdldisplay in graphics.c
SDL_Window *gloWindow = NULL; // New for SDL 2

// Set in function initSDL in main.c
SDL_GLContext gloContext;	   // New for SDL2
const GLdouble pi = 3.1415926535897932384626433832795;

// gloUsingLowResolution indicates if we are using low resolution or not.
// Smoke and explosion special effects only work if we are NOT using low resolution
int gloUsingLowResolution = 1; // indicate we are using low resolution

// gloTexturesAvailable indicates how many textures are loaded or randomly-generated
// Set in function loadTerrainTextures in terrain.c and loadTreeTextures in trees.c
// Used (read) in functions loadTreeTextures in trees.c and loadMapTextureIndices in terrain.c
int gloTexturesAvailable = 0; 

// MAG is used in function clearScreen in graphics.c
// MAG can be increased by 10 by the user at runtime by clicking on the 't' key.
float MAG = 60.0;

// gloView can have 4 values: 1, 2, 3, 4
// 1 => external view, looking at plane from behind and right of the plane
// 2 => external view, looking down at plane from directly above
// 3 => external view, looking at right side of plane, from directly to the right side of the plane
// 4 => internal view, from inside plane looking outside of cockpit
// The user can switch through the various values of gloView by pressing the 'o' button
//
// Used (read) in functions updatePQRAxes and updateVirtualCameraPos in graphics.c
int gloView = 1; // start w external view

float x_cockpit_view = 0.6;
float y_cockpit_view = 0.6;
float z_cockpit_view = 0.6;

int aboard = 1;
float x_pilot, y_pilot, z_pilot;
float prev_x_pilot, prev_y_pilot;

// autoset by general graphics procedure!! 
// posizione telecamera vituale / virtual camera position
// Updated in function updateVirtualCameraPos in graphics.c
float x = 0.0, y = 0.0, z = 0.0;		  

// coordinates of airplane in game... external view is just needed here to make this game 
// show accidental crashes due to overflow of the float datatype variables after going far 
// more than 4300 Km from origin 
float xp = 200.0, yp = 200.0, zp = 400.0; 

// versori dei 3 assi della telecamera virtuale 
// versors of the 3 axes of the virtual camera
//
// Updated in function updatePQRAxes in graphics.c
float P[3], Q[3], R[3];

// versori dei 3 assi del sistema di riferimento rispetto cui sono dati i vertici dello aeroplano 
// versors of the 3 axes of the reference system with respect to which the vertices of the airplane are given
//
// Set/reset in function reorientAxes
// Used in functions drawTerrain in terrain.c, drawAirplane in airplane.c, and updatePQRAxes in graphics.c
float Pa[3], Qa[3], Ra[3]; 

struct subterrain gloTerrain;

// NTRIS indicates the maximum number of triangular facets we will support.
// NTRIS is a worst-case consideration in which each triangular face has a different texture.
#define NTRIS 610

// gloTexIds is used only to store id number to reall...  It is filled at texture load-in at first call to GLaddpoly
int gloTexIds[NTRIS]; 

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
const int texcoord_x[4] = {0, 1, 0, 1}; //...x
const int texcoord_y[4] = {0, 0, 1, 1}; //...y

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
// Set in function main. Drawn in function drawTrees in trees.c
float gloTrees[NTREES][5];

// gloTreeTextureIDBounds[0] is set in function main
// gloTreeTextureIDBounds[0] is read in function initTrees in trees.c
int gloTreeTextureIDBounds[2]; // texture_ID -related thing

// treeR1 is used in function drawTrees in trees.c
// Not modified, so this can be declared a constant.
float treeR1 = 0.5;

// auxilliary for giving right texture coordinates... the right way of 'splattering' the 
// quadrangular texture image / color-matrix onto some triangles into which all 3D objects 
// in the simplest case are divided.
//
// Used in function drawTerrain in terrain.c, where it is used as a parameter to drawTexturedTriangle.
// It would be better to declare this variable as a local variable in drawTerrain.
float texcoords_gnd_tri1[3][2] = {
	{0.0, 0.0},
	{1.0, 0.0},
	{0.0, 1.0}
};

// Used in function drawTerrain in terrain.c, where it is used as a parameter to drawTexturedTriangle.
// It would be better to declare this variable as a local variable in drawTerrain.
float texcoords_gnd_tri2[3][2] = {
	{1.0, 0.0},
	{0.0, 1.0},
	{1.0, 1.0}
};

#define NLINES 20
#define NVERTEXES 200

// Aereo: Definizione dei vertici
// Plane: Definition of the vertexes
//
// Set with data from a file in function importAirplanePolyhedron in airplane.c
// Drawn in function drawAirplane in airplane.c
// Used in function makeInertiaTensor in physics.c
float gloPunti[NVERTEXES][3] = {// scatola  / box
	{  74,   76,  -57}, // 0
	{  74,   76,   57},
	{  74,   76,  262},
	{ -74,   76,  262},
	{ -74,   76,   57},
	{ -74,   76,  -57},
	{ -74,   76, -262},
	{  74,   76,  262},
	{  74,   76, -549},
	{ -44,   76, -549},
	{  74,   76,  549}, // 10
	{ -44,   76,  549},
	{ 244,   -1,   50},
	{ 244,   -1,  -50},
	{-354,   10,   12},
	{-354,   10,  -12},
	{-370,   10, -165},
	{-434,   10, -165},
	{-434,   10,    0},
	{-434,   10,  165},
	{-370,   10,  165}, // 20
	{ 244,  -75,   50},
	{ 244,  -75,  -50},
	{  74,   10,  -80},
	{  74,   10,   50},
	{ -74,   40,   50},
	{ -74,   40,  -50},
	{-377,   25,    0},
	{-159,   10,    0},
	{-445,  108,    0},
	{  90, -135, -134}, // 30
	{-220, -135,  -30},
	{-220, -135,   30},
	{  90, -135,  134},
	{  17,   75,   50},
	{  74,   76, -262},
	{  17,   10,  -40},
	{  17,  -10,   40},
	{ -17,   20,  -40},
	{ -17,   20,   40},
	{ -17,  -40,  -60}, // 40
	{ -17,  -20,   40},
	{-445,    5,    0},
	{-445,    5,    0},
	{  65,  -50,  -40},
	{  65,  -50,   40},
	{  65,   10,  -40},
	{-156,   10,    0},
	{  35,   20,  -40},
	{  35,   20,   40},
	{ -35,   20,   89}, // 50
	{ -35,    0,   89},
	{  10,   70,  -50},
	{  56,   71,  -40},
	{  56,   71,  -79},
	{  10,   70,   50},
	{  16,  -50,  -40},
	{  16,  -50,  -40},
	{  17,   20,  -40},
	{   0,    5,  -40},
	{  17,   20,  -40}, // 60
	{  17,   20,  -40},
	{ -10,   20,  -40},
	{ -35,   20,  -40},
	{ -35,   20,  -40},
	{ 120,   75,  -40},
	{ 120,   75,  -40},
	{   0,   75,  -40},
	{ -15,  -50,  -40},
	{ -15,  -50,  -40},
	{ -15,  -20,    6}, // 70
	{  56,   73,   40},
	{ -30,   20,  -40},
	{ -30,   20,  -40},
	{ -35,   20,   39},
	{ -35,    0,   39},
	{-110,  -50,  -90},
	{  14,  -50,  -80},
	{  14,  -50,   80},
	{-110,  -50,   -4},
	{  16,  -50,    0}, // 80
	{  16,  -50,    0},
	{  18,   20,    0},
	{   6,   75,    5},
	{  80,  -20,    0},
	{  80,  -20,    0},
	{-100,   20,    0},
	{ -20,   50,    7},
	{ -10,   50,    0},
	{ 120,  -75,   40},
	{ 120,  -75,  -40}, // 90
	{   0,   75,   -8},
	{-165,  -50,   -0},
	{-165,  -50,    7},
	{-155,  -20,   -0},
	{-100,   20,    8},
	{  10,   70,   -6},
	{ -35,   20,    8},
	{-120, -140,  -80},
	{-120, -140,   80} // 99
};

// chi sono i 2 punti estremi delle linee da disegnare? / who are the 2 extreme points of the lines to draw?
// NOTA: estremi[ i-esima linea][quale punto e' l'estremo ] / NOTE: extremes [i-th line] [which point is the extreme]
// Initialized below with 61 elements, indexed from 0 to 60.
// 
// Used in function drawAirplane in airplane.c as indices into matrix gloPunti.
int tris[NTRIS][3] = {
	{ 0,  4,  5}, // 0
	{ 1,  2,  3},
	{ 1,  3,  4},
	{ 0,  1,  4},
	{ 0,  5,  6},
	{ 0,  6, 35},
	{ 6,  8, 35},
	{ 6,  8,  9},
	{ 2, 10, 11},
	{ 2, 11,  3},
	{ 5, 13,  4}, // 10
	{ 4, 12, 13},
	{ 0,  1, 15},
	{ 1, 14, 15},
	{15, 16, 18},
	{16, 18, 17},
	{14, 20, 19},
	{14, 18, 19},
	{13, 22, 23},
	{12, 22, 21},
	{15, 84, 92}, // 20
	{23,  5,  0},
	{23, 24,  4},
	{40, 23, 22},
	{21, 24, 26},
	{18, 28, 29},
	{28, 27, 18},
	{ 0, 31,  5},
	{ 1,  4, 32},
	{13, 12, 22},
	{14, 38, 50}, // 30
	{89, 88, 94},
	{94, 95, 89},
	{11, 45, 82},
	{11, 91, 82},
	{17, 19, 91},
	{91, 19, 43},
	{90, 94, 95},
	{16, 18, 40},
	{83, 79,  7},
	{66, 90, 95}, // 40
	{96, 67, 31},
	{31,  7, 96},
	{13, 37, 67},
	{23, 26, 28},
	{28, 97, 23},
	{43, 82, 91},
	{42, 43, 82},
	{42, 58, 82},
	{37, 38, 49},
	{31, 39, 49}, // 50
	{49, 67, 31},
	{49, 67, 37},
	{10, 12, 20},
	{40, 42, 43},
	{43, 41, 40},
	{18, 40, 42},
	{40, 20, 21},
	{20, 21, 26},
	{20, 25, 26},
	{25, 26,  4} // 60
};

// ovvio a cosa serve... / obvious what it's for ...
// col_tris holds the airplane's colors
// Initialized with random colors in initAirplaneColors in airplane.c
// Loaded with data from file input/facecolor.txt in function importAirplanePolyhedron in airplane.c
// Used (read) in function drawAirplane in airplane.c
float col_tris[NTRIS][3] = {
	{0.9, 0.9, 0.3}, // 0
	{0.9, 0.9, 0.3},

	{0.7, 0.5, 0.3}, // 2
	{0.7, 0.5, 0.3},

	{0.3, 0.3, 0.3}, // 4
	{0.3, 0.3, 0.3},

	{0.3, 0.8, 0.9}, // 6
	{0.3, 0.8, 0.9},

	{0.3, 0.4, 0.3}, // 8
	{0.3, 0.4, 0.3},

	{0.4, 0.2, 0.3}, // 10
	{0.4, 0.2, 0.3},

	{0.6, 0.6, 0.6}, // 12
	{0.6, 0.6, 0.6},

	{0.3, 0.6, 0.9}, // 14
	{0.3, 0.6, 0.9},

	{0.3, 0.4, 0.3}, // 16
	{0.3, 0.4, 0.3},

	{0.7, 0.4, 0.3}, // 18
	{0.4, 0.4, 0.3}
};

// nvertexes is set in function importAirplanePolyhedron in airplane.c to the number of rows set in array gloPunti 
// with data read from file input/vertexes.txt.
int nvertexes = 100; // default value

// ntris is used in function drawAirplane in airplane.c
// It is set in function importAirplanePolyhedron in airplane.c to the number of rows set in array tris 
// with data read from file input/triangulation.txt.
int ntris = 33;		 // default value

// quantities used for simulation / realistic motion and rebounce from ground
// v = velocity
float v[3] = {0.0, 0.0, 0.0}; // (needs initial value!) 

// p = momentum
float p[3];

double Rm[3][3] = {
	{1.0, 0.0, 0.0},
	{0.0, 1.0, 0.0},
	{0.0, 0.0, 1.0}
};
// orientation (angular): needs inital value (IDENTITY MATRIX!!!)

double w[3] = {0.0, 0.0, 0.0};

// L = angular momentum
double L[3] = {0.0, 0.0, 0.0};

// constants which characterize dynamically the body
float MASS = 1000.0; // total mass (linear motion) 

double It_init[3][3] = {
	{100.0, 0.0, 0.0},
	{0.0, 200.0, 0.0},
	{0.0, 0.0, 110.0}
};
// sort of "rotational mass" (angular motion) 

// we build also the inverse matrix of R_3x3 matrix 
double It_initINV[3][3];

// it's updated according to orientation
double It_now[3][3] = {
	{100.0, 0.0, 0.0},
	{0.0, 100.0, 0.0},
	{0.0, 0.0, 100.0}
}; 

// (DON'T CARE; info: Google --> "moment of inertia tensor" ) 
// influences from outside: force vectors
float Fcm[3] = {0.0, 0.0, 0.0};			// total force on center-of-mass "CM" 
double gloTtlTorque[3] = {0.0, 0.0, 0.0}; // total torque-force on rigid body

// costants specific to simplest **AIRPLANE** game. 
// some constant deriving from viscosity of air 
// air friction due to wing and backwing total area... *parp-fall resistance*
double k_visc = 2900.9; 

// some constant deriving from viscosity of air **side-fall resistance**
double k_visc2 = 100.9; 

// some constant deriving from viscosity of air 
double k_visc3 = 200.9; 

// some constant deriving from viscosity of air around main axis... from nose to back 
double k_visc_rot = 2.9; 

// some constant deriving from viscosity of air around main axis... around the axis perpendicular to wings' plane 
double k_visc_rot2 = 20.9; 

// some constant vaguely related to viscosity of air **CODINO alignment**?? 
double k_visc_rot3 = 290.9; 

// some constant deriving form viscosity of air around main axis... 
// around the axis passing through the wings, you know... *???* 
double k_visc_rot_STABILIZE = 110.0; 

// Force of the propeller driven by the motor... a propulsion force. 
double Pforce = 0.0; 

// updated at each cycle... part of the super-simplified formula to give forces to the body 
// in a way to make it fly a bit like a real airplane.
double vpar, vperp;	 

double gloResultMatrix[3][3];
double gloTempMatrix[3][3];

double R_T[3][3];

double Id[3][3] = {
	{1.0, 0.0, 0.0},
	{0.0, 1.0, 0.0},
	{0.0, 0.0, 1.0}
};

double dR[3][3] = {
	{1.0, 0.0, 0.0},
	{0.0, 1.0, 0.0},
	{0.0, 0.0, 1.0}
};

double inv_It_now[3][3] = {
	{1.0, 0.0, 0.0},
	{0.0, 1.0, 0.0},
	{0.0, 0.0, 1.0}
};

// gloOriginalAxis1, gloOrigAxis2, and gloOrigAxis3 are only used in function reorientAxes in graphics.c
double gloOrigAxis1[3] = {1.0, 0.0, 0.0};
double gloOrigAxis2[3] = {0.0, 1.0, 0.0};
double gloOrigAxis3[3] = {0.0, 0.0, 1.0};

// according to present orientation: 3 principal axes of inertia.
// Used in function checkForPlaneCollision in airplane.c
double gloAxis1[3], gloAxis2[3], gloAxis3[3]; 

// auxillaries:
double SD[3][3], SD2[3][3], u1, u2, u3, w_abs, dAng;

// ==END OF SIMULATION physical QUANTITIES DECLARATIONS========================

// gloLogo holds the logo
// which is used to display the words "FLightCRaftbY SiMON HASUR"
// Drawn in function drawLogo in graphics.c
int gloLogo[8][128] = {
	// F               L           i        g           h        t           C              R           a        f           t           b              Y                    S        i           M                 O               N                         H                    A                 S                U                  R            
	{1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0},
	{1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0},
	{1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0},
	{1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0},
	{1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0},
	{1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1},
//   0                             1                             2                             3                             4                             5                             6                             7                             8                             9                             1                             1            
//                                 0                             0                             0                             0                             0                             0                             0                             0                             0                             0                             1
//                                                                                                                                                                                                                                                                                                               0                             0
};
