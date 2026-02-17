#include "il2p.h"
#include "rs2.h"
#include "crc.h"

void InitIL2P(IL2P_TRX_struct *self){
    // Save memory by sharing the Galois Field structure between two Reed Solomon structures.
    InitGF2(IL2P_GF_POLY, &self->GF);
    self->RS[0].GF = &self->GF;
    self->RS[1].GF = &self->GF;

    InitRS2(0, 2, &self->RS[0]); // 2 roots for header encoder
    InitRS2(0, 16, &self->RS[1]); // 16 roots for payload encoder

    InitLFSR(IL2P_LFSR_POLY, &self->RXLFSR);
    self->RXLFSR.Invert = 0;
    InitLFSR(IL2P_LFSR_POLY, &self->TXLFSR);
    self->TXLFSR.Invert = 0;

    self->RXState = IL2P_RX_SEARCH;
}
