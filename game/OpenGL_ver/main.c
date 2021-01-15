#include <stdio.h>
#include <math.h>
#include <time.h>

#define GL_GLEXT_PROTOTYPES
// include SDL library's header
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>

#include <GL/gl.h>
#include <GL/glcorearb.h>

// the structure representing relevant quantities concerning game's terrain
#define TERRAIN_SIZE 300
struct subterrain
{
	// Set in function initTerrain twice. 
	// At the top of initTerrain, it is set to 300. 
	// At the bottom of initTerrain, it is set to the return value of a call to load_hmap_from_bitmap.
	// Used (read) in functions drawTerrain and say_terrain_height
	int map_size;

	// Set in function initTerrain to 50.
	// Used (read) in functions initTerrain, drawTerrain, projectile_launch, say_terrain_height
	float GPunit;

	// Set in function initTerrain to a random value.
	// Set in functions load_hmap_from_bitmap and projectile_launch.
	// Used (read) in functions drawTerrain and say_terrain_height.
	float shmap[TERRAIN_SIZE][TERRAIN_SIZE];

	// Field scol holds color data (see function drawTerrain)
	// Set in function initTerrain and projectile_launch
	// Used (read) in function drawTerrain
	float scol[TERRAIN_SIZE][TERRAIN_SIZE][3]; 

	// Set in functions initTerrain and load_maptex_from_bitmap
	// Used as an index into global array gloTexIds in function drawTerrain
	int map_texture_indexes[TERRAIN_SIZE][TERRAIN_SIZE]; 

	// Set in function say_terrain_height (this field is often set)
	// Used (read) in function drawTerrain, after calling function say_terrain_height
	// Used (read) in function checkForPlaneCollision, after calling function say_terrain_height
	// Used (read) in function addsmoke_wsim, after calling function say_terrain_height
	// Used (read) in function addfrantumation_wsim, after calling function say_terrain_height
	// Used (read) in function projectile_launch, after calling function say_terrain_height
	float auxnormal[3]; 
};

// ####################################################################################################################
// # Function prototypes                                                                                              #
// #                                                                                                                  #

// getpixel is used for bitmap conversion to RGB value matrix (usual stuff)
Uint32 getpixel(SDL_Surface *surface, int x, int y);

// this is the prototype of the function which will draw the pixel.
void sdldisplay();

void xclearpixboard(int xlimit, int ylimit);

// Function xadd1pix is not called.
// add 1 pixel to output image but in a failsafe manner: no accidental segfaults. 
void xadd1pix(int x, int y, float color[3],
			  int xlimit, int ylimit);

// Function xaddline is not called.
// interpolate 2 points graphically 
void xaddline(int x1, int y1,
			  int x2, int y2, float color[3], // change this to int color[3] 
			  int xlimit, int ylimit);

// now we define the function which, given 2 points in 3D, calculates where they end up on the
// virtual camera pointing toward positive z-axis and passes them to the 2D line drawing function. 
void xaddftriang_persp(float x1, float y1, float z1,
					   float x2, float y2, float z2,
					   float x3, float y3, float z3,
					   float color[3]);

// Function xaddftriang_perspTEXTURED_pp is not used; it is not called
// textured triangles, drawn exactly, no approximations
void xaddftriang_perspTEXTURED_pp(float x1, float y1, float z1,
								  float x2, float y2, float z2,
								  float x3, float y3, float z3,
								  int step,
								  float color[3]);

void GLaddftriang_perspTEXTURED(float x1, float y1, float z1,
								float x2, float y2, float z2,
								float x3, float y3, float z3,
								int texId, float texcoords[3][2],
								float color[3]);

void load_textures_wOpenGL();
void load_textures96x96_SEMITRANSPARENT_wOpenGL(); // similarly but wwww alpha values transparency info too, etc.
int load_hmap_from_bitmap(char *filename); // ...description at definition
int load_maptex_from_bitmap(char *filename); // ...description at definition

void xaddpoint_persp(float x1, float y1, float z1, float color[3]);

void xaddline_persp(float x1, float y1, float z1, float x2, float y2, float z2, float color[3]);

// basic special effects in videogames
// add new explosion or just process those already started 
void addfrantumation_wsim(float x0, float y0, float z0, double dft, int option);

// add new explosion or just process those already started 
void addsmoke_wsim(double x0, double y0, double z0, double dft, int option);

void projectile_launch(float x, float y, float z, float vx, float vy, float vz, double dft, int do_add);

// misc for import 3D models.
long int check_vector_elements(char filename[]);
void read_vector(char filename[], float dest_string[], long int maxsize);

// polyherdron definition importing from simple text file containing list of coordinate triplets. 
// does same for face definition and colors and texture orderting if needed.
void import_airplane_polyheron(void); 

int waitdt_ms(double tt_ms);

double say_terrain_height(struct subterrain *ite, double x, double z);

// mat3x3_mult multiplies two 3x3 matrixes 3x3, placing result in global variable gloResultMatrix
void mat3x3_mult(double mat1[3][3], double mat2[3][3]);

// inv calculates the inverse of a 3x3 matrix (used for obtaining the inverse of the inertia tensor)
void inv(double in_3x3_matrix[3][3]);

double body_rebounce(double rx, double ry, double rz,
					 double nx, double ny, double nz, double e);

void make_inertia_tensor(int n_vertexs); 

void initSDL(void);
void initOpenGL(void);
void initPoints(void);
void initAirplaneColors(void);
void initTrees(void);
void initTerrain(void);
void initPhysicsVars(void);
void drawLogo(void);
void drawAxes(void);
void drawTerrain(void);
void drawTrees(void);
void drawAirplane(float h);
void loadAirplaneModel(void);
void checkForPlaneCollision(void);
void updateTorque(int plane_up, int plane_down, int plane_inclleft, int plane_inclright);
void updatePQRAxes(float theta, float fi);
void updateVirtualCameraPos(float RR);
void reorientAxes(void);
void simulatePhysics(int plane_up, int plane_down, int plane_inclleft, int plane_inclright, float h, double g);
void displayStatusInfo(int cycles, float h, float theta, float fi);
void processEvent(float *turnch, float *turncv, float *RR, int *plane_up, int *plane_down, int *plane_inclleft, int *plane_inclright, float *h);

// #                                                                                                                  #
// # End function prototypes                                                                                          #
// ####################################################################################################################


// ####################################################################################################################
// # Global variable declarations                                                                                     #
// #                                                                                                                  #

#define WIDTH 480
#define HEIGHT 320
#define COLDEPTH 16

// The glo prefix to global variable names is supposed to remind you that it is a global variable.
// Though I usually use "gl" to prefix global variables, in this case it might get confused with 
// something relating to OpenGL, as most OpenGL functions start with "gl".

// gloScreen is used only in function main
SDL_Surface *gloScreen;

// gloEvent is used only in function main
SDL_Event gloEvent;		   // for real-time interactivity functions provided by SDL library 

// gloWindow is used in functions initSDL and sdldisplay
SDL_Window *gloWindow = NULL; // New for SDL 2

// Set in function initSDL
SDL_GLContext gloContext;	   // New for SDL2
const GLdouble pi = 3.1415926535897932384626433832795;

// gloUsingLowResolution is used only in function main.
int gloUsingLowResolution = 1;

// pixmatrix is populated in functions xaddline and xadd1pix
// However neither xaddline nor xadd1pix is called.
float pixmatrix[HEIGHT][WIDTH][3];

// gloTexturesAvailable indicates how many textures are loaded or randomly-generated
// Set in function load_textures_wOpenGL and load_textures96x96_SEMITRANSPARENT_wOpenGL.
// Used (read) in functions main, load_textures96x96_SEMITRANSPARENT_wOpenGL, and load_maptex_from_bitmap.
int gloTexturesAvailable = 0; 

// MAG is used in function xclearpixboard.
// MAG can be increased by 10 by the user at runtime by clicking on the 't' key.
float MAG = 60.0;

// gloView can have 4 values: 1, 2, 3, 4
// 1 => external view, looking at plane from behind and right of the plane
// 2 => external view, looking down at plane from directly above
// 3 => external view, looking at right side of plane, from directly to the right side of the plane
// 4 => internal view, from inside plane looking outside of cockpit
// The user can switch through the various values of gloView by pressing the 'o' button
//
// Used (read) in functions updatePQRAxes and updateVirtualCameraPos
int gloView = 1; // start w external view

float x_cockpit_view = 0.6;
float y_cockpit_view = 0.6;
float z_cockpit_view = 0.6;

int aboard = 1;
float x_pilot, y_pilot, z_pilot;

// autoset by general graphics procedure!! 
// posizione telecamera vituale / virtual camera position
// Updated in function updateVirtualCameraPos
float x = 0.0, y = 0.0, z = 0.0;		  

// coordinates of airplane in game... external view is just needed here to make this game 
// show accidental crashes due to overflow of the float datatype variables after going far 
// more than 4300 Km from origin 
float xp = 200.0, yp = 200.0, zp = 400.0; 

// versori dei 3 assi della telecamera virtuale 
// versors of the 3 axes of the virtual camera
//
// Updated in function updatePQRAxes.
float P[3], Q[3], R[3];

// versori dei 3 assi del sistema di riferimento rispetto cui sono dati i vertici dello aeroplano 
// versors of the 3 axes of the reference system with respect to which the vertices of the airplane are given
//
// Set/reset in function reorientAxes
// Used in functions drawTerrain, drawAirplane, and updatePQRAxes
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
// Set in function main. Drawn in function drawTrees.
float gloTrees[NTREES][5];

// gloTreeTextureIDBounds[0] is set in function main.
// gloTreeTextureIDBounds[0] is read in function initTrees.
int gloTreeTextureIDBounds[2]; // texture_ID -related thing

// treeR1 is used in function drawTrees.
// Not modified, so this can be declared a constant.
float treeR1 = 0.5;

// auxilliary for giving right texture coordinates... the right way of 'splattering' the 
// quadrangular texture image / color-matrix onto some triangles into which all 3D objects 
// in the simplest case are divided.
//
// Used in function drawTerrain, where it is used as a parameter to GLaddftriang_perspTEXTURED.
// It would be better to declare this variable as a local variable in drawTerrain.
float texcoords_gnd_tri1[3][2] = {
	{0.0, 0.0},
	{1.0, 0.0},
	{0.0, 1.0}
};

// Used in function drawTerrain, where it is used as a parameter to GLaddftriang_perspTEXTURED.
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
// Used in functions main and make_inertia_tensor.
// Set in function import_airplane_polyheron.
float gloPunti[NVERTEXES][3] = {// scatola  / box
	{  74,   76,  -57},
	{  74,   76,   57},
	{  74,   76,  262},
	{ -74,   76,  262},
	{ -74,   76,   57},
	{ -74,   76,  -57},
	{ -74,   76, -262},
	{  74,   76,  262},
	{  74,   76, -549},
	{ -44,   76, -549},
	{  74,   76,  549},
	{ -44,   76,  549},
	{ 244,   -1,   50},
	{ 244,   -1,  -50},
	{-354,   10,   12},
	{-354,   10,  -12},
	{-370,   10, -165},
	{-434,   10, -165},
	{-434,   10,    0},
	{-434,   10,  165},
	{-370,   10,  165},
	{ 244,  -75,   50},
	{ 244,  -75,  -50},
	{  74,   10,  -80},
	{  74,   10,   50},
	{ -74,   40,   50},
	{ -74,   40,  -50},
	{-377,   25,    0},
	{-159,   10,    0},
	{-445,  108,    0},
	{  90, -135, -134},
	{-220, -135,  -30},
	{-220, -135,   30},
	{  90, -135,  134},
	{  17,   75,   50},
	{  74,   76, -262},
	{  17,   10,  -40},
	{  17,  -10,   40},
	{ -17,   20,  -40},
	{ -17,   20,   40},
	{ -17,  -40,  -60},
	{ -17,  -20,   40},
	{-445,    5,    0},
	{-445,    5,    0},
	{  65,  -50,  -40},
	{  65,  -50,   40},
	{  65,   10,  -40},
	{-156,   10,    0},
	{  35,   20,  -40},
	{  35,   20,   40},
	{ -35,   20,   89},
	{ -35,    0,   89},
	{  10,   70,  -50},
	{  56,   71,  -40},
	{  56,   71,  -79},
	{  10,   70,   50},
	{  16,  -50,  -40},
	{  16,  -50,  -40},
	{  17,   20,  -40},
	{   0,    5,  -40},
	{  17,   20,  -40},
	{  17,   20,  -40},
	{ -10,   20,  -40},
	{ -35,   20,  -40},
	{ -35,   20,  -40},
	{ 120,   75,  -40},
	{ 120,   75,  -40},
	{   0,   75,  -40},
	{ -15,  -50,  -40},
	{ -15,  -50,  -40},
	{ -15,  -20,    6},
	{  56,   73,   40},
	{ -30,   20,  -40},
	{ -30,   20,  -40},
	{ -35,   20,   39},
	{ -35,    0,   39},
	{-110,  -50,  -90},
	{  14,  -50,  -80},
	{  14,  -50,   80},
	{-110,  -50,   -4},
	{  16,  -50,    0},
	{  16,  -50,    0},
	{  18,   20,    0},
	{   6,   75,    5},
	{  80,  -20,    0},
	{  80,  -20,    0},
	{-100,   20,    0},
	{ -20,   50,    7},
	{ -10,   50,    0},
	{ 120,  -75,   40},
	{ 120,  -75,  -40},
	{   0,   75,   -8},
	{-165,  -50,   -0},
	{-165,  -50,    7},
	{-155,  -20,   -0},
	{-100,   20,    8},
	{  10,   70,   -6},
	{ -35,   20,    8},
	{-120, -140,  -80},
	{-120, -140,   80}
};

