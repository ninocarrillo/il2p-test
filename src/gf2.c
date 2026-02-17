#include "gf2.h"

void lfsr_step(GF2_def_struct *gf) {
// utilize Galois configuration to implement LFSR
   int feedback = gf->LFSR & gf->GenPoly; // save feedback
   gf->LFSR >>= 1; // shift memory register right one bit
   if (feedback & 1) { // if the feedback bit is one
      gf->LFSR ^= (gf->GenPoly >> 1); // then XOR the tapped bits
   }
   return;
}

int GF2GetOrder(GF2_def_struct *gf) {
    return gf->Order;
}

int GF2Pow(int i, GF2_def_struct *gf) {
    return gf->Table[i & gf->Mask];
}

int GF2Log(int i, GF2_def_struct *gf) {
    return gf->Index[i & gf->Mask];
}

int GF2Inv(int i, GF2_def_struct *gf) {
    return gf->Inverse[i & gf->Mask];
}

int GF2Mod(int i, GF2_def_struct *gf) {
	while (i < 0) {
		i += (gf->Order - 1);
	}
	while (i > (gf->Order - 2)) {
		i -= (gf->Order - 1);
	}
	return i;
}

int GF2Clamp(int i, GF2_def_struct *gf) {
	while (i > (gf->Order - 2)) {
		i -= (gf->Order - 1);
	}
	return i;
}

int InitGF2(int genpoly, GF2_def_struct *gf) {
	if (!(genpoly & 1)) {
		// Generator polynomial must be odd.
		return(-1);
	}
	gf->GenPoly = genpoly;
	gf->Order = 1;
	gf->Power = 0;
	while ((gf->Order<<1) < gf->GenPoly) {
		gf->Order<<= 1;
		gf->Power++;
	}
	gf->Mask = gf->Order - 1;
    // generate the field table and index
    int status = 0;
    gf->LFSR = 1; // start with GF element a^0
    for (int i = gf->Order - 2; i >= 0; i--) {
        lfsr_step(gf);
        gf->Table[i & gf->Mask] = gf->LFSR;
        gf->Index[gf->LFSR & gf->Mask] = i;
        if ((gf->LFSR == 1) && (i > 0)) {
            status++; // number of times sequence repeated during generation
        }
    }
    if (status == 0) {
	    gf->Index[0] = 0;
	    // generate the inverse table
	    gf->Inverse[0] = 0;
	    for (int i = 1; i < gf->Order; i++) {
	        int j = 1;
	        while (GF2Mul(i, j, gf) != 1) {
	            j++;
	        }
	        gf->Inverse[i] = j;
	    }
	}
    return status;
}

int GF2Mul(int a, int b, GF2_def_struct *gf) {
	if ((a == 0) | (b == 0)) {
		return 0;
	}
	a = gf->Index[a & gf->Mask];
	b = gf->Index[b & gf->Mask];
	a = GF2Mod(a + b, gf);
	return gf->Table[a & gf->Mask];
}

int GF2Div(int a_arg, int b_arg, GF2_def_struct *gf) {
	int a = a_arg;
	int b = b_arg;
	if (b == 0) {
		return -1;
	}
	if (a == 0) {
		return 0;
	}
	a = gf->Index[a & gf->Mask];
	b = gf->Index[b & gf->Mask];
	a = GF2Mod(a - b, gf);
	return gf->Table[a & gf->Mask];
}

int GF2Conv(int *p1, int p1n, int *p2, int p2n, GF2_def_struct *gf) {
// convolves two gf polynomials
// p1 points to polynomial1 containing p1n elements
// p2 points to polynomial2 containing p2n elements
// p1 and p2 are stored with lowest power at lowest address
// returns p1n + p2n - 1
// places result in p1
// p1 must have length p1n + p2n - 1
	int k = p1n + p2n - 1;
	int pr[k];
	for (int i = 0; i < k; i++) {
		pr[i] = 0;
	}
	for (int i = 0; i < p1n; i++) {
		for (int j = 0; j < p2n; j++) {
			pr[i + j] = pr[i + j] ^ GF2Mul(p1[i], p2[j], gf);
		}
	}
	for (int i = 0; i < k; i++) {
		p1[i] = pr[i];
	}
	return k;
}
