/* 
 * File:   ax25_receiver_struct.h
 * Author: nino
 *
 * Created on October 15, 2017, 3:40 PM
 */

#ifndef AX25_RECEIVER_STRUCT_H
#define	AX25_RECEIVER_STRUCT_H

#include "stdint.h"

#define AX25_MAX_RX_BUFFER 1500
#define AX25_OUTPUT_WORD_WIDTH 8
#define AX25_INPUT_WORD_WIDTH 8

typedef struct {
    uint8_t LastHeardDestCallsign[7];
    long int SisterCRC;
    int RxPktCount;
    int UniqueCount;
    int BitIndex;
    int WordIndex;
    int OneCounter;
    long int CRC;
    int RxByteCount;
    uint8_t WorkingWord8;
    uint8_t Buffer[AX25_MAX_RX_BUFFER];
    unsigned CRCOK :1;
    unsigned CRCBad :1;
    unsigned Duplicate :1;
    unsigned NewSisterCRC :1;
} AX25_Receiver_struct;

#endif	/* AX25_RECEIVER_STRUCT_H */