// chi sono i 2 punti estremi delle linee da disegnare? / who are the 2 extreme points of the lines to draw?
// NOTA: estremi[ i-esima linea][quale punto e' l'estremo ] / NOTE: extremes [i-th line] [which point is the extreme]
// Initialized below with 61 elements, indexed from 0 to 60.
// 
// Used in function drawAirplane as indices into matrix gloPunti.
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
// Initialized with random colors in initAirplaneColors
// Loaded with data from file input/facecolor.txt in function import_airplane_polyheron
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

// nvertexes is set in function import_airplane_polyheron to the number of rows set in array gloPunti 
// with data read from file input/vertexes.txt.
int nvertexes = 100; // default value

// ntris is used in function drawAirplane.
// It is set in function import_airplane_polyheron to the number of rows set in array tris 
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


double gloOrigAxis1[3] = {1.0, 0.0, 0.0};
double gloOrigAxis2[3] = {0.0, 1.0, 0.0};
double gloOrigAxis3[3] = {0.0, 0.0, 1.0};

// according to present orientation: 3 principal axes of inertia.
// Used in function checkForPlaneCollision.
double gloAxis1[3], gloAxis2[3], gloAxis3[3]; 

// auxillaries:
double SD[3][3], SD2[3][3], u1, u2, u3, w_abs, dAng;

// ==END OF SIMULATION physical QUANTITIES DECLARATIONS========================

// Drawn in function drawLogo
int gloLogo[8][128] = {
	{1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0},
	{1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0},
	{1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0},
	{1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0},
	{1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0},
	{1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1},
};

// #                                                                                                                  #
// # End global variable declarations                                                                                 #
// ####################################################################################################################

// ####################################################################################################################
// the C  main() function  
// ####################################################################################################################
int main()
{
	float turnch = 0.0; // used for smooth turning of virtual camera 
	float turncv = 0.0; // used for smooth turning if virtual camera 
	int plane_up = 0;
	int plane_down = 0;
	int plane_inclleft = 0;
	int plane_inclright = 0;

	float RR = 20.0;

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

	// if there is an external polyhedron definition for better-defined airplanes, let's use it.

	// ============================"INITIALISATION" PROCEDURE STARTS HERE=============
	initPoints();

	cycles = 0;

	// clear the screen
	xclearpixboard(WIDTH, HEIGHT);

	// initTerrain initializes global variable gloTerrain with random values
	initTerrain();

	// initAirplaneColors initializes global variable col_tris with random values
	initAirplaneColors();

	// load textures in
	load_textures_wOpenGL(); 
	gloTreeTextureIDBounds[0] = gloTexturesAvailable; // at what ID do tree textures begin (and end also...)

	// ditto but with "bitmap" image files with alpha value for transparency information on each pixel... 
	// this is mainly used for drawing trees in a quick, nice and simple way.
	load_textures96x96_SEMITRANSPARENT_wOpenGL(); 

	// this comes after, because so during loading we can check if no texture index superior to the number 
	// of available textures is inserted... .
	load_maptex_from_bitmap("terrain_data/maptex_300x300.bmp"); 

	// initTrees initializes global variable gloTrees
	initTrees();

	// If we're not using low resolution ...
	if (gloUsingLowResolution == 0)
	{
		// Add smoke effect at plane location xp, yp, zp
		addsmoke_wsim(xp, yp, zp, h, 1); // commented out for testing
	}

	// initPhysicsVars will set global variables It_init, p, L, It_initINV, Fcm and gloTtlTorque
	initPhysicsVars();

	// loadAirplaneModel sets global variables gloPunti, 
	loadAirplaneModel();

	//============================"INITIALISATION" PROCEDURES DONE=============

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

		// None of the parameters passed to simulatePhysics is modified
		simulatePhysics(plane_up, plane_down, plane_inclleft, plane_inclright, h, g);

		checkForPlaneCollision();

		// Cancella schermata/lavagna
		// Clear screen/board
		xclearpixboard(WIDTH, HEIGHT); 

		updatePQRAxes(theta, fi);

		// Update the virtual camera position x, y, z
		updateVirtualCameraPos(RR);

		// If we are not using low resolution ...
		if (gloUsingLowResolution == 0)
		{
			// Add smoke effect at plane location xp, yp, zp
			addsmoke_wsim(xp, yp, zp, h, 2); // DEACTIVATED TO TEST
		}

		drawAxes();

		drawLogo();

		drawTerrain();

		drawTrees();

		// If we're not using low resolution ...
		if (gloUsingLowResolution == 0)
		{
			// Why are we adding smoke effect at coordinates (x, y, z) = (1, 2, 3)?
			addsmoke_wsim(1, 2, 3, h, 0); // we must make special effect sequences go on. 
		}

		drawAirplane(h);

		// call function which displays the matrix of pixels in a true graphics window 
		sdldisplay();

		// Every ten cycles ...
		if (cycles % 10 == 0)
		{
			// Display status/debugging information
			displayStatusInfo(cycles, h, theta, fi);
		}

		SDL_Delay(10);
		cycles++;

		//===============================|SDL's real-time interactivity|=============================
		
		while (SDL_PollEvent(&gloEvent))
		{ 
			processEvent(&turnch, &turncv, &RR, &plane_up, &plane_down, &plane_inclleft, &plane_inclright, &h);
			
			// extra case: if graphic window is closed, terminate program 
			if (gloEvent.type == SDL_QUIT)
			{ 
				printf("GRAPHICS WINDOW CLOSED: PROGRAM TERMINATED\n");
				SDL_Quit();

				exit(1); 
			} // end of extra case handling part 
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
		gloTrees[k][2] = say_terrain_height(&gloTerrain, gloTrees[k][0], gloTrees[k][1]);

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
			gloTrees[k + j][2] = say_terrain_height(&gloTerrain, gloTrees[k + j][0], gloTrees[k + j][1]);

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
	waitdt_ms(1000.0);

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

	gloTerrain.map_size = load_hmap_from_bitmap("terrain_data/hmap_300x300.bmp");
} // end initTerrain function

// ####################################################################################################################
// Function initPhysicsVars 
//
// Assumptions:
//    Vectors v and w have already been initialized.
// ####################################################################################################################
void initPhysicsVars()
{
	int i, j;

	// make_inertia_tensor will set global variable It_init
	make_inertia_tensor(NVERTEXES);

	// momentum p (linear quantity)
	// momentum = mass x velocity
	//
	// Note that since v is set to {0.0, 0.0, 0.0} when defined (i.e., 0 velocity), 
	// the plane will initially have no momentum, hence the plane is falling when the game starts.
	// The user can increase the force of the propeller (Pforce) by pressing the 9 key, but that 
	// does not seem to affect the velocity variable v.
	p[0] = MASS * v[0];
	p[1] = MASS * v[1];
	p[2] = MASS * v[2];

	// angular momentum (angular/rotational quantity)
	L[0] = It_now[0][0] * w[0] + It_now[0][1] * w[1] + It_now[0][2] * w[2];
	L[1] = It_now[1][0] * w[0] + It_now[1][1] * w[1] + It_now[1][2] * w[2];
	L[2] = It_now[2][0] * w[0] + It_now[2][1] * w[1] + It_now[2][2] * w[2];

	inv(It_init); // puts the inverse matrix into the "gloResultMatrix" global variable. 

	// we copy it (the gloResultMatrix) into "R_INV"... 
	for (j = 0; j < 3; j++)
	{
		for (i = 0; i < 3; i++)
		{
			It_initINV[j][i] = gloResultMatrix[j][i];
		}
	}

	// reset forces to 0.0 
	for (i = 0; i < 3; i++)
	{
		Fcm[i] = 0.0;
		gloTtlTorque[i] = 0.0;
	}

	for (j = 0; j < 3; j++)
	{ 
		printf("CHECK Inertia tensor: %f  %f  %f \n", It_init[j][0], It_init[j][1], It_init[j][2]);
	}

	for (j = 0; j < 3; j++)
	{ 
		printf("CHECK its inverse   : %f  %f  %f \n", It_initINV[j][0], It_initINV[j][1], It_initINV[j][2]);
	}
} // end initPhysicsVars function

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
	color[0] = 1.0;
	color[1] = 0.0;
	color[2] = 0.0;
	
	// IMPORTANT NOTE: 2 meters far from camera virtual 'lens' along it perpendiculat to it 
	// towards screen, axes are each 1 meter long. use in an intelligent way the measures... 
	// in these cases, go use unit-length references. It's the obvious choice but saying it is not bad.

	// xc
	xaddline_persp(0.0 - 1.0, 0.0 - 1.0, 2.0,
					1.0 - 1.0, 0.0 - 1.0, 2.0, color);

	// yc
	color[0] = 0.0;
	color[1] = 1.0;
	color[2] = 0.0;

	xaddline_persp(0.0 - 1.0, 0.0 - 1.0, 2.0,
					0.0 - 1.0, 1.0 - 1.0, 2.0, color);

	// zc
	color[0] = 0.0;
	color[1] = 0.0;
	color[2] = 1.0;

	xaddline_persp(0.0 - 1.0, 0.0 - 1.0, 2.0,
					0.0 - 1.0, 0.0 - 1.0, 2.0 + 1.0, color);
} // end drawAxes function

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

		xaddline_persp(x_c[i], y_c[i], -z_c[i], 
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
				say_terrain_height(&gloTerrain, Xo[0] + 0.01, Yo[0] + 0.01); // check what is the local normal 

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
			xaddline_persp(x_c[0], y_c[0], -z_c[0],
							x_c[4], y_c[4], -z_c[4], color);

			if (xi >= 0 && yi >= 0)
			{
				// Triangolo 1 / Triangle 1
				GLaddftriang_perspTEXTURED(x_c[0], y_c[0], z_c[0],
											x_c[1], y_c[1], z_c[1],
											x_c[2], y_c[2], z_c[2],
											gloTexIds[gloTerrain.map_texture_indexes[xi][yi]], texcoords_gnd_tri1,
											color);

				// Triangolo 2 / Triangle 2 (change color a little bit)
				color[1] = color[1] + 0.1; // draw with slightly different color... 

				GLaddftriang_perspTEXTURED(x_c[1], y_c[1], z_c[1],
											x_c[2], y_c[2], z_c[2],
											x_c[3], y_c[3], z_c[3],
											gloTexIds[gloTerrain.map_texture_indexes[xi][yi]], texcoords_gnd_tri2,
											color);
			}
		}
	}
} // end function drawTerrain

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
			xaddftriang_persp(x1, y1, -z1,
								x2, y2, -z2,
								x3, y3, -z3, color);
		}
		else
		{
			// Draw a perspective, filled triangle using the three points in our triangular facet
			xaddftriang_persp(x1, y1, -z1,
								x2, y2, -z2,
								x3, y3, -z3, color);
		}
	}

	if (gloUsingLowResolution == 0)
	{
		addfrantumation_wsim(x1, y1, z1, h, 0);	// we must make special effect sequences go on. 
		projectile_launch(10, 10, 10, 20, 10, -0.1, h, 0); // idem / ditto
	}
} // end drawAirplane function

