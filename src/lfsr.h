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

#define MAXPOWER 24
#define INITSR 0x0
#define TGTWIDTH16 16
#define TGTMASK16 0xFFFF
#define TGTINMASK16 0x8000

#define TGTWIDTH8 8
#define TGTMASK8 0xFF
#define TGTINMASK8 0x80

#define INIT_RX_SR_A 0x1F
#define INIT_RX_SR_B 0xFFF
#define INIT_TX_SR_A 0x3FF
#define INIT_TX_SR_B 0x3F83
#define APOLY 0x600001


int Scramble(uint8_t *, int, int, LFSR_struct *, int, int);

void UnScramble(uint8_t *, int, LFSR_struct *);

void InitLFSR(uint32_t, LFSR_struct *);

void StepLFSR(LFSR_struct *, int16_t );
#endif	/* LFSR_H */
