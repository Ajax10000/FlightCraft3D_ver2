#ifndef FC_MATH_H
#define FC_MATH_H

// multTwo3x3Matrices multiplies two 3x3 matrices, placing the product in global variable gloResultMatrix
void multTwo3x3Matrices(double mat1[3][3], double mat2[3][3]);

// invert3x3Matrix calculates the inverse of a 3x3 matrix 
// Used for obtaining the inverse of the inertia tensor
void invert3x3Matrix(double in_3x3_matrix[3][3]);
#endif