// ####################################################################################################################
// Function displayStatusInfo
// ####################################################################################################################
void displayStatusInfo(int cycles, float h, float theta, float fi)
{
	int i, j;

	// Display status/debugging information
	printf("GAME TIME [sec] =  %f \n", cycles * h);
	printf("GOING ON...game cycle %i : plane position: x = %f, y = %f, z = %f \n theta = %3.2f, fi = %f3.2\n", cycles, x, y, z, theta, fi);

	for (j = 0; j < 10; j++)
	{
		for (i = 0; i < 10; i++)
		{
			printf("%2i|", gloTexIds[gloTerrain.map_texture_indexes[j][i]]);
		}
		printf("\n");
	}
} // end displayStatusInfo function

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
		import_airplane_polyheron();
	}
} // end loadAirplaneModel function

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

		double he_id = say_terrain_height(&gloTerrain, xp + xw, yp + yw);
		if (zp + zw < he_id)
		{ 
			// just as any vertex of airplane touches ground and tries to go below 
			body_rebounce(xw, yw, zw, gloTerrain.auxnormal[0], gloTerrain.auxnormal[1], gloTerrain.auxnormal[2], 0.06);
			printf("TOUCH GND \n");
			zp = zp + (he_id - zp - zw);
			// check is normal < 0 and eventually calculated and assigns new velocities and so.
		}
	}
} // end checkForPlaneCollision function

// ####################################################################################################################
// Function updateTorque
// ####################################################################################################################
void updateTorque(int plane_up, int plane_down, int plane_inclleft, int plane_inclright)
{
	if (plane_up == 1)
	{
		gloTtlTorque[0] += 2620.0 * gloAxis3[0];
		gloTtlTorque[1] += 2620.0 * gloAxis3[1];
		gloTtlTorque[2] += 2620.0 * gloAxis3[2];
	}
	if (plane_down == 1)
	{
		gloTtlTorque[0] += -2621.0 * gloAxis3[0];
		gloTtlTorque[1] += -2621.0 * gloAxis3[1];
		gloTtlTorque[2] += -2621.0 * gloAxis3[2];
	}
	if (plane_inclleft == 1)
	{
		gloTtlTorque[0] += -2620.0 * gloAxis1[0];
		gloTtlTorque[1] += -2620.0 * gloAxis1[1];
		gloTtlTorque[2] += -2620.0 * gloAxis1[2];
	}
	if (plane_inclright == 1)
	{
		gloTtlTorque[0] += 2621.0 * gloAxis1[0];
		gloTtlTorque[1] += 2621.0 * gloAxis1[1];
		gloTtlTorque[2] += 2621.0 * gloAxis1[2];
	}
} // end updateTorque function

// ####################################################################################################################
// Function updatePQRAxes
// ####################################################################################################################
void updatePQRAxes(float theta, float fi)
{
	// scenario inquadrature 3D / 3D shot scenery
	if (gloView == 1)
	{
		P[0] = -sin(theta);
		P[1] = cos(theta);
		P[2] = 0.0;

		Q[0] = -cos(theta) * cos(fi);
		Q[1] = -sin(theta) * cos(fi);
		Q[2] = sin(fi);

		R[0] = cos(theta) * sin(fi);
		R[1] = sin(theta) * sin(fi);
		R[2] = cos(fi);
	}
	else if (gloView == 2)
	{
		// I know this negative index stuff is awkward but in the prototype stage 
		// it was good to have 1 and -1. now just extend this to support more views.
		P[0] = -Pa[0]; // had to be mirrored sorry
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
		P[0] = Pa[0]; // had to be mirrored sorry
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
		// INTERNAL VIEW COCKPIT
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

		// Ri, Pi, Qi - this is most practical axis-order.
		P[0] = Rp[0] * (-sin(theta)) + Pp[0] * (cos(theta)) + Qp[0] * 0.0;
		P[1] = Rp[1] * (-sin(theta)) + Pp[1] * (cos(theta)) + Qp[1] * 0.0;
		P[2] = Rp[2] * (-sin(theta)) + Pp[2] * (cos(theta)) + Qp[2] * 0.0;

		Q[0] = Rp[0] * (-cos(theta) * cos(fi)) + Pp[0] * (-sin(theta) * cos(fi)) + Qp[0] * sin(fi);
		Q[1] = Rp[1] * (-cos(theta) * cos(fi)) + Pp[1] * (-sin(theta) * cos(fi)) + Qp[1] * sin(fi);
		Q[2] = Rp[2] * (-cos(theta) * cos(fi)) + Pp[2] * (-sin(theta) * cos(fi)) + Qp[2] * sin(fi);

		R[0] = Rp[0] * (cos(theta) * sin(fi)) + Pp[0] * (sin(theta) * sin(fi)) + Qp[0] * cos(fi);
		R[1] = Rp[1] * (cos(theta) * sin(fi)) + Pp[1] * (sin(theta) * sin(fi)) + Qp[1] * cos(fi);
		R[2] = Rp[2] * (cos(theta) * sin(fi)) + Pp[2] * (sin(theta) * sin(fi)) + Qp[2] * cos(fi);
	}
} // end updatePQRAxes function

// ####################################################################################################################
// Function simulatePhysics
// ####################################################################################################################
void simulatePhysics(int plane_up, int plane_down, int plane_inclleft, int plane_inclright, float h, double g)
{
	int i, j;

	updateTorque(plane_up, plane_down, plane_inclleft, plane_inclright);

	// ===========PHYSICS PROCEDURE=============
	// Sembra tutto OK / Everything seems OK
	reorientAxes();

	// Calulate total Fcm and total torque, starting from external forces applied to vertices 
	// NOTE: g is already done below... so it's no bother----forces  of ultrasimple flight model

	// NOT needed for dynamics, it's only in this game: some approx of force on CM 
	// and torque due to a rudimental plane aerodynamical forces' consideration
	float vpar;
	float vlat;

	float rot1;
	float rot2;
	float rot3;

	vlat = gloAxis3[0] * v[0] + gloAxis3[1] * v[1] + gloAxis3[2] * v[2]; // dot product calculated directly

	vperp = gloAxis2[0] * v[0] + gloAxis2[1] * v[1] + gloAxis2[2] * v[2]; // dot product calculated directly

	vpar = gloAxis1[0] * v[0] + gloAxis1[1] * v[1] + gloAxis1[2] * v[2];

	rot1 = gloAxis1[0] * w[0] + gloAxis1[1] * w[1] + gloAxis1[2] * w[2]; // how much it rotates around axis 1 (nose--> back)

	rot2 = gloAxis2[0] * w[0] + gloAxis2[1] * w[1] + gloAxis2[2] * w[2]; // how much it rotates around axis 2 (perp to wings)

	rot3 = gloAxis3[0] * w[0] + gloAxis3[1] * w[1] + gloAxis3[2] * w[2]; // how much it rotates around axis 3 (parallel to wings)
	// effect (IN A VERY RUDIMENTAL CONCEPTION OF AERODYNAMICS, NOT SCIENTIFIC AT ALL) of air friction on the motion of Center of Mass directly.

	Fcm[0] += -k_visc * vperp * gloAxis2[0] - k_visc2 * vlat * gloAxis3[0] - k_visc3 * vpar * gloAxis1[0] + Pforce * gloAxis1[0];
	Fcm[1] += -k_visc * vperp * gloAxis2[1] - k_visc2 * vlat * gloAxis3[1] - k_visc3 * vpar * gloAxis1[1] + Pforce * gloAxis1[1];
	Fcm[2] += -k_visc * vperp * gloAxis2[2] - k_visc2 * vlat * gloAxis3[2] - k_visc3 * vpar * gloAxis1[2] + Pforce * gloAxis1[2];

	// same for the rotational motion. other effects are not considered.
	// if you're an expert of aeromobilism, you can write better, just substitute these weak formulas for better ones.

	// generic stabilization (very poor approximation)
	gloTtlTorque[0] += -vpar * k_visc_rot_STABILIZE * w[0];
	gloTtlTorque[1] += -vpar * k_visc_rot_STABILIZE * w[1];
	gloTtlTorque[2] += -vpar * k_visc_rot_STABILIZE * w[2];

	double boh = gloAxis3[0] * v[0] + gloAxis3[1] * v[1] + gloAxis3[2] * v[2];

	// effetto coda verticale: decente...
	// vertical tail effect: decent ...
	gloTtlTorque[0] += -k_visc_rot3 * (boh)*gloAxis2[0];
	gloTtlTorque[1] += -k_visc_rot3 * (boh)*gloAxis2[1];
	gloTtlTorque[2] += -k_visc_rot3 * (boh)*gloAxis2[2];

	// =======tota Fcm and toal torque DONE=============

	// ===UPDATE VELOCITY, LINEAR AND ANGULAR TOO===
	// momentum p (linear quantity)
	p[0] = MASS * v[0];
	p[1] = MASS * v[1];
	p[2] = MASS * v[2];

	p[0] = p[0] + Fcm[0] * h;
	p[1] = p[1] + Fcm[1] * h; // we model gravity as a force given by: g*MASS, downward 
	p[2] = p[2] + Fcm[2] * h + g * MASS * h;

	v[0] = p[0] / MASS;
	v[1] = p[1] / MASS;
	v[2] = p[2] / MASS;

	L[0] = L[0] + gloTtlTorque[0] * h;
	L[1] = L[1] + gloTtlTorque[1] * h;
	L[2] = L[2] + gloTtlTorque[2] * h;

	// now we get the updated velocity, component by component.
	w[0] = inv_It_now[0][0] * L[0] + inv_It_now[0][1] * L[1] + inv_It_now[0][2] * L[2];

	w[1] = inv_It_now[1][0] * L[0] + inv_It_now[1][1] * L[1] + inv_It_now[1][2] * L[2];

	w[2] = inv_It_now[2][0] * L[0] + inv_It_now[2][1] * L[1] + inv_It_now[2][2] * L[2];

	// angular momentum (angular/rotational quantity)

	// reset forces to 0.0 
	for (i = 0; i < 3; i++)
	{
		Fcm[i] = 0.0;
		gloTtlTorque[i] = 0.0;
	}

	// update position and orientation
	// xp, yp, zp give the location of the airplane
	xp = xp + v[0] * h;
	yp = yp + v[1] * h;
	zp = zp + v[2] * h;

	// update orientation 
	w_abs = sqrt(w[0] * w[0] + w[1] * w[1] + w[2] * w[2]);

	if (w_abs > 0.0000001)
	{
		u1 = w[0] / w_abs;
		u2 = w[1] / w_abs;
		u3 = w[2] / w_abs;

		dAng = w_abs * h;
	}
	else
	{
		u1 = 1.0;
		u2 = 0.0;
		u3 = 0.0;

		dAng = 0.0;
	}

	// explicitly writing the Skew(w_vector) ... and also D_vector's square 
	SD[0][0] = 0.0;
	SD[0][1] = -u3;
	SD[0][2] = u2;
	SD[1][0] = u3;
	SD[1][1] = 0.0;
	SD[1][2] = -u1;
	SD[2][0] = -u2;
	SD[2][1] = u1;
	SD[2][2] = 0.0;

	// -----------------------------------------------------------
	// Calculate SD2 = SD*SD
	// Multiply SD times SD and place the product in the global variable gloResultMatrix
	mat3x3_mult(SD, SD); 

	// Copy gloResultMatrix to matrix SD2
	for (j = 0; j < 3; j++)
	{
		for (i = 0; i < 3; i++)
		{
			SD2[j][i] = gloResultMatrix[j][i];
		}
	}
	// Done calculating SD2
	// -----------------------------------------------------------

	// Set matrix dR
	for (j = 0; j < 3; j++)
	{
		for (i = 0; i < 3; i++)
		{
			dR[j][i] = Id[j][i] + sin(dAng) * SD[j][i] + (1.0 - cos(dAng)) * SD2[j][i];
		}
	}

	// -----------------------------------------------------------
	// Calculate Rm = dR*Rm
	// Multiply dR times Rm and place the product in the global variable gloResultMatrix
	mat3x3_mult(dR, Rm); 

	// Copy glResultMatrix into Rm
	for (j = 0; j < 3; j++)
	{
		for (i = 0; i < 3; i++)
		{
			Rm[j][i] = gloResultMatrix[j][i];
		}
	}
	// Done calculating Rm = dR*Rm
	// -----------------------------------------------------------

	// -----------------------------------------------------------
	// Set matrix R_T
	// update inertia tensor according to new orientation: It_now = R*It_init*transpose(R) 
	// we build the transpose matrix of R_3x3 matrix, just here 
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			R_T[i][j] = Rm[j][i];
		}
	}
	// Done setting matrix R_T
	// -----------------------------------------------------------

	// -----------------------------------------------------------
	// Calculate It_now = Rm*It_init*R_T
	// by first calculating gloTempMatrix = Rm*It_init
	// and then calculating gloResultMatrix = gloTempMatrix*R_T
	// and then copying gloResultMatrix into It_now
	//
	// we perform the 2 matrix products 
	// gloResultMatrix = Rm*It_Init
	mat3x3_mult(Rm, It_init);

	// Now copy gloResultMatrix into gloTempMatrix
	for (j = 0; j < 3; j++)
	{
		for (i = 0; i < 3; i++)
		{
			gloTempMatrix[j][i] = gloResultMatrix[j][i]; //SAFE COPY!!! PASSING EXTERN VARIABLE AND THEN MODIFYING IT IS NOT SAFE... COMPILERS MAY FAIL TO DO IT CORRECTLY!!!
			//SAFEST SIMPLE METHOD --> BEST METHOD.
		}
	}

	// Now calculate gloResultMatrix = gloTempMatrix * R_T
	mat3x3_mult(gloTempMatrix, R_T);

	// and copy gloResultMatrix into It_now
	for (j = 0; j < 3; j++)
	{
		for (i = 0; i < 3; i++)
		{
			It_now[j][i] = gloResultMatrix[j][i];
		}
	}
	// Done calculating It_now
	// -----------------------------------------------------------

	// -----------------------------------------------------------
	// Calculate inv_It_now = Rm * It_initINV * R_T 
	// by first calculating gloTempMatrix = Rm*It_initINV
	// and then calculating gloResultMatrix = gloTempMatrix*R_T
	// and then copying gloResultMatrix into inv_It_now
	//
	// its inverse too, since it's needed: 
	// we perform the 2 matrix products 
	mat3x3_mult(Rm, It_initINV);

	for (j = 0; j < 3; j++)
	{
		for (i = 0; i < 3; i++)
		{
			gloTempMatrix[j][i] = gloResultMatrix[j][i]; //SAFE COPY!!! PASSING EXTERN VARIABLE AND THEN MODIFYING IT IS NOT SAFE... COMPILERS MAY FAIL TO DO IT CORRECTLY!!!
			//SAFEST SIMPLE METHOD --> BEST METHOD.
		}
	}

	// Calculate gloResultMatrix = gloTempMatrix*R_T
	mat3x3_mult(gloTempMatrix, R_T);

	// we copy gloResultMatrix into inv_It_now
	for (j = 0; j < 3; j++)
	{
		for (i = 0; i < 3; i++)
		{
			inv_It_now[j][i] = gloResultMatrix[j][i];
		}
	}
	// Done calculating inv_It_now
	// -----------------------------------------------------------

	//=================DONE UPDATE OF ORIENTATION MATRIX and intertia tensor=================
	//====END PHYSICALLY SIMULATED UPDATE OF AIRPLANE POS AND ROTATION, ORIENTAION=====	
} // end simulatePhysics function 

