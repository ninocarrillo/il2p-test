#include "il2p.h"
#include "rs2.h"
#include "crc.h"
#include "kiss.h"
#include "stdio.h"

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
    self->TransparentMode = 0;
    self->FSK4Syncword = 0;
}


// Hamming(7,4) Encoding Table
// Enter this table with the 4-bit value to be encoded.
// Returns 7-bit encoded value, with high bit zero'd.
uint8_t Hamming74EncodeTable[16] = {  \
      0x0, \
      0x71, \
      0x62, \
      0x13, \
      0x54, \
      0x25, \
      0x36, \
      0x47, \
      0x38, \
      0x49, \
      0x5a, \
      0x2b, \
      0x6c, \
      0x1d, \
      0xe, \
      0x7f };


uint16_t UCTLtoIL2P(uint16_t AX25CTL) {
    // Returns IL2P CTL given AX25 CTL. Returns 0xFF if no match. Returns 0x08 if SABME.
    uint16_t IL2PCTL;
    AX25CTL &= 0xEF; // mask P/F bit
    switch (AX25CTL) {
    case 0x6F: // SABME
        IL2PCTL = 0x08;
        break;
    case 0x2F: // SABM
        IL2PCTL = 0x00;
        break;
    case 0x43: // DISC
        IL2PCTL = 0x01;
        break;
    case 0x0F: // DM
        IL2PCTL = 0x02;
        break;
    case 0x63: // UA
        IL2PCTL = 0x03;
        break;
    case 0x87: // FRMR
        IL2PCTL = 0x04;
        break;
    case 0x03: // UI
        IL2PCTL = 0x05;
        break;
    case 0xAF: // XID
        IL2PCTL = 0x06;
        break;
    case 0xE3: // TEST
        IL2PCTL = 0x07;
        break;
    default:
        IL2PCTL = 0xFF;
        break;
    }
    return IL2PCTL;
}

uint16_t PIDtoIL2P(uint16_t AX25PID) {
    // Returns IL2P PID given AX25 PID. Returns 0 if no match.
    uint16_t IL2PPID;
    switch (AX25PID) {
    case 0x10:
        IL2PPID = 0x02;
        break;
    case 0x01:
        IL2PPID = 0x03;
        break;
    case 0x06:
        IL2PPID = 0x04;
        break;
    case 0x07:
        IL2PPID = 0x05;
        break;
    case 0x08:
        IL2PPID = 0x06;
        break;
    case 0xC3:
        IL2PPID = 0x07;
        break;
    case 0xC4:
        IL2PPID = 0x08;
        break;
    case 0xCA:
        IL2PPID = 0x09;
        break;
    case 0xCB:
        IL2PPID = 0x0A;
        break;
    case 0xCC:
        IL2PPID = 0x0B;
        break;
    case 0xCD:
        IL2PPID = 0x0C;
        break;
    case 0xCE:
        IL2PPID = 0x0D;
        break;
    case 0xCF:
        IL2PPID = 0x0E;
        break;
    case 0xF0:
        IL2PPID = 0x0F;
        break;
    default:
        // Indicate this PID is not encodable, this will trigger transparent mode.
        IL2PPID = 0x0;
        break;
    }
    return IL2PPID;
}


int16_t IL2PHeaderInstallSync(uint8_t *output) {
    uint32_t x;
    int16_t i;
    x = IL2P_SYNCWORD;
    for (int i = 0; i < IL2P_SYNCWORD_LENGTH; i++) {
        output[(IL2P_SYNCWORD_LENGTH - 1) - i] = x & 0xFF;
        x >>= 8;
    }
    return 3;
}
int16_t IL2PHeaderInstallSpecialSync(uint8_t *output) {
    uint32_t x;
    int16_t i;
    x = 0x77775D;
    //x = 0x55FDDD;
    for (int i = 0; i < 3; i++) {
        output[2 - i] = x & 0xFF;
        x >>= 8;
    }
    x = 0x57DF7F;
    for (int i = 0; i < 3; i++) {
        output[5 - i] = x & 0xFF;
        x >>= 8;
    }
    return 6;
}

void IL2PHeaderInstallCount(uint8_t *output, uint16_t count) {
    int16_t i;
    for (int i = 0; i < 10; i++) {
        if (count & 0x200) {
            output[i] |= 0x80;
        } else {
            output[i] &= 0x7F;
        }
        count <<= 1;
    }
}

uint16_t IL2PHeaderGetCount(uint8_t *output) {
    uint16_t count = 0;
    uint16_t tap = 0x200;
    int16_t i;
    for (int i = 0; i < 10; i++) {
        if (output[i] & 0x80) {
            count += tap;
        }
        tap >>= 1;
    }
    return count;
}

