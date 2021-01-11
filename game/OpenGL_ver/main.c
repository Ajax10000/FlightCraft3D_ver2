#include <stdio.h>
#include <math.h>
#include <time.h>

// include SDL library's header and declare external 5 spec variables 
//=====================|SDL CODE BLOCK 1| =====================================
#define GL_GLEXT_PROTOTYPES
// include SDL library's header
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>

#include <GL/gl.h>
#include <GL/glcorearb.h>

//===the structure representing relevant quantities concerning game's terrain====
#define TERRAIN_SIZE 300
struct subterrain
{
	int map_size;
	float GPunit;
	float shmap[TERRAIN_SIZE][TERRAIN_SIZE];
	float scol[TERRAIN_SIZE][TERRAIN_SIZE][3];
	int map_texture_indexes[TERRAIN_SIZE][TERRAIN_SIZE];
	float auxnormal[3];
};

// ####################################################################################################################
// # Function prototypes                                                                                              #
// #                                                                                                                  #

// SUPERSAFE TOOK IT FROM PROFESSIONAL SITE: used for bitmap conversion to RGB value matrix (usual stuff)
Uint32 getpixel(SDL_Surface *surface, int x, int y);

// this is the prototype of the function which will draw the pixel.
void sdldisplay(int sw, int sh);

void xclearpixboard(int xlimit, int ylimit);

// add 1 pixel to output image but in a failsafe manner: no accidental segfaults. 
void xadd1pix(int x, int y, float color[3],
			  int xlimit, int ylimit);

// interpolate 2 points graphically 
void xaddline(int x1, int y1,
			  int x2, int y2, float color[3], // change this to int color[3] 
			  int xlimit, int ylimit);

//====================================== draw a filled trinagle to pixel matrix ==================================
void xaddftriang(int x1, int y1,
				 int x2, int y2,
				 int x3, int y3,
				 float color[3],
				 int step,
				 int xlimit, int ylimit);

// now we define the function which, given 2 points in 3D, calculates where they end up on the
// virtual camera pointing toward positive z-s and passes them to the 2D line drawing function. 
void xaddftriang_persp(float x1, float y1, float z1,
					   float x2, float y2, float z2,
					   float x3, float y3, float z3,
					   int step,
					   float color[3], int pbwidth, int pbheight);

// textured triangles, drawn with NO APPOX... it's EXCACT 
void xaddftriang_perspTEXTURED_pp(float x1, float y1, float z1,
								  float x2, float y2, float z2,
								  float x3, float y3, float z3,
								  int step,
								  float color[3], int pbwidth, int pbheight);

void xaddftriang_perspTEXTURED(float x1, float y1, float z1,
							   float x2, float y2, float z2,
							   float x3, float y3, float z3,
							   int step,
							   float color[3], int pbwidth, int pbheight);

void GLaddftriang_perspTEXTURED(float x1, float y1, float z1,
								float x2, float y2, float z2,
								float x3, float y3, float z3,
								int texId, float texcoords[3][2],
								float color[3], int pbwidth, int pbheight);

void load_textures_wOpenGL();
void load_textures96x96_SEMITRANSPARENT_wOpenGL(); // similarly but wwww alpha values transparency info too, etc.
int load_hmap_from_bitmap(char *filename); // ...describtion at definition
int load_maptex_from_bitmap(char *filename); // ...describtion at definition

void xaddpoint_persp(float x1, float y1, float z1, float color[3],
					 int pbwidth, int pbheight);

void xaddline_persp(float x1, float y1, float z1, float x2, float y2, float z2, float color[3],
					int pbwidth, int pbheight);

// basic SPECIAL EFFECTS IN VIDEOGAMES... 
// add new explosion or just process those already started 
void addfrantumation_wsim(float x0, float y0, float z0, double dft, int option);

// add new explosion or just process those already started 
void addsmoke_wsim(double x0, double y0, double z0, double dft, int option);

void projectile_launch(float x, float y, float z, float vx, float vy, float vz, double dft, int do_add);

// misc for import 3D models.
long int check_vector_elements(char filename[]);
void read_vector(char filename[], float dest_string[], long int maxsize);

// polyherdron definition importing from simple text file containing list of coordinate triplets. 
// does seme for face definition and colors and texture orderting if needed.
void import_airplane_polyheron(void); 

int waitdt_ms(double tt_ms);

double say_terrain_height(struct subterrain *ite, double x, double z);

// #                                                                                                                  #
// # End function prototypes                                                                                          #
// ####################################################################################################################


// ####################################################################################################################
// # Global variable declarations                                                                                     #
// #                                                                                                                  #

#define WIDTH 480
#define HEIGHT 320
#define COLDEPTH 16
SDL_Surface *screen;
SDL_Event event;		   // for real-time interactivity functions provided by SDL library 
SDL_Window *window = NULL; // New for SDL 2
SDL_GLContext context;	   // New for SDL2
const GLdouble pi = 3.1415926535897932384626433832795;

int low_graphics = 1;

float pixmatrix[HEIGHT][WIDTH][3];

double best_dt_ms = 10.0;

int texture_ids[100];

int textures_available = 0; // this is quite obvious... how many textures are loaded or remdom-genrated... ok.

float MAG = 60.0;

int view = 1; // start w external viwìew

float x_cockpit_view = 0.6;
float y_cockpit_view = 0.6;
float z_cockpit_view = 0.6;

int aboard = 1;
float x_pilot, y_pilot, z_pilot;

// AUTOSET by general graphics procedure!! posizione telecamera vituale 
float x = 0.0, y = 0.0, z = 0.0;		  

// coordinates of airplane in game... external view is just needed here to make this game 
// show accidental crashes due to overflow of the float 
// datatype variables after going far more than 4300 Km from Origin 
float xp = 200.0, yp = 200.0, zp = 400.0; 

float P[3], Q[3], R[3];	   // versori dei 3 assi della telecamera virtuale 
float Pa[3], Qa[3], Ra[3]; // versori dei 3 assi del sistema di riferimento rispetto cui sono dati i vertici dello aeroplano 

struct subterrain terrain1;

#define NTRIS 610

// USED ONLY TO STORE ID NUMBER TO REALL... NTRIS is a WORST-CASE CONSIDERATON IN WHICH 
// EACH TRIANGULAR FACE HAS A DIFFRENT TEXTURE. IT IS FILLED AT TEXTURE LOAD-IN AT FIRST CALL TO GLaddpoly......
int texid[NTRIS]; 

// which of pre-loaded textures to appl on triangular face? they can ba also random-generated of course... .
int face_texid[NTRIS] = {
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11}; 

int face_texord[NTRIS][3] = {
	{0, 1, 2},
	{0, 1, 2},
	{0, 3, 2},
	{0, 2, 3},
	{1, 2, 3},
	{1, 2, 3},
	{0, 1, 2},
	{0, 1, 2},
	{0, 3, 2},
	{0, 2, 3},
	{1, 2, 3},
	{1, 2, 3}};

// texture abcd side order riplet... tells how texture iage is treched ontriangular face...
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

// tis nevr changes... it's a fixed convention... of cource ocan changeit inthe 
// source cnde here and recomple for havinga different flavour of editor 
// ==vertexes of tex square:===a=b=c=d===========
const int texcoord_x[4] = {0, 1, 0, 1}; //...x
const int texcoord_y[4] = {0, 0, 1, 1}; //...y

#define NTREES 10000

// 4 data define a tree: x, y,z coordinates of where its convetional geometric center is... 
// that was enough obvious ; the other is a parameter saying how much the 96x96 or 128x128 (or more) 
// 'slice' images must be magnified with restpct to the standar 1 unit-per-side case. 
// the 5-th parameter says which type of tree... chosen from a pre-defined set... this means, 
// in our case, the Id of the texture used to do the 2 'slices'.
// NOTE: this is only a basic support for trees of course... . the most extreme 
// realism would be achieved by requiring a complete polyhedral definition for each type of tree... 
// after all, this way trees would be polyherda as any generic polyhedra with also semitransparent 
// faces for the 'slices' where desired. 
float trees[NTREES][5];
int tree_texture_ID_bounds[2]; // texture_ID -related thing

float treeR1 = 0.5;

// auxiloiary for giving right texture coordinates... the right way of 'splattering' the 
// quadrangular texture image / color-matrix onto the some triangles into which all 3Dobjects 
// in the simplest ase are divided.
float texcoords_gnd_tri1[3][2] = {
	{0.0, 0.0},
	{1.0, 0.0},
	{0.0, 1.0}};

float texcoords_gnd_tri2[3][2] = {
	{1.0, 0.0},
	{0.0, 1.0},
	{1.0, 1.0}};

#define NLINES 20
#define NVERTEXES 200

// AEREO: DEFINIZIONE DEI VERTICI 
float punti[NVERTEXES][3] = {// scatola 
							 {74, 76, -57},
							 {74, 76, 57},
							 {74, 76, 262},
							 {-74, 76, 262},
							 {-74, 76, 57},
							 {-74, 76, -57},
							 {-74, 76, -262},
							 {74, 76, 262},
							 {74, 76, -549},
							 {-44, 76, -549},
							 {74, 76, 549},
							 {-44, 76, 549},
							 {244, -1, 50},
							 {244, -1, -50},
							 {-354, 10, 12},
							 {-354, 10, -12},
							 {-370, 10, -165},
							 {-434, 10, -165},
							 {-434, 10, 0},
							 {-434, 10, 165},
							 {-370, 10, 165},
							 {244, -75, 50},
							 {244, -75, -50},
							 {74, 10, -80},
							 {74, 10, 50},
							 {-74, 40, 50},
							 {-74, 40, -50},
							 {-377, 25, 0},
							 {-159, 10, 0},
							 {-445, 108, 0},
							 {90, -135, -134},
							 {-220, -135, -30},
							 {-220, -135, 30},
							 {90, -135, 134},
							 {17, 75, 50},
							 {74, 76, -262},
							 {17, 10, -40},
							 {17, -10, 40},
							 {-17, 20, -40},
							 {-17, 20, 40},
							 {-17, -40, -60},
							 {-17, -20, 40},
							 {-445, 5, 0},
							 {-445, 5, 0},
							 {65, -50, -40},
							 {65, -50, 40},
							 {65, 10, -40},
							 {-156, 10, 0},
							 {35, 20, -40},
							 {35, 20, 40},
							 {-35, 20, 89},
							 {-35, 0, 89},
							 {10, 70, -50},
							 {56, 71, -40},
							 {56, 71, -79},
							 {10, 70, 50},
							 {16, -50, -40},
							 {16, -50, -40},
							 {17, 20, -40},
							 {0, 5, -40},
							 {17, 20, -40},
							 {17, 20, -40},
							 {-10, 20, -40},
							 {-35, 20, -40},
							 {-35, 20, -40},
							 {120, 75, -40},
							 {120, 75, -40},
							 {0, 75, -40},
							 {-15, -50, -40},
							 {-15, -50, -40},
							 {-15, -20, 6},
							 {56, 73, 40},
							 {-30, 20, -40},
							 {-30, 20, -40},
							 {-35, 20, 39},
							 {-35, 0, 39},
							 {-110, -50, -90},
							 {14, -50, -80},
							 {14, -50, 80},
							 {-110, -50, -4},
							 {16, -50, 0},
							 {16, -50, 0},
							 {18, 20, 0},
							 {6, 75, 5},
							 {80, -20, 0},
							 {80, -20, 0},
							 {-100, 20, 0},
							 {-20, 50, 7},
							 {-10, 50, 0},
							 {120, -75, 40},
							 {120, -75, -40},
							 {0, 75, -8},
							 {-165, -50, -0},
							 {-165, -50, 7},
							 {-155, -20, -0},
							 {-100, 20, 8},
							 {10, 70, -6},
							 {-35, 20, 8},
							 {-120, -140, -80},
							 {-120, -140, 80}};

// chi sono i 2 punti estremi delle linee da disegnare? 
// NOTA: estremi[ i-esima linea][quale punto e' l'estremo ] 
int estremi[NLINES][2] = {
						  {0, 1},
						  {1, 2},
						  {0, 3},
						  {2, 3},

						  {4, 5},
						  {6, 7},
						  {5, 6},
						  {4, 7},

						  {0, 4},
						  {1, 5},
						  {2, 6},
						  {3, 7},

						  {8, 9},
						  {9, 10},
						  {10, 11},
						  {8, 11},

						  {12, 13},
						  {13, 14},
						  {14, 15},
						  {12, 15}};

// chi sono i 2 punti estremi delle linee da disegnare? 
// NOTA: estremi[ i-esima linea][quale punto e' l'estremo ] 
int tris[NTRIS][3] = {
					  {0, 4, 5},
					  {1, 2, 3},
					  {1, 3, 4},
					  {0, 1, 4},
					  {0, 5, 6},
					  {0, 6, 35},
					  {6, 8, 35},
					  {6, 8, 9},
					  {2, 10, 11},
					  {2, 11, 3},
					  {5, 13, 4},
					  {4, 12, 13},
					  {0, 1, 15},
					  {1, 14, 15},
					  {15, 16, 18},
					  {16, 18, 17},
					  {14, 20, 19},
					  {14, 18, 19},
					  {13, 22, 23},
					  {12, 22, 21},
					  {15, 84, 92},
					  {23, 5, 0},
					  {23, 24, 4},
					  {40, 23, 22},
					  {21, 24, 26},
					  {18, 28, 29},
					  {28, 27, 18},
					  {0, 31, 5},
					  {1, 4, 32},
					  {13, 12, 22},
					  {14, 38, 50},
					  {89, 88, 94},
					  {94, 95, 89},
					  {11, 45, 82},
					  {11, 91, 82},
					  {17, 19, 91},
					  {91, 19, 43},
					  {90, 94, 95},
					  {16, 18, 40},
					  {83, 79, 7},
					  {66, 90, 95},
					  {96, 67, 31},
					  {31, 7, 96},
					  {13, 37, 67},
					  {23, 26, 28},
					  {28, 97, 23},
					  {43, 82, 91},
					  {42, 43, 82},
					  {42, 58, 82},
					  {37, 38, 49},
					  {31, 39, 49},
					  {49, 67, 31},
					  {49, 67, 37},
					  {10, 12, 20},
					  {40, 42, 43},
					  {43, 41, 40},
					  {18, 40, 42},
					  {40, 20, 21},
					  {20, 21, 26},
					  {20, 25, 26},
					  {25, 26, 4}};