// ####################################################################################################################
// Function reorientAxes
// ####################################################################################################################
void reorientAxes()
{
		// now calculate axes in their new 'orientation', using the orientation matrix.

		//R:
		gloAxis1[0] = Rm[0][0] * gloOrigAxis1[0] +
					  Rm[0][1] * gloOrigAxis1[1] +
					  Rm[0][2] * gloOrigAxis1[2];

		gloAxis1[1] = Rm[1][0] * gloOrigAxis1[0] +
					  Rm[1][1] * gloOrigAxis1[1] +
					  Rm[1][2] * gloOrigAxis1[2];

		gloAxis1[2] = Rm[2][0] * gloOrigAxis1[0] +
					  Rm[2][1] * gloOrigAxis1[1] +
					  Rm[2][2] * gloOrigAxis1[2];

		gloAxis2[0] = Rm[0][0] * gloOrigAxis2[0] +
					  Rm[0][1] * gloOrigAxis2[1] +
					  Rm[0][2] * gloOrigAxis2[2];

		gloAxis2[1] = Rm[1][0] * gloOrigAxis2[0] +
					  Rm[1][1] * gloOrigAxis2[1] +
					  Rm[1][2] * gloOrigAxis2[2];

		gloAxis2[2] = Rm[2][0] * gloOrigAxis2[0] +
					  Rm[2][1] * gloOrigAxis2[1] +
					  Rm[2][2] * gloOrigAxis2[2];

		gloAxis3[0] = Rm[0][0] * gloOrigAxis3[0] +
					  Rm[0][1] * gloOrigAxis3[1] +
					  Rm[0][2] * gloOrigAxis3[2];

		gloAxis3[1] = Rm[1][0] * gloOrigAxis3[0] +
					  Rm[1][1] * gloOrigAxis3[1] +
					  Rm[1][2] * gloOrigAxis3[2];

		gloAxis3[2] = Rm[2][0] * gloOrigAxis3[0] +
					  Rm[2][1] * gloOrigAxis3[1] +
					  Rm[2][2] * gloOrigAxis3[2];

		Pa[0] = gloAxis1[0];
		Pa[1] = gloAxis1[1];
		Pa[2] = gloAxis1[2];

		Qa[0] = gloAxis2[0];
		Qa[1] = gloAxis2[1];
		Qa[2] = gloAxis2[2];

		Ra[0] = gloAxis3[0];
		Ra[1] = gloAxis3[1];
		Ra[2] = gloAxis3[2];
		// =======END OF AXES REORIENTATION DONE============
} // end reorientAxes function

// ####################################################################################################################
// Function updateVirtualCameraPos
// ####################################################################################################################
void updateVirtualCameraPos(float RR)
{
	// moving plane's CM 
	if (aboard == 1)
	{
		x = xp + RR * R[0];
		y = yp + RR * R[1];
		z = zp + RR * R[2];

		if (gloView == 4)
		{ 
			// INTERNAL VIEW COCKPIT
			// now: here the camera position is very important.
			x = xp + z_cockpit_view * P[0] + y_cockpit_view * Q[0] + x_cockpit_view * R[0];
			y = yp + z_cockpit_view * P[1] + y_cockpit_view * Q[1] + x_cockpit_view * R[1];
			z = zp + z_cockpit_view * P[2] + y_cockpit_view * Q[2] + x_cockpit_view * R[2];
		}
	}
	else
	{
		x = x_pilot;
		y = y_pilot;
		z = say_terrain_height(&gloTerrain, x_pilot, y_pilot) + 1.75;
	}
} // end updateVirtualCameraPos function

// ####################################################################################################################
// Function processEvent
// ####################################################################################################################
void processEvent(float *turnch, float *turncv, float *RR, 
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
			*RR = *RR - 2.0; 
			if (gloView == 4)
			{
				x_cockpit_view = x_cockpit_view - 0.1;
			}
		}
		if (gloEvent.key.keysym.sym == SDLK_2)
		{
			*RR = *RR + 2.0;
			if (gloView == 4)
			{
				x_cockpit_view = x_cockpit_view + 0.1;
			}
		}

		if (gloEvent.key.keysym.sym == SDLK_3)
		{
			y_cockpit_view = y_cockpit_view + 0.1;
			if (y_cockpit_view > 2.0)
			{
				y_cockpit_view = -2.0;
			}
		}
		if (gloEvent.key.keysym.sym == SDLK_4)
		{
			z_cockpit_view = z_cockpit_view + 0.1;
			if (z_cockpit_view > 2.0)
			{
				z_cockpit_view = -2.0;
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
				import_airplane_polyheron();
			}
		}

		if (gloEvent.key.keysym.sym == SDLK_e)
		{
			addfrantumation_wsim(20, 20, 20, *h, 1);
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
			aboard = -1 * aboard;
			x_pilot = xp;
			y_pilot = yp;
			*RR = 1.5;
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
			projectile_launch(xp, yp, zp, v[0] + 100.0 * Pa[0], v[1] + 100.0 * Pa[1], v[2] + 100.0 * Pa[2], *h, 1);
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
			// test
			*plane_inclleft = 1;
		}
		if (gloEvent.key.keysym.sym == SDLK_RIGHT || gloEvent.key.keysym.sym == SDLK_d)
		{
			// test
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

// ####################################################################################################################
// Function xclearpixboard
// ####################################################################################################################
void xclearpixboard(int xlimit, int ylimit)
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
} // end xclearpixboard function

// ####################################################################################################################
// Function sdldisplay
// ####################################################################################################################
void sdldisplay() 
{
	SDL_GL_SwapWindow(gloWindow);
} // end sdldisplay function

// ####################################################################################################################
// Function waitdt_ms is a timer function.
// A timer function to pause to regulate FPS is a good utility to have...
// ####################################################################################################################
int waitdt_ms(double tt_ms)
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
} // end waitdt_ms() function 

// ####################################################################################################################
// Function xaddline
// NOT USED IN OPENGL VERSION 
// interpolate 2 points graphically 
// 
// This function is not called.
// ####################################################################################################################
void xaddline(int x1, int y1,
			  int x2, int y2, float color[3], // change this to int color[3] 
			  int xlimit, int ylimit)
{

	float m, fx1, fx2, fy1, fy2, fi;
	int i, j, yh, temp, yh_aux;

	if (fabs(x2 - x1) > 0)
	{ // IF LINE IS NON-VERTICAL: avoid divide-by-zero case!! 
		fx1 = (float)x1;
		fx2 = (float)x2;

		fy1 = (float)y1;
		fy2 = (float)y2;

		m = (fy2 - fy1) / (fx2 - fx1);
		// case x2 > x1 : augment from x1 to come to x2... 
		if (x1 > x2)
		{ // interchange them... 
			temp = x2;
			x2 = x1;
			x1 = temp;
		}

		for (i = x1; i < x2; i++)
		{
			fi = (float)i;
			yh = (int)(m * fi - m * fx1 + fy1);

			if ((i >= 0 && i < xlimit) && (yh >= 0 && yh < ylimit))
			{ // limits! 
				pixmatrix[yh][i][0] = color[0];
				pixmatrix[yh][i][1] = color[1];
				pixmatrix[yh][i][2] = color[2];
				// this will have 3 components, thanks to hi color-res of SDL 

				// these points are good for cases -1.0 < m < 1.0 but are part of the 
				// super-filling for cases of m outside ( -1.0, 1.0 ) range. 

				// nice continuous lines for cases of m outside ( -1.0, 1.0 ) range. 

				if ((m > 1.0))
				{
					fi = (float)(i + 1);
					yh_aux = (int)(m * fi - m * fx1 + fy1);
					for (j = yh; j < yh_aux; j++)
					{
						if ((i >= 0 && i < xlimit) && (j >= 0 && j < ylimit))
						{
							pixmatrix[j][i][0] = color[0];
							pixmatrix[j][i][1] = color[1];
							pixmatrix[j][i][2] = color[2];
						}
					}
				}
				else if ((m < -1.0))
				{
					fi = (float)(i + 1);
					yh_aux = (int)(m * fi - m * fx1 + fy1);
					for (j = yh; j > yh_aux; j--)
					{
						if ((i >= 0 && i < xlimit) && (j >= 0 && j < ylimit))
						{
							pixmatrix[j][i][0] = color[0];
							pixmatrix[j][i][1] = color[1];
							pixmatrix[j][i][2] = color[2];
						}
					} // for j
				}
			}
		} // for i
	}
	else
	{ // IF LINE IS VERTICAL 
		if (y1 < y2)
		{ // case y1 < y2 : augment from y1 to come to y2... 
			for (yh = y1; yh < y2; yh++)
			{
				if ((x1 >= 0 && x1 < xlimit) && (yh >= 0 && yh < ylimit))
				{ // limits! 
					pixmatrix[yh][x1][0] = color[0];
					pixmatrix[yh][x1][1] = color[1];
					pixmatrix[yh][x1][2] = color[2];
				}
			}
		}
		else
		{ // case y2 < y1 : augment from y1 to come to y2... 
			for (yh = y2; yh < y1; yh++)
			{
				if ((x1 >= 0 && x1 < xlimit) && (yh >= 0 && yh < ylimit))
				{ // limits! 
					pixmatrix[yh][x1][0] = color[0];
					pixmatrix[yh][x1][1] = color[1];
					pixmatrix[yh][x1][2] = color[2];
				}
			}
		}
	}
} // end xaddline function

