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

typedef struct {
    uint8_t Buffer[AX25_MAX_RX_BUFFER];
    uint8_t WorkingWord8;
    long int ExcludeChecksum;
    int Result;
    int RxPktCount;
    int UniqueCount;
    int BitIndex;
    int WordIndex;
    int OneCounter;
    long int CRC;
} AX25_Receiver_struct;

#endif	/* AX25_RECEIVER_STRUCT_H */