void IL2PHeaderInstallCallsign(uint8_t *output, uint8_t *callsign) {
    for (int i = 0; i < 6; i++) {
        output[i] &= 0xC0;
        output[i] |= (callsign[i] - 0x20) & 0x3F; // Convert callsign to SIXBIT
    }
}

void IL2PHeaderGetCallsign(uint8_t *output, uint16_t *callsign) {
    int16_t i;
    for (int i = 0; i < 6; i++) {
        callsign[i] = (output[i] & 0x3F) + 0x20; // Convert callsign from SIXBIT
    }
}

void IL2PHeaderInstallSSID(uint8_t *output, uint16_t dest_ssid, uint16_t src_ssid) {
    output[0] = 0;
    output[0] |= src_ssid & 0xF;
    output[0] |= (dest_ssid & 0xF) << 4;
}

uint16_t IL2PHeaderGetDestSSID(uint8_t output_byte) {
    uint16_t dest_ssid;
    dest_ssid = (output_byte >> 4) & 0xF;
    return dest_ssid;
}

uint16_t IL2PHeaderGetSrcSSID(uint8_t output_byte) {
    uint16_t src_ssid;
    src_ssid = output_byte & 0xF;
    return src_ssid;
}

void IL2PHeaderInstallType(uint8_t *output, uint8_t type) {
    output[0] &= 0x7F;
    output[1] &= 0x7F;
    if (type & 1) {
        output[1] |= 0x80;
    }
}

uint16_t IL2PHeaderGetType(uint8_t *output) {
    uint16_t type = 0;
    type |= (output[0] & 0x80) >> 6;
    type |= (output[1] & 0x80) >> 7;
    return type;
}

uint16_t IL2PHeaderGetPID(uint8_t *output) {
    uint16_t pid = 0;
    pid |= (output[0] & 0x40) >> 3;
    pid |= (output[1] & 0x40) >> 4;
    pid |= (output[2] & 0x40) >> 5;
    pid |= (output[3] & 0x40) >> 6;
    return pid;
}

void IL2PHeaderInstallN(uint8_t *output, uint16_t n) {
    int16_t i;
    for (int i = 0; i < 3; i++) {
        if (n & 0x4) {
            output[i] |= 0x40;
        } else {
            output[i] &= 0xBF;
        }
        n <<= 1;
    }
}

void IL2PHeaderInstallPID(uint8_t *output, uint8_t pid) {
    int16_t i;
    for (int i = 0; i < 4; i++) {
        if (pid & 0x8) {
            output[i] |= 0x40;
        } else {
            output[i] &= 0xBF;
        }
        pid <<= 1;
    }
}

void IL2PHeaderInstallSCTL(uint8_t *output, uint8_t ctl) {
    output[0] &= 0xBF;
    output[0] |= (ctl & 0x2) << 5;
    output[1] &= 0xBF;
    output[1] |= (ctl & 0x1) << 6;
}

uint16_t IL2PHeaderGetN(uint8_t *output) {
    uint16_t opcode = 0;
    opcode |= (output[0] & 0x40) >> 4;
    opcode |= (output[1] & 0x40) >> 5;
    opcode |= (output[2] & 0x40) >> 6;
    return opcode;
}

uint16_t IL2PHeaderGetSOpcode(uint8_t *output) {
    uint16_t opcode = 0;
    opcode |= (output[0] & 0x40) >> 5;
    opcode |= (output[1] & 0x40) >> 6;
    return opcode;
}


