/*
 * File:   il2p.h
 * Author: nino
 *
 * Created on November 11, 2019, 9:16 PM
 */

#ifndef IL2P_H
#define	IL2P_H

#include "stdint.h"
#include "rs2_def_struct.h"
#include "gf2_def_struct.h"
#include "lfsr_struct.h"
#include "lfsr.h"
#include "gf2.h"

#define IL2P_GF_POLY 0x11D
#define IL2P_LFSR_POLY 0x211
#define IL2P_LFSR_TX_PRE 0xF
#define IL2P_LFSR_RX_PRE 0x1F0

// Receiver state machine states
#define IL2P_RX_SEARCH 0
#define IL2P_RX_HEADER 1
#define IL2P_RX_BIGBLOCKS 2
#define IL2P_RX_SMALLBLOCKS 3
#define IL2P_RX_INTERLEAVE 4
#define IL2P_RX_PROCESS 5
#define IL2P_RX_CRC 6

typedef union {
    uint32_t ULI;
    uint8_t Byte[4];
} IL2PUnLongType;

typedef struct {
    IL2PUnLongType Work;
    GF2_def_struct GF;
    RS2_def_struct RS[2];
    LFSR_struct RXLFSR;
    LFSR_struct TXLFSR;
    uint8_t TXBuffer[1121];
    uint8_t RXBuffer[1121];
    int RXState;
} IL2P_TRX_struct;

void InitIL2P(IL2P_TRX_struct *);

#endif	/* IL2P_H */

