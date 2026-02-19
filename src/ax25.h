/* 
 * File:   ax25.h
 * Author: nino
 *
 * Created on June 24, 2017, 5:38 PM
 */

#ifndef AX25_H
#define	AX25_H

#include "stdint.h"
#include "ax25_receiver_struct.h"

#define AX25_WORKING_WORD_TYPE uint32_t
#define AX25_WORKING_WORD_WIDTH 32
#define AX25_WORKING_WORD_MASK 0x80000000

#define G3RUH_LFSR_POLY 0x21001
#define SHORT_LFSR_POLY 0x211
#define DIF_POLY 0x3
#define AX25_LFSR_PRELOAD 0x1FFE // with differential encoder


#define AX25_START_FLAG 0x7E // Must be 32 bits or less
#define AX25_START_FLAG_BITS 8 // 32 or less
#define AX25_END_FLAG 0x7E
#define AX25_END_FLAG_BITS 8
#define MIN_AX25_FRAME_LENGTH 10 // bytes
#define BPQ_CHECKSUM_START 0x0

void InitAX25(AX25_Receiver_struct*);
int AX25BuildFrame(uint8_t*, int, uint8_t*, int);
// void AX25ReceivetoKISS(AX25_Receiver_struct*, UInt16_Buffer_struct*, UART_struct*);

#endif	/* AX25_H */

