#include "crc.h"

uint16_t CCITT16CalcCRC_uint16(uint16_t* data, int16_t count) {
    int16_t i, j;
    uint16_t x;
    uint16_t fcsval = 0xFFFF;
    unsigned fcsbit;
    for (i = 0; i < count; i++) {
        x = data[i];
        for (j = 0; j < 8; j++) {
            fcsbit = 0;
            if (fcsval & 1) {
                fcsbit = 1;
            }
            fcsval >>= 1;            
            if (fcsbit ^ (x & 1)) {
                fcsval ^= CCITT16_CRC_POLY;
            }
            x >>= 1;
        }
    }
    return ~fcsval;
}


uint16_t CCITT16CalcCRC(uint8_t* data, int16_t count) {
    int16_t i, j;
    uint16_t x;
    uint16_t fcsval = 0xFFFF;
    unsigned fcsbit;
    for (i = 0; i < count; i++) {
        x = data[i];
        for (j = 0; j < 8; j++) {
            fcsbit = 0;
            if (fcsval & 1) {
                fcsbit = 1;
            }
            fcsval >>= 1;            
            if (fcsbit ^ (x & 1)) {
                fcsval ^= CCITT16_CRC_POLY;
            }
            x >>= 1;
        }
    }
    return ~fcsval;
}

int16_t CCITT16CheckCRC(uint8_t* data, int16_t count) {
    int16_t result;
    uint16_t x, y;
    uint16_t FCS = CCITT16CalcCRC(data, count);
    x = data[count];
    y = data[count + 1] & 0xFF;
    x |= y << 8;
    if (FCS == x) {
        result = 1;
    } else {
        result = 0;
    }
    return result;
}

uint32_t CCITT32CalcCRC(uint8_t* data, int16_t count) {
    int16_t i, j;
    uint32_t x;
    uint32_t fcsval = 0xFFFFFFFF;
    unsigned fcsbit;
    for (i = 0; i < count; i++) {
        x = data[i];
        for (j = 0; j < 8; j++) {
            fcsbit = 0;
            if (fcsval & 1) {
                fcsbit = 1;
            }
            fcsval >>= 1;            
            if (fcsbit ^ (x & 1)) {
                fcsval ^= CCITT32_CRC_POLY;
            }
            x >>= 1;
        }
    }
    return ~fcsval;
}

int16_t CCITT32CheckCRC(uint8_t* data, int16_t count) {
    int16_t result;
    uint32_t x, y;
    uint32_t FCS = CCITT32CalcCRC(data, count);
    x = data[count];
    y = data[count + 1] & 0xFF;
    x |= y << 8;
    y = data[count + 2] & 0xFF;
    x |= y << 16;
    y = data[count + 3] & 0xFF;
    x |= y << 24;
    if (FCS == x) {
        result = 1;
    } else {
        result = 0;
    }
    return result;
}

uint32_t CCITT32CalcCRCInt(uint16_t* data, int16_t count) {
    int16_t i, j;
    uint32_t x;
    uint32_t fcsval = 0xFFFFFFFF;
    unsigned fcsbit;
    for (i = 0; i < count; i++) {
        x = data[i];
        for (j = 0; j < 16; j++) {
            fcsbit = 0;
            if (fcsval & 1) {
                fcsbit = 1;
            }
            fcsval >>= 1;            
            if (fcsbit ^ (x & 1)) {
                fcsval ^= CCITT32_CRC_POLY;
            }
            x >>= 1;
        }
    }
    return ~fcsval;    
}