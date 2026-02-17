/* 
 * File:   CRC.h
 * Author: nino
 *
 * Created on November 18, 2017, 8:22 AM
 */

#ifndef CRC_H
#define	CRC_H

#include "stdint.h"

#define CCITT16_CRC_POLY 0x8408 // bit-reversed 0x11021, MSB testing
#define CCITT32_CRC_POLY 0xEDB88320 // bit-reversed 0x104C11DB6, MSB testing

#define CRC_CCITT_16 0
#define CRC_CCITT_32 1

uint16_t CCITT16CalcCRC(uint8_t*, int16_t);
uint16_t CCITT16CalcCRC_uint16(uint16_t*, int16_t);
int16_t CCITT16CheckCRC(uint8_t*, int16_t);
uint32_t CCITT32CalcCRC(uint8_t*, int16_t);
uint32_t CCITT32CalcCRCInt(uint16_t*, int16_t);
int16_t CCITT32CheckCRC(uint8_t*, int16_t);

#endif	/* CRC_H */