// ovvio a cosa serve... 
float col_tris[NTRIS][3] = {
	{0.9, 0.9, 0.3},
	{0.9, 0.9, 0.3},

	{0.7, 0.5, 0.3},
	{0.7, 0.5, 0.3},

	{0.3, 0.3, 0.3},
	{0.3, 0.3, 0.3},

	{0.3, 0.8, 0.9},
	{0.3, 0.8, 0.9},

	{0.3, 0.4, 0.3},
	{0.3, 0.4, 0.3},

	{0.4, 0.2, 0.3},
	{0.4, 0.2, 0.3},

	{0.6, 0.6, 0.6},
	{0.6, 0.6, 0.6},

	{0.3, 0.6, 0.9},
	{0.3, 0.6, 0.9},

	{0.3, 0.4, 0.3},
	{0.3, 0.4, 0.3},

	{0.7, 0.4, 0.3},
	{0.4, 0.4, 0.3}};

int nvertexes = 100; // default value
int ntris = 33;		 // default value

// quatities used FOR SIMULATION / REALISTIC MOTION AND REBOUNCE FROM GROUND
float v[3] = {0.0, 0.0, 0.0}; // (needs initial value!) 
float p[3];

double Rm[3][3] = {{1.0, 0.0, 0.0},
				   {0.0, 1.0, 0.0},
				   {0.0, 0.0, 1.0}};
// orientation (angular): needs inital value(IDENTITY MATRIX!!!)

double w[3] = {0.0, 0.0, 0.0};
double L[3] = {0.0, 0.0, 0.0};

// constants which characterize dynamically the body
float MASS = 1000.0; // total mass (linear motion) 
double It_init[3][3] = {{100.0, 0.0, 0.0},
						{0.0, 200.0, 0.0},
						{0.0, 0.0, 110.0}};
// sort of "rotational mass" (angular motion) 

// we build also the inverse matrix of R_3x3 matrix 
double It_initINV[3][3];

// it's updated accord to orientation
double It_now[3][3] = {{100.0, 0.0, 0.0},
					   {0.0, 100.0, 0.0},
					   {0.0, 0.0, 100.0}}; 

// (DON'T CARE; info: Google --> "moment of inertia tensor" ) 
// influesces from outside: FORCE vectors
float Fcm[3] = {0.0, 0.0, 0.0};			//  total foce on center-of-mass "CM" 
double torque_tot[3] = {0.0, 0.0, 0.0}; //  total torque-force on rigid body

// costants specific to simplest **AIRPLANE** game. 
double k_visc = 2900.9; // some constant deriving form viscosity of air AIR FRICTION DUE TO WING AND BACHWING TOTAL AREA... *PARP-FALL RESISTENCE* 
double k_visc2 = 100.9; // some constant deriving form viscosity of air **SIDE-FALL RESISTANCE**  
double k_visc3 = 200.9; // some constant deriving form viscosity of air 

double k_visc_rot = 2.9;	// some constant deriving form viscosity of air around main axis... from nose to back 
double k_visc_rot2 = 20.9;	// some constant deriving form viscosity of air around main axis... around the axis perpendicular to wings' plane 
double k_visc_rot3 = 290.9; // some constant vaguely related to viscosity of air **CODINO aligment**?? 

double k_visc_rot_STABILIZE = 110.0; // some constant deriving form viscosity of air around main axis... around the axis passing through the wings, you know... *???* 

double Pforce = 0.0; // Force of the propeller driven by the motor... a propulsion force. 
double vpar, vperp;	 // updated at each cycle... part of the super-siplified formulla to give forces to the body 3 in a way to make it fly a bit like a real airplane.

double result_matrix[3][3], R_T[3][3], temp_mat[3][3];

double Id[3][3] = {
	{1.0, 0.0, 0.0},
	{0.0, 1.0, 0.0},
	{0.0, 0.0, 1.0}};

double dR[3][3] = {
	{1.0, 0.0, 0.0},
	{0.0, 1.0, 0.0},
	{0.0, 0.0, 1.0}};

double inv_It_now[3][3] = {{1.0, 0.0, 0.0},
						   {0.0, 1.0, 0.0},
						   {0.0, 0.0, 1.0}};

double axis1_orig[3] = {1.0, 0.0, 0.0};
double axis2_orig[3] = {0.0, 1.0, 0.0};
double axis3_orig[3] = {0.0, 0.0, 1.0};

// according to present orientation: 3 principal axes of inertia... or anyway.
double axis_1[3], axis_2[3], axis_3[3]; 

// auxiliaries:
double SD[3][3], SD2[3][3], u1, u2, u3, w_abs, dAng;

// multiplication of 2  matrixes 3x3, in size 
void mat3x3_mult(double mat1[3][3], double mat2[3][3]);

// INVERSE OF 3x3 MATRIC (USED FOR OBTAINING THE INVERSE OF THE INERTIA TENSOR): 
void inv(double in_3x3_matrix[3][3]);

double body_rebounce(double rx, double ry, double rz,
					 double nx, double ny, double nz, double e, double lat);

void make_inertia_tensor(int n_vertexs); // very useful....
// ==END OF IMULATION physical QUANTITIES DECLARATIONS========================

