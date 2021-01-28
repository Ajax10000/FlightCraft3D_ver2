#include <math.h>            // for cos(), pow(), sin()
#include <stdio.h>           // for printf()

#include <SDL2/SDL.h>

#include "globals.h"         // for the global variables
#include "graphics.h"        // for reorientAxes()
#include "math.h"            // for invert3x3Matrix(), multTwo3x3Matrices()
#include "physics.h"         // for our prototypes

// ####################################################################################################################
// Function initPhysicsVars 
//
// Assumptions:
//    Vectors v and w have already been initialized.
// ####################################################################################################################
void initPhysicsVars()
{
	int i, j;

	// makeInertiaTensor will set global variable It_init
	makeInertiaTensor(NVERTEXES);

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
	L[0] = It_now[0][0] * w[0]  +  It_now[0][1] * w[1]  +  It_now[0][2] * w[2];
	L[1] = It_now[1][0] * w[0]  +  It_now[1][1] * w[1]  +  It_now[1][2] * w[2];
	L[2] = It_now[2][0] * w[0]  +  It_now[2][1] * w[1]  +  It_now[2][2] * w[2];

	invert3x3Matrix(It_init); // puts the inverse matrix into the "gloResultMatrix" global variable. 

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
// Function makeInertiaTensor
// ####################################################################################################################
void makeInertiaTensor(int n_vertexs)
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
		Ixxe[i] = std_vxmass * (pow(gloPunti[i][1], 2) + pow(gloPunti[i][2], 2)); // y^2 + z^2
		Iyye[i] = std_vxmass * (pow(gloPunti[i][0], 2) + pow(gloPunti[i][2], 2)); // x^2 + z^2
		Izze[i] = std_vxmass * (pow(gloPunti[i][0], 2) + pow(gloPunti[i][1], 2)); // x^2 + y^2

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
} // end makeInertiaTensor function

