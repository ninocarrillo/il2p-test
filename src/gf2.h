/*
 * File:   GF2.h
 * Author: nino
 *
 * Created on May 14, 2017, 6:00pm
 */

#ifndef GF2_H
#define	GF2_H

#include "gf2_def_struct.h"

// InitGF2
// Initializes Galois Field tables in memory to support library functions.
// Arg1: Power of base-2 field, ie 2^Power. 2^8 for 8-bit field.
// Arg2: Generator (reducing) polynomial, in binary. 285 = x^8+x^4+x^3+x^2+1
// Returns 0 for maximal field, number of repetition cycles otherwise.
int InitGF2(int, GF2_def_struct*);

// GF2GetOrder
// Returns order (size) of specified field.
int GF2GetOrder(GF2_def_struct*);

// GF2Mul
// Performs Galois Field multiplication by addition of exponents.
// Arg1: multiplicand
// Arg2: multiplier
// Returns product.
int GF2Mul(int, int, GF2_def_struct*);

// GF2Div
// Performs Galois Field division by subtraction of exponents.
// Arg1: dividend
// Arg2: divisor
// Returns quotient.
int GF2Div(int, int, GF2_def_struct*);

// GF2Conv
// Convolves two Galois Field polynomials.
// Arg1: pointer to least significant coefficient of polynomial 1
// Arg2: number of coefficients in polynomial 1
// Arg3: pointer to least significant coefficient of polynomial 2
// Arg4: number of coefficients in polynomial 2
// Returns convolved polynomial of length (p1n + p2n - 1).
int GF2Conv(int*, int, int*, int, GF2_def_struct*);

// GF2Pow
// Returns field primitive (2) raised to Arg.
int GF2Pow(int, GF2_def_struct*);

// GF2Log
// Returns log base <field primitive> (2) of Arg.
int GF2Log(int, GF2_def_struct*);

// GF2Inv
// Returns 1/Arg.
int GF2Inv(int, GF2_def_struct*);

int GF2Mod(int, GF2_def_struct*);
int GF2Clamp(int, GF2_def_struct*);

#endif	/* GF2_H */