int logo[8][128] = {
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
	float turnch = 0.0; // used for smooth  turning of virtual camera 
	float turncv = 0.0; // used for smooth  turning if virtual camera 
	int plane_up = 0;
	int plane_down = 0;
	int plane_inclleft = 0;
	int plane_inclright = 0;
	// ALL non-SDL PROGRAM CODE HERE, OPERATE HERE:.... 
	// AUCILIARY INTEGERL VARIABLES some coordinates of some point which may be drawn... 
	int i = 0;
	int j = 0;
	int k = 0;

// how many segments to draw in scenary
#define NDOTS 3000

	float px[NDOTS];
	float py[NDOTS];
	float pz[NDOTS];

	float varR = 10.0;
	float RR = 20.0;

	// per ora non lo usiamo 
	// 3 angoli CHE INDICANO LA ORIENTAZIONE del sistema di riferimento della telecaera virtuale, per dire 
	float theta = 4.2, fi = 1.2, psi = 0.0; 

	// per ora non lo usiamo 
	// 3 angoli CHE INDICANO LA ORIENTAZIONE del sistema di riferimento dello aeroplano di questo game 
	float thp = 0.2, fip = 0.2, psip = 0.0; 

	// FOR INTERNAL VIEW!!! COCKPIT VIEW 
	float th_add = 0.1, fi_add = 0.1, psi_add = 0.0; 

	int cycles;

	// uno step di simulazione che da' un risultato non troppo disastroso.
	float h = 0.01; 

	double x1, y1, z1;

	double g = -9.81; // POSITIVE +++++ ASSUMED!!

	float color[4] = {0.0, 0.0, 0.0, 1.0};
	// code to start SDL graphics system: 
	//==========================|SDL CODE BLOCK 2|=========================
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
	window = SDL_CreateWindow("FlightCraft_3D (GL) - by Simon Hasur",
							  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
							  WIDTH, HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (window == NULL)
	{
		printf("Couldn't create window.");
		SDL_Quit();
		exit(2);
	}
	printf("Created window\n");
	// Create context
	context = SDL_GL_CreateContext(window);
	if (context == NULL)
	{
		printf("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
		SDL_Quit();
		exit(1);
	}

	screen = SDL_GetWindowSurface(window);
	printf("Set global screen variable by calling SDL_GetWindowSurface\n");

	if (!screen)
	{
		printf("Couldn't set %dx%d GL video mode: %s\n", WIDTH,
			   HEIGHT, SDL_GetError());
		SDL_Quit();
		exit(2);
	}
	SDL_UpdateWindowSurface(window);

	glEnable(GL_DEPTH_TEST); // activate hidden_surface_removal in OpenGL.
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	// SET SOME GL OPTIONS FOR LEAST PRECISION, SINCE THIS IS NOT A GRAPHICS PROGRAM... 
	// WE NEED COMPUTER POWER FOR PHYSICS CALCS MORE 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST); // Fastest - less expensive - Perspective Calculations
	glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);			// Fastest - less expensive - Perspective Calculations

	printf("1 check graphics Window!\n");

	// if there is an external polyhedron definition for better-defined airplanes, let's use it.

	// ============================"INITIALISATION" PROCEDURE STARTS HERE=============
	float tmp[NVERTEXES][3];
	for (i = 0; i < NVERTEXES; i++)
	{
		tmp[i][0] = 0.01 * punti[i][0]; // from centimeters to meters (x/100)
		tmp[i][1] = 0.01 * punti[i][1];
		tmp[i][2] = 0.01 * punti[i][2];
	}

	for (i = 0; i < NVERTEXES; i++)
	{
		punti[i][0] = tmp[i][0];
		punti[i][1] = tmp[i][1];
		punti[i][2] = tmp[i][2];
	}

	for (i = 0; i < NDOTS; i++)
	{
		px[i] = 0;
		py[i] = 0;
		pz[i] = 0;
	}

	cycles = 0;

	xclearpixboard(WIDTH, HEIGHT);

	// GAME TERRAIN INITIAL VALUES.... 
	terrain1.GPunit = 50.0; // BE CAREFUL!!! says how many meters per side, IF SEEN FROM ABOVE... BECAUSE INCLINED SIDES OBAY sqrt(x^2 + z^2) but this is referred to the case when it's seen **FROM ABOVE**
	terrain1.map_size = 300;

	srand(1234); //test... the pseudocasual num sequence gotten using the rand() function is THE SEME. on the seme system so... it's not casual.
	//but so it is casual sunce the time now is a number, the time in a second is another number and except system time reset of overflow the num is always different.
	srand((long int)time(NULL));
	printf("TIME number used to generate random environment: %li\n\n", (long int)time(NULL));
	waitdt_ms(1000.0);

	// generate random height samples 
	for (j = 0; j < TERRAIN_SIZE; j++)
	{
		for (i = 0; i < TERRAIN_SIZE; i++)
		{
			terrain1.shmap[j][i] = 0.2 * (((double)rand()) / ((double)RAND_MAX));

			terrain1.scol[j][i][0] = (float)0.1 * rand() / (float)RAND_MAX;
			terrain1.scol[j][i][1] = (float)0.6 * rand() / (float)RAND_MAX;
			terrain1.scol[j][i][2] = (float)0.0 * rand() / (float)RAND_MAX;
			terrain1.map_texture_indexes[j][i] = 1; // BULK TEXTURE... DEFAULT TEXTURE.
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
					terrain1.shmap[y][x] = terrain1.shmap[y][x] + 0.2;

					terrain1.scol[y][x][0] = terrain1.scol[y][x][0] * 0.98;
					terrain1.scol[y][x][1] = terrain1.scol[y][x][1] * 0.99;
					terrain1.map_texture_indexes[y][x] = 4 + k % 3;
				}
			}

		} // vyvle with k
	}	  //cycle with kux

	// put plane, lenghly airports at random positions, with random size 
	for (k = 0; k < 50; k++)
	{ // be careful... this is no rescaled
		int x, y, airport_x, airport_y;
		j = (int)TERRAIN_SIZE * ((float)rand() / (float)RAND_MAX);
		i = (int)TERRAIN_SIZE * ((float)rand() / (float)RAND_MAX);

		// x= 200 meters + 20*some  [meters]
		// y= 200 meters + 100*some [meters]
		airport_x = (200 + (int)20 * ((float)rand() / (float)RAND_MAX)) / terrain1.GPunit;
		airport_y = (200 + (int)500 * ((float)rand() / (float)RAND_MAX)) / terrain1.GPunit;

		for (y = j; y < j + airport_x && y < TERRAIN_SIZE; y++)
		{
			for (x = i; x < i + airport_y && x < TERRAIN_SIZE; x++)
			{
				terrain1.shmap[y][x] = terrain1.shmap[j][i] + 1.0;

				terrain1.scol[y][x][0] = 0.4;
				terrain1.scol[y][x][1] = 0.4;
				terrain1.map_texture_indexes[y][x] = 3;
			}
		}
	}
	// --------------END OF RANDOM TERRAN GENERATON-----------------

	terrain1.map_size = load_hmap_from_bitmap("terrain_data/hmap_300x300.bmp");

	// airplane random colors 
	for (i = 18; i < NTRIS; i++)
	{
		col_tris[i][0] = (float)0.3 * rand() / (float)RAND_MAX;
		col_tris[i][1] = (float)0.3 * rand() / (float)RAND_MAX;
		col_tris[i][2] = (float)0.9 * rand() / (float)RAND_MAX;
	}

	// load textures in
	load_textures_wOpenGL(); 
	tree_texture_ID_bounds[0] = textures_available; // at what ID do tree textures begin (and end also...)

	// idem but with "bitmap" image files with alpha value for transparency information on each pixel... 
	// this is mainly used ofr drawing trees in a quick, nie and simple way.
	load_textures96x96_SEMITRANSPARENT_wOpenGL(); 

	// this comes after, because so during loading we can check if no texture index superior to the number 
	// of available textures is inserted... .
	load_maptex_from_bitmap("terrain_data/maptex_300x300.bmp"); 

	int tree_group_n = 30;
	// trees' position and defult parameters for trees.
	for (k = 0; k < NTREES - tree_group_n; k = k + tree_group_n)
	{
		//random x and y coordinates within a rectangular area... simple.
		trees[k][0] = 5000 * ((double)-0 * RAND_MAX / 2 + rand()) / ((double)RAND_MAX);
		trees[k][1] = 5000 * ((double)-0 * RAND_MAX / 2 + rand()) / ((double)RAND_MAX);
		trees[k][2] = say_terrain_height(&terrain1, trees[k][0], trees[k][1]);

		trees[k][4] = floor(2.0 * ((double)rand()) / ((double)RAND_MAX));

		if (trees[k][4] == 0)
		{
			trees[k][4] = tree_texture_ID_bounds[0];
			trees[k][3] = 3;
		}
		else if (trees[k][4] == 1)
		{
			trees[k][4] = tree_texture_ID_bounds[0] + 2;
			trees[k][3] = 1;
		}

		for (j = 1; j < 1 + tree_group_n; j++)
		{
			// MACCHIA attorno ad un punto / STRAIN around a point...
			trees[k + j][0] = trees[k][0] + 100 * ((double)-0 * RAND_MAX / 2 + rand()) / ((double)RAND_MAX);
			trees[k + j][1] = trees[k][1] + 100 * ((double)-0 * RAND_MAX / 2 + rand()) / ((double)RAND_MAX);
			trees[k + j][2] = say_terrain_height(&terrain1, trees[k + j][0], trees[k + j][1]);

			trees[k + j][4] = floor(2.0 * ((double)rand()) / ((double)RAND_MAX));

			if (trees[k + j][4] == 0.0)
			{
				trees[k + j][4] = tree_texture_ID_bounds[0]; // texture ID number
				trees[k + j][3] = 3;
			}
			else if (trees[k + j][4] == 1.0)
			{
				trees[k + j][4] = tree_texture_ID_bounds[0] + 2;
				trees[k + j][3] = 1; // texture ID number
			}
		}
	}
	//trees ok.

	if (low_graphics == 0)
	{
		addsmoke_wsim(xp, yp, zp, h, 1); // commented out for testing
	}

	make_inertia_tensor(NVERTEXES);

	//-----momentum p (linear quantity)--
	p[0] = MASS * v[0];
	p[1] = MASS * v[1];
	p[2] = MASS * v[2];

	// angular momentum (angular/ rotational quantity ) 
	L[0] = It_now[0][0] * w[0] + It_now[0][1] * w[1] + It_now[0][2] * w[2];
	L[1] = It_now[1][0] * w[0] + It_now[1][1] * w[1] + It_now[1][2] * w[2];
	L[2] = It_now[2][0] * w[0] + It_now[2][1] * w[1] + It_now[2][2] * w[2];

	inv(It_init); // fine: it puts the inverse into the "result_matrix" global variable. 

	// we copy it into "R_INV"... 
	for (j = 0; j < 3; j++)
	{
		for (i = 0; i < 3; i++)
		{
			It_initINV[j][i] = result_matrix[j][i];
		}
	}

	// reset forces to 0.0 
	for (i = 0; i < 3; i++)
	{
		Fcm[i] = 0.0;
		torque_tot[i] = 0.0;
	}

	for (j = 0; j < 3; j++)
	{ 
		printf("CHECK Inertia tensor: %f  %f  %f \n", It_init[j][0], It_init[j][1], It_init[j][2]);
	}

	for (j = 0; j < 3; j++)
	{ 
		printf("CHECK its inverse   : %f  %f  %f \n", It_initINV[j][0], It_initINV[j][1], It_initINV[j][2]);
	}

	printf("TRYING TO IMPORT VERTEX LIST OF 3D MODEL\n");
	FILE *FilePtr; // pointer to input file 

	FilePtr = fopen("input/vertexes.txt", "r");
	if (FilePtr < 0)
	{
		printf("NO FILE TO IMPORT VERTEX LIST...USING DEFULT...\n");
		fclose(FilePtr);
	}
	else
	{
		import_airplane_polyheron();
	}
	//============================"INITIALISATION" PROCEDURES DONE=============

	while (cycles < 50000)
	{
		// -------- moving camera position and airplane intractive control -------- 
		if (turncv == 1.0 || turncv == -1.0)
		{							 // vertical cam displacement
			fi = fi - turncv * 0.02; // rotate camera's reference system's fi angle
			fi_add = fi_add - turncv * 0.01;
		}
		if (turnch == 1.0 || turnch == -1.0)
		{
			theta = theta + turnch * 0.02; // totate camera's rweference system's theta angle 
			th_add = th_add + turnch * 0.01;
		}
		if (plane_up == 1)
		{
			torque_tot[0] += 2620.0 * axis_3[0];
			torque_tot[1] += 2620.0 * axis_3[1];
			torque_tot[2] += 2620.0 * axis_3[2];
		}
		if (plane_down == 1)
		{
			torque_tot[0] += -2621.0 * axis_3[0];
			torque_tot[1] += -2621.0 * axis_3[1];
			torque_tot[2] += -2621.0 * axis_3[2];
		}
		if (plane_inclleft == 1)
		{
			torque_tot[0] += -2620.0 * axis_1[0];
			torque_tot[1] += -2620.0 * axis_1[1];
			torque_tot[2] += -2620.0 * axis_1[2];
		}
		if (plane_inclright == 1)
		{
			torque_tot[0] += 2621.0 * axis_1[0];
			torque_tot[1] += 2621.0 * axis_1[1];
			torque_tot[2] += 2621.0 * axis_1[2];
		}

		// ===========PHYSICS PROCEDURE=============
		// SEMBRA TUTTO OK 
		// now calculate axes in their new 'orientation', using the orientation matrix.

		//R:
		axis_1[0] = Rm[0][0] * axis1_orig[0] +
					Rm[0][1] * axis1_orig[1] +
					Rm[0][2] * axis1_orig[2];

		axis_1[1] = Rm[1][0] * axis1_orig[0] +
					Rm[1][1] * axis1_orig[1] +
					Rm[1][2] * axis1_orig[2];

		axis_1[2] = Rm[2][0] * axis1_orig[0] +
					Rm[2][1] * axis1_orig[1] +
					Rm[2][2] * axis1_orig[2];

		axis_2[0] = Rm[0][0] * axis2_orig[0] +
					Rm[0][1] * axis2_orig[1] +
					Rm[0][2] * axis2_orig[2];

		axis_2[1] = Rm[1][0] * axis2_orig[0] +
					Rm[1][1] * axis2_orig[1] +
					Rm[1][2] * axis2_orig[2];
		;

		axis_2[2] = Rm[2][0] * axis2_orig[0] +
					Rm[2][1] * axis2_orig[1] +
					Rm[2][2] * axis2_orig[2];

		axis_3[0] = Rm[0][0] * axis3_orig[0] +
					Rm[0][1] * axis3_orig[1] +
					Rm[0][2] * axis3_orig[2];

		axis_3[1] = Rm[1][0] * axis3_orig[0] +
					Rm[1][1] * axis3_orig[1] +
					Rm[1][2] * axis3_orig[2];

		axis_3[2] = Rm[2][0] * axis3_orig[0] +
					Rm[2][1] * axis3_orig[1] +
					Rm[2][2] * axis3_orig[2];

		Pa[0] = axis_1[0];
		Pa[1] = axis_1[1];
		Pa[2] = axis_1[2];

		Qa[0] = axis_2[0];
		Qa[1] = axis_2[1];
		Qa[2] = axis_2[2];

		Ra[0] = axis_3[0];
		Ra[1] = axis_3[1];
		Ra[2] = axis_3[2];
		// =======END OF AXES REORIENTATION DONE============

		// Calulate total Fcm and total torque, starting from external forces applied to vertices 
		// of using some 'magic' formula
		// NOTE: g is already done below... so it's no bother----forces  of ultrasimple flight model

		// NOT needed for dynamics, it' only in this game: some approx of force on CM 
		// and torque due to a rudimental plane aerodynamical forces' consideration
		float vpar;
		float vlat;

		float rot1;
		float rot2;
		float rot3;

		vlat = axis_3[0] * v[0] + axis_3[1] * v[1] + axis_3[2] * v[2]; // dot product explicited directly: clean work.

		vperp = axis_2[0] * v[0] + axis_2[1] * v[1] + axis_2[2] * v[2]; // dot product explicited directly: clean work.

		vpar = axis_1[0] * v[0] + axis_1[1] * v[1] + axis_1[2] * v[2];

		rot1 = axis_1[0] * w[0] + axis_1[1] * w[1] + axis_1[2] * w[2]; // how much it rotates around axis 1 (nose--> back)

		rot2 = axis_2[0] * w[0] + axis_2[1] * w[1] + axis_2[2] * w[2]; // how much it rotates around axis 2 (perp to wings)

		rot3 = axis_3[0] * w[0] + axis_3[1] * w[1] + axis_3[2] * w[2]; // how much it rotates around axis 3 (parallel to wings)
		// effect (IN A VERY RUDIMENTAL CONCEPTION OF AERODYNAMICS, NOT SCIENTIFIC AT ALL) of air friction on the motion of Center of Mass directly.

		Fcm[0] += -k_visc * vperp * axis_2[0] - k_visc2 * vlat * axis_3[0] - k_visc3 * vpar * axis_1[0] + Pforce * axis_1[0];
		Fcm[1] += -k_visc * vperp * axis_2[1] - k_visc2 * vlat * axis_3[1] - k_visc3 * vpar * axis_1[1] + Pforce * axis_1[1];
		Fcm[2] += -k_visc * vperp * axis_2[2] - k_visc2 * vlat * axis_3[2] - k_visc3 * vpar * axis_1[2] + Pforce * axis_1[2];

		// seme for the rotational motion. other effects are not considered.
		// if you re experto of aeromobilism, you can write better, just sobstitute theese weak formulas to better things.

		// generic stabilization (very poor approximation)
		torque_tot[0] += -vpar * k_visc_rot_STABILIZE * w[0];
		torque_tot[1] += -vpar * k_visc_rot_STABILIZE * w[1];
		torque_tot[2] += -vpar * k_visc_rot_STABILIZE * w[2];

		double boh = axis_3[0] * v[0] + axis_3[1] * v[1] + axis_3[2] * v[2];

		// effetto coda verticale: decente...
		torque_tot[0] += -k_visc_rot3 * (boh)*axis_2[0];
		torque_tot[1] += -k_visc_rot3 * (boh)*axis_2[1];
		torque_tot[2] += -k_visc_rot3 * (boh)*axis_2[2];

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

		L[0] = L[0] + torque_tot[0] * h;
		L[1] = L[1] + torque_tot[1] * h;
		L[2] = L[2] + torque_tot[2] * h;

		// now we get the updated velocity, component by component.
		w[0] = inv_It_now[0][0] * L[0] + inv_It_now[0][1] * L[1] + inv_It_now[0][2] * L[2];

		w[1] = inv_It_now[1][0] * L[0] + inv_It_now[1][1] * L[1] + inv_It_now[1][2] * L[2];

		w[2] = inv_It_now[2][0] * L[0] + inv_It_now[2][1] * L[1] + inv_It_now[2][2] * L[2];

		// angular momentum (angular/ rotational quantity )

		// reset forces to 0.0 
		for (i = 0; i < 3; i++)
		{
			Fcm[i] = 0.0;
			torque_tot[i] = 0.0;
		}

		// UPDATE POSITION AND ORIENTTION 
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

		// ok it does the product and resulting matrix is put in the "result_matrix" global variable. 
		mat3x3_mult(SD, SD); 

		for (j = 0; j < 3; j++)
		{
			for (i = 0; i < 3; i++)
			{
				SD2[j][i] = result_matrix[j][i];
			}
		}

		for (j = 0; j < 3; j++)
		{
			for (i = 0; i < 3; i++)
			{
				dR[j][i] = Id[j][i] + sin(dAng) * SD[j][i] + (1.0 - cos(dAng)) * SD2[j][i];
			}
		}

		// ok it does the product and resulting matrix is put in the "result_matrix" global variable. 
		mat3x3_mult(dR, Rm); 

		for (j = 0; j < 3; j++)
		{
			for (i = 0; i < 3; i++)
			{
				Rm[j][i] = result_matrix[j][i];
			}
		}

		// update inertia tensor according to new orientation: It_now = R*It_init*transpose(R) 
		// we build the transpose matrix of R_3x3 matrix, just here 
		for (i = 0; i < 3; i++)
		{
			for (j = 0; j < 3; j++)
			{
				R_T[i][j] = Rm[j][i];
			}
		}

		// we perform the 2 matrix poroducts 
		mat3x3_mult(Rm, It_init);

		for (j = 0; j < 3; j++)
		{
			for (i = 0; i < 3; i++)
			{
				temp_mat[j][i] = result_matrix[j][i]; //SAFE COPY!!! PASSING EXTERN VARIABLE AND THEN MODIFYING IT IS NOT SAFE... COMPILERS MAY FAIL TO DO IT CORRECTLY!!!
				//SAFEST SIMPLE METHOD --> BEST METHOD.
			}
		}

		mat3x3_mult(temp_mat, R_T);

		// and put result into "It_now" 
		for (j = 0; j < 3; j++)
		{
			for (i = 0; i < 3; i++)
			{
				It_now[j][i] = result_matrix[j][i];
			}
		}

		// its inverse too, since it's needed: 
		// we perform the 2 matrix poroducts 
		mat3x3_mult(Rm, It_initINV);

		for (j = 0; j < 3; j++)
		{
			for (i = 0; i < 3; i++)
			{
				temp_mat[j][i] = result_matrix[j][i]; //SAFE COPY!!! PASSING EXTERN VARIABLE AND THEN MODIFYING IT IS NOT SAFE... COMPILERS MAY FAIL TO DO IT CORRECTLY!!!
				//SAFEST SIMPLE METHOD --> BEST METHOD.
			}
		}
		mat3x3_mult(temp_mat, R_T);

		// we copy it into "inv_It_now"... 
		for (j = 0; j < 3; j++)
		{
			for (i = 0; i < 3; i++)
			{
				inv_It_now[j][i] = result_matrix[j][i];
			}
		}

		//=================DONE UPDATE OF ORIENTATION MATRIX and intertia tensor=================
		//====END PHYSICALLY SIMULATED UPDATE OF  AIRPLANE POS AND ROTATION, ORIENTAION=====

		// DETECTING AND RESOLVING COLLISION WITH GROUND
		// simplified 'collision' with ground. well... just to illustrate the concept, it's quite bad as solution.
		for (i = 0; i < NVERTEXES; i++)
		{					   // AIRPLANE...
			// coordinates of plane's vertexes in the "Game World"'s reference frame. Very simple, look code below 
			double xw, yw, zw; 
			xw = punti[i][0] * axis_1[0] + punti[i][1] * axis_2[0] + punti[i][2] * axis_3[0];
			yw = punti[i][0] * axis_1[1] + punti[i][1] * axis_2[1] + punti[i][2] * axis_3[1];
			zw = punti[i][0] * axis_1[2] + punti[i][1] * axis_2[2] + punti[i][2] * axis_3[2];

			double he_id = say_terrain_height(&terrain1, xp + xw, yp + yw);
			if (zp + zw < he_id)
			{ // just as any vertex of airplane touches ground and tries to go below 
				body_rebounce(xw, yw, zw, terrain1.auxnormal[0], terrain1.auxnormal[1], terrain1.auxnormal[2], 0.06, 0);
				printf("TOUCH GND \n");
				zp = zp + (he_id - zp - zw);
				// check is normvel < 0 and eventually calculated and assigns new velocities and so.
			}
		}

		// representation and graphics perocedure
		xclearpixboard(WIDTH, HEIGHT); // CANCELLA SCHERMATA/ LAVAGNA.

		//------------------ scenario inquadrature 3D---------------------------------- 
		if (view == 1)
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
		else if (view == 2)
		{
			// i know this negative index stuff is awkward but in the prototype stage 
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
		else if (view == 3)
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
		else if (view == 4)
		{ // INTERNAL VIEW COCKPIT
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

			//------Ri, Pi, Qi------ this is most practival axis-order.
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

		// moving plane's CM 
		if (aboard == 1)
		{
			x = xp + RR * R[0];
			y = yp + RR * R[1];
			z = zp + RR * R[2];

			if (view == 4)
			{ // INTERNAL VIEW COCKPIT
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
			z = say_terrain_height(&terrain1, x_pilot, y_pilot) + 1.75;
		}

		// LE COORDINATE DEI PUNTI DELLA PISTA, NEL SISTEMA DI RIFERIMENTO DEL PARACADUTISTA 
		float x1, y1, z1, x2, y2, z2; 

		if (low_graphics == 0)
		{
			addsmoke_wsim(xp, yp, zp, h, 2); // DEACTIVATED TO TEST
		}

		// test asse perpendiclare allo 'schermo' grafica 3D: 
		// let's avoid stupid mistakes... check if all basic things work 
		color[0] = 1.0;
		color[1] = 0.0;
		color[2] = 0.0;
		// IMPORTANT NOTE: 2 meters far from camera virtual 'lens' along it perpendiculat ro it 
		// towards screen, axes are each 1 meter long. use in an intelligent way the measures... 
		// in theese cases, go use uni-lenght references.it's obvious choice but saying it is not bad.

		// xc
		xaddline_persp(0.0 - 1.0, 0.0 - 1.0, 2.0,
					   1.0 - 1.0, 0.0 - 1.0, 2.0, color, WIDTH, HEIGHT);

		// yc
		color[0] = 0.0;
		color[1] = 1.0;
		color[2] = 0.0;

		xaddline_persp(0.0 - 1.0, 0.0 - 1.0, 2.0,
					   0.0 - 1.0, 1.0 - 1.0, 2.0, color, WIDTH, HEIGHT);

		// zc
		color[0] = 0.0;
		color[1] = 0.0;
		color[2] = 1.0;

		xaddline_persp(0.0 - 1.0, 0.0 - 1.0, 2.0,
					   0.0 - 1.0, 0.0 - 1.0, 2.0 + 1.0, color, WIDTH, HEIGHT);

		// GAME LOGO
		glColor3f(1.0, 1.0, 1.0);

		glPointSize(2);
		for (j = 0; j < 8; j++)
		{
			for (i = 0; i < 120; i++)
			{
				if (logo[7 - j][i] > 0)
				{
					glBegin(GL_POINTS);
						glVertex3f(0.012 * i + 0.14, 0.012 * j - 1, -2.0);
					glEnd();
				}
			}
		}

		// 3 ASSI DELLO SPAZIO CARTESIANO... COSI' SI CAPISCE DOVE STANNO LE COSE 
		// TRIANGOLO 1 
		float Xo[5], Yo[5], Zo[5], x_c[5], y_c[5], z_c[5];
		float GPunit;
		int xi, yi, Xi, Yi;

		GPunit = terrain1.GPunit;

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

			xaddline_persp(x_c[i], y_c[i], -z_c[i], x_c[3], y_c[3], -z_c[3], color, WIDTH, HEIGHT);
		}

		// DISEGNA IL TERRENO IN MODO ALGORITMICO, UNA GRIGILIA RETTANGOLARE COME AL SOLITO,
		// 'REDERING' WIREFRAME O A TRINGOLI RIPIENI 

		// draw only near triangles, in order to avoid overloading graphics computational heavyness 
		int lv = 6;

		Xi = floor(xp / (terrain1.GPunit));
		Yi = floor(yp / (terrain1.GPunit));

		lv = 12;

		if (low_graphics == 1)
		{
			lv = 6;
		}

		for (xi = Xi - lv; xi < Xi + lv; xi = xi + 1)
		{
			for (yi = Yi - lv; yi < Yi + lv; yi = yi + 1)
			{
				// TRIANGOLO 1 
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

				// default color :
				color[0] = 0.4;
				color[1] = 0.4;
				color[2] = 0.4;

				// failsafe variable preset 
				Xo[4] = 1;
				Yo[4] = 1;
				Zo[4] = 1;

				if (Xi + lv < terrain1.map_size && Yi + lv < terrain1.map_size)
				{
					// escamotage for a 0-terrain outside sampled limit. but sampled within, good 
					Zo[0] = GPunit * terrain1.shmap[xi][yi];		 // the hight sample 
					Zo[1] = GPunit * terrain1.shmap[xi + 1][yi];	 // the hight sample 
					Zo[2] = GPunit * terrain1.shmap[xi][yi + 1];	 // the hight sample 
					Zo[3] = GPunit * terrain1.shmap[xi + 1][yi + 1]; // the hight sample 

					color[0] = terrain1.scol[xi][yi][0];
					color[1] = terrain1.scol[xi][yi][1];
					color[2] = terrain1.scol[xi][yi][2];

					// now prepare a freshly calculated normal vector and then we draw it. 
					// it's fundamental that normals are ok for rebounce 
					say_terrain_height(&terrain1, Xo[0] + 0.01, Yo[0] + 0.01); // check what is the local normal 

					Xo[4] = Xo[0] + 20.0 * terrain1.auxnormal[0];
					Yo[4] = Yo[0] + 20.0 * terrain1.auxnormal[1];
					Zo[4] = Zo[0] + 20.0 * terrain1.auxnormal[2]; // the hight sample 
				}

				// calcola lecoordinate di questi 3 punti nel sistema P-Q-R del paracadutista/pilota 
				for (i = 0; i < 5; i++)
				{
					x_c[i] = P[0] * (Xo[i] - x) + P[1] * (Yo[i] - y) + P[2] * (Zo[i] - z);
					y_c[i] = Q[0] * (Xo[i] - x) + Q[1] * (Yo[i] - y) + Q[2] * (Zo[i] - z);
					z_c[i] = R[0] * (Xo[i] - x) + R[1] * (Yo[i] - y) + R[2] * (Zo[i] - z);
				}

				xaddline_persp(x_c[0], y_c[0], -z_c[0],
							   x_c[4], y_c[4], -z_c[4], color, WIDTH, HEIGHT);

				if (xi >= 0 && yi >= 0)
				{
					// TRIANGOLO 1
					GLaddftriang_perspTEXTURED(x_c[0], y_c[0], z_c[0],
											   x_c[1], y_c[1], z_c[1],
											   x_c[2], y_c[2], z_c[2],
											   texid[terrain1.map_texture_indexes[xi][yi]], texcoords_gnd_tri1,
											   color, WIDTH, HEIGHT);

					// TRIANGOLO 2 (change color a little bit)
					color[1] = color[1] + 0.1; // draw with slightly different color... 

					GLaddftriang_perspTEXTURED(x_c[1], y_c[1], z_c[1],
											   x_c[2], y_c[2], z_c[2],
											   x_c[3], y_c[3], z_c[3],
											   texid[terrain1.map_texture_indexes[xi][yi]], texcoords_gnd_tri2,
											   color, WIDTH, HEIGHT);
				}
			}
		}

		//trees
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

		for (k = 0; k < NTREES; k++)
		{
			float xctree, yctree, zctree, sctree;

			xctree = trees[k][0]; // x of conventinal geometric center
			yctree = trees[k][1]; // y of conventinal geometric center

			if ((xctree < x + 1000.0 && xctree > x - 1000.0) && (yctree < y + 1000.0 && yctree > y - 1000.0))
			{
				// draw only trees not too far away
				// z of conventinal geometric center. this was pre-calculated so that trees 
				// stay nicely ON the terrain surface.
				zctree = trees[k][2]; 

				// magnification value: how much to magnify original (1_unit x 1_unit) square?.
				sctree = trees[k][3]; 

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

				glBindTexture(GL_TEXTURE_2D, texid[(int)trees[k][4]]);

				// calcola lecoordinate di questi 3 punti nel sistema P-Q-R del paracadutista/pilota 
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

				// change theese coordinates:
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

				// again this to obtain coordintes in the virtual camera's reference frame.
				// calcola lecoordinate di questi 3 punti nel sistema P-Q-R del paracadutista/pilota 
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

		if (low_graphics == 0)
		{
			addsmoke_wsim(1, 2, 3, h, 0); // we must make special effect sequences go on. 
		}

		float x1a, y1a, z1a, x2a, y2a, z2a, x3a, y3a, z3a, x3, y3, z3;

		// DISEGNA SOLO LO AEREO... GRAFICA VETTORIALE BELLO SEMPLICE...
		color[0] = 1.0;
		color[1] = 1.0;
		color[2] = 1.0;

		for (i = 0; i < ntris; i++)
		{ // AIRPLANE...
			x1a = Pa[0] * punti[tris[i][0]][0] + Qa[0] * punti[tris[i][0]][1] + Ra[0] * punti[tris[i][0]][2];
			y1a = Pa[1] * punti[tris[i][0]][0] + Qa[1] * punti[tris[i][0]][1] + Ra[1] * punti[tris[i][0]][2];
			z1a = Pa[2] * punti[tris[i][0]][0] + Qa[2] * punti[tris[i][0]][1] + Ra[2] * punti[tris[i][0]][2];

			x2a = Pa[0] * punti[tris[i][1]][0] + Qa[0] * punti[tris[i][1]][1] + Ra[0] * punti[tris[i][1]][2];
			y2a = Pa[1] * punti[tris[i][1]][0] + Qa[1] * punti[tris[i][1]][1] + Ra[1] * punti[tris[i][1]][2];
			z2a = Pa[2] * punti[tris[i][1]][0] + Qa[2] * punti[tris[i][1]][1] + Ra[2] * punti[tris[i][1]][2];

			x3a = Pa[0] * punti[tris[i][2]][0] + Qa[0] * punti[tris[i][2]][1] + Ra[0] * punti[tris[i][2]][2];
			y3a = Pa[1] * punti[tris[i][2]][0] + Qa[1] * punti[tris[i][2]][1] + Ra[1] * punti[tris[i][2]][2];
			z3a = Pa[2] * punti[tris[i][2]][0] + Qa[2] * punti[tris[i][2]][1] + Ra[2] * punti[tris[i][2]][2];

			// estremo 1 
			x1 = P[0] * (x1a + xp - x) + P[1] * (y1a + yp - y) + P[2] * (z1a + zp - z);
			y1 = Q[0] * (x1a + xp - x) + Q[1] * (y1a + yp - y) + Q[2] * (z1a + zp - z);
			z1 = R[0] * (x1a + xp - x) + R[1] * (y1a + yp - y) + R[2] * (z1a + zp - z);

			// estremo 2 
			x2 = P[0] * (x2a + xp - x) + P[1] * (y2a + yp - y) + P[2] * (z2a + zp - z);
			y2 = Q[0] * (x2a + xp - x) + Q[1] * (y2a + yp - y) + Q[2] * (z2a + zp - z);
			z2 = R[0] * (x2a + xp - x) + R[1] * (y2a + yp - y) + R[2] * (z2a + zp - z);

			// vertex 3 
			x3 = P[0] * (x3a + xp - x) + P[1] * (y3a + yp - y) + P[2] * (z3a + zp - z);
			y3 = Q[0] * (x3a + xp - x) + Q[1] * (y3a + yp - y) + Q[2] * (z3a + zp - z);
			z3 = R[0] * (x3a + xp - x) + R[1] * (y3a + yp - y) + R[2] * (z3a + zp - z);

			color[0] = col_tris[i][0];
			color[1] = col_tris[i][1];
			color[2] = col_tris[i][2];

			if (low_graphics == 0)
			{
				xaddftriang_persp(x1, y1, -z1,
								  x2, y2, -z2,
								  x3, y3, -z3, 1, color, WIDTH, HEIGHT);
			}
			else
			{
				xaddftriang_persp(x1, y1, -z1,
								  x2, y2, -z2,
								  x3, y3, -z3, 1, color, WIDTH, HEIGHT);
			}
		}

		if (low_graphics == 0)
		{
			addfrantumation_wsim(x1, y1, z1, h, 0);			   // we must make special effect sequences go on. 
			projectile_launch(10, 10, 10, 20, 10, -0.1, h, 0); // idem 
		}

		// call function which displays the matrix of pixels in a true graphics window 

		// ==================|SDL CODE BLOCK 4|=============================
		sdldisplay(WIDTH, HEIGHT);
		if (cycles % 10 == 0)
		{
			printf("GAME TIME [sec] =  %f \n", cycles * h);
			printf("GOING ON...game cycle %i : plane position: x = %f, y = %f, z = %f \n theta = %3.2f, fi = %f3.2\n", cycles, x, y, z, theta, fi);
			printf("Xi =  %i , Yi = %i \n", Xi, Yi);

			for (j = 0; j < 10; j++)
			{
				for (i = 0; i < 10; i++)
				{
					printf("%2i|", texid[terrain1.map_texture_indexes[j][i]]);
				}
				printf("\n");
			}
		}
		SDL_Delay(10);
		cycles++;

		/*===============================|SDL's real-time interactivity|=============================*/
		while (SDL_PollEvent(&event))
		{ 
			// Loop until there are no events left on the queue 
			if (event.type == SDL_KEYDOWN)
			{ 
				// condition: keypress event detected 
				if (event.key.keysym.sym == SDLK_ESCAPE)
				{
					printf("ESC KEY PRESSED: PROGRAM TERMINATED\n");
					SDL_Quit();
					exit(1); 
				}

				if (event.key.keysym.sym == SDLK_c)
				{
					turnch = -1.0; 
				}
				if (event.key.keysym.sym == SDLK_v)
				{
					turnch = 1.0; 
				}
				if (event.key.keysym.sym == SDLK_r)
				{
					turncv = 1.0; 
				}
				if (event.key.keysym.sym == SDLK_f)
				{
					turncv = -1.0; 
				}
				if (event.key.keysym.sym == SDLK_9)
				{
					Pforce = Pforce + 5000.0; 
				}
				if (event.key.keysym.sym == SDLK_8)
				{
					Pforce = Pforce - 4000.0; 
					if (Pforce <= 0.0)
					{
						Pforce = 0.0;
					}
				}
				if (event.key.keysym.sym == SDLK_1)
				{
					RR = RR - 2.0; 
					if (view == 4)
					{
						x_cockpit_view = x_cockpit_view - 0.1;
					}
				}
				if (event.key.keysym.sym == SDLK_2)
				{
					RR = RR + 2.0;
					if (view == 4)
					{
						x_cockpit_view = x_cockpit_view + 0.1;
					}
				}

				if (event.key.keysym.sym == SDLK_3)
				{
					y_cockpit_view = y_cockpit_view + 0.1;
					if (y_cockpit_view > 2.0)
					{
						y_cockpit_view = -2.0;
					}
				}
				if (event.key.keysym.sym == SDLK_4)
				{
					z_cockpit_view = z_cockpit_view + 0.1;
					if (z_cockpit_view > 2.0)
					{
						z_cockpit_view = -2.0;
					}
				}

				if (event.key.keysym.sym == SDLK_t)
				{
					MAG = MAG + 10.0; 
				}
				if (event.key.keysym.sym == SDLK_i)
				{
					FILE *FilePtr; // pointer to input file 

					printf("TRYING TO IMPORT VERTEX LIST OF 3D MODEL\n");

					FilePtr = fopen("input/vertexes.txt", "r");
					if (FilePtr < 0)
					{
						printf("NO FILE TO IMPORT VERTEX LIST...USING DEFULT...\n");
						fclose(FilePtr);
					}
					else
					{
						import_airplane_polyheron();
					}
				}
				if (event.key.keysym.sym == SDLK_e)
				{
					addfrantumation_wsim(20, 20, 20, h, 1);
				}
				if (event.key.keysym.sym == SDLK_m)
				{
					low_graphics = 1; // LOW GRAPHICS MODE for slow computers 
				}
				if (event.key.keysym.sym == SDLK_o)
				{
					view = view + 1; // change view
					if (view == 5)
					{
						view = 1; // restore to view 1 (normal external)
					}
				}
				if (event.key.keysym.sym == SDLK_p)
				{
					aboard = -1 * aboard;
					x_pilot = xp;
					y_pilot = yp;
					RR = 1.5;
				}
				if (event.key.keysym.sym == SDLK_5)
				{

					h = h - 0.002;
				}
				if (event.key.keysym.sym == SDLK_6)
				{
					h = h + 0.001;
				}
				if (event.key.keysym.sym == SDLK_s)
				{
					// velocity of Plane's CM + velocity of projectile... rotation ignored.
					projectile_launch(xp, yp, zp, v[0] + 100.0 * Pa[0], v[1] + 100.0 * Pa[1], v[2] + 100.0 * Pa[2], h, 1);
				}
				if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w)
				{
					x_pilot = x_pilot - 2.0 * R[0];
					y_pilot = y_pilot - 2.0 * R[1];

					plane_down = 1;
				}
				if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_z)
				{
					plane_up = 1;
				}
				if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a)
				{
					// test
					plane_inclleft = 1;
				}
				if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d)
				{
					// test
					plane_inclright = 1;
				}
			} // end condition of if-keypress-is-detected 

			if (event.type == SDL_KEYUP)
			{ 
				// condition: key-RELEASE event detected 
				if (event.key.keysym.sym == SDLK_c)
				{
					turnch = 0.0; 
				}
				if (event.key.keysym.sym == SDLK_v)
				{
					turnch = 0.0; 
				}
				if (event.key.keysym.sym == SDLK_r)
				{
					turncv = 0.0; 
				}
				if (event.key.keysym.sym == SDLK_f)
				{
					turncv = 0.0; 
				}
				if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_z)
				{
					plane_up = 0;
				}
				if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w)
				{
					plane_down = 0;
				}
				if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a)
				{
					plane_inclleft = 0;
				}
				if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d)
				{
					plane_inclright = 0;
				}
			}

			// extra case: if graphic window is closed, terminate program 
			if (event.type == SDL_QUIT)
			{ 
				printf("GRAPHICS WINDOW CLOSED: PROGRAM TERMINATED\n");
				SDL_Quit();

				exit(1); 
			} // end of extra case handling part 
		} // end of continual event-check loop. 
	}

	printf("check graphics Window!\n");
	// close graphics Window to avoid abnormal terminations.
	SDL_Quit();

	getchar();

	return 0;
} // end main function