// ####################################################################################################################
// Function simulatePhysics
// ####################################################################################################################
void simulatePhysics(int plane_up, int plane_down, int plane_inclleft, int plane_inclright, float h, double g)
{
	int i, j;

	updateTorque(plane_up, plane_down, plane_inclleft, plane_inclright);

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

	vlat  = gloAxis3[0] * v[0] + gloAxis3[1] * v[1] + gloAxis3[2] * v[2]; // dot product calculated directly
	vperp = gloAxis2[0] * v[0] + gloAxis2[1] * v[1] + gloAxis2[2] * v[2]; // dot product calculated directly
	vpar  = gloAxis1[0] * v[0] + gloAxis1[1] * v[1] + gloAxis1[2] * v[2];

	rot1  = gloAxis1[0] * w[0] + gloAxis1[1] * w[1] + gloAxis1[2] * w[2]; // how much it rotates around axis 1 (nose--> back)
	rot2  = gloAxis2[0] * w[0] + gloAxis2[1] * w[1] + gloAxis2[2] * w[2]; // how much it rotates around axis 2 (perp to wings)
	rot3  = gloAxis3[0] * w[0] + gloAxis3[1] * w[1] + gloAxis3[2] * w[2]; // how much it rotates around axis 3 (parallel to wings)
	// effect (in a very rudimentary approximation of aerodynamics, not scientific at all) of air friction
	// on the motion of Center of Mass directly.

	Fcm[0] += -k_visc * vperp * gloAxis2[0]  -  k_visc2 * vlat * gloAxis3[0]  -  k_visc3 * vpar * gloAxis1[0]  +  Pforce * gloAxis1[0];
	Fcm[1] += -k_visc * vperp * gloAxis2[1]  -  k_visc2 * vlat * gloAxis3[1]  -  k_visc3 * vpar * gloAxis1[1]  +  Pforce * gloAxis1[1];
	Fcm[2] += -k_visc * vperp * gloAxis2[2]  -  k_visc2 * vlat * gloAxis3[2]  -  k_visc3 * vpar * gloAxis1[2]  +  Pforce * gloAxis1[2];

	// same for the rotational motion. other effects are not considered.
	// if you're an expert of aeromobilism, you can write better, just substitute these weak formulas for better ones.

	// generic stabilization (very poor approximation)
	gloTtlTorque[0] += -vpar * k_visc_rot_STABILIZE * w[0];
	gloTtlTorque[1] += -vpar * k_visc_rot_STABILIZE * w[1];
	gloTtlTorque[2] += -vpar * k_visc_rot_STABILIZE * w[2];

	double boh = gloAxis3[0] * v[0]  +  gloAxis3[1] * v[1]  +  gloAxis3[2] * v[2];

	// effetto coda verticale: decente...
	// vertical tail effect: decent ...
	gloTtlTorque[0] += -k_visc_rot3 * (boh)*gloAxis2[0];
	gloTtlTorque[1] += -k_visc_rot3 * (boh)*gloAxis2[1];
	gloTtlTorque[2] += -k_visc_rot3 * (boh)*gloAxis2[2];

	// total Fcm and total torque done

	// Update velocity, linear and angular too
	// momentum p (linear quantity)
	p[0] = MASS * v[0];
	p[1] = MASS * v[1];
	p[2] = MASS * v[2];

	p[0] = p[0]  +  Fcm[0] * h;
	p[1] = p[1]  +  Fcm[1] * h; // we model gravity as a force given by: g*MASS, downward 
	p[2] = p[2]  +  Fcm[2] * h  +  g * MASS * h;

	v[0] = p[0] / MASS;
	v[1] = p[1] / MASS;
	v[2] = p[2] / MASS;

	L[0] = L[0]  +  gloTtlTorque[0] * h;
	L[1] = L[1]  +  gloTtlTorque[1] * h;
	L[2] = L[2]  +  gloTtlTorque[2] * h;

	// now we get the updated velocity, component by component.
	w[0] = inv_It_now[0][0] * L[0]  +  inv_It_now[0][1] * L[1]  +  inv_It_now[0][2] * L[2];
	w[1] = inv_It_now[1][0] * L[0]  +  inv_It_now[1][1] * L[1]  +  inv_It_now[1][2] * L[2];
	w[2] = inv_It_now[2][0] * L[0]  +  inv_It_now[2][1] * L[1]  +  inv_It_now[2][2] * L[2];

	// angular momentum (angular/rotational quantity)

	// reset forces to 0.0 
	for (i = 0; i < 3; i++)
	{
		Fcm[i] = 0.0;
		gloTtlTorque[i] = 0.0;
	}

	// update position and orientation
	// xp, yp, zp give the location of the airplane
	xp = xp  +  v[0] * h;
	yp = yp  +  v[1] * h;
	zp = zp  +  v[2] * h;

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
	multTwo3x3Matrices(SD, SD); 

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
	multTwo3x3Matrices(dR, Rm); 

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
	multTwo3x3Matrices(Rm, It_init);

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
	multTwo3x3Matrices(gloTempMatrix, R_T);

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
	multTwo3x3Matrices(Rm, It_initINV);

	for (j = 0; j < 3; j++)
	{
		for (i = 0; i < 3; i++)
		{
			gloTempMatrix[j][i] = gloResultMatrix[j][i]; //SAFE COPY!!! PASSING EXTERN VARIABLE AND THEN MODIFYING IT IS NOT SAFE... COMPILERS MAY FAIL TO DO IT CORRECTLY!!!
			//SAFEST SIMPLE METHOD --> BEST METHOD.
		}
	}

	// Calculate gloResultMatrix = gloTempMatrix*R_T
	multTwo3x3Matrices(gloTempMatrix, R_T);

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
} // end simulatePhysics function 