int IL2PBuildPacket(KISS_struct *kiss, uint16_t *output, IL2P_TRX_struct *TRX) {
    int payload_count = 0;
    int16_t blocks = 0;
    int16_t bigblocks = 0;
    int16_t smallsize = 0;
    int16_t output_addr = 0;
    int16_t blockstart;
    int16_t input_addr;
    int16_t encoder;
    int16_t parity_symbols_per_block;

    // Determine if this AX25 packet can be translated to IL2P. If not, use Transparent mode.
    TRX->TransparentMode = 0;
    if (!kiss->RipValid) { // Invalid header rip
        TRX->TransparentMode = 1;
    }
    if (kiss->AX25Callsign7Bit) { // Callsigns aren't SIXBIT compatible
        TRX->TransparentMode = 1;
    }
    if (kiss->AX25ExtendedMode) { // AX.25 extended mode
        TRX->TransparentMode = 1;
    }
    if ((kiss->AX25ControlByte & 0xEF) == 0x6F) { // SABME
        TRX->TransparentMode = 1;
    }
    if (kiss->AX25PIDByteExists) {
        if (PIDtoIL2P(kiss->AX25PIDByte) == 0) {
            TRX->TransparentMode = 1;
        }
    }
    // Install the 24-bit sync word.
    if (TRX->FSK4Syncword == 1) {
        output_addr += IL2PHeaderInstallSpecialSync(&TRX->TXBuffer[0]);
    } else {
        output_addr += IL2PHeaderInstallSync(&TRX->TXBuffer[0]);
    }
    
    if (TRX->TransparentMode) {
        for (int i = 0; i < 13; i++) {
            TRX->TXBuffer[output_addr + i] = 0; // Clear out the IL2P header.
        }
        payload_count = kiss->OutputCount;
        // Check maximum packet length constraint, abort if exceeded.
        if (payload_count > IL2P_MAX_PAYLOAD) {
            return(0);
        }
        IL2PHeaderInstallCount(&TRX->TXBuffer[output_addr + 2], payload_count);
        TRX->TXHdrType = 0; // Transparent mode.
        IL2PHeaderInstallType(&TRX->TXBuffer[output_addr], TRX->TXHdrType);
    } else {
        TRX->TXHdrType = 1; // Translated AX.25.
        IL2PHeaderInstallType(&TRX->TXBuffer[output_addr], TRX->TXHdrType);

        // Install the callsigns packed in SIXBIT:
        IL2PHeaderInstallCallsign(&TRX->TXBuffer[output_addr], kiss->AX25DestCall);
        IL2PHeaderInstallCallsign(&TRX->TXBuffer[output_addr + 6], kiss->AX25SourceCall);

        // Install the SSIDs:
        IL2PHeaderInstallSSID(&TRX->TXBuffer[output_addr + 12], kiss->AX25DestSSID, kiss->AX25SourceSSID);


        // Install Control field:
        switch (kiss->AX25FrameType) {
        case AX25_I:
            payload_count = kiss->OutputCount - 16;
            // Needs UI, PID, P/F, N(R), N(S)
            TRX->TXBuffer[output_addr] &= 0x3F; // Clear UI and Reserved bits
            IL2PHeaderInstallPID(&TRX->TXBuffer[output_addr + 1], PIDtoIL2P(kiss->AX25PIDByte));
            if (kiss->AX25PFBit) {
                TRX->TXBuffer[output_addr + 5] |= 0x40; // Set P/F bit
            } else {
                TRX->TXBuffer[output_addr + 5] &= 0xBF; // Clear P/F bit
            }
            IL2PHeaderInstallN(&TRX->TXBuffer[output_addr + 6], kiss->AX25_NR);
            IL2PHeaderInstallN(&TRX->TXBuffer[output_addr + 9], kiss->AX25_NS);
            break;
        case AX25_U:
            // Needs UI, PID, P/F, OPCODE, C
            if (kiss->AX25UControlFieldType == AX25_UCTL_UI) {
                payload_count = kiss->OutputCount - 16;
                TRX->TXBuffer[output_addr] |= 0x40; // Set UI bit
                IL2PHeaderInstallPID(&TRX->TXBuffer[output_addr + 1], PIDtoIL2P(kiss->AX25PIDByte)); // Install PID
            } else {
                payload_count = kiss->OutputCount - 15;
                TRX->TXBuffer[output_addr] &= 0xBF; // Clear UI bit
                IL2PHeaderInstallPID(&TRX->TXBuffer[output_addr + 1], 1); // Install PID = 1
            }
            if (kiss->AX25PFBit) {
                TRX->TXBuffer[output_addr + 5] |= 0x40; // Set P/F bit
            } else {
                TRX->TXBuffer[output_addr + 5] &= 0xBF; // Clear P/F bit
            }
            IL2PHeaderInstallN(&TRX->TXBuffer[output_addr + 6], UCTLtoIL2P(kiss->AX25ControlByte)); // Install Control OPCODE
            if (kiss->AX25CBit) {
                TRX->TXBuffer[output_addr + 9] |= 0x40; // Set C bit
            } else {
                TRX->TXBuffer[output_addr + 9] &= 0xBF; // Clear C bit
            }
            break;
        case AX25_S:
            payload_count = kiss->OutputCount - 15;
            // Needs UI=0, PID=0, P/F, N(R), C, OPCODE
            TRX->TXBuffer[output_addr] &= 0xBF; // Clear UI bit
            IL2PHeaderInstallPID(&TRX->TXBuffer[output_addr + 1], 0);
            if (kiss->AX25PFBit) {
                TRX->TXBuffer[output_addr + 5] |= 0x40; // Set P/F bit
            } else {
                TRX->TXBuffer[output_addr + 5] &= 0xBF; // Clear P/F bit
            }
            IL2PHeaderInstallN(&TRX->TXBuffer[output_addr + 6], kiss->AX25_NR);
            if (kiss->AX25CBit) {
                TRX->TXBuffer[output_addr + 9] |= 0x40; // Set C bit
            } else {
                TRX->TXBuffer[output_addr + 9] &= 0xBF; // Clear C bit
            }
            IL2PHeaderInstallSCTL(&TRX->TXBuffer[output_addr + 10], (kiss->AX25ControlByte >> 2) & 0x3); // Install OPCODE
            break;
        default:
            break;
        }
        // Check maximum packet length constraint, abort if exceeded.
        if (payload_count > IL2P_MAX_PAYLOAD) {
            return(0);
        }
        // Install the count:
        if (payload_count <= kiss->OutputCount) {
            IL2PHeaderInstallCount(&TRX->TXBuffer[output_addr + 2], payload_count);
        }
    }
    // DC-balance the header through LFSR scrambling
    TRX->TXLFSR.ShiftRegister = IL2P_LFSR_TX_PRE;
    Scramble8(&TRX->TXBuffer[output_addr], IL2P_HEADER_BYTES * 8, 8, &TRX->TXLFSR, 1, 1);

    // RS encode header with two roots (can correct one symbol error)
    RSEncode(&TRX->TXBuffer[output_addr], 13, &TRX->RS[0]);

    output_addr = output_addr + 15; // Starting address for information in the IL2P frame.


    if (payload_count > 0) {
            blocks = Ceiling(payload_count, IL2P_MAXFEC_RS_BLOCKSIZE); // largest block size is 255-IL2P_MAXFEC_NUMROOTS=IL2P_MAXFEC_BLOCKSIZE
            smallsize = payload_count / blocks;
            bigblocks = payload_count - (smallsize * blocks); // bigblock size is 1 bigger than smallblocks
            parity_symbols_per_block = IL2P_MAXFEC_NUMROOTS;

        encoder = 1;
        // Install bigblocks
        if (TRX->TransparentMode) {
            input_addr = 0;
        } else {
            if (kiss->AX25PIDByteExists) {
                input_addr = 16; // Skip past the AX25 header in the input buffer
            } else {
                input_addr = 15;
            }
        }
        
        for (int i = 0; i < bigblocks; i++) {
            blockstart = output_addr;
            for (int j = 0; j < smallsize + 1; j++) {
                TRX->TXBuffer[output_addr++] = kiss->Output[input_addr++];
            }

            // DC-balance block through LFSR scrambling
            TRX->TXLFSR.ShiftRegister = IL2P_LFSR_TX_PRE;
            Scramble8(&TRX->TXBuffer[blockstart], (smallsize + 1) * 8, 8, &TRX->TXLFSR, 1, 1);

            // Encode this block
            RSEncode(&TRX->TXBuffer[blockstart], smallsize + 1, &TRX->RS[encoder]);
            output_addr+= parity_symbols_per_block;
        }

        // Install smallblocks
        for (int i = bigblocks; i < blocks; i++) {
            blockstart = output_addr;
            for (int j = 0; j < smallsize; j++) {
                TRX->TXBuffer[output_addr++] = kiss->Output[input_addr++];
            }

            // DC-balance block through LFSR scrambling
            TRX->TXLFSR.ShiftRegister = IL2P_LFSR_TX_PRE;
            Scramble8(&TRX->TXBuffer[blockstart], smallsize * 8, 8, &TRX->TXLFSR, 1, 1);

            // Encode this block
            RSEncode(&TRX->TXBuffer[blockstart], smallsize, &TRX->RS[encoder]);
            output_addr+= parity_symbols_per_block;
        }
    }
    // Calculate the CRC value based on the original parsed KISS frame.
    TRX->TransmitCRC = CCITT16CalcCRC(kiss->Output, kiss->OutputCount);
    // Convert each nibble of the CRC into a Hamming(7,4) word and add it to the transmit buffer.
    int j = 12;
    for (int i = 0; i < 4; i++) {
        TRX->TXBuffer[output_addr++] = Hamming74EncodeTable[(TRX->TransmitCRC>>j) & 0xF];
        //TRX->TXBuffer[output_addr++] = i;
        j -= 4;
    }

    for (int i = 0; i < output_addr; i++) {
        output[i] = TRX->TXBuffer[i];
    }
    return(output_addr * 8);
    
    // Returns number of bits in packet
}
