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
    uint32_t ShiftRegister;
    uint32_t Polynomial;
    uint32_t FeedbackMask;
    uint32_t InputMask;
    uint32_t OutputMask;
    uint32_t Order;
    int16_t Tap[MAXTAPS];
    int16_t TapCount;
    int16_t BitDelay;
    int16_t BitsInProgress;
    uint16_t Initialized :1;
    uint16_t Invert :1;
} LFSR_struct;

#endif	/* LFSR_STRUCT_H */