// ####################################################################################################################
// Function xclearpixboard
// ####################################################################################################################
void xclearpixboard(int xlimit, int ylimit)
{
	int i, j;
	GLdouble fW, fH;
	double aspect;

	//set GL stuff
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	/* reset all values to 0 */
	glClearColor(0.4, 0.4, 0.8, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	aspect = (double)640 / (double)480;
	fH = tan(MAG / 360 * pi) * 0.1;
	fW = fH * aspect;
	glFrustum(-fW, fW, -fH, fH, 0.1, 100000.0);

	glViewport(0, 0, xlimit, ylimit);
} // end xclearpixboard function

// ####################################################################################################################
// THE SDL graphics function: ALL SDL DRAW COMMANDS HERE... 
// ===============================|SDL CODE BLOCK 3|==================================
// ####################################################################################################################
void sdldisplay(int sw, int sh)
{
	int i, j;

	SDL_GL_SwapWindow(window);
} // end sdldisplay function

// ####################################################################################################################
// a timer function to pause to regulate FPS is a good utility to have...
// define function waitdt_sec(double): 
// ####################################################################################################################
int waitdt_ms(double tt_ms)
{
	/* declare variables used specifically to measure time and a normal double */
	clock_t time1, time2;
	double dt_ms = 0;

	/* set the variables according to the time-measurement process */
	time1 = clock(); /*REQUEST PROCESSOR TIME, OR LOCAL TIME, NO PROBLEM WHICH.*/

	while (dt_ms < tt_ms)
	{ /* WAIT: holds the program execution here until tt_sec passers. */
		time2 = clock();
		dt_ms = (time2 - time1) / (CLOCKS_PER_SEC / 1000.0);
	}
	return 1;
} // end waitdt_ms() function 

// ####################################################################################################################
// NOT USED IN OPENGL VERSION 
// interpolate 2 points graphically 
// ####################################################################################################################
void xaddline(int x1, int y1,
			  int x2, int y2, float color[3], /* change this to int color[3] */
			  int xlimit, int ylimit)
{

	float m, fx1, fx2, fy1, fy2, fi;
	int i, j, yh, temp, yh_aux;

	if (fabs(x2 - x1) > 0)
	{ /* IF LINE IS NON-VERTICAL: avoid divide-by-zero case!! */
		fx1 = (float)x1;
		fx2 = (float)x2;

		fy1 = (float)y1;
		fy2 = (float)y2;

		m = (fy2 - fy1) / (fx2 - fx1);
		/* case x2 > x1 : augment from x1 to come to x2... */
		if (x1 > x2)
		{ /* interchange them... */
			temp = x2;
			x2 = x1;
			x1 = temp;
		}

		for (i = x1; i < x2; i++)
		{
			fi = (float)i;
			yh = (int)(m * fi - m * fx1 + fy1);

			if ((i >= 0 && i < xlimit) && (yh >= 0 && yh < ylimit))
			{ /* limits! */
				pixmatrix[yh][i][0] = color[0];
				pixmatrix[yh][i][1] = color[1];
				pixmatrix[yh][i][2] = color[2];
				/* this will have 3 components, thanks to hi color-res of SDL */

				/* these points are good for cases -1.0 < m < 1.0 but are part of the super-filling for cases of m outside ( -1.0, 1.0 ) range. */

				/* nice continuous lines for cases of m outside ( -1.0, 1.0 ) range. look code it's easy to figure out how it works. */

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
					}
				}
			}
		}
	}
	else
	{ /* IF LINE IS VERTICAL */
		if (y1 < y2)
		{ /* case y1 < y2 : augment from y1 to come to y2... */
			for (yh = y1; yh < y2; yh++)
			{
				if ((x1 >= 0 && x1 < xlimit) && (yh >= 0 && yh < ylimit))
				{ /* limits! */
					pixmatrix[yh][x1][0] = color[0];
					pixmatrix[yh][x1][1] = color[1];
					pixmatrix[yh][x1][2] = color[2];
				}
			}
		}
		else
		{ /* case y2 < y1 : augment from y1 to come to y2... */
			for (yh = y2; yh < y1; yh++)
			{
				if ((x1 >= 0 && x1 < xlimit) && (yh >= 0 && yh < ylimit))
				{ /* limits! */
					pixmatrix[yh][x1][0] = color[0];
					pixmatrix[yh][x1][1] = color[1];
					pixmatrix[yh][x1][2] = color[2];
				}
			}
		}
	}
} // end xaddline function

