#include <stdio.h>           // for getchar(), printf()

#include "globals.h"         // for the global variables
#include "math.h"            // for our prototypes

// ####################################################################################################################
// Function multTwo3x3Matrices multiplies two 3x3 matrices and places the result in global variable gloResultMatrix.
// ####################################################################################################################
void multTwo3x3Matrices(double mat1[3][3], double mat2[3][3])
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
			gloResultMatrix[im][jm] = sum; 
		}
	}
} // end multTwo3x3Matrices function


// ####################################################################################################################
// Function invert3x3Matrix calculates the inverse of a 3x3 matrix (used for obtaining the inverse of the inertia tensor).
// ####################################################################################################################
void invert3x3Matrix(double in_3x3_matrix[3][3])
{
	double A[3][3]; // the matrix that is entered by user 
	double B[3][3]; // the transpose of a matrix A 
	double C[3][3]; // the adjunct matrix of transpose of a matrix A, not adjunct of A
	int i, j;
	double x, n = 0; // n is the determinant of A

	// Set A to the matrix passed in.
	// Set B and C to the zero matrix.
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

	// Set B to the transpose of matrix A.
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			B[i][j] = A[j][i];
		}
	}

	C[0][0] =         B[1][1] * B[2][2] - (B[2][1] * B[1][2]);
	C[0][1] = (-1) * (B[1][0] * B[2][2] - (B[2][0] * B[1][2]));
	C[0][2] =         B[1][0] * B[2][1] - (B[2][0] * B[1][1]);

	C[1][0] = (-1) * (B[0][1] * B[2][2] - B[2][1] * B[0][2]);
	C[1][1] =         B[0][0] * B[2][2] - B[2][0] * B[0][2];
	C[1][2] = (-1) * (B[0][0] * B[2][1] - B[2][0] * B[0][1]);

	C[2][0] =         B[0][1] * B[1][2] - B[1][1] * B[0][2];
	C[2][1] = (-1) * (B[0][0] * B[1][2] - B[1][0] * B[0][2]);
	C[2][2] =         B[0][0] * B[1][1] - B[1][0] * B[0][1];

	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			gloResultMatrix[i][j] = C[i][j] * x; 
		}
	}
} // end invert3x3Matrix function