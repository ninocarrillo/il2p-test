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

#define AX25_TX_WORKING_WORD_TYPE uint32_t
#define AX25_TX_WORKING_WORD_WIDTH 32
#define AX25_TX_WORKING_WORD_MASK 0x80000000
#define AX25_TX_OUTPUT_WORD_WIDTH 8

#define AX25_RX_INPUT_WORD_WIDTH 8
#define AX25_RX_INPUT_WORD_MASK 0x80

#define AX25_START_FLAG 0x7E // Must be 32 bits or less
#define AX25_START_FLAG_BITS 8 // 32 or less
#define AX25_END_FLAG 0x7E
#define AX25_END_FLAG_BITS 8
#define MIN_AX25_FRAME_LENGTH 10 // bytes

void InitAX25(AX25_Receiver_struct*);
int AX25BuildFrame(uint8_t*, int, uint8_t*, int);
int AX25Receive(AX25_Receiver_struct*, uint8_t*, int, uint8_t*);

#endif	/* AX25_H */