// ####################################################################################################################
// add 1 pixel to output image but in a failsafe manner: no accidental segfaults. 
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
// now we define the function which, given 1 point in 3D, calculates where it ends up on the
// virtual camera pointing toward positive z-s and passes them to the failsafe pixel drawing function. 
// ####################################################################################################################
void xaddpoint_persp(float x1, float y1, float z1, float color[3],
					 int pbwidth, int pbheight)
{
	glColor3f(color[0], color[1], color[2]);
	glPointSize(2);

	glBegin(GL_POINTS);
		glVertex3f(x1, y1, -z1);
	glEnd();

	glFlush();
} // end xaddpoint_persp function

// ####################################################################################################################
// now we define the function which, given 2 points in 3D, calculates where they end up on the
// virtual camera pointing toward positive z-s and passes them to the 2D line drawing function. 
// ####################################################################################################################
void xaddline_persp(float x1, float y1, float z1, float x2, float y2, float z2, float color[3],
					int pbwidth, int pbheight)
{
	glColor3f(color[0], color[1], color[2]);

	glBegin(GL_LINES);
		glVertex3f(x1, y1, -z1);
		glVertex3f(x2, y2, -z2);
	glEnd();

	glFlush();
} // end xaddline_persp function

// ####################################################################################################################
// point frantumation sequence function (a special effect)
// ####################################################################################################################
void addsmoke_wsim(double x0, double y0, double z0, double dft, int option /*add new explosion or just process those already started */)
{
#define NPS 800
#define NAUTSM 44
	static int count[NAUTSM], LT = 900 /*sequence lifetime*/, visc = 3.9 /* coefficine of viscous force */, MAXN /*STATIC */, N_as = 1;
	static double xc, yc, zc, radius[NAUTSM][NPS], Vix = 0.0, Viy = 0.0, Viz = 2.0;
	int i, j, k;
	static double xm[NAUTSM][NPS], ym[NAUTSM][NPS], zm[NAUTSM][NPS], vx[NAUTSM][NPS], vy[NAUTSM][NPS], vz[NAUTSM][NPS];
	float xt, yt, zt, xt2, yt2, zt2, xt3, yt3, zt3, color[4], colors[NAUTSM][NPS][3];
	static int auxc = 0;

	glDisable(GL_DEPTH_TEST);
	if (option == 1 && N_as < NAUTSM)
	{ /* FIRST PLACE RESERVED TO AUTOSMOKE... POINTLIKE EVAPORATING SMOKE */
		count[N_as] = LT;
		xc = (float)x0;
		yc = (float)y0;
		zc = (float)z0;

		auxc = 0;
		/* careful.. total MUST EQUAL MP contant, otherwise segfault will happen. */
		for (i = -2; i < 3; i++)
		{ // SO: -1, 0 , 1
			for (j = -2; j < 3; j++)
			{
				for (k = -2; k < 3; k++)
				{

					xm[N_as][auxc] = xc + 0.1 * auxc * (double)rand() / (double)RAND_MAX; // do (LT-count) because 'count' variable starts froma LT, say, 30. just be coherent with yourself and it works.
					ym[N_as][auxc] = yc + 0.1 * auxc * (double)rand() / (double)RAND_MAX;
					zm[N_as][auxc] = zc + 0.1 * auxc * (double)rand() / (double)RAND_MAX;

					vx[N_as][auxc] = Vix * i; // do (LT-count) because 'count' variable starts froma LT, say, 30. just be coherent with yourself and it works.
					vy[N_as][auxc] = Viy * j;
					vz[N_as][auxc] = Viz * k;

					radius[N_as][auxc] = 0.01;

					colors[N_as][auxc][0] = 0.6 + (float)0.2 * rand() / (float)RAND_MAX; /* a base gray plus a little random */
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
		auxc = 0; // put it back to zero!!! see option 2 to undersan why!

		N_as++; /* we augment it... FIRST SLOT IS USED FOR DYNAMICALLY DRAGGED SMOKE SEQ*/
	}

	if (option == 2)
	{
		/* "ACCUMULO" PER COSIDDIRE LE POSIZIONI NEL BLOCCO DI NUMERI float px,py e pz,CHE ABBIAMO CREATO APPOSTA   */
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

		xm[0][0] = x0; /* nella primissima casella mettiamo la posizione di "ADESSO" */
		ym[0][0] = y0;
		zm[0][0] = z0;

		radius[0][i] = 0.01;

		colors[0][i][0] = 1.0; /* a base gray plus a little random */
		colors[0][i][1] = 1.0;
		colors[0][i][2] = 1.0;
		/*==FATTO==*/
	}

	for (j = 0; j < N_as; j++)
	{
		if (count[j] > 0)
		{ /* CHECK EXTENSION process stuff and decrease count only as long as it's above 0 yet */
			/*this anyway*/
			/* all combination of 1 and 0, see why... JUST A BASIC TRICK, this is NOT A PROFI SPECIAL EFFECT...*/
			float F_pullup = 3.0;

			/* update positions and draw, at the seme time... */
			for (i = 0; i < LT - count[j] /*!!!!!*/ && i < MAXN - 1; i++)
			{																	  /* start 'simulating' progressively more of the total particles... 
	  so smoke will seem to be fed from it's start point... */
				float rand_F = 300.0 * ((double)0.3 * rand() / (double)RAND_MAX); /* it makes dissolution */

				radius[j][i] = radius[j][i] + 13.8 * ((double)rand() / (double)RAND_MAX) * dft;

				xm[j][i] = xm[j][i] + vx[j][i] * dft; // do (LT-count) because 'count' variable starts froma LT, say, 30. just be coherent with yourself and it works.
				ym[j][i] = ym[j][i] + vy[j][i] * dft;
				zm[j][i] = zm[j][i] + vz[j][i] * dft;

				vx[j][i] = vx[j][i] - visc * vx[j][i] * dft + rand_F * dft;						   /* inertial... */
				vy[j][i] = vy[j][i] - visc * vy[j][i] * dft + rand_F * dft;						   /* inertial... */
				vz[j][i] = vz[j][i] - visc * vz[j][i] * dft + 0.1 * rand_F * dft + F_pullup * dft; /* NOT accelerated by classical gravity because it would not be belivable... its like a Brownian motion... */

				if (zm[j][i] < say_terrain_height(&terrain1, xm[j][i], ym[j][i]))
				{
					double je;
					je = terrain1.auxnormal[0] * vx[j][i] + terrain1.auxnormal[1] * vy[j][i] + terrain1.auxnormal[2] * vz[j][i];

					vx[j][i] = vx[j][i] - (1.0 + 0.95) * je * terrain1.auxnormal[0] / 1.0; // OK.
					vy[j][i] = vy[j][i] - (1.0 + 0.95) * je * terrain1.auxnormal[1] / 1.0;
					vz[j][i] = vz[j][i] - (1.0 + 0.95) * je * terrain1.auxnormal[2] / 1.0;
				}

				xt = P[0] * (xm[j][i] - x) + P[1] * (ym[j][i] - y) + P[2] * (zm[j][i] - z);
				yt = Q[0] * (xm[j][i] - x) + Q[1] * (ym[j][i] - y) + Q[2] * (zm[j][i] - z);
				zt = R[0] * (xm[j][i] - x) + R[1] * (ym[j][i] - y) + R[2] * (zm[j][i] - z);

				/* better with OpenGL graphics as long as this. or a better organized 3D drawind routine. */
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

				xaddftriang_persp(xt, yt, -zt,
								  xt2, yt2, -zt2,
								  xt3, yt3, -zt3, 2,
								  color, WIDTH, HEIGHT);
			}
		}

		count[0] = 100;
		if (count[j] > 0 && j != 0)
		{
			count[j]--;
		}
		else if (/*j != 0 && */ count[j] == 0)
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
			N_as--; // reduce total num of smoke sequences in scene but 'delete' the ended slot too bu meking shrink into it all previous ones.
		}			// else block
	}				// NAUTSM count

	glEnable(GL_DEPTH_TEST);
} // end addsmoke_wsim function

// ####################################################################################################################
// GLI EFFETTI SPECIALI di base NEI GAMES 
// point frantumation sequence function (a special effect)
// ####################################################################################################################
void addfrantumation_wsim(float x0, float y0, float z0, double dft, int option /*add new explosion or just process those already started */)
{
#define NP 100
	static int count = 0, LT = 400 /*sequence lifetime*/;
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
					xm[auxc] = xc + 1.0 * (double)rand() / (double)RAND_MAX; // do (LT-count) because 'count' variable starts froma LT, say, 30. just be coherent with yourself and it works.
					ym[auxc] = yc + 1.0 * (double)rand() / (double)RAND_MAX;
					zm[auxc] = zc + 1.0 * (double)rand() / (double)RAND_MAX;

					vx[auxc] = Vix * i + 4.2 * (double)rand() / (double)RAND_MAX; // do (LT-count) because 'count' variable starts froma LT, say, 30. just be coherent with yourself and it works.
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
	{ /* process stuff and decrease count only as long as it's above 0 yet */
		printf("DRAWING FRANTUMATION SEQUENCE...\n");
		double visca = 0; /* copy */
		/* all combination of 1 and 0, see why... JUST A BASIC TRICK, this is NOT A PROFI SPECIAL EFFECT...*/

		/* update positions and draw, at the seme time... */
		for (i = 0; i < 27; i++)
		{ // SO: -1, 0 , 1
			if (zm[i] < say_terrain_height(&terrain1, xm[i], ym[i]))
			{
				double je;
				je = terrain1.auxnormal[0] * vx[i] + terrain1.auxnormal[1] * vy[i] + terrain1.auxnormal[2] * vz[i];

				if (je < 0.0)
				{
					visca = visc;
					vx[i] = vx[i] - (1.0 + 0.8) * je * terrain1.auxnormal[0] / 1.0; // OK.
					vy[i] = vy[i] - (1.0 + 0.8) * je * terrain1.auxnormal[1] / 1.0;
					vz[i] = vz[i] - (1.0 + 0.8) * je * terrain1.auxnormal[2] / 1.0;
					printf(">>>>>>>>>IMPACT \n");
				}
			}

			xm[i] = xm[i] + vx[i] * dft; // do (LT-count) because 'count' variable starts froma LT, say, 30. just be coherent with yourself and it works.
			ym[i] = ym[i] + vy[i] * dft;
			zm[i] = zm[i] + vz[i] * dft;

			vx[i] = vx[i] - visca * vx[i] * dft + 0.0 * dft;  /* inertial... */
			vy[i] = vy[i] - visca * vy[i] * dft + 0.0 * dft;  /* inertial... */
			vz[i] = vz[i] - visca * vz[i] * dft - 9.81 * dft; /* accelerated by classical gravity */

			xt = P[0] * (xm[i] - x) + P[1] * (ym[i] - y) + P[2] * (zm[i] - z);
			yt = Q[0] * (xm[i] - x) + Q[1] * (ym[i] - y) + Q[2] * (zm[i] - z);
			zt = R[0] * (xm[i] - x) + R[1] * (ym[i] - y) + R[2] * (zm[i] - z);

			/* better with OpenGL graphics as long as this. or a better organized 3D drawind routine. */
			xt2 = P[0] * (xm[i] + radius - x) + P[1] * (ym[i] - y) + P[2] * (zm[i] - z);
			yt2 = Q[0] * (xm[i] + radius - x) + Q[1] * (ym[i] - y) + Q[2] * (zm[i] - z);
			zt2 = R[0] * (xm[i] + radius - x) + R[1] * (ym[i] - y) + R[2] * (zm[i] - z);

			xt3 = P[0] * (xm[i] - x) + P[1] * (ym[i] + 1.2 * radius - y) + P[2] * (zm[i] + radius - z);
			yt3 = Q[0] * (xm[i] - x) + Q[1] * (ym[i] + 1.2 * radius - y) + Q[2] * (zm[i] + radius - z);
			zt3 = R[0] * (xm[i] - x) + R[1] * (ym[i] + 1.2 * radius - y) + R[2] * (zm[i] + radius - z);

			xaddpoint_persp(xt, yt, -zt, color, WIDTH, HEIGHT); /* draw points in 3D scnario Z NEGATIVE!!!!!! */

			color[0] = colors[i][0];
			color[1] = colors[i][1];
			color[2] = colors[i][2];
			color[3] = 1.0;
			xaddftriang_persp(xt, yt, -zt,
							  xt2, yt2, -zt2,
							  xt3, yt3, -zt3, 1,
							  color, WIDTH, HEIGHT);
		}

		count--;
	}
} // end addfrantumation_wsim function

void projectile_launch(float xpr, float ypr, float zpr, 
					   float vx, float vy, float vz, 
					   double dft, int do_add)
{
	static int n = 0;
	static float poss[100][3];
	static float vels[100][3];
	float color[3] = {1.0, 0.0, 1.0};
	float th_sph; /* theta or sperical coordinated, used for a spherical 'explosion' */
	static int life[100];
	float xm, ym, zm, xt, yt, zt;
	float x1, y1, z1, x2, y2, z2;
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
	if (life[0] > 0)
	{
	}
	/* draw projectiles */
	for (i = 0; i < n; i++)
	{
		if (life[i] > 0)
		{

			xm = poss[i][0]; // do (LT-count) because 'count' variable starts froma LT, say, 30. just be coherent with yourself and it works.
			ym = poss[i][1];
			zm = poss[i][2];

			/* "x is an extern variable!!! be careful!!" */
			xt = P[0] * (xm - x) + P[1] * (ym - y) + P[2] * (zm - z);
			yt = Q[0] * (xm - x) + Q[1] * (ym - y) + Q[2] * (zm - z);
			zt = R[0] * (xm - x) + R[1] * (ym - y) + R[2] * (zm - z);

			xaddpoint_persp(xt, yt, -zt, color, WIDTH, HEIGHT); /* draw points in 3D scnario Z NEGATIVE!!!!!! */

			/* "x is an extern variable!!! be careful!!" */
			x1 = P[0] * (-x) + P[1] * (ym - y) + P[2] * (zm - z);
			y1 = Q[0] * (-x) + Q[1] * (ym - y) + Q[2] * (zm - z);
			z1 = R[0] * (-x) + R[1] * (ym - y) + R[2] * (zm - z);

			x2 = P[0] * (xm - x) + P[1] * (-y) + P[2] * (zm - z);
			y2 = Q[0] * (xm - x) + Q[1] * (-y) + Q[2] * (zm - z);
			z2 = R[0] * (xm - x) + R[1] * (-y) + R[2] * (zm - z);
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
			vels[i][2] = vels[i][2] - 9.81 * dft; /* classical gravity */

			if (poss[i][2] < say_terrain_height(&terrain1, poss[i][0], poss[i][1]))
			{
				double je;
				je = terrain1.auxnormal[0] * vels[i][0] + terrain1.auxnormal[1] * vels[i][1] + terrain1.auxnormal[2] * vels[i][2];
				printf(">>>>IMPACT??? j = %3.3f \n", je);

				if (je < 0.0)
				{
					vels[i][0] = vels[i][0] - (1.0 + 0.1) * je * terrain1.auxnormal[0] / 1.0; 
					vels[i][1] = vels[i][1] - (1.0 + 0.1) * je * terrain1.auxnormal[1] / 1.0;
					vels[i][2] = vels[i][2] - (1.0 + 0.1) * je * terrain1.auxnormal[2] / 1.0;
					life[i] = 0; /* put its lifetime near the end.... so soon explosion cycle will start  */

					terrain1.shmap[(int)(poss[i][0] / terrain1.GPunit)][(int)(poss[i][1] / terrain1.GPunit)] = terrain1.shmap[(int)(poss[i][0] / terrain1.GPunit)][(int)(poss[i][1] / terrain1.GPunit)] - 0.02;
					terrain1.scol[(int)(poss[i][0] / terrain1.GPunit)][(int)(poss[i][1] / terrain1.GPunit)][1] = 0.8 * terrain1.scol[(int)(poss[i][0] / terrain1.GPunit)][(int)(poss[i][1] / terrain1.GPunit)][1];

					printf(">>>>>>>>>IMPACT \n");
					addsmoke_wsim(poss[i][0], poss[i][1], poss[i][2], dft, 1);		  // add a smoke sequance at disappeared point.
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
					xm = poss[i][0] + 0.1 * radius * cos(th_sph) * sin(fi_sph); // do (LT-count) because 'count' variable starts froma LT, say, 30. just be coherent with yourself and it works.
					ym = poss[i][1] + 0.1 * radius * sin(th_sph) * sin(fi_sph);
					zm = poss[i][2] + 0.1 * radius * cos(fi_sph);

					xt = P[0] * (xm - x) + P[1] * (ym - y) + P[2] * (zm - z);
					yt = Q[0] * (xm - x) + Q[1] * (ym - y) + Q[2] * (zm - z);
					zt = R[0] * (xm - x) + R[1] * (ym - y) + R[2] * (zm - z);

					color[0] = 1.0;
					color[1] = 0.6;
					color[2] = 0.1;
					xaddpoint_persp(xt, yt, -zt, color, WIDTH, HEIGHT); /* draw points in 3D scnario Z NEGATIVE!!!!!! */
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
			} // END for
		}	  // END if
	}
} // end projectile_launch function

// ####################################################################################################################
// Function say_terrain_height
// ####################################################################################################################
double say_terrain_height(struct subterrain *ite, double x, double z /* this will be set... */)
{
	double apl, bpl, cpl, dpl, Xtri, Ytri, Ztri, dist_fp1, dist_fp2, Xf, Zf;
	int Xi, Yi, col;
	double y;
	float vector0[3], vector1[3], s_nloc[3], lenght;

	//index of which square's region it is within.
	Xi = floor(x / ite[0].GPunit); //x axis (in/out-screen)
	Yi = floor(z / ite[0].GPunit); //z axis (right/left-of-screen)

	Xf = x / ite[0].GPunit;
	Zf = z / ite[0].GPunit;

	/*a cautional correction to avoi SegmentationFault: SECURE. */
	if (Xi < 0 || Xi > ite[0].map_size)
	{ /* if it accidentally becomes negative, put it zero but nuder that there is no terrain!! */
		Xi = 0;
	}
	if (Yi < 0 || Yi > ite[0].map_size)
	{ /* if it accidentally becomes negative, put it zero but nuder that there is no terrain!! */
		Yi = 0;
	}

	dist_fp1 = sqrt(pow(Xf - Xi, 2) + pow(Zf - Yi, 2));
	dist_fp2 = sqrt(pow(Xf - (Xi + 1), 2) + pow(Zf - (Yi + 1), 2));
	//reusing Xi and Yi...:
	if (dist_fp1 < dist_fp2)
	{			 //careful if negativ or positive the Z axis... in fact.
		col = 0; //lower triangle region.
	}
	else
	{
		col = 1;
	}

	// ===THIS IS TRIANLGE 1 , BUT WE MUST ALSO IMPLEMENT THAT IT SEES IF TRI_1 OR TRI_2
	//vect1.component-by component.
	if (col == 0)
	{
		vector0[0] = 1.0; //pick from the on-purpose triangle storer....
		vector0[1] = ite[0].shmap[Xi + 1][Yi] - ite[0].shmap[Xi][Yi];
		vector0[2] = 0.0;

		vector1[0] = 0.0; //pick from the on-purpose triangle storer....
		vector1[1] = ite[0].shmap[Xi][Yi + 1] - ite[0].shmap[Xi][Yi];
		vector1[2] = 1.0;
	}
	else if (col == 1)
	{
		vector0[0] = -1.0; //pick from the on-purpose triangle storer....
		vector0[1] = ite[0].shmap[Xi][Yi + 1] - ite[0].shmap[Xi + 1][Yi + 1];
		vector0[2] = 0.0;

		vector1[0] = 0.0; //pick from the on-purpose triangle storer....
		vector1[1] = ite[0].shmap[Xi + 1][Yi] - ite[0].shmap[Xi + 1][Yi + 1];
		vector1[2] = -1.0;
	}

	/* we do directly the cross product */
	s_nloc[0] = vector0[1] * vector1[2] - vector0[2] * vector1[1]; // IMPLEMENT IT WARNING
	s_nloc[1] = vector0[2] * vector1[0] - vector0[0] * vector1[2]; //              WARNING
	s_nloc[2] = vector0[0] * vector1[1] - vector0[1] * vector1[0]; //              WARNING

	/*------------calculate apl, bpl and cpl on the fly---------------------- */
	/* use cross_product method. */
	//put equation parameters.
	apl = s_nloc[0];
	bpl = s_nloc[1];
	cpl = s_nloc[2];
	dpl = 0; //this is still to calculate... it's not = 1  usually.

	/* now be VERY careful because one SHOULD use a point which is common to bot TRI 1 (col == 0 ) and TRI 2: Xi,Yi IS NOT such a point.... so: */
	Xtri = (double)(Xi + 1) * ite[0].GPunit;
	Ytri = (double)ite[0].GPunit * ite[0].shmap[Xi + 1][Yi];
	Ztri = (double)Yi * ite[0].GPunit;

	//now finally calculate dpl , the 'd' of the   ax + by + cz + d = 0   plane equation.
	dpl = -apl * Xtri - bpl * Ytri - cpl * Ztri;

	/* now set heigt and that's it. */
	y = -(		   //is negative!chenck equation members always!!
		(apl * x + //(ax +
		 cpl * z + // cx +
		 dpl) /
		bpl // d )/b
	);

	/* additional stuff( NOT part of previous procedures) */
	lenght = sqrt(pow(s_nloc[0], 2) + pow(s_nloc[1], 2) + pow(s_nloc[2], 2));

	/* normalize: this vector must be unit-lenght */
	s_nloc[0] = s_nloc[0] / lenght;
	s_nloc[1] = s_nloc[1] / lenght;
	s_nloc[2] = s_nloc[2] / lenght;

	//look if verse is good... not always; must check if the y of the normal is positive... easy. if negative, invert vector.
	if (s_nloc[1] < 0.0)
	{ // be coherent with indexes, which is x which y and z for your implementation.
		s_nloc[0] = -s_nloc[0];
		s_nloc[1] = -s_nloc[1];
		s_nloc[2] = -s_nloc[2];
	}

	/* copy here... very very useful to have such an auxiliary variable */
	/* WARNING standard notation... z points 'upwards' */
	ite[0].auxnormal[0] = s_nloc[0];
	ite[0].auxnormal[1] = s_nloc[2];
	ite[0].auxnormal[2] = s_nloc[1];

	return y;
} // end say_terrain_height function

// ####################################################################################################################
// Function xaddftriang draws a filled triangle to pixel matrix 
// ####################################################################################################################
void xaddftriang(int x1, int y1,
				 int x2, int y2,
				 int x3, int y3,
				 float color[3],
				 int step,
				 int xlimit, int ylimit)
{
	/*====== convert RGB color intensities, call the allocate/activate function provided by Xlib,
 and then call the function that puts it into use.======= */
	int color_index = 0;

	if (color[0] > 1.0)
	{
		color[0] = 1.0;
	}

	if (color[1] > 1.0)
	{
		color[1] = 1.0;
	}

	if (color[2] > 1.0)
	{
		color[2] = 1.0;
	}
} // end xaddftriang function

// ####################################################################################################################
// now we define the function which, given 2 points in 3D, calculates where they end up on the
// virtual camera pointing toward positive z-s and passes them to the 2D line drawing function. 
// ####################################################################################################################
void xaddftriang_persp(float x1, float y1, float z1,
					   float x2, float y2, float z2,
					   float x3, float y3, float z3,
					   int step,
					   float color[4], int pbwidth, int pbheight)
{
	glColor4f(color[0], color[1], color[2], color[3]);

	glBegin(GL_TRIANGLES);
		glVertex3f(x1, y1, -z1);
		glVertex3f(x2, y2, -z2);
		glVertex3f(x3, y3, -z3);
	glEnd();

	glFlush();

	/* this causes deformation and it is both gemetrically and visually WRONG, but the deformations are minimal, and it is just a trick to avoid bad looking scenaries, like pieces missing from the car just because in an internal visualisation some triangles have a vertex behind the obserzer ( z < 0 ) */
} // end xaddftriang_persp function

// ####################################################################################################################
// Function GLaddftriang_perspTEXTURED
// ####################################################################################################################
void GLaddftriang_perspTEXTURED(float x1, float y1, float z1,
								float x2, float y2, float z2,
								float x3, float y3, float z3,
								int texId, float texcoords[3][2],
								float color[3], int pbwidth, int pbheight)
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
								  float color[3], int pbwidth, int pbheight)
{
	static int alternative = 0;
	static int texture_generated = 0; /* at first call of this function, a 32x32 texture sample will be generated */
	float mag;
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
	{ //side 2 longer than the others: x2 IS the hypothenuse of a quasi-rect triagle.
		// lave p1, p2, p3 as they are... they are ok.
		alternative = 1; // first case: 1-2 and 1-3 are the catetes... .
	}
	else if ((pow(x2 - x1, 2) + pow(y2 - y1, 2) + pow(z2 - z1, 2)) > (pow(x3 - x1, 2) + pow(y3 - y1, 2) + pow(z3 - z1, 2)) && (pow(x2 - x1, 2) + pow(y2 - y1, 2) + pow(z2 - z1, 2)) > pow(x2 - x3, 2) + pow(y2 - y3, 2) + pow(z2 - z3, 2))
	{
		// chenge order... .
		float tx, ty, tz;

		alternative = 2; // second case. 1-3 and 2-3 are cathetes
		// must chenge order so that 1-2 and 1-3 be cathetes:

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

	float v_hor[3] = {3.4, 1.2, 2.0}; // the one with greter x-component... ('more horizonal'
	float v_ver[3] = {1.2, 2.3, 1.0}; // the one wiht greter y-component... ('more vertical' )
									  /* first x-y couple is intact... MIDPOINT, DOWNER */

	float jff, iff, limh, limv, texres;
	texres = step; //texture resolution

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
		{ // SEEMS OK... LOOKS OK.
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
			{ // DON'T DRAW THIS if we are on edge. figure out why... very simple
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

/* Prototype of the struct meant to be the main container of data, 
be it single numbers, couples, tripets or heterogeneous mixes of data */
struct mystruct
{
	float z;
	int ind_triang[3];
	int color_index;
};

// ####################################################################################################################
// Interchange *px and *py  STRUCT 
// ####################################################################################################################
void swap(struct mystruct *px, struct mystruct *py)
{
	/*we clone a structure of the kind needed, it will be used as a temorary store. */
	struct mystruct temp;
	temp = *px;
	*px = *py;
	*py = temp;
} // end function swap

// ####################################################################################################################
// Shell Sort STRUCT 
// shellsort: sort v[0]...v[n-1] into increasing order, with respect to some elment of the struct.
// Calls swap(). 
// ####################################################################################################################
void shellsort_struct(struct mystruct *v, int n)
{
	int gap, i, j;
	for (gap = n / 2; gap > 0; gap /= 2)
	{
		for (i = gap; i < n; i++)
		{
			for (j = i - gap; j >= 0 && (v[j].z) > (v[j + gap].z); j -= gap)
			{							  /* sort with respect to d */
				swap(&v[j], &v[j + gap]); /*  */
			}
		}
	}
} // end shellsort_struct function

// ####################################################################################################################
// Function mat3x3_mult multiplies two 3x3 matrices and places the result in global variable result_matrix.
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
			result_matrix[im][jm] = sum; /* EXTERN VALUE!!! It's an easy way to implement all this. */
		}
	}
} // end mat3x3_mult function

// ####################################################################################################################
// INVERSE OF 3x3 MATRIC (USED FOR OBTAINING THE INVERSE OF THE INERTIA TENSOR)
// ####################################################################################################################
void inv(double in_3x3_matrix[3][3])
{
	double A[3][3]; /* the matrix that is entered by user */
	double B[3][3]; /*the transpose of a matrix A */
	double C[3][3]; /*the adjunct matrix of transpose of a matrix A not adjunct of A*/
	double X[3][3]; /*the inverse*/
	int i, j;
	double x, n = 0; /*n is the determinant of A*/

	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			A[i][j] = in_3x3_matrix[i][j];
			B[i][j] = 0;
			C[i][j] = 0;
		}
	}

	/*determinant of A (presumebly the inertia tensor):*/
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
			result_matrix[i][j] = C[i][j] * x; /* EXTERN VALUE!!!  */
		}
	}
} // end inv function

