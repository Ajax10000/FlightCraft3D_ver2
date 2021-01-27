#include <stdio.h>           // for printf()
#include <stdlib.h>          // for exit(), rand(), RAND_MAX

#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <GL/glcorearb.h>

#include "globals.h"         // for the global variables
#include "graphics.h"        // for draw... functions
#include "specialEffects.h"  // for our prototypes
#include "terrain.h"         // for getTerrainHeight()

// ####################################################################################################################
// Function addSmokeAtPoint draws smoke at the point (x0, y0, z0)
//
// point frantumation sequence function (a special effect)
// add new explosion or just process those already started
// ####################################################################################################################
void addSmokeAtPoint(double x0, double y0, double z0, double dft, int option)
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

				if (zm[j][i] < getTerrainHeight(&gloTerrain, xm[j][i], ym[j][i]))
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

				drawFilledPerspTriangle(xt,  yt,  zt,
										xt2, yt2, zt2,
										xt3, yt3, zt3, 
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
} // end addSmokeAtPoint function

// ####################################################################################################################
// Function addExplosionAtPoint
// Gli effetti speciali di base nei games / the basic special effects in the games
// point frantumation sequence function (a special effect)
// add new explosion or just process those already started
// ####################################################################################################################
void addExplosionAtPoint(float x0, float y0, float z0, double dft, int option)
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
			if (zm[i] < getTerrainHeight(&gloTerrain, xm[i], ym[i]))
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

			drawPerspPoint(xt, yt, -zt, color); // draw points in 3D scenario Z NEGATIVE!!!!!! 

			color[0] = colors[i][0];
			color[1] = colors[i][1];
			color[2] = colors[i][2];
			color[3] = 1.0;

			drawFilledPerspTriangle(xt,  yt,  zt,
									xt2, yt2, zt2,
									xt3, yt3, zt3, 
									color);
		}

		count--;
	}
} // end addExplosionAtPoint function

// ####################################################################################################################
// Function launchProjectiles
// ####################################################################################################################
void launchProjectiles(float xpr, float ypr, float zpr, 
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

			drawPerspPoint(xt, yt, -zt, color); // draw points in 3D scenario Z NEGATIVE!!!!!! 
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

			if (poss[i][2] < getTerrainHeight(&gloTerrain, poss[i][0], poss[i][1]))
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
					addSmokeAtPoint(poss[i][0], poss[i][1], poss[i][2], dft, 1);		  // add a smoke sequence at disappeared point.
					addExplosionAtPoint(poss[i][0], poss[i][1], poss[i][2], dft, 1); // add a frantumation sequance at disappeared point.
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
					drawPerspPoint(xt, yt, -zt, color); // draw points in 3D scenario Z NEGATIVE!!!!!! 
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
} // end launchProjectiles function