// ####################################################################################################################
// Function xadd1pix
// add 1 pixel to output image but in a failsafe manner: no accidental segfaults. 
//
// This function is not called.
// ####################################################################################################################
void xadd1pix(int x, int y, float color[3],
			  int xlimit, int ylimit)
{
	if ((x >= 0 && x <= xlimit) && (y <= ylimit && y >= 0))
	{
		pixmatrix[y][x][0] = color[0];
		pixmatrix[y][x][1] = color[1];
		pixmatrix[y][x][2] = color[2];
	}
} // end xadd1pix function

// ####################################################################################################################
// Function xaddpoint_persp draws a (perspective) point in 3D space.
//
// now we define the function which, given 1 point in 3D, calculates where it ends up on the
// virtual camera pointing toward positive z-axis and passes them to the failsafe pixel drawing function. 
// ####################################################################################################################
void xaddpoint_persp(float x1, float y1, float z1, float color[3])
{
	glColor3f(color[0], color[1], color[2]);
	glPointSize(2);

	glBegin(GL_POINTS);
		glVertex3f(x1, y1, -z1);
	glEnd();

	glFlush();
} // end xaddpoint_persp function

// ####################################################################################################################
// Function xaddline_persp draws a (perspective) line in 3D space.
//
// now we define the function which, given 2 points in 3D, calculates where they end up on the
// virtual camera pointing toward positive z-axis and passes them to the 2D line drawing function. 
// ####################################################################################################################
void xaddline_persp(float x1, float y1, float z1, 
					float x2, float y2, float z2, float color[3])
{
	glColor3f(color[0], color[1], color[2]);

	glBegin(GL_LINES);
		glVertex3f(x1, y1, -z1);
		glVertex3f(x2, y2, -z2);
	glEnd();

	glFlush();
} // end xaddline_persp function

// ####################################################################################################################
// Function addsmoke_wsim draws smoke at the point (x0, y0, z0)
//
// point frantumation sequence function (a special effect)
// add new explosion or just process those already started
// ####################################################################################################################
void addsmoke_wsim(double x0, double y0, double z0, double dft, int option)
{
#define NPS 800
#define NAUTSM 44
	static int count[NAUTSM], LT = 900; // sequence lifetime
	// visc = coefficient of viscous force 
	static int visc = 3.9; // why is visc given the value 3.9 when it is defined as an int?
	static int MAXN, N_as = 1;
	static double xc, yc, zc, radius[NAUTSM][NPS], Vix = 0.0, Viy = 0.0, Viz = 2.0;
	int i, j, k;
	static double xm[NAUTSM][NPS], ym[NAUTSM][NPS], zm[NAUTSM][NPS], vx[NAUTSM][NPS], vy[NAUTSM][NPS], vz[NAUTSM][NPS];
	float xt, yt, zt, xt2, yt2, zt2, xt3, yt3, zt3, color[4], colors[NAUTSM][NPS][3];
	static int auxc = 0;

	glDisable(GL_DEPTH_TEST);
	if (option == 1 && N_as < NAUTSM)
	{
		// First place reserved to autosmoke... pointlike evaporating smoke
		count[N_as] = LT;
		xc = (float)x0;
		yc = (float)y0;
		zc = (float)z0;

		auxc = 0;
		// careful.. total must equal MP contant, otherwise segfault will happen. 
		for (i = -2; i < 3; i++)
		{ // SO: -1, 0 , 1
			for (j = -2; j < 3; j++)
			{
				for (k = -2; k < 3; k++)
				{
					// do (LT-count) because 'count' variable starts froma LT, say, 30. 
					xm[N_as][auxc] = xc + 0.1 * auxc * (double)rand() / (double)RAND_MAX; 
					ym[N_as][auxc] = yc + 0.1 * auxc * (double)rand() / (double)RAND_MAX;
					zm[N_as][auxc] = zc + 0.1 * auxc * (double)rand() / (double)RAND_MAX;

					vx[N_as][auxc] = Vix * i; 
					vy[N_as][auxc] = Viy * j;
					vz[N_as][auxc] = Viz * k;

					radius[N_as][auxc] = 0.01;

					// a base gray plus a little random 
					colors[N_as][auxc][0] = 0.6 + (float)0.2 * rand() / (float)RAND_MAX; 
					colors[N_as][auxc][1] = 0.6 + (float)0.2 * rand() / (float)RAND_MAX;
					colors[N_as][auxc][2] = 0.6 + (float)0.2 * rand() / (float)RAND_MAX;

					auxc++;
					if (auxc >= NPS)
					{ // precaution against overflow
						printf("HAY WRONG NUMBER OF PARTICLES!!!! CORRECT!!\n");
						SDL_Quit();
						exit(1);
						break;
					}
				}
			}
		}

		printf("CHECK NUMBER OF PARTICLES!!!! ===%i=== CORRECT??\n", auxc);
		MAXN = auxc;
		auxc = 0; // put it back to zero!!! see option 2 to undersand why!

		N_as++; // we augment it... first slot is used for dynamically dragged smoke seq
	}

	if (option == 2)
	{
		// "accumulo" per cosiddire le posizioni nel blocco di numeri float px, py e pz, che abbiamo creato apposta
		// "accumulation" to consider the positions in the block of numbers float px, py, and pz, which we have created only
		for (i = MAXN - 1; i > 0; i--)
		{
			xm[0][i] = xm[0][i - 1];
			ym[0][i] = ym[0][i - 1];
			zm[0][i] = zm[0][i - 1];

			vx[0][i] = vx[0][i - 1];
			vy[0][i] = vy[0][i - 1];
			vz[0][i] = vz[0][i - 1];

			radius[0][i] = radius[0][i - 1];
		}

		// nella primissima casella mettiamo la posizione di "ADESSO" 
		// in the very first box we put the position of "NOW"
		xm[0][0] = x0; 
		ym[0][0] = y0;
		zm[0][0] = z0;

		radius[0][i] = 0.01;

		// a base gray plus a little random 
		colors[0][i][0] = 1.0; 
		colors[0][i][1] = 1.0;
		colors[0][i][2] = 1.0;
	}

	for (j = 0; j < N_as; j++)
	{
		if (count[j] > 0)
		{ 
			// Check extension process stuff and decrease count only as long as it's above 0 yet
			// all combination of 1 and 0, see why... JUST A BASIC TRICK, this is NOT A PROFI SPECIAL EFFECT
			float F_pullup = 3.0;

			// update positions and draw, at the same time... 
			for (i = 0; i < LT - count[j] && i < MAXN - 1; i++)
			{
				// start 'simulating' progressively more of the total particles
				// so smoke will seem to be fed from it's start point
				float rand_F = 300.0 * ((double)0.3 * rand() / (double)RAND_MAX); // it makes dissolution 

				radius[j][i] = radius[j][i] + 13.8 * ((double)rand() / (double)RAND_MAX) * dft;

				// do (LT-count) because 'count' variable starts from LT, say, 30. 
				xm[j][i] = xm[j][i] + vx[j][i] * dft; 
				ym[j][i] = ym[j][i] + vy[j][i] * dft;
				zm[j][i] = zm[j][i] + vz[j][i] * dft;

				vx[j][i] = vx[j][i] - visc * vx[j][i] * dft + rand_F * dft;	// inertial... 
				vy[j][i] = vy[j][i] - visc * vy[j][i] * dft + rand_F * dft;	// inertial... 

				// NOT accelerated by classical gravity because it would not be believable... its like a Brownian motion 
				vz[j][i] = vz[j][i] - visc * vz[j][i] * dft + 0.1 * rand_F * dft + F_pullup * dft; 

				if (zm[j][i] < say_terrain_height(&gloTerrain, xm[j][i], ym[j][i]))
				{
					double je;
					je = gloTerrain.auxnormal[0] * vx[j][i] + gloTerrain.auxnormal[1] * vy[j][i] + gloTerrain.auxnormal[2] * vz[j][i];

					vx[j][i] = vx[j][i] - (1.0 + 0.95) * je * gloTerrain.auxnormal[0] / 1.0; 
					vy[j][i] = vy[j][i] - (1.0 + 0.95) * je * gloTerrain.auxnormal[1] / 1.0;
					vz[j][i] = vz[j][i] - (1.0 + 0.95) * je * gloTerrain.auxnormal[2] / 1.0;
				}

				xt = P[0] * (xm[j][i] - x) + P[1] * (ym[j][i] - y) + P[2] * (zm[j][i] - z);
				yt = Q[0] * (xm[j][i] - x) + Q[1] * (ym[j][i] - y) + Q[2] * (zm[j][i] - z);
				zt = R[0] * (xm[j][i] - x) + R[1] * (ym[j][i] - y) + R[2] * (zm[j][i] - z);

				// better with OpenGL graphics as long as this. or a better organized 3D drawing routine.
				xt2 = P[0] * (xm[j][i] + radius[j][i] - x) + P[1] * (ym[j][i] + radius[j][i] - y) + P[2] * (zm[j][i] - z);
				yt2 = Q[0] * (xm[j][i] + radius[j][i] - x) + Q[1] * (ym[j][i] + radius[j][i] - y) + Q[2] * (zm[j][i] - z);
				zt2 = R[0] * (xm[j][i] + radius[j][i] - x) + R[1] * (ym[j][i] + radius[j][i] - y) + R[2] * (zm[j][i] - z);

				xt3 = P[0] * (xm[j][i] + 0.6 * radius[j][i] - x) + P[1] * (ym[j][i] + 1.3 * radius[j][i] - y) + P[2] * (zm[j][i] + radius[j][i] - z);
				yt3 = Q[0] * (xm[j][i] + 0.6 * radius[j][i] - x) + Q[1] * (ym[j][i] + 1.3 * radius[j][i] - y) + Q[2] * (zm[j][i] + radius[j][i] - z);
				zt3 = R[0] * (xm[j][i] + 0.6 * radius[j][i] - x) + R[1] * (ym[j][i] + 1.3 * radius[j][i] - y) + R[2] * (zm[j][i] + radius[j][i] - z);

				color[0] = colors[j][i][0];
				color[1] = colors[j][i][1];
				color[2] = colors[j][i][2];
				color[3] = 0.1;

				xaddftriang_persp( xt,  yt,  -zt,
								  xt2, yt2, -zt2,
								  xt3, yt3, -zt3, 
								  color);
			}
		}

		count[0] = 100;
		if (count[j] > 0 && j != 0)
		{
			count[j]--;
		}
		else if (count[j] == 0) // j != 0 && 
		{
			for (k = j; k < N_as; k++)
			{
				for (i = MAXN - 1; i > 0; i--)
				{
					xm[k][i] = xm[k + 1][i];
					ym[k][i] = ym[k + 1][i];
					zm[k][i] = zm[k + 1][i];

					vx[k][i] = vx[k + 1][i];
					vy[k][i] = vy[k + 1][i];
					vz[k][i] = vz[k + 1][i];

					radius[k][i] = radius[k + 1][i];
				}

				if (count[k + 1] >= 0 && count[k + 1] < NAUTSM)
				{
					count[k] = count[k + 1];
				}
				else
				{
					count[k] = 0;
				}
			}

			// reduce total num of smoke sequences in scene but 'delete' the ended 
			// slot too by making shrink into it all previous ones.
			N_as--; 
		} // else block
	} // NAUTSM count

	glEnable(GL_DEPTH_TEST);
} // end addsmoke_wsim function