// ####################################################################################################################
// Function body_rebounce
// ####################################################################################################################
double body_rebounce(double rx, double ry, double rz,
					 double nx, double ny, double nz, double e, double lat)
{
	double jelf = 0, jel = 300.0;
	double vector0[3], vector1[3], axis[3];
	double vvertex[3], vnorm;
	double auxv[3], auxv2[3], UP, DOWN; // auxiliary to brake up some longer formulas into feasibly small parts.

	vector0[0] = nx;
	vector0[1] = ny;
	vector0[2] = nz;

	vector1[0] = rx;
	vector1[1] = ry;
	vector1[2] = rz;

	axis[0] = vector0[1] * vector1[2] - vector0[2] * vector1[1]; // x component
	axis[1] = vector0[2] * vector1[0] - vector0[0] * vector1[2]; // y component
	axis[2] = vector0[0] * vector1[1] - vector0[1] * vector1[0]; // z component

	// let's do the hit resolution in a correct way, also becuase when it can be done, let's do it: good collision simulation for sigle ridig-body make good landings.

	vvertex[0] = v[0] + w[1] * vector1[2] - w[2] * vector1[1]; // x component
	vvertex[1] = v[1] + w[2] * vector1[0] - w[0] * vector1[2]; // y component
	vvertex[2] = v[2] + w[0] * vector1[1] - w[1] * vector1[0]; // z component

	vnorm = nx * vvertex[0] + ny * vvertex[1] + nz * vvertex[2];
	if (vnorm < 0)
	{ // safecheck of right collision... it never will certainly rebounce *towards* the very terrain from which it is rebouncing.
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
		// "jel" is the rright impulse, now appy the impulse and assign final vleocity and rotation.

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
//be very careful to assign storage space correctly!!!! ohterwise it brigs to 0 all elements!!!
#define ne 1000
	int i, j, k;
	double Ixxe[ne], Iyye[ne], Izze[ne]; /* those  'principal moments of inertia'.... */
	double Ixye[ne], Ixze[ne], Iyze[ne]; /* those  'products of inertia'....          */
	double Ixx = 0, Iyy = 0, Izz = 0;	 /* the total principal moments of inertia to put into the diagonals of the 3x3 tensor matrix. */
	double Ixy = 0, Ixz = 0, Iyz = 0;	 /* these are automatically set = 0, that's important.... */
	double std_vxmass = 10.0;			 // 10 Kg

	//compute the elemets of the final tensor matrix:
	for (i = 0; i < n_vertexs; i++)
	{
		//those 'principal moments of inertia'...:
		Ixxe[i] = std_vxmass * (pow(punti[i][1], 2) + pow(punti[i][2], 2)); // y2 + z2
		Iyye[i] = std_vxmass * (pow(punti[i][0], 2) + pow(punti[i][2], 2)); // x2 + z2
		Izze[i] = std_vxmass * (pow(punti[i][0], 2) + pow(punti[i][1], 2)); // x2 + y2

		//those 'products of inertia'...:
		Ixye[i] = std_vxmass * (punti[i][0] * punti[i][1]); // xy
		Ixze[i] = std_vxmass * (punti[i][0] * punti[i][2]); // xz
		Iyze[i] = std_vxmass * (punti[i][1] * punti[i][2]); // yz
	}

	//sum up:
	for (k = 0; k < n_vertexs; k++)
	{
		Ixx = Ixx + Ixxe[k];
		Iyy = Iyy + Iyye[k];
		Izz = Izz + Izze[k];

		Ixy = Ixy + Ixye[k];
		Ixz = Ixz + Ixze[k];
		Iyz = Iyz + Iyze[k];
	}

	//put principal moments of inertia into the diagonals of the result matrix:
	It_init[0][0] = Ixx;
	It_init[1][1] = Iyy;
	It_init[2][2] = Izz;

	//put inertia products in result_matrix tensor:
	It_init[0][1] = -Ixy;
	It_init[1][0] = -Ixy;

	It_init[0][2] = -Ixz;
	It_init[2][0] = -Ixz;

	It_init[1][2] = -Iyz;
	It_init[2][1] = -Iyz;

	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
		}
	}
} // end make_inertia_tensor function

