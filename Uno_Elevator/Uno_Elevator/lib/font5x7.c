/*
 * font5x7.c
 *
 * Created: 4/12/2026 5:45:03 AM
 *  Author: Hamidur
 */ 

#include "font5x7.h"

const uint8_t FONT_SPACE[7] = {0, 0, 0, 0, 0, 0, 0};

// Digits
const uint8_t FONT_0[7] = {14, 17, 19, 21, 25, 17, 14};
const uint8_t FONT_1[7] = {4, 12, 4, 4, 4, 4, 14};
const uint8_t FONT_2[7] = {14, 17, 1, 2, 4, 8, 31};
const uint8_t FONT_3[7] = {30, 1, 1, 14, 1, 1, 30};
const uint8_t FONT_4[7] = {2, 6, 10, 18, 31, 2, 2};
const uint8_t FONT_5[7] = {31, 16, 16, 30, 1, 1, 30};
const uint8_t FONT_6[7] = {14, 16, 16, 30, 17, 17, 14};
const uint8_t FONT_7[7] = {31, 1, 2, 4, 8, 8, 8};
const uint8_t FONT_8[7] = {14, 17, 17, 14, 17, 17, 14};
const uint8_t FONT_9[7] = {14, 17, 17, 15, 1, 1, 14};

// Letters
const uint8_t FONT_A[7] = {14, 17, 17, 31, 17, 17, 17};
const uint8_t FONT_B[7] = {30, 17, 17, 30, 17, 17, 30};
const uint8_t FONT_C[7] = {14, 17, 16, 16, 16, 17, 14};
const uint8_t FONT_D[7] = {30, 17, 17, 17, 17, 17, 30};
const uint8_t FONT_E[7] = {31, 16, 16, 30, 16, 16, 31};
const uint8_t FONT_F[7] = {31, 16, 16, 30, 16, 16, 16};
const uint8_t FONT_G[7] = {14, 17, 16, 16, 19, 17, 14};
const uint8_t FONT_H[7] = {17, 17, 17, 31, 17, 17, 17};
const uint8_t FONT_I[7] = {14, 4, 4, 4, 4, 4, 14};
const uint8_t FONT_J[7] = {1, 1, 1, 1, 17, 17, 14};
const uint8_t FONT_K[7] = {17, 18, 20, 24, 20, 18, 17};
const uint8_t FONT_L[7] = {16, 16, 16, 16, 16, 16, 31};
const uint8_t FONT_M[7] = {17, 27, 21, 21, 17, 17, 17};
const uint8_t FONT_N[7] = {17, 17, 25, 21, 19, 17, 17};
const uint8_t FONT_O[7] = {14, 17, 17, 17, 17, 17, 14};
const uint8_t FONT_P[7] = {30, 17, 17, 30, 16, 16, 16};
const uint8_t FONT_Q[7] = {14, 17, 17, 17, 21, 18, 13};
const uint8_t FONT_R[7] = {30, 17, 17, 30, 20, 18, 17};
const uint8_t FONT_S[7] = {15, 16, 16, 14, 1, 1, 30};
const uint8_t FONT_T[7] = {31, 4, 4, 4, 4, 4, 4};
const uint8_t FONT_U[7] = {17, 17, 17, 17, 17, 17, 14};
const uint8_t FONT_V[7] = {17, 17, 17, 17, 17, 10, 4};
const uint8_t FONT_W[7] = {17, 17, 17, 21, 21, 21, 10};
const uint8_t FONT_X[7] = {17, 17, 10, 4, 10, 17, 17};
const uint8_t FONT_Y[7] = {17, 17, 10, 4, 4, 4, 4};
const uint8_t FONT_Z[7] = {31, 1, 2, 4, 8, 16, 31};

const uint8_t* font5x7_get_char(char c)
{
	switch (c)
	{
		case 'A': return FONT_A;
		case 'B': return FONT_B;
		case 'C': return FONT_C;
		case 'D': return FONT_D;
		case 'E': return FONT_E;
		case 'F': return FONT_F;
		case 'G': return FONT_G;
		case 'H': return FONT_H;
		case 'I': return FONT_I;
		case 'J': return FONT_J;
		case 'K': return FONT_K;
		case 'L': return FONT_L;
		case 'M': return FONT_M;
		case 'N': return FONT_N;
		case 'O': return FONT_O;
		case 'P': return FONT_P;
		case 'Q': return FONT_Q;
		case 'R': return FONT_R;
		case 'S': return FONT_S;
		case 'T': return FONT_T;
		case 'U': return FONT_U;
		case 'V': return FONT_V;
		case 'W': return FONT_W;
		case 'X': return FONT_X;
		case 'Y': return FONT_Y;
		case 'Z': return FONT_Z;

		case '0': return FONT_0;
		case '1': return FONT_1;
		case '2': return FONT_2;
		case '3': return FONT_3;
		case '4': return FONT_4;
		case '5': return FONT_5;
		case '6': return FONT_6;
		case '7': return FONT_7;
		case '8': return FONT_8;
		case '9': return FONT_9;

		case ' ': return FONT_SPACE;
		default:  return FONT_SPACE;
	}
}