// ####################################################################################################################
// Function addfrantumation_wsim
// Gli effetti speciali di base nei games / the basic special effects in the games
// point frantumation sequence function (a special effect)
// add new explosion or just process those already started
// ####################################################################################################################
void addfrantumation_wsim(float x0, float y0, float z0, double dft, int option)
{
#define NP 100
	static int count = 0, LT = 400; // sequence lifetime
	static float xc, yc, zc, radius = 4.2, color[4], colors[NP][3], Vix = 4.0, Viy = 4.0, Viz = 20.3, visc = 0.95;
	int i, j, k, auxc;
	static float xm[NP], ym[NP], zm[NP], vx[NP], vy[NP], vz[NP];
	float xt, yt, zt, xt2, yt2, zt2, xt3, yt3, zt3;

	if (option == 1)
	{
		count = LT;
		xc = x0;
		yc = y0;
		zc = z0;

		auxc = 0;
		for (i = -1; i < 2; i++)
		{ // SO: -1, 0 , 1
			for (j = -1; j < 2; j++)
			{
				for (k = -1; k < 2; k++)
				{
					xm[auxc] = xc + 1.0 * (double)rand() / (double)RAND_MAX; 
					ym[auxc] = yc + 1.0 * (double)rand() / (double)RAND_MAX;
					zm[auxc] = zc + 1.0 * (double)rand() / (double)RAND_MAX;

					vx[auxc] = Vix * i + 4.2 * (double)rand() / (double)RAND_MAX; 
					vy[auxc] = Viy * j + 4.2 * (double)rand() / (double)RAND_MAX;
					vz[auxc] = Viz * k + 4.2 * (double)rand() / (double)RAND_MAX;

					colors[auxc][0] = (float)rand() / (float)RAND_MAX;
					colors[auxc][1] = (float)0.5 * rand() / (float)RAND_MAX;
					colors[auxc][2] = (float)0.3 * rand() / (float)RAND_MAX;

					auxc++;
				}
			}
		}
	}

	if (count > 0)
	{ 
		// process stuff and decrease count only as long as it's above 0 yet 
		printf("DRAWING FRANTUMATION SEQUENCE...\n");
		double visca = 0; // copy 
		// all combination of 1 and 0, see why... JUST A BASIC TRICK, this is NOT A PROFI SPECIAL EFFECT

		// update positions and draw, at the same time
		for (i = 0; i < 27; i++)
		{ // SO: -1, 0 , 1
			if (zm[i] < say_terrain_height(&gloTerrain, xm[i], ym[i]))
			{
				double je;
				je = gloTerrain.auxnormal[0] * vx[i] + gloTerrain.auxnormal[1] * vy[i] + gloTerrain.auxnormal[2] * vz[i];

				if (je < 0.0)
				{
					visca = visc;
					vx[i] = vx[i] - (1.0 + 0.8) * je * gloTerrain.auxnormal[0] / 1.0; 
					vy[i] = vy[i] - (1.0 + 0.8) * je * gloTerrain.auxnormal[1] / 1.0;
					vz[i] = vz[i] - (1.0 + 0.8) * je * gloTerrain.auxnormal[2] / 1.0;
					printf(">>>>>>>>>IMPACT \n");
				}
			}

			xm[i] = xm[i] + vx[i] * dft; 
			ym[i] = ym[i] + vy[i] * dft;
			zm[i] = zm[i] + vz[i] * dft;

			vx[i] = vx[i] - visca * vx[i] * dft + 0.0 * dft;  // inertial...
			vy[i] = vy[i] - visca * vy[i] * dft + 0.0 * dft;  // inertial...
			vz[i] = vz[i] - visca * vz[i] * dft - 9.81 * dft; // accelerated by classical gravity 

			xt = P[0] * (xm[i] - x) + P[1] * (ym[i] - y) + P[2] * (zm[i] - z);
			yt = Q[0] * (xm[i] - x) + Q[1] * (ym[i] - y) + Q[2] * (zm[i] - z);
			zt = R[0] * (xm[i] - x) + R[1] * (ym[i] - y) + R[2] * (zm[i] - z);

			// better with OpenGL graphics as long as this. or a better organized 3D drawing routine.
			xt2 = P[0] * (xm[i] + radius - x) + P[1] * (ym[i] - y) + P[2] * (zm[i] - z);
			yt2 = Q[0] * (xm[i] + radius - x) + Q[1] * (ym[i] - y) + Q[2] * (zm[i] - z);
			zt2 = R[0] * (xm[i] + radius - x) + R[1] * (ym[i] - y) + R[2] * (zm[i] - z);

			xt3 = P[0] * (xm[i] - x) + P[1] * (ym[i] + 1.2 * radius - y) + P[2] * (zm[i] + radius - z);
			yt3 = Q[0] * (xm[i] - x) + Q[1] * (ym[i] + 1.2 * radius - y) + Q[2] * (zm[i] + radius - z);
			zt3 = R[0] * (xm[i] - x) + R[1] * (ym[i] + 1.2 * radius - y) + R[2] * (zm[i] + radius - z);

			xaddpoint_persp(xt, yt, -zt, color); // draw points in 3D scenario Z NEGATIVE!!!!!! 

			color[0] = colors[i][0];
			color[1] = colors[i][1];
			color[2] = colors[i][2];
			color[3] = 1.0;
			xaddftriang_persp( xt,  yt,  -zt,
							  xt2, yt2, -zt2,
							  xt3, yt3, -zt3, 
							  color);
		}

		count--;
	}
} // end addfrantumation_wsim function

// ####################################################################################################################
// Function projectile_launch
// ####################################################################################################################
void projectile_launch(float xpr, float ypr, float zpr, 
					   float vx, float vy, float vz, 
					   double dft, int do_add)
{
	static int n = 0;
	static float poss[100][3];
	static float vels[100][3];
	float color[3] = {1.0, 0.0, 1.0};
	float th_sph; // theta or spherical coordinated, used for a spherical 'explosion' 
	static int life[100];
	float xm, ym, zm, xt, yt, zt;
	int i, k;

	if (do_add == 1)
	{
		poss[n][0] = xpr;
		poss[n][1] = ypr;
		poss[n][2] = zpr;

		vels[n][0] = vx;
		vels[n][1] = vy;
		vels[n][2] = vz;

		life[n] = 1; // do not set to 0 because it mekes start the explosion cycle.

		n++;
	}

	// draw projectiles
	for (i = 0; i < n; i++)
	{
		if (life[i] > 0)
		{
			xm = poss[i][0]; 
			ym = poss[i][1];
			zm = poss[i][2];

			// "x is an extern variable!!! be careful!!" 
			xt = P[0] * (xm - x) + P[1] * (ym - y) + P[2] * (zm - z);
			yt = Q[0] * (xm - x) + Q[1] * (ym - y) + Q[2] * (zm - z);
			zt = R[0] * (xm - x) + R[1] * (ym - y) + R[2] * (zm - z);

			xaddpoint_persp(xt, yt, -zt, color); // draw points in 3D scenario Z NEGATIVE!!!!!! 
		}
	}

	// update positions.
	for (i = 0; i < n; i++)
	{
		if (life[i] > 0)
		{ // normal life cycle ( goes ahead )
			poss[i][0] = poss[i][0] + vels[i][0] * dft;
			poss[i][1] = poss[i][1] + vels[i][1] * dft;
			poss[i][2] = poss[i][2] + vels[i][2] * dft;

			vels[i][0] = vels[i][0] + 0.0;
			vels[i][1] = vels[i][1] + 0.0;
			vels[i][2] = vels[i][2] - 9.81 * dft; // classical gravity 

			if (poss[i][2] < say_terrain_height(&gloTerrain, poss[i][0], poss[i][1]))
			{
				double je;
				je = gloTerrain.auxnormal[0] * vels[i][0] + gloTerrain.auxnormal[1] * vels[i][1] + gloTerrain.auxnormal[2] * vels[i][2];
				printf(">>>>IMPACT??? j = %3.3f \n", je);

				if (je < 0.0)
				{
					vels[i][0] = vels[i][0] - (1.0 + 0.1) * je * gloTerrain.auxnormal[0] / 1.0; 
					vels[i][1] = vels[i][1] - (1.0 + 0.1) * je * gloTerrain.auxnormal[1] / 1.0;
					vels[i][2] = vels[i][2] - (1.0 + 0.1) * je * gloTerrain.auxnormal[2] / 1.0;
					life[i] = 0; // put its lifetime near the end.... so soon explosion cycle will start 

					gloTerrain.shmap[(int)(poss[i][0] / gloTerrain.GPunit)][(int)(poss[i][1] / gloTerrain.GPunit)]    =       gloTerrain.shmap[(int)(poss[i][0] / gloTerrain.GPunit)][(int)(poss[i][1] / gloTerrain.GPunit)] - 0.02;
					gloTerrain.scol [(int)(poss[i][0] / gloTerrain.GPunit)][(int)(poss[i][1] / gloTerrain.GPunit)][1] = 0.8 * gloTerrain.scol [(int)(poss[i][0] / gloTerrain.GPunit)][(int)(poss[i][1] / gloTerrain.GPunit)][1];

					printf(">>>>>>>>>IMPACT \n");
					addsmoke_wsim(poss[i][0], poss[i][1], poss[i][2], dft, 1);		  // add a smoke sequence at disappeared point.
					addfrantumation_wsim(poss[i][0], poss[i][1], poss[i][2], dft, 1); // add a frantumation sequance at disappeared point.
				}
			}
		}
		else if (life[i] < 0)
		{ // explosion cycle
			for (th_sph = 0; th_sph < 6.3; th_sph = th_sph + 0.3)
			{
				float fi_sph, radius;
				radius = (life[i] + 40.0);

				for (fi_sph = 0; fi_sph < 6.3; fi_sph = fi_sph + 0.3)
				{
					xm = poss[i][0] + 0.1 * radius * cos(th_sph) * sin(fi_sph); 
					ym = poss[i][1] + 0.1 * radius * sin(th_sph) * sin(fi_sph);
					zm = poss[i][2] + 0.1 * radius * cos(fi_sph);

					xt = P[0] * (xm - x) + P[1] * (ym - y) + P[2] * (zm - z);
					yt = Q[0] * (xm - x) + Q[1] * (ym - y) + Q[2] * (zm - z);
					zt = R[0] * (xm - x) + R[1] * (ym - y) + R[2] * (zm - z);

					color[0] = 1.0;
					color[1] = 0.6;
					color[2] = 0.1;
					xaddpoint_persp(xt, yt, -zt, color); // draw points in 3D scenario Z NEGATIVE!!!!!! 
				}
			}
		}

		life[i]++; // works for lifecycle and explosion cycle too.
		if (life[i] > 245)
		{
			life[i] = 0; // start explosion cycle.
		}

		if (life[i] == 0)
		{
			n--;
			for (k = 1; k < n; k++)
			{
				poss[k - 1][0] = poss[k][0];
				poss[k - 1][1] = poss[k][1];
				poss[k - 1][2] = poss[k][2];

				vels[k - 1][0] = vels[k][0];
				vels[k - 1][1] = vels[k][1];
				vels[k - 1][2] = vels[k][2];

				life[k - 1] = life[k];
			} // for k
		}
	}
} // end projectile_launch function

// ####################################################################################################################
// Function say_terrain_height
// ####################################################################################################################
double say_terrain_height(struct subterrain *ite, double x, double z)
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
} // end say_terrain_height function

// ####################################################################################################################
// Function xaddftriang_persp draws a (perspective) triangle in 3D space.
//
// now we define the function which, given 3 points in 3D, calculates where they end up on the
// virtual camera pointing toward positive z-s and passes them to the 3D line drawing function. 
// ####################################################################################################################
void xaddftriang_persp(float x1, float y1, float z1,
					   float x2, float y2, float z2,
					   float x3, float y3, float z3,
					   float color[4])
{
	glColor4f(color[0], color[1], color[2], color[3]);

	glBegin(GL_TRIANGLES);
		glVertex3f(x1, y1, -z1);
		glVertex3f(x2, y2, -z2);
		glVertex3f(x3, y3, -z3);
	glEnd();

	glFlush();
} // end xaddftriang_persp function

// ####################################################################################################################
// Function GLaddftriang_perspTEXTURED
//
// Called only from function drawTerrain.
// ####################################################################################################################
void GLaddftriang_perspTEXTURED(float x1, float y1, float z1,
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
} // end GLaddftriang_perspTEXTURED function

