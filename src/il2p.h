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
#include "intmath.h"
#include "kiss.h"

#define IL2P_GF_POLY 0x11D
#define IL2P_LFSR_POLY 0x211
#define IL2P_LFSR_TX_PRE 0xF
#define IL2P_LFSR_RX_PRE 0x1F0

#define IL2P_SYNCWORD 0xF15E48
#define IL2P_4FSK_SYNCWORD_1 0x55FDDD
#define IL2P_4FSK_SYNCWORD_2 0x57DF7F
#define IL2P_SYNCWORD_LENGTH 3

#define IL2P_MAX_PAYLOAD 1023

// Receiver state machine states
#define IL2P_RX_SEARCH 0
#define IL2P_RX_HEADER 1
#define IL2P_RX_BIGBLOCKS 2
#define IL2P_RX_SMALLBLOCKS 3
#define IL2P_RX_INTERLEAVE 4
#define IL2P_RX_PROCESS 5
#define IL2P_RX_CRC 6


#define IL2P_MAXFEC_RS_BLOCKSIZE 239
#define IL2P_MAXFEC_NUMROOTS 16
#define IL2P_HEADER_BYTES 15
#define IL2P_HEADER_BLOCK_SIZE 15
#define IL2P_HEADER_NUMROOTS 2

typedef union {
    uint32_t ULI;
    uint8_t Byte[4];
} IL2PUnLongType;

typedef struct {
    GF2_def_struct GF;
    IL2PUnLongType Work;
    IL2PUnLongType SyncWord;
    RS2_def_struct RS[2];
    LFSR_struct RXLFSR;
    LFSR_struct TXLFSR;
    int32_t SignalPower;
    int32_t NoisePower;
    uint8_t TXBuffer[1300];
    uint8_t RXBuffer[1300];
    uint16_t RXBufferIndex;
    int16_t RXState;
    int16_t RXErrCount;
    uint16_t TXHdrPID;
    uint16_t TXHdrCtrl;
    uint16_t TXHdrCount;
    uint16_t TXHdrType;
    uint16_t RXHdrType;
    uint16_t RXHdrUI;
    uint16_t RXHdrAX25FrameType;
    uint16_t RXHdrAX25PIDExists;
    uint16_t RXHdrPFBit;
    uint16_t RXHdrCBit;
    uint16_t RXHdrNR;
    uint16_t RXHdrNS;
    uint16_t RXHdrOpcode;
    uint16_t RXHdrPID;
    uint16_t RXDestCall[6];
    uint16_t RXSrcCall[6];
    uint16_t RXDestSSID;
    uint16_t RXSrcSSID;
    uint16_t RXHdrCount;
    uint16_t RXBlocks;
    uint16_t RXBlocksize;
    uint16_t RXBigBlocks;
    uint16_t RXBlockIndex;
    uint16_t RXDecoder;
    uint16_t RXNumRoots;
    uint16_t RXBlockByteCount;
    uint16_t BitIndex;
    uint16_t RxPktCount;
    uint32_t SisterLastChecksum;
    uint32_t MyLastChecksum;
    int16_t Result;
    uint16_t RxByteCount;
    uint16_t HasNewChecksum;
    int16_t InterleaveRowIndex;
    int16_t InterleaveColumnIndex;
    int16_t InterleaveRSBlockSize;
    int16_t InterleaveBlockSize;
    int16_t InterleaveNumroots;
    int16_t MaxPayload;
    int16_t InterleaveInterval;
    int16_t CRCIndex;
    int16_t SyncTolerance;
    uint16_t TransmitCRC;
    uint16_t ReceiveCRC;
    uint16_t Synced: 1;
    uint16_t TransparentMode :1;
    uint16_t MaxFEC :1;
    uint16_t Duplicate :1;
    uint16_t Interleave :1;
    uint16_t InterleaveBlockFull :1;
    uint16_t TrailingCRC :1;
    uint16_t FSK4Syncword :1;
    uint16_t InvertRXData :1;
} IL2P_TRX_struct;

void InitIL2P(IL2P_TRX_struct *);
int IL2PBuildPacket(KISS_struct *, uint8_t *, IL2P_TRX_struct *);
void IL2PReceive(IL2P_TRX_struct *, uint8_t *, int, uint8_t *);
void IL2PConvertToAX25(IL2P_TRX_struct *);

#endif	/* IL2P_H */

