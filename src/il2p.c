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


// Hamming(7,4) Decoding Table
// Enter this table with 7-bit encoded value, high bit mased.
// Returns 4-bit decoded value.
uint16_t Hamming74DecodeTable[128] = {  \
      0x0, 0x0, 0x0, 0x3, 0x0, 0x5, 0xe, 0x7, \
      0x0, 0x9, 0xe, 0xb, 0xe, 0xd, 0xe, 0xe, \
      0x0, 0x3, 0x3, 0x3, 0x4, 0xd, 0x6, 0x3, \
      0x8, 0xd, 0xa, 0x3, 0xd, 0xd, 0xe, 0xd, \
      0x0, 0x5, 0x2, 0xb, 0x5, 0x5, 0x6, 0x5, \
      0x8, 0xb, 0xb, 0xb, 0xc, 0x5, 0xe, 0xb, \
      0x8, 0x1, 0x6, 0x3, 0x6, 0x5, 0x6, 0x6, \
      0x8, 0x8, 0x8, 0xb, 0x8, 0xd, 0x6, 0xf, \
      0x0, 0x9, 0x2, 0x7, 0x4, 0x7, 0x7, 0x7, \
      0x9, 0x9, 0xa, 0x9, 0xc, 0x9, 0xe, 0x7, \
      0x4, 0x1, 0xa, 0x3, 0x4, 0x4, 0x4, 0x7, \
      0xa, 0x9, 0xa, 0xa, 0x4, 0xd, 0xa, 0xf, \
      0x2, 0x1, 0x2, 0x2, 0xc, 0x5, 0x2, 0x7, \
      0xc, 0x9, 0x2, 0xb, 0xc, 0xc, 0xc, 0xf, \
      0x1, 0x1, 0x2, 0x1, 0x4, 0x1, 0x6, 0xf, \
      0x8, 0x1, 0xa, 0xf, 0xc, 0xf, 0xf, 0xf };

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

uint16_t UCTLtoAX25(uint16_t IL2PCTL) {
    // Returns AX25 CTL given IL2P CTL.
    uint16_t AX25CTL;
    uint16_t AX25CTLTable[8] = { 0x2F, 0x43, 0x0F, 0x63, 0x87, 0x03, 0xAF, 0xE3 };
    AX25CTL = AX25CTLTable[IL2PCTL & 0x7];
    return AX25CTL;
}


