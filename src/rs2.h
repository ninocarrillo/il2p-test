/* 
 * File:   rs2.h
 * Author: nino
 *
 * Created on May 17, 2017, 7:34 AM
 */

#ifndef RS2_H
#define	RS2_H

#include "gf2.h"

// Change to suit your application. Bigger = more memory allocated.
#define MAX_GENPOLY_ROOTS 16

typedef struct {
    GF2_def_struct *GF;
    GF2_type Genpoly[MAX_GENPOLY_ROOTS + 1];
	GF2_type SavedSyndromes[MAX_GENPOLY_ROOTS];
	GF2_type Syndromes[MAX_GENPOLY_ROOTS];
    GF2_type ErrorIndices[MAX_GENPOLY_ROOTS];
	GF2_type ErrorMagPoly[MAX_GENPOLY_ROOTS];
    GF2_type ErrorMags[MAX_GENPOLY_ROOTS];
	GF2_type ErrorLocatorPoly[MAX_GENPOLY_ROOTS];
	GF2_type ErrorLocatorRoots[MAX_GENPOLY_ROOTS];
	GF2_type *DataBlock;
    GF2_type FirstRoot;
    int NumRoots;
    int FieldOrder;
	int BlockSize;
    int ErrorCount;
} RS2_def_struct;

// InitRS2
// Initializes Reed Solomon parameters in memory for library functions.
// Arg1: starting root for generator polynomial
// Arg2: number of roots in generator polynomial
void InitRS2(GF2_type, int, RS2_def_struct*);

// RSEncode
// Computes Reed Solomon parity symbols for input array and appends them to the
// end of the original array.
// Arg1: pointer to first word of input array
// Arg2: word count of input array
void RSEncode(GF2_type*, int, RS2_def_struct*);

// RSDecode
// Computes error locations and values based on Reed Solomon parity symbols
// appended to end of input array. Corrects input array if possible.
// Arg1: pointer to first word of input array
// Arg2: word count of input array
// Returns number of errors corrected. Returns negative if correction failed.
// Clobbers input array.
int RSDecode(GF2_type*, int, RS2_def_struct*);



#endif	/* RS2_H */

