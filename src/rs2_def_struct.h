/* 
 * File:   rs2_def_struct.h
 * Author: nino
 *
 * Created on November 11, 2017, 7:54 AM
 */



#ifndef RS2_DEF_STRUCT_H
#define	RS2_DEF_STRUCT_H

#include "gf2_def_struct.h"

// Change to suit your application. Bigger = more memory allocated.
#define MAX_GENPOLY_ROOTS 256

typedef struct {
    GF2_def_struct *GF;
    int Genpoly[MAX_GENPOLY_ROOTS + 1];
	int SavedSyndromes[MAX_GENPOLY_ROOTS];
	int Syndromes[MAX_GENPOLY_ROOTS];
    int ErrorIndices[MAX_GENPOLY_ROOTS];
	int ErrorMagPoly[MAX_GENPOLY_ROOTS];
    int ErrorMags[MAX_GENPOLY_ROOTS];
	int ErrorLocatorPoly[MAX_GENPOLY_ROOTS];
	int ErrorLocatorRoots[MAX_GENPOLY_ROOTS];
	int *DataBlock;
    int FirstRoot;
    int NumRoots;
    int FieldOrder;
	int BlockSize;
    int ErrorCount;
} RS2_def_struct;

#endif	/* RS2_DEF_STRUCT_H */