// ####################################################################################################################
// Function load_textures_wOpenGL
// ####################################################################################################################
void load_textures_wOpenGL()
{
	static int texture_generated = 0; /* at first call of this function, a 32x32 texture sample will be generated */

	/*	Create texture	*/
/* maximal vlues... IF POSSIBLE DON'T EXPLOIT MAXIMUMS. */
#define txtWidth 128
#define txtHeight 128

	int txtRES = 128; // A REASONAMBLE TEXTURE RESOUTION

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
			SDL_Surface *image; //This pointer will reference our bitmap.

			int bytes_per_color, imhe;
			Uint8 red, green, blue;
			Uint32 color_to_convert;

			bytes_per_color = COLDEPTH / 8;

			sprintf(filename, "textures/terrain_texture_%i.bmp", texn);
			printf("TRYING TO OPEN FILE: %s", filename);

			//little example:  image = SDL_LoadBMP("image.bmp") ;
			image = SDL_LoadBMP(filename);

			imhe = 128;
			if (image != NULL)
			{
				printf("bitmap found: %s\n", filename);

				imhe = image->h;
				txtRES = imhe; // set TEXTURE RESOLUTION txt must be SQUARE!!!
				SDL_Delay(5);

				/*---------feed into 'the' array used for this.....---------*/
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

				//Release the surface
				SDL_FreeSurface(image);

				texName = texn;

				//---------| TEXTURE PROCESSING |-----THIS PART MUST BE EXECUTED ONLY ONCE!!! OTHEERWISE IT SILENTLY OVERLOADS MEMORY AT EACH CALL-----------

				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

				glGenTextures(1, &texName);

				texid[texn - 1] = (int)texName; // [texn-1] because startd fron 1, be careful

				printf("teName %i\n", texid[texn - 1]);

				glBindTexture(GL_TEXTURE_2D, texid[texn - 1]); // [texn-1] because startd fron 1, be careful

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST /* GL_LINEAR  */); // what OpgnGL should do when texture is magnified GL_NEAREST: non-smoothed texture | GL_LINEAR: smoothed
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);	// ...when texture is miniaturized because far; GL_NEAREST: non-smoothed tecture

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB4, txtRES, txtRES, 0, GL_RGB, GL_UNSIGNED_BYTE, txt1);
				glGenerateMipmap(GL_TEXTURE_2D);

				glEnable(GL_TEXTURE_2D);
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, /*GL_COMBINE*/ GL_DECAL); // the GL_DECAL option draws texture as is: no color mixing thigs. GL_MODULATE lets mixing.

				glBindTexture(GL_TEXTURE_2D, texName);
				//--------------------------| END OF TEXTURE LOAD PROCESSING |-------------------------------

				texn++;					   // augment count... next texture
				textures_available = texn; // at left is extern... you know... .
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
	static int texture_generated = 0; /* at first call of this function, a 32x32 texture sample will be generated */

	/*	Create texture	*/
