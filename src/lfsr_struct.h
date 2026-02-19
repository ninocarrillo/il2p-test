/* 
 * File:   lfsr_struct.h
 * Author: nino
 *
 * Created on November 22, 2017, 5:58 PM
 */

#ifndef LFSR_STRUCT_H
#define	LFSR_STRUCT_H

#include "stdint.h"

#define MAXTAPS 16

typedef struct {
    long int ShiftRegister;
    long int Polynomial;
    long int FeedbackMask;
    long int InputMask;
    long int OutputMask;
    int Order;
    int Tap[MAXTAPS];
    int TapCount;
    int BitDelay;
    int BitsInProgress;
    unsigned Initialized :1;
    unsigned Invert :1;
} LFSR_struct;

#endif	/* LFSR_STRUCT_H */

