/* 
 * File:   kiss.h
 * Author: nino
 *
 * Created on June 24, 2017, 8:53 AM
 */

#ifndef KISS_H
#define	KISS_H

#include "stdint.h"

#define AX25_I 0x00
#define AX25_S 0x01
#define AX25_U 0x03

#define AX25_UCTL_SABME 0b01101111
#define AX25_UCTL_SABM  0b00101111
#define AX25_UCTL_DISC  0b01000011
#define AX25_UCTL_DM    0b00001111
#define AX25_UCTL_UA    0b01100011
#define AX25_UCTL_FRMR  0b10000111
#define AX25_UCTL_UI    0b00000011
#define AX25_UCTL_XID   0b10101111
#define AX25_UCTL_TEST  0b11100011
#define AX25_UCTL_NONUI 0xFC
#define AX25_SABME 0x6F

typedef struct {
    int16_t UART_byte;
    int16_t OutputCount;
    uint8_t Output[1500];
    uint8_t AX25SourceCall[6];
    uint8_t AX25DestCall[6];
    uint8_t AX25DestSSID;
    uint8_t AX25SourceSSID;
    uint8_t AX25ControlByte;
    uint8_t AX25UControlFieldType;
    uint8_t AX25FrameType;
    uint8_t AX25PIDByte;
    uint8_t AX25PFBit;
    uint8_t AX25CBit;
    uint8_t AX25_NR;
    uint8_t AX25_NS;
    uint8_t Data;
    uint8_t Port;
    uint8_t FrameCommand;
    uint16_t Escaped :1;
    uint16_t FirstByte :1;
    uint16_t FrameComplete :1;
    uint16_t Segment :1;
    uint16_t Error :1;
    uint16_t RipValid :1;
    uint16_t AX25PIDByteExists :1;
    uint16_t AX25Callsign7Bit :1;
    uint16_t AX25ExtendedMode :1;
    uint16_t SendAsAX25 :1;
    uint16_t DataForTransmit :1;
} KISS_struct;

#endif	/* KISS_H */