// ####################################################################################################################
// Function xaddftriang_perspTEXTURED_pp
// ####################################################################################################################
void xaddftriang_perspTEXTURED_pp(float x1, float y1, float z1,
								  float x2, float y2, float z2,
								  float x3, float y3, float z3,
								  int step,
								  float color[3])
{
	static int alternative = 0;
	static int texture_generated = 0; // at first call of this function, a 32x32 texture sample will be generated 
	float mag; // this local variable is not used
	static float txt[32][32];
	int j, i;

	if (texture_generated == 0)
	{
		texture_generated = 1;

		for (j = 0; j < 32; j++)
		{
			for (i = 0; i < 32; i++)
			{
				txt[j][i] = 0.5 + (float)0.5 * rand() / (float)RAND_MAX;
			}
		}
	}

	if ((pow(x3 - x2, 2) + pow(y3 - y2, 2) + pow(z3 - z2, 2)) > (pow(x3 - x1, 2) + pow(y3 - y1, 2) + pow(z3 - z1, 2)) && (pow(x3 - x2, 2) + pow(y3 - y2, 2) + pow(z3 - z2, 2)) > pow(x2 - x1, 2) + pow(y2 - y1, 2) + pow(z2 - z1, 2))
	{ 
		// side 2 longer than the others: x2 is the hypothenuse of a quasi-rect triangle.
		// leave p1, p2, p3 as they are... they are ok.
		alternative = 1; // first case: 1-2 and 1-3 are the catetes... .
	}
	else if ((pow(x2 - x1, 2) + pow(y2 - y1, 2) + pow(z2 - z1, 2)) > (pow(x3 - x1, 2) + pow(y3 - y1, 2) + pow(z3 - z1, 2)) && (pow(x2 - x1, 2) + pow(y2 - y1, 2) + pow(z2 - z1, 2)) > pow(x2 - x3, 2) + pow(y2 - y3, 2) + pow(z2 - z3, 2))
	{
		// change order... .
		float tx, ty, tz;

		alternative = 2; // second case. 1-3 and 2-3 are cathetes
		// must change order so that 1-2 and 1-3 be cathetes:

		// 2 interchanged with 3 .
		tx = x3;
		ty = y3;
		tz = z3;

		x3 = x2;
		y3 = y2;
		z3 = z2;

		x2 = tx;
		y2 = ty;
		z2 = tz;

		// 1 interchanged with 2 .
		tx = x1;
		ty = y1;
		tz = z1;

		x1 = x2;
		y1 = y2;
		z1 = z2;

		x2 = tx;
		y2 = ty;
		z2 = tz;
	}
	else
	{
		float tx, ty, tz;

		alternative = 3.0;

		// 2 interchanged with 3 .
		tx = x3;
		ty = y3;
		tz = z3;

		x3 = x2;
		y3 = y2;
		z3 = z2;

		x2 = tx;
		y2 = ty;
		z2 = tz;

		// 1 interchanged with 3 .
		tx = x1;
		ty = y1;
		tz = z1;

		x1 = x3;
		y1 = y3;
		z1 = z3;

		x3 = tx;
		y3 = ty;
		z3 = tz;
	}

	float v_hor[3] = {3.4, 1.2, 2.0}; // the one with greater x-component... ('more horizonal'
	float v_ver[3] = {1.2, 2.3, 1.0}; // the one wiht greater y-component... ('more vertical' )
									  // first x-y couple is intact... MIDPOINT, DOWNER 

	float jff, iff, limh, limv, texres;
	texres = step; // texture resolution

	v_ver[0] = (x2 - x1) / texres;
	v_ver[1] = (y2 - y1) / texres;
	v_ver[2] = (z2 - z1) / texres;

	v_hor[0] = (x3 - x1) / texres;
	v_hor[1] = (y3 - y1) / texres;
	v_hor[2] = (z3 - z1) / texres;

	//=== dimension = texres!!! ===
	float color_app[3];

	for (jff = 0; jff < texres; jff = jff + 1.0)
	{
		limh = -jff + texres;

		for (iff = 0; iff < limh; iff = iff + 1.0)
		{
			color_app[0] = color[0] * (txt[(int)jff][(int)iff]);
			color_app[1] = color[1] * (txt[(int)jff][(int)iff]);
			color_app[2] = color[2] * (txt[(int)jff][(int)iff]);

			// tri 1
			glColor3f(color_app[0], color_app[1], color_app[2]);

			glBegin(GL_TRIANGLES);
				glVertex3f(x1 + jff * v_hor[0] + iff * v_ver[0],
						   y1 + jff * v_hor[1] + iff * v_ver[1],
						  (z1 + jff * v_hor[2] + iff * v_ver[2]));

				glVertex3f(x1 + (jff + 1.0) * v_hor[0] + (iff + 0.0) * v_ver[0],
						   y1 + (jff + 1.0) * v_hor[1] + (iff + 0.0) * v_ver[1],
						  (z1 + (jff + 1.0) * v_hor[2] + (iff + 0.0) * v_ver[2]));

				glVertex3f(x1 + (jff + 0.0) * v_hor[0] + (iff + 1.0) * v_ver[0],
						   y1 + (jff + 0.0) * v_hor[1] + (iff + 1.0) * v_ver[1],
						  (z1 + (jff + 0.0) * v_hor[2] + (iff + 1.0) * v_ver[2]));
			glEnd();
			glFlush();

			// tri 2
			if (iff != limh - 1)
			{
				// Don't draw this if we are on edge.
				glColor3f(color_app[0], color_app[1] + 0.01, color_app[2]);
				glBegin(GL_TRIANGLES);
					glVertex3f(x1 + (jff + 1.0) * v_hor[0] + (iff + 0.0) * v_ver[0],
							   y1 + (jff + 1.0) * v_hor[1] + (iff + 0.0) * v_ver[1],
							  (z1 + (jff + 1.0) * v_hor[2] + (iff + 0.0) * v_ver[2]));

					glVertex3f(x1 + (jff + 1.0) * v_hor[0] + (iff + 1.0) * v_ver[0],
							   y1 + (jff + 1.0) * v_hor[1] + (iff + 1.0) * v_ver[1],
							  (z1 + (jff + 1.0) * v_hor[2] + (iff + 1.0) * v_ver[2]));

					glVertex3f(x1 + (jff + 0.0) * v_hor[0] + (iff + 1.0) * v_ver[0],
							   y1 + (jff + 0.0) * v_hor[1] + (iff + 1.0) * v_ver[1],
							  (z1 + (jff + 0.0) * v_hor[2] + (iff + 1.0) * v_ver[2]));
				glEnd();
				glFlush();
			}
		}
	}
} // end xaddftriang_perspTEXTURED_pp function

// Prototype of the struct meant to be the main container of data, 
// be it single numbers, couples, tripets or heterogeneous mixes of data 
struct mystruct
{
	float z;
	int ind_triang[3];
	int color_index;
};

// ####################################################################################################################
// Function mat3x3_mult multiplies two 3x3 matrices and places the result in global variable gloResultMatrix.
// ####################################################################################################################
void mat3x3_mult(double mat1[3][3], double mat2[3][3])
{
	double sum;
	int im, jm, k;

	for (im = 0; im < 3; im++)
	{ 
		for (jm = 0; jm < 3; jm++)
		{ 
			sum = 0;
			for (k = 0; k < 3; k++)
			{
				sum = sum + mat1[im][k] * mat2[k][jm];
			}
			gloResultMatrix[im][jm] = sum; // EXTERN VALUE!!! It's an easy way to implement all this.
		}
	}
} // end mat3x3_mult function

// ####################################################################################################################
// Function inv calculates the inverse of a 3x3 matrix (used for obtaining the inverse of the inertia tensor).
// ####################################################################################################################
void inv(double in_3x3_matrix[3][3])
{
	double A[3][3]; // the matrix that is entered by user 
	double B[3][3]; // the transpose of a matrix A 
	double C[3][3]; // the adjunct matrix of transpose of a matrix A, not adjunct of A
	double X[3][3]; // the inverse
	int i, j;
	double x, n = 0; // n is the determinant of A

	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			A[i][j] = in_3x3_matrix[i][j];
			B[i][j] = 0;
			C[i][j] = 0;
		}
	}

	// determinant of A (presumebly the inertia tensor)
	for (i = 0, j = 0; j < 3; j++)
	{
		if (j == 2)
			n = n + A[i][j] * A[i + 1][0] * A[i + 2][1];
		else if (j == 1)
			n = n + A[i][j] * A[i + 1][j + 1] * A[i + 2][0];
		else
			n = n + A[i][j] * A[i + 1][j + 1] * A[i + 2][j + 2];
	}

	for (i = 2, j = 0; j < 3; j++)
	{
		if (j == 2)
			n = n - A[i][j] * A[i - 1][0] * A[i - 2][1];
		else if (j == 1)
			n = n - A[i][j] * A[i - 1][j + 1] * A[i - 2][0];
		else
			n = n - A[i][j] * A[i - 1][j + 1] * A[i - 2][j + 2];
	}

	if (n != 0.0)
	{
		x = 1.0 / n;
	}
	else
	{
		printf("in: inverse matrix function: Det = 0, CHECK!!!! -->> no inverse exists ( /0 ) !!! \n");
		getchar();
	}

	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			B[i][j] = A[j][i];
		}
	}

	C[0][0] = B[1][1] * B[2][2] - (B[2][1] * B[1][2]);
	C[0][1] = (-1) * (B[1][0] * B[2][2] - (B[2][0] * B[1][2]));
	C[0][2] = B[1][0] * B[2][1] - (B[2][0] * B[1][1]);

	C[1][0] = (-1) * (B[0][1] * B[2][2] - B[2][1] * B[0][2]);
	C[1][1] = B[0][0] * B[2][2] - B[2][0] * B[0][2];
	C[1][2] = (-1) * (B[0][0] * B[2][1] - B[2][0] * B[0][1]);

	C[2][0] = B[0][1] * B[1][2] - B[1][1] * B[0][2];
	C[2][1] = (-1) * (B[0][0] * B[1][2] - B[1][0] * B[0][2]);
	C[2][2] = B[0][0] * B[1][1] - B[1][0] * B[0][1];

	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			X[i][j] = C[i][j] * x;
			gloResultMatrix[i][j] = C[i][j] * x; // EXTERN VALUE!!!
		}
	}
} // end inv function