uint16_t PIDtoAX25(uint16_t IL2PPID) {
    // Returns AX25 PID given IL2P PID. Returns 0 to signal "omit AX25 PID byte".
    uint16_t AX25PID;
    uint16_t AX25PIDTable[16] = { 0, 0, 0x10, 0x01, 0x06, 0x07, 0x08, 0xC3, 0xC4, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xF0 };
    AX25PID = AX25PIDTable[IL2PPID & 0xF];
    return AX25PID;
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

#define IL2P_CHECK_AND_SEND\
    Receiver->MyLastChecksum = CCITT16CalcCRC(Receiver->RXBuffer, Receiver->RXBufferIndex);\
    if ((uint32_t)Receiver->MyLastChecksum != Receiver->SisterLastChecksum) {\
        Receiver->HasNewChecksum = 1;\
        Receiver->Result = 1;\
        for (int i = 0; i < Receiver->RXBufferIndex; i++) {\
            output_buffer[i] = Receiver->RXBuffer[i];\
        }\
        /*SendKISS(UART, 0, 0, Receiver->RXBuffer, Receiver->RXBufferIndex);*/\
        Receiver->RxByteCount = Receiver->RXBufferIndex;\
    } else {\
        Receiver->Duplicate = 1;\
    }


#define CHOOSE_CRC_OR_NOT \
    if ((Receiver->TrailingCRC) && (Receiver->RXErrCount > 0)) { \
        Receiver->RXState = IL2P_RX_CRC; \
    } else { \
        Receiver->RXState = IL2P_RX_SEARCH; \
        IL2P_CHECK_AND_SEND \
        Receiver->RXBufferIndex = 0; \
    }

void IL2PReceive(IL2P_TRX_struct *Receiver, uint8_t *input_buffer, int input_count, uint8_t *output_buffer) {
    int16_t i, j;
    uint16_t input_data;
    uint16_t k = 0;
    int16_t x;
    //uint32_t Inverted4FSKSyncword = Invert4FSKDouble(0x5D57DF7F);
    if ((Receiver->TrailingCRC == 1) || (Receiver->Interleave == 1)) {
//        Receiver->RS[0].MinimumErrorDistance = 0;
//        Receiver->RS[1].MinimumErrorDistance = 0;
        Receiver->SyncTolerance = 2;
    } else {
//        Receiver->RS[0].MinimumErrorDistance = 1;
//        Receiver->RS[1].MinimumErrorDistance = 1;
        Receiver->SyncTolerance = 0;
    }
    
    if (Receiver->FSK4Syncword == 1) {
        Receiver->SyncTolerance = 0;
    }
    
    Receiver->Result = 0; // 0 = no packet, 1 = packet received, -1 = packet failed
    for (i = 0; i < input_count; i++) { // step through each input word
        input_data = input_buffer[i];
        for (j = 0; j < 8; j++) { //step through each bit, MSB first
            k++;
            int16_t sync_dist_norm;
//            int16_t sync_dist_inverted;
            switch (Receiver->RXState) {
                case IL2P_RX_SEARCH:
                    
                    // Search for sync word.
                    Receiver->Work.ULI <<= 1;
                    if (input_data & 0x80) { // one bit
                        Receiver->Work.ULI |= 1;
                    }
                    input_data <<= 1;
                    // Detect sync word match.
                        Receiver->SyncWord.ULI = IL2P_SYNCWORD;
                        sync_dist_norm = BitDistance8(Receiver->Work.Byte, Receiver->SyncWord.Byte, IL2P_SYNCWORD_LENGTH);
//                        sync_dist_inverted = 24 - sync_dist_norm;
                    
//                    if (sync_dist_inverted < sync_dist_norm) {
//                        Receiver->InvertRXData = 1;
//                        x = sync_dist_inverted;
//                    } else {
//                        Receiver->InvertRXData = 0;
//                        x = sync_dist_norm;
//                    }
                    
                    Receiver->InvertRXData = 0;
                    x = sync_dist_norm;
                    
                    if (x <= Receiver->SyncTolerance) {
                        Receiver->RXState = IL2P_RX_HEADER;
                        Receiver->RXBufferIndex = 0;
                        Receiver->BitIndex = 0;
                        Receiver->RXErrCount = 0;
                        Receiver->CRCIndex = 0;
                        Receiver->ReceiveCRC = 0;
                    }
                    break;
                case IL2P_RX_HEADER:
                    // Collect and decode the packet header.
                    Receiver->Work.Byte[3] <<= 1;
                    if (input_data & 0x80) { // one bit
                        Receiver->Work.Byte[3] |= 1;
                    }
                    Receiver->BitIndex++;
                    input_data <<= 1;
                    if (Receiver->BitIndex == 8) {
                        // One whole byte read. Write it to output.
                        Receiver->BitIndex = 0;
                        if (Receiver->InvertRXData) {
                                Receiver->RXBuffer[Receiver->RXBufferIndex++] = ~Receiver->Work.Byte[3];
                            
                        } else {
                            Receiver->RXBuffer[Receiver->RXBufferIndex++] = Receiver->Work.Byte[3];
                        }
                        if (Receiver->RXBufferIndex == IL2P_HEADER_BLOCK_SIZE) {
                            // RS Decode header
                            x = RSDecode(Receiver->RXBuffer, IL2P_HEADER_BLOCK_SIZE, &Receiver->RS[0]);
                            Receiver->RXBufferIndex -= IL2P_HEADER_NUMROOTS;
                            
                            if (x >= 0) { // Decode successful
                                Receiver->RXErrCount += x;
                                // Unscramble header
                                Receiver->RXLFSR.ShiftRegister = IL2P_LFSR_RX_PRE;
                                Unscramble8(Receiver->RXBuffer, IL2P_HEADER_BYTES * 8, &Receiver->RXLFSR);
                                // Extract data from header

                                // Identify type of IL2P header
                                Receiver->RXHdrType = IL2PHeaderGetType(&Receiver->RXBuffer[0]);
                                Receiver->RXHdrCount = IL2PHeaderGetCount(&Receiver->RXBuffer[0x2]);

                                if ((Receiver->RXHdrType & 1) == 1) { // Translated AX.25
                                    Receiver->RXHdrUI = (Receiver->RXBuffer[0] >> 6) & 0x1;
                                    Receiver->RXHdrPID = IL2PHeaderGetPID(&Receiver->RXBuffer[0x1]);
                                    if (Receiver->RXHdrUI) {
                                        // This is an AX.25 UI frame. PID exists.
                                        Receiver->RXHdrAX25FrameType = AX25_U;
                                        Receiver->RXHdrAX25PIDExists = 1;
                                        // Get P/F bit, OPCODE, C bit
                                        Receiver->RXHdrPFBit = (Receiver->RXBuffer[0x5] >> 6) & 0x1;
                                        Receiver->RXHdrOpcode = IL2PHeaderGetN(&Receiver->RXBuffer[0x6]);
                                        Receiver->RXHdrCBit = (Receiver->RXBuffer[0x9] >> 6) & 0x1;
                                    } else {
                                        switch (Receiver->RXHdrPID) {
                                        case 0:
                                            // AX.25 Supervisory Frame. No PID.
                                            Receiver->RXHdrAX25FrameType = AX25_S;
                                            Receiver->RXHdrAX25PIDExists = 0;
                                            // Get P/F bit, N(R), C bit, OPCODE
                                            Receiver->RXHdrPFBit = (Receiver->RXBuffer[0x5] >> 6) & 0x1;
                                            Receiver->RXHdrNR = IL2PHeaderGetN(&Receiver->RXBuffer[0x6]);
                                            Receiver->RXHdrOpcode = IL2PHeaderGetSOpcode(&Receiver->RXBuffer[0xA]);
                                            Receiver->RXHdrCBit = (Receiver->RXBuffer[0x9] >> 6) & 0x1;
                                            break;
                                        case 1:
                                            // AX.25 Unnumbered Frame (not UI). No PID.
                                            Receiver->RXHdrAX25FrameType = AX25_U;
                                            Receiver->RXHdrAX25PIDExists = 0;
                                            // Get P/F bit, OPCODE, C bit
                                            Receiver->RXHdrPFBit = (Receiver->RXBuffer[0x5] >> 6) & 0x1;
                                            Receiver->RXHdrOpcode = IL2PHeaderGetN(&Receiver->RXBuffer[0x6]);
                                            Receiver->RXHdrCBit = (Receiver->RXBuffer[0x9] >> 6) & 0x1;
                                            break;
                                        default:
                                            // AX.25 Information Frame. PID exists.
                                            Receiver->RXHdrAX25FrameType = AX25_I;
                                            Receiver->RXHdrAX25PIDExists = 1;
                                            // Get P/F bit, N(R), N(S)
                                            Receiver->RXHdrPFBit = (Receiver->RXBuffer[0x5] >> 6) & 0x1;
                                            Receiver->RXHdrNR = IL2PHeaderGetN(&Receiver->RXBuffer[0x6]);
                                            Receiver->RXHdrNS = IL2PHeaderGetN(&Receiver->RXBuffer[0x9]);
                                            Receiver->RXHdrCBit = 1; // All I frames are commands.
                                            break;
                                        }
                                    }

                                    // Extract callsigns.
                                    IL2PHeaderGetCallsign(&Receiver->RXBuffer[0], Receiver->RXDestCall);
                                    IL2PHeaderGetCallsign(&Receiver->RXBuffer[0x6], Receiver->RXSrcCall);

                                    // Extract SSIDs.
                                    Receiver->RXDestSSID = (Receiver->RXBuffer[12] >> 4) & 0xF;
                                    Receiver->RXSrcSSID = Receiver->RXBuffer[12] & 0xF;

                                    // Set the write index to make room for an AX.25 header.
                                    if (Receiver->RXHdrAX25PIDExists) {
                                        Receiver->RXBufferIndex = 16; // I and UI frames have a PID byte.
                                    } else {
                                        Receiver->RXBufferIndex = 15; // Everything else does not.
                                    }

                                    IL2PConvertToAX25(Receiver); // Write the recomposed header to the accumulator buffer.

                                } else if ((Receiver->RXHdrType & 1) == 0) { // Transparent encapsulation.
                                    Receiver->RXBufferIndex = 0;

                                }


                                if (Receiver->RXHdrCount > 0) {

                                    // This packet contains data

                                            // MaxFEC header
                                            Receiver->RXBlocks = Ceiling(Receiver->RXHdrCount, IL2P_MAXFEC_RS_BLOCKSIZE);
                                            Receiver->RXBlocksize = Receiver->RXHdrCount / Receiver->RXBlocks;
                                            Receiver->RXBigBlocks = Receiver->RXHdrCount - (Receiver->RXBlocks * Receiver->RXBlocksize);
                                            Receiver->RXBlockIndex = 0;
                                            Receiver->RXBlockByteCount = 0;
                                            Receiver->RXNumRoots = IL2P_MAXFEC_NUMROOTS;
                                    Receiver->RXDecoder = 1;
                                    InitRS2(0, Receiver->RXNumRoots, &Receiver->RS[1]);
                                    if (Receiver->RXBigBlocks > 0) {
                                        Receiver->RXState = IL2P_RX_BIGBLOCKS;
                                        Receiver->RXBlocksize += 1;
                                    } else {
                                        Receiver->RXState = IL2P_RX_SMALLBLOCKS;
                                    }

                                } else {
                                    // This packet is only a header
                                    CHOOSE_CRC_OR_NOT

                                }

                            } else {
                                // Receive failure
                                Receiver->RXState = IL2P_RX_SEARCH;
                                Receiver->Result = -1;
                            }
                        }
                    }
                    break;
                case IL2P_RX_BIGBLOCKS:
                    // Collect and decode payload blocks.
                    Receiver->Work.Byte[3] <<= 1;
                    if (input_data & 0x80) { // one bit
                        Receiver->Work.Byte[3] |= 1;
                    }
                    Receiver->BitIndex++;
                    input_data <<= 1;
                    if (Receiver->BitIndex == 8) {
                        // One whole byte read. Write it to output.
                        Receiver->BitIndex = 0;
                        if (Receiver->InvertRXData) {
                                Receiver->RXBuffer[Receiver->RXBufferIndex++] = ~Receiver->Work.Byte[3];
                        } else {
                            Receiver->RXBuffer[Receiver->RXBufferIndex++] = Receiver->Work.Byte[3];
                        }
                        Receiver->RXBlockByteCount++;
                        if (Receiver->RXBlockByteCount == Receiver->RXBlocksize + Receiver->RXNumRoots) {
                            // RS Decode this block
                            x = RSDecode(&Receiver->RXBuffer[Receiver->RXBufferIndex - Receiver->RXBlockByteCount], Receiver->RXBlockByteCount, &Receiver->RS[Receiver->RXDecoder]);

                            if (x >= 0) {
                                // Decode successful
                                Receiver->RXErrCount += x;
                                Receiver->RXLFSR.ShiftRegister = IL2P_LFSR_RX_PRE;
                                Unscramble8(&Receiver->RXBuffer[Receiver->RXBufferIndex - Receiver->RXBlockByteCount], Receiver->RXBlocksize * 8, &Receiver->RXLFSR);
                                Receiver->RXBlockByteCount = 0;
                                // Back up the output write index to overwrite the parity symbols
                                Receiver->RXBufferIndex -= Receiver->RXNumRoots;
                                Receiver->RXBlockIndex++;
                                if (Receiver->RXBlockIndex == Receiver->RXBigBlocks) {
                                    if (Receiver->RXBlockIndex == Receiver->RXBlocks) {
                                        // Packet complete

                                        CHOOSE_CRC_OR_NOT

                                    } else {
                                        Receiver->RXState = IL2P_RX_SMALLBLOCKS;
                                        Receiver->RXBlockByteCount = 0;
                                        // Set RXBlocksize to small block size
                                        Receiver->RXBlocksize -= 1;
                                    }
                                }
                            } else {
                                // Receive failure for this packet
                                Receiver->RXState = IL2P_RX_SEARCH;
                                Receiver->Result = -2;
                            }
                        }
                    }
                    break;
                case IL2P_RX_SMALLBLOCKS:

                    // Collect and decode payload blocks.
                    Receiver->Work.Byte[3] <<= 1;
                    if (input_data & 0x80) { // one bit
                        Receiver->Work.Byte[3] |= 1;
                    }
                    Receiver->BitIndex++;
                    input_data <<= 1;
                    if (Receiver->BitIndex == 8) {
                        // One whole byte read. Write it to output.
                        Receiver->BitIndex = 0;
                        if (Receiver->InvertRXData) {
                                Receiver->RXBuffer[Receiver->RXBufferIndex++] = ~Receiver->Work.Byte[3];
                        } else {
                            Receiver->RXBuffer[Receiver->RXBufferIndex++] = Receiver->Work.Byte[3];
                        }
                        Receiver->RXBlockByteCount++;
                        if (Receiver->RXBlockByteCount == Receiver->RXBlocksize + Receiver->RXNumRoots) {
                            // RS Decode this block
                            x = RSDecode(&Receiver->RXBuffer[Receiver->RXBufferIndex - Receiver->RXBlockByteCount], Receiver->RXBlockByteCount, &Receiver->RS[Receiver->RXDecoder]);

                            if (x >= 0) {
                                // Decode successful
                                Receiver->RXErrCount += x;
                                Receiver->RXLFSR.ShiftRegister = IL2P_LFSR_RX_PRE;
                                Unscramble8(&Receiver->RXBuffer[Receiver->RXBufferIndex - Receiver->RXBlockByteCount], Receiver->RXBlocksize * 8, &Receiver->RXLFSR);
                                Receiver->RXBlockByteCount = 0;
                                // Back up the output write index to overwrite the parity symbols
                                Receiver->RXBufferIndex -= Receiver->RXNumRoots;
                                Receiver->RXBlockIndex++;
                                if (Receiver->RXBlockIndex == Receiver->RXBlocks) {
                                    // Packet complete

                                    CHOOSE_CRC_OR_NOT

                                }
                            } else {
                                // Receive failure for this packet
                                Receiver->RXState = IL2P_RX_SEARCH;
                                Receiver->Result = -3;
                            }
                        }
                    }
                    break;
                case IL2P_RX_CRC:
                    // Collect and decode the Hamming encoded CRC.
                    Receiver->Work.Byte[3] <<= 1;
                    if (input_data & 0x80) { // one bit
                        Receiver->Work.Byte[3] |= 1;
                    }
                    Receiver->BitIndex++;
                    input_data <<= 1;
                    if (Receiver->BitIndex == 8) {
                        Receiver->BitIndex = 0;
                        Receiver->ReceiveCRC <<= 4;
                        if (Receiver->InvertRXData) {
                                Receiver->RXBuffer[Receiver->RXBufferIndex++] = ~Receiver->Work.Byte[3];
                        } else {
                            Receiver->ReceiveCRC |= Hamming74DecodeTable[Receiver->Work.Byte[3] & 0x7F];
                        }
                        Receiver->CRCIndex++;
                        if (Receiver->CRCIndex == 4) {
                            Receiver->RXState = IL2P_RX_SEARCH;
                            Receiver->MyLastChecksum = CCITT16CalcCRC(Receiver->RXBuffer, Receiver->RXBufferIndex);
                            if ((uint32_t)Receiver->MyLastChecksum != Receiver->SisterLastChecksum) {
                                if (Receiver->ReceiveCRC == (uint16_t)Receiver->MyLastChecksum) {
                                    Receiver->HasNewChecksum = 1;
                                    Receiver->Result = 1;
                                    for (int i = 0; i < Receiver->RXBufferIndex; i++) {
                                        output_buffer[i] = Receiver->RXBuffer[i];
                                    }

                                    /*SendKISS(UART, 0, 0, Receiver->RXBuffer, Receiver->RXBufferIndex);*/
                                    Receiver->RxByteCount = Receiver->RXBufferIndex;
                                } else {
                                    Receiver->Result = -4;
                                    
                                }
                            } else {
                                Receiver->Duplicate = 1;
                            }
                        }
                    }
                    break;
            }
        }
    }
}

void IL2PConvertToAX25(IL2P_TRX_struct *Receiver) {
    int16_t i;
    // Install callsigns:
    for (i = 0; i < 6; i++) {
        Receiver->RXBuffer[i] = Receiver->RXDestCall[i] << 1;
        Receiver->RXBuffer[i + 7] = Receiver->RXSrcCall[i] << 1;
    }

    // Install SSIDs
    Receiver->RXBuffer[6] = Receiver->RXDestSSID << 1;
    Receiver->RXBuffer[13] = (Receiver->RXSrcSSID << 1) | 0x01; // Set HDLC address extension bit.

    // Set C bits.
    if (Receiver->RXHdrCBit) {
        Receiver->RXBuffer[6] |= 0x80;
        Receiver->RXBuffer[13] &= 0x7F;
    } else {
        Receiver->RXBuffer[13] |= 0x80;
        Receiver->RXBuffer[6] &= 0x7F;
    }

    // Set RR bits.
    Receiver->RXBuffer[6] |= 0x60;
    Receiver->RXBuffer[13] |= 0x60;

    // Install Control byte.
    switch (Receiver->RXHdrAX25FrameType) {
    case AX25_U:
        Receiver->RXBuffer[14] = UCTLtoAX25(Receiver->RXHdrOpcode);
        Receiver->RXBuffer[14] |= (Receiver->RXHdrPFBit << 4);
        break;
    case AX25_S:
        Receiver->RXBuffer[14] = 0x01;
        Receiver->RXBuffer[14] |= (Receiver->RXHdrOpcode << 2);
        Receiver->RXBuffer[14] |= (Receiver->RXHdrPFBit << 4);
        Receiver->RXBuffer[14] |= (Receiver->RXHdrNR << 5);
        break;
    case AX25_I:
        Receiver->RXBuffer[14] = 0;
        Receiver->RXBuffer[14] |= (Receiver->RXHdrNS << 1);
        Receiver->RXBuffer[14] |= (Receiver->RXHdrPFBit << 4);
        Receiver->RXBuffer[14] |= (Receiver->RXHdrNR << 5);
        break;
    }

    // Install PID byte if it exists.
    if (Receiver->RXHdrAX25PIDExists) {
        Receiver->RXBuffer[15] = PIDtoAX25(Receiver->RXHdrPID);
    }

}

