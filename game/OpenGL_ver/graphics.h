#ifndef FC_GRAPHICS_H
#define FC_GRAPHICS_H

// getPixel is used for bitmap conversion to RGB value matrix (usual stuff)
Uint32 getPixel(SDL_Surface *surface, int x, int y);

// this is the prototype of the function which will draw the pixel.
void sdldisplay();

void clearScreen(int xlimit, int ylimit);

// now we define the function which, given 2 points in 3D, calculates where they end up on the
// virtual camera pointing toward positive z-axis and passes them to the 2D line drawing function. 
void drawFilledPerspTriangle(float x1, float y1, float z1,
							 float x2, float y2, float z2,
							 float x3, float y3, float z3,
							 float color[3]);

void drawTexturedTriangle(float x1, float y1, float z1,
						  float x2, float y2, float z2,
						  float x3, float y3, float z3,
						  int texId, float texcoords[3][2],
						  float color[3]);

void drawPerspPoint(float x1, float y1, float z1, float color[3]);

void drawPerspLine(float x1, float y1, float z1, float x2, float y2, float z2, float color[3]);

void drawLogo(void);
void drawAxes(void);
void updatePQRAxes(float theta, float fi);
void updateVirtualCameraPos(float zoomFactor);
void reorientAxes(void);
#endif