// ####################################################################################################################
// Function bounceAirplane bounces the airplane off of the ground (i.e., terrain).
// ####################################################################################################################
double bounceAirplane(double rx, double ry, double rz,
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

	axis[0] = vector0[1] * vector1[2]  -  vector0[2] * vector1[1]; // x component
	axis[1] = vector0[2] * vector1[0]  -  vector0[0] * vector1[2]; // y component
	axis[2] = vector0[0] * vector1[1]  -  vector0[1] * vector1[0]; // z component

	// let's do the hit resolution in a correct way, also becuase when it can be done, let's do it: 
	// good collision simulation for single rigid-body make good landings.

	vvertex[0] = v[0]  +  w[1] * vector1[2]  -  w[2] * vector1[1]; // x component
	vvertex[1] = v[1]  +  w[2] * vector1[0]  -  w[0] * vector1[2]; // y component
	vvertex[2] = v[2]  +  w[0] * vector1[1]  -  w[1] * vector1[0]; // z component

	vnorm = nx * vvertex[0]  +  ny * vvertex[1]  +  nz * vvertex[2];
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

		auxv[0] = vector0[1] * vector1[2]  -  vector0[2] * vector1[1]; // x component
		auxv[1] = vector0[2] * vector1[0]  -  vector0[0] * vector1[2]; // y component
		auxv[2] = vector0[0] * vector1[1]  -  vector0[1] * vector1[0]; // z component

		// implement directly this: mat3x3_vect( inbody[0].TIworInv, auxv )
		auxv2[0] = inv_It_now[0][0] * auxv[0]  +  inv_It_now[0][1] * auxv[1]  +  inv_It_now[0][2] * auxv[2]; // x component
		auxv2[1] = inv_It_now[1][0] * auxv[0]  +  inv_It_now[1][1] * auxv[1]  +  inv_It_now[1][2] * auxv[2]; // y component
		auxv2[2] = inv_It_now[2][0] * auxv[0]  +  inv_It_now[2][1] * auxv[1]  +  inv_It_now[2][2] * auxv[2]; // z component

		UP = -(1.0 + e) * vnorm;
		DOWN = 1.0 / MASS  +  (auxv[0] * auxv2[0] + auxv[1] * auxv2[1] + auxv[2] * auxv2[2]);

		jel = UP / DOWN;
		// "jel" is the right impulse, now appy the impulse and assign final velocity and rotation.

		// update velocity of CM...
		v[0] = v[0]  +  (jel / MASS) * nx;
		v[1] = v[1]  +  (jel / MASS) * ny;
		v[2] = v[2]  +  (jel / MASS) * nz;

		// update rotation... this is the hardest one:
		vector0[0] = rx;
		vector0[1] = ry;
		vector0[2] = rz;

		vector1[0] = jel * nx;
		vector1[1] = jel * ny;
		vector1[2] = jel * nz;

		auxv[0] = vector0[1] * vector1[2]  -  vector0[2] * vector1[1]; // x component
		auxv[1] = vector0[2] * vector1[0]  -  vector0[0] * vector1[2]; // y component
		auxv[2] = vector0[0] * vector1[1]  -  vector0[1] * vector1[0]; // z component

		// implement directly this: mat3x3_vect( inbody[0].TIworInv, auxv )

		auxv2[0] = inv_It_now[0][0] * auxv[0]  +  inv_It_now[0][1] * auxv[1]  +  inv_It_now[0][2] * auxv[2]; // x component
		auxv2[1] = inv_It_now[1][0] * auxv[0]  +  inv_It_now[1][1] * auxv[1]  +  inv_It_now[1][2] * auxv[2]; // y component
		auxv2[2] = inv_It_now[2][0] * auxv[0]  +  inv_It_now[2][1] * auxv[1]  +  inv_It_now[2][2] * auxv[2]; // z component

		w[0] = w[0] + auxv2[0];
		w[1] = w[1] + auxv2[1];
		w[2] = w[2] + auxv2[2];

		// update also this, to be sure you know... .
		L[0] = It_now[0][0] * w[0]  +  It_now[0][1] * w[1]  +  It_now[0][2] * w[2];
		L[1] = It_now[1][0] * w[0]  +  It_now[1][1] * w[1]  +  It_now[1][2] * w[2];
		L[2] = It_now[2][0] * w[0]  +  It_now[2][1] * w[1]  +  It_now[2][2] * w[2];

		printf("COLLISION BEING RESOLVED ....\n\n");
	}

	return jel;
} // end bounceAirplane function

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