// ####################################################################################################################
// Function body_rebounce
// ####################################################################################################################
double body_rebounce(double rx, double ry, double rz,
					 double nx, double ny, double nz, 
					 double e)
{
	double jelf = 0, jel = 300.0;
	double vector0[3], vector1[3], axis[3];
	double vvertex[3], vnorm;
	double auxv[3], auxv2[3], UP, DOWN; // auxilliary to break up some longer formulas into feasibly small parts.

	vector0[0] = nx;
	vector0[1] = ny;
	vector0[2] = nz;

	vector1[0] = rx;
	vector1[1] = ry;
	vector1[2] = rz;

	axis[0] = vector0[1] * vector1[2] - vector0[2] * vector1[1]; // x component
	axis[1] = vector0[2] * vector1[0] - vector0[0] * vector1[2]; // y component
	axis[2] = vector0[0] * vector1[1] - vector0[1] * vector1[0]; // z component

	// let's do the hit resolution in a correct way, also becuase when it can be done, let's do it: 
	// good collision simulation for single rigid-body make good landings.

	vvertex[0] = v[0] + w[1] * vector1[2] - w[2] * vector1[1]; // x component
	vvertex[1] = v[1] + w[2] * vector1[0] - w[0] * vector1[2]; // y component
	vvertex[2] = v[2] + w[0] * vector1[1] - w[1] * vector1[0]; // z component

	vnorm = nx * vvertex[0] + ny * vvertex[1] + nz * vvertex[2];
	if (vnorm < 0)
	{ 
		// safecheck of right collision... it never will certainly rebounce *towards* the 
		// very terrain from which it is rebouncing.
		vector0[0] = rx;
		vector0[1] = ry;
		vector0[2] = rz;

		vector1[0] = nx;
		vector1[1] = ny;
		vector1[2] = nz;

		auxv[0] = vector0[1] * vector1[2] - vector0[2] * vector1[1]; // x component
		auxv[1] = vector0[2] * vector1[0] - vector0[0] * vector1[2]; // y component
		auxv[2] = vector0[0] * vector1[1] - vector0[1] * vector1[0]; // z component

		// implement directly this: mat3x3_vect( inbody[0].TIworInv, auxv )
		auxv2[0] = inv_It_now[0][0] * auxv[0] + inv_It_now[0][1] * auxv[1] + inv_It_now[0][2] * auxv[2]; // x component
		auxv2[1] = inv_It_now[1][0] * auxv[0] + inv_It_now[1][1] * auxv[1] + inv_It_now[1][2] * auxv[2]; // y component
		auxv2[2] = inv_It_now[2][0] * auxv[0] + inv_It_now[2][1] * auxv[1] + inv_It_now[2][2] * auxv[2]; // z component

		UP = -(1.0 + e) * vnorm;
		DOWN = 1.0 / MASS + (auxv[0] * auxv2[0] + auxv[1] * auxv2[1] + auxv[2] * auxv2[2]);

		jel = UP / DOWN;
		// "jel" is the right impulse, now appy the impulse and assign final velocity and rotation.

		// update velocity of CM...
		v[0] = v[0] + (jel / MASS) * nx;
		v[1] = v[1] + (jel / MASS) * ny;
		v[2] = v[2] + (jel / MASS) * nz;

		// update rotation... this is the hardest one:
		vector0[0] = rx;
		vector0[1] = ry;
		vector0[2] = rz;

		vector1[0] = jel * nx;
		vector1[1] = jel * ny;
		vector1[2] = jel * nz;

		auxv[0] = vector0[1] * vector1[2] - vector0[2] * vector1[1]; // x component
		auxv[1] = vector0[2] * vector1[0] - vector0[0] * vector1[2]; // y component
		auxv[2] = vector0[0] * vector1[1] - vector0[1] * vector1[0]; // z component

		// implement directly this: mat3x3_vect( inbody[0].TIworInv, auxv )

		auxv2[0] = inv_It_now[0][0] * auxv[0] + inv_It_now[0][1] * auxv[1] + inv_It_now[0][2] * auxv[2]; // x component
		auxv2[1] = inv_It_now[1][0] * auxv[0] + inv_It_now[1][1] * auxv[1] + inv_It_now[1][2] * auxv[2]; // y component
		auxv2[2] = inv_It_now[2][0] * auxv[0] + inv_It_now[2][1] * auxv[1] + inv_It_now[2][2] * auxv[2]; // z component

		w[0] = w[0] + auxv2[0];
		w[1] = w[1] + auxv2[1];
		w[2] = w[2] + auxv2[2];

		// update also this, to be sure you know... .
		L[0] = It_now[0][0] * w[0] + It_now[0][1] * w[1] + It_now[0][2] * w[2];
		L[1] = It_now[1][0] * w[0] + It_now[1][1] * w[1] + It_now[1][2] * w[2];
		L[2] = It_now[2][0] * w[0] + It_now[2][1] * w[1] + It_now[2][2] * w[2];

		printf("COLLISION BEING RESOLVED ....\n\n");
	}

	return jel;
} // end body_rebounce function

// ####################################################################################################################
// Function make_inertia_tensor
// ####################################################################################################################
void make_inertia_tensor(int n_vertexs)
{
// be very careful to assign storage space correctly!!!! otherwise it brings to 0 all elements!!!
#define ne 1000
	int i, k;
	double Ixxe[ne], Iyye[ne], Izze[ne]; // those  'principal moments of inertia'....
	double Ixye[ne], Ixze[ne], Iyze[ne]; // those  'products of inertia'....
	double Ixx = 0, Iyy = 0, Izz = 0;	 // the total principal moments of inertia to put into the diagonals of the 3x3 tensor matrix. 
	double Ixy = 0, Ixz = 0, Iyz = 0;	 // these are automatically set = 0, that's important.... 
	double std_vxmass = 10.0;			 // 10 Kg

	// compute the elements of the final tensor matrix:
	for (i = 0; i < n_vertexs; i++)
	{
		// those 'principal moments of inertia'...
		Ixxe[i] = std_vxmass * (pow(gloPunti[i][1], 2) + pow(gloPunti[i][2], 2)); // y2 + z2
		Iyye[i] = std_vxmass * (pow(gloPunti[i][0], 2) + pow(gloPunti[i][2], 2)); // x2 + z2
		Izze[i] = std_vxmass * (pow(gloPunti[i][0], 2) + pow(gloPunti[i][1], 2)); // x2 + y2

		// those 'products of inertia'...
		Ixye[i] = std_vxmass * (gloPunti[i][0] * gloPunti[i][1]); // xy
		Ixze[i] = std_vxmass * (gloPunti[i][0] * gloPunti[i][2]); // xz
		Iyze[i] = std_vxmass * (gloPunti[i][1] * gloPunti[i][2]); // yz
	}

	// sum up
	for (k = 0; k < n_vertexs; k++)
	{
		Ixx = Ixx + Ixxe[k];
		Iyy = Iyy + Iyye[k];
		Izz = Izz + Izze[k];

		Ixy = Ixy + Ixye[k];
		Ixz = Ixz + Ixze[k];
		Iyz = Iyz + Iyze[k];
	}

	// put principal moments of inertia into the diagonals of the result matrix
	It_init[0][0] = Ixx;
	It_init[1][1] = Iyy;
	It_init[2][2] = Izz;

	// put inertia products in It_init tensor
	It_init[0][1] = -Ixy;
	It_init[1][0] = -Ixy;

	It_init[0][2] = -Ixz;
	It_init[2][0] = -Ixz;

	It_init[1][2] = -Iyz;
	It_init[2][1] = -Iyz;
} // end make_inertia_tensor function

// ####################################################################################################################
// Function load_textures_wOpenGL
// ####################################################################################################################
void load_textures_wOpenGL()
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
						color_to_convert = getpixel(image, i, txtRES - 1 - j);
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
} // end load_textures_wOpenGL function

// ####################################################################################################################
// Function load_textures96x96_SEMITRANSPARENT_wOpenGL
// ####################################################################################################################
void load_textures96x96_SEMITRANSPARENT_wOpenGL()
{
	static int texture_generated = 0; // at first call of this function, a 32x32 texture sample will be generated 

	//	Create texture
// maximal values, if possible don't exploit maximums
#define txtWidth2 32
#define txtHeight2 32
#define txtWidth3 96
#define txtHeight3 96

	int txtRES2 = 96; // A reasonable texture resolution
	static GLubyte txt1[txtHeight3][txtWidth3][4];

#ifdef GL_VERSION_1_1
	static GLuint texName;
#endif

	int j, i, k;

	if (texture_generated == 0)
	{
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

		//=================GROUND TEXTURE PERSONALISED...=====================
		int texn = 1; // > 0 ABSOLUTELY!!!

		while (texn > 0)
		{
			char filename[100];
			SDL_Surface *image; // This pointer will reference our bitmap.
			int bytes_per_color, imhe;
			Uint8 red, green, blue, alpha;
			Uint32 color_to_convert;

			bytes_per_color = COLDEPTH / 8;

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

				// feed into 'the' array used for this
				for (j = 0; j < txtRES2; j++)
				{ // vertical
					for (i = 0; i < txtRES2; i++)
					{ // horizontal
						color_to_convert = getpixel(image, i, j);
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

				// Release the surface
				SDL_FreeSurface(image);

				texName = gloTexturesAvailable + texn - 1; // VERY CAREFUL!!! NOT OVERWRITE ALREADY OCCUPIED TEXTURES!!

				//---------| TEXTURE PROCESSING |-----THIS PART MUST BE EXECUTED ONLY ONCE!!! 
				// Otherwise it silently overloads memory at each call
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

				glGenTextures(1, &texName);

				gloTexIds[gloTexturesAvailable + texn - 1] = texName; // [texn-1] because started from 1, be careful

				glBindTexture(GL_TEXTURE_2D, gloTexIds[gloTexturesAvailable + texn - 1]); // [texn-1] because started from 1, be careful

				// what OpenGL should do when texture is magnified GL_NEAREST: non-smoothed texture | GL_LINEAR: smoothed
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); 

				// ...when texture is miniaturized because far; GL_NEAREST: non-smoothed texture
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);	

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, txtRES2, txtRES2, 0, GL_RGBA, GL_UNSIGNED_BYTE, txt1);
				glGenerateMipmap(GL_TEXTURE_2D);

				glEnable(GL_TEXTURE_2D);
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL); // the GL_DECAL option draws texture as is: no color mixing thigs. GL_MODULATE lets mixing.

				glBindTexture(GL_TEXTURE_2D, texName);
				//--------------------------| END OF TEXTURE LOAD PROCESSING |-------------------------------

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
} // end load_textures96x96_SEMITRANSPARENT_wOpenGL function

// ####################################################################################################################
// Function load_hmap_from_bitmap populates field shmap of structure gloTerrain.
// ####################################################################################################################
int load_hmap_from_bitmap(char *filename)
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
				color_to_convert = getpixel(image, i, TERRAIN_SIZE - 1 - j);
				SDL_GetRGB(color_to_convert, image->format, &red, &green, &blue);

				gloTerrain.shmap[i][j] = ((float)red + 256.0 * ((float)green)) / (256.0); // simplified way
			}
		}
		printf("HEIGHTMAP LOADED FROM FILE: %s\n", filename);
	}

	// Release the surface
	SDL_FreeSurface(image);

	return isz;
} // end load_hmap_from_bitmap function

// ####################################################################################################################
// Function load_maptex_from_bitmap
// load texture ID map from a bitmap deviced by an editor (or with a graphics editor program, but 
// that would be RATHER UNPRACTICAL... )
// ####################################################################################################################
int load_maptex_from_bitmap(char *filename)
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
				color_to_convert = getpixel(sdl_image, i, j);
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
} // end load_maptex_from_bitmap function

// ####################################################################################################################
// Function check_vector_elements
// leggi file e controlla quanti numeri ci sono / read file and check how many numbers there are
// N.B.: numeri separati da spazi o da a-capo. Con virgole o altro si blocca. 
// N.B.: numbers separated by spaces or by a-line. With commas or other it blocks.
// ####################################################################################################################
long int check_vector_elements(char filename[])
{
	FILE *InFilePtr; // pointer to input file
	InFilePtr = fopen(filename, "r");
	long int i = 0;
	float test;

	while (fscanf(InFilePtr, "%f", &test) != EOF)
	{
		i++;
		printf("%f  \n", test);
	}

	fclose(InFilePtr); 
	return i;
} // end check_vector_elements function

// ####################################################################################################################
// Function read_vector
// Read in numeric vector from file
// only space-separated or newline-separated numbers!! else goes error
// ####################################################################################################################
void read_vector(char filename[], float dest_string[], long int maxsize)
{
	FILE *FilePtr; // pointer to input file 
	FilePtr = fopen(filename, "r");
	long int i = 0; // MUST put it = 0 

	while (fscanf(FilePtr, "%f", &dest_string[i]) != EOF && i < maxsize)
	{
		i++; // augment index of casel in dest_string[]
		printf("%f . \n", dest_string[i]);
	}
	fclose(FilePtr); 
	printf("\nFILE FOUND & READ IN. LENGHT LIMIT WAS FIXED TO: %li . \n", maxsize);
} // end read_vector function

// ####################################################################################################################
// Function import_airplane_polyheron
// ####################################################################################################################
void import_airplane_polyheron(void)
{
	// read in poly definition from file
	float auxxv[3 * NVERTEXES];
	int nelem = 0, i, j;

	printf("TRYING TO IMPORT VERTEX LIST OF 3D MODEL\n");

	nelem = check_vector_elements("input/vertexes.txt");
	read_vector("input/vertexes.txt", auxxv, nelem); // read file and values in the auxxv array.

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

	nelem = check_vector_elements("input/triangulation.txt");
	read_vector("input/triangulation.txt", auxxv, nelem); // read file and values into the auxxv array.

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

	nelem = check_vector_elements("input/facecolor.txt");
	read_vector("input/facecolor.txt", auxxv, nelem); // read file and values into the auxxv array.

	// feed into 'the' array used for this
	for (j = 0; j < nelem / 3; j++)
	{
		for (i = 0; i < 3; i++)
		{
			col_tris[j][i] = auxxv[j * 3 + i];
		}
	}
} // end import_airplane_polyheron function

// ####################################################################################################################
// Function getpixel
// If you want to understand all color and pixel info in SDL, go to http://sdl.beuc.net/sdl.wiki/Pixel_Access
// ####################################################################################################################
Uint32 getpixel(SDL_Surface *surface, int x, int y)
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
} // end getpixel function
