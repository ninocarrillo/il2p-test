/* 
 * File:   LFSR.h
 * 
 * Author: nino
 *
 * Created on March 27, 2017, 9:56 PM
 */
#ifndef LFSR_H
#define	LFSR_H

#include "stdint.h"
#include "lfsr_struct.h"

#define LFSR_MAXPOWER 24

#define TGTWIDTH8 8
#define TGTMASK8 0xFF
#define TGTINMASK8 0x80

int Scramble(uint8_t *, int, int, LFSR_struct *, int, int);

void UnScramble(uint8_t *, int, LFSR_struct *);

void InitLFSR(long int, LFSR_struct *);

#endif	/* LFSR_H */