/* maximal vlues... IF POSSIBLE DON'T EXPLOIT MAXIMUMS. */
#define txtWidth2 32
#define txtHeight2 32
#define txtWidth3 96
#define txtHeight3 96

	int txtRES2 = 96; // A REASONAMBLE TEXTURE RESOUTION
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
			SDL_Surface *image; //This pointer will reference our bitmap.

			int bytes_per_color, imhe;
			Uint8 red, green, blue, alpha;
			Uint32 color_to_convert;

			bytes_per_color = COLDEPTH / 8;

			sprintf(filename, "textures/semitransparent/texture_%i.bmp", texn);
			printf("TRYING TO OPEN FILE: %s", filename);

			//little example:  image = SDL_LoadBMP("image.bmp") ;
			image = SDL_LoadBMP(filename);

			imhe = 96;
			if (image != NULL)
			{
				printf("bitmap found: %s\n", filename);

				imhe = image->h;
				txtRES2 = imhe; // set TEXTURE RESOLUTION txt must be SQUARE!!!
				printf("bitmap RES: %i\n", imhe);
				SDL_Delay(5);

				/*---------feed into 'the' array used for this.....---------*/
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
						{					   // ADD TRANSPARECY VALUE ARTIFICIALLY ACCORDING TO SOME CONVENTION LIKE BLACK => TRANSPARENT
							txt1[j][i][3] = 0; // make this pixel totally transparent (alpha = 0) ; opaque is alpha = 255. .
						}

						if (alpha < 255)
						{
							printf("pixel : [%d,%d,%d ,alpha_value: %d]\n", red, green, blue, alpha);
						}
					}
				}

				//Release the surface
				SDL_FreeSurface(image);

				texName = textures_available + texn - 1; // VERY CAREFUL!!! NOT OVERWRITE ALREADY OCCUPIED TEXTURES!!

				//---------| TEXTURE PROCESSING |-----THIS PART MUST BE EXECUTED ONLY ONCE!!! OTHEERWISE IT SILENTLY OVERLOADS MEMORY AT EACH CALL-----------
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

				glGenTextures(1, &texName);

				texid[textures_available + texn - 1] = texName; // [texn-1] because startd fron 1, be careful

				glBindTexture(GL_TEXTURE_2D, texid[textures_available + texn - 1]); // [texn-1] because startd fron 1, be careful

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST /* GL_LINEAR  */); // what OpgnGL should do when texture is magnified GL_NEAREST: non-smoothed texture | GL_LINEAR: smoothed
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);	// ...when texture is miniaturized because far; GL_NEAREST: non-smoothed tecture

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, txtRES2, txtRES2, 0, GL_RGBA, GL_UNSIGNED_BYTE, txt1);
				glGenerateMipmap(GL_TEXTURE_2D);

				glEnable(GL_TEXTURE_2D);
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, /*GL_COMBINE*/ GL_DECAL); // the GL_DECAL option draws texture as is: no color mixing thigs. GL_MODULATE lets mixing.

				glBindTexture(GL_TEXTURE_2D, texName);
				//--------------------------| END OF TEXTURE LOAD PROCESSING |-------------------------------

				texn++;				  // augment count... next texture
				textures_available++; // idem but on an aextern variable... .
			}
			else
			{
				printf("File opening error ocurred. Using random generated texture.\n");
				printf("SDL_GetError() notify: %s\n", SDL_GetError());
				texn = -1; // cause exiting from while loop.
			}
		}
	}
} // end load_textures96x96_SEMITRANSPARENT_wOpenGL function

// ####################################################################################################################
// loads and fill in hmap, colmap, and the image indicating where to give bulk filling textures, and 
// where to visualizare colmap's colo, wher elet instead texture with it's original colors 
// ####################################################################################################################
int load_hmap_from_bitmap(char *filename)
{
	int i, j, isz;
	SDL_Surface *image; // This pointer will reference our bitmap.
	Uint8 red, green, blue;
	Uint32 color_to_convert;

	isz = TERRAIN_SIZE;

	// generate defaul map... so if no hmap image file, one can start editing from 0 and save a map later.
	for (j = 0; j < TERRAIN_SIZE; j++)
	{
		for (i = 0; i < TERRAIN_SIZE; i++)
		{
			terrain1.shmap[j][i] = (float)rand() / (float)RAND_MAX;
		}
	}
	//=================GROUND TEXTURE PERSONALISED...=====================

	printf("TRYING TO OPEN FILE: %s\n", filename);

	//little example:  image = SDL_LoadBMP("image.bmp") ;
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

				terrain1.shmap[i][j] = ((float)red + 256.0 * ((float)green)) / (256.0); /* SIMPLIFICED WAY... */
			}
		}
		printf("HEIGHTMAP LOADED FROM FILE: %s\n", filename);
	}

	//Release the surface
	SDL_FreeSurface(image);

	return isz;
} // end load_hmap_from_bitmap function

// ####################################################################################################################
// load texture ID map from a bitmap deviced by the edito (or with a graphics editor program, but 
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

		/*---------feed into 'the' array used for this.....---------*/
		for (j = 0; j < 300; j++)
		{ // vertical
			for (i = 0; i < 300; i++)
			{ // horizontal
				color_to_convert = getpixel(sdl_image, i, j);
				SDL_GetRGB(color_to_convert, sdl_image->format, &red, &green, &blue);

				terrain1.map_texture_indexes[i][299 - j] = -0 + (int)red + ((int)green) * 256; // divided by two because of our convention...
				if (terrain1.map_texture_indexes[i][299 - j] >= textures_available)
				{												  // if indes is superior to the number of total loaded textures, put it to some defult number within num of availble textures.
					terrain1.map_texture_indexes[i][299 - j] = 0; // defult.
				}

				if (i < 14 && i < 22)
				{
					printf("%2i|", terrain1.map_texture_indexes[i][299 - j]);
				}
			}
			printf("\n");
		}

		//Release the surface
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
// leggi file e controlla quanti numeri ci sono: 
// N.B.: numeri separati da spazi o da a-capo. Con virgole o altro si blocca. 
// ####################################################################################################################
long int check_vector_elements(char filename[])
{
	FILE *InFilePtr; /* pointer to input file */
	InFilePtr = fopen(filename, "r");
	long int i = 0;
	float test;

	while (fscanf(InFilePtr, "%f", &test) != EOF)
	{
		i++;
		printf("%f  \n", test);
	}

	fclose(InFilePtr); /* safe file closure. */
	return i;
} // end check_vector_elements function

/*=======================| Read in numeric vector form file |==========
only space-separated or newline-separated numbers!! else goes error ====*/
void read_vector(char filename[], float dest_string[], long int maxsize)
{
	FILE *FilePtr; /* pointer to input file */
	FilePtr = fopen(filename, "r");
	long int i = 0; /* MUST put it   =0  .... */

	while (fscanf(FilePtr, "%f", &dest_string[i]) != EOF && i < maxsize)
	{
		i++; /* augment index of casel in dest_string[].... */
		printf("%f . \n", dest_string[i]);
	}
	fclose(FilePtr); /* safe file closure. */
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
	FILE *FilePtr; /* pointer to input file */

	printf("TRYING TO IMPORT VERTEX LIST OF 3D MODEL\n");

	nelem = check_vector_elements("input/vertexes.txt");
	read_vector("input/vertexes.txt", auxxv, nelem); //read file and values in the auxxv array.

	/* feed into 'the' array used for this..... */
	for (j = 0; j < nelem / 3; j++)
	{
		for (i = 0; i < 3; i++)
		{
			punti[j][i] = 2.4 * auxxv[j * 3 + i];
		}
	}
	nvertexes = nelem / 3;

	printf("TRYING TO IMPORT TRIANGULATION OF 3D MODEL\n");

	nelem = check_vector_elements("input/triangulation.txt");
	read_vector("input/triangulation.txt", auxxv, nelem); //read file and values in the auxxv array.

	/* feed into 'the' array used for this..... */
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
	read_vector("input/facecolor.txt", auxxv, nelem); //read file and values in the auxxv array.

	/* feed into 'the' array used for this..... */
	for (j = 0; j < nelem / 3; j++)
	{
		for (i = 0; i < 3; i++)
		{
			col_tris[j][i] = auxxv[j * 3 + i];
		}
	}
} // end import_airplane_polyheron function

// ####################################################################################################################
// IF WANT TO UNDERSTAND ALL COLOR ANMD PIXEL INFO IN SDL, LOOK HERE: http://sdl.beuc.net/sdl.wiki/Pixel_Access
// SUPERSAFE TOOK IT FROM PROFESSIONAL SITE
// ####################################################################################################################
Uint32 getpixel(SDL_Surface *surface, int x, int y)
{
	int bpp = surface->format->BytesPerPixel;
	/* Here p is the address to the pixel we want to retrieve */
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch (bpp)
	{
	case 1:
		return *p;
		break;

	case 2:
		return *(Uint16 *)p;
		break;

	case 3: /* don't care about this bullshit! */
		if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
			return p[0] << 16 | p[1] << 8 | p[2];
		else
			return p[0] | p[1] << 8 | p[2] << 16;
		break;

	case 4:
		return *(Uint32 *)p;
		break;

	default:
		return 0; /* shouldn't happen, but avoids warnings */
	}
} // end getpixel function
