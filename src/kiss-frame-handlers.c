#include "stdint.h"
#include "kiss-frame-handlers.h"
#include "kiss.h"

int CheckPID(int AX25PID) {
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

void RipAX25Header(KISS_struct *kiss) {
    int i;
    kiss->RipValid = 1;
    
    // Check this AX.25 header conforms to IL2P conversion.
    
    // Check first 13 bytes have Address Extension bit clear:
    for (i = 0; i < 13; i++) {
        if (kiss->Output[i] & 1) {
            kiss->RipValid = 0;
        }
    }
    
    // -- Address Extension bit is 1 (LSB of index 13)
    if ((kiss->Output[13] & 1) == 0) {
        kiss->RipValid = 0;
    }
    // -- Reserved bits set to 1 (Bits 5 and 6 of index 6 and 13)
    if ((kiss->Output[6] & 0x60) != 0x60) {
        kiss->RipValid = 0;
    }
    if ((kiss->Output[13] & 0x60) != 0x60) {
        kiss->RipValid = 0;
    }
    // -- Command bits are not the same (MSB of index 6 and 13)
    if ((kiss->Output[6] & 0x80) == (kiss->Output[13] & 0x80)) {
        kiss->RipValid = 0;
    }
    
    if (kiss->RipValid == 1) {
        // All AX.25 frames have a Control byte.
        kiss->AX25ControlByte = kiss->Output[14];

        // Extract the Poll/Final bit.
        kiss->AX25PFBit = (kiss->AX25ControlByte & 0x10) >> 4;

        // Determine what type of frame this is.
        if (kiss->AX25ControlByte & 1) { // Check the LSB, if set:
            // Either S (Supervisory) or U (Unnumbered) frame.
            kiss->AX25FrameType = kiss->AX25ControlByte & 0x3;
        } else {
            // This is an I (Information) frame.
            kiss->AX25FrameType = AX25_I;
            kiss->AX25_NS = (kiss->AX25ControlByte >> 1) & 0x7;
            kiss->AX25_NR = (kiss->AX25ControlByte >> 5) & 0x7;
        }

        if (kiss->AX25FrameType == AX25_S) {
            kiss->AX25_NR = (kiss->AX25ControlByte >> 5) & 0x7;
        }

        kiss->AX25UControlFieldType = AX25_UCTL_NONUI;
        if (kiss->AX25FrameType == AX25_U) {
            // This is a U frame, determine what type.
            // Will test later if this is a UI frame.
            kiss->AX25UControlFieldType = kiss->AX25ControlByte & 0xEF; // Mask the P/F bit
            switch(kiss->AX25UControlFieldType) {
                case (AX25_UCTL_SABM):
                case (AX25_UCTL_DISC):
                case (AX25_UCTL_DM):
                case (AX25_UCTL_UA):
                case (AX25_UCTL_FRMR):
                case (AX25_UCTL_UI):
                case (AX25_UCTL_XID):
                case (AX25_UCTL_TEST):
                    // These are mappable U-frame control opcodes
                    break;
                default:
                    // Unknown U-frame control opcode, invalid rip.
                    // This may not be an AX.25 frame, send transparent.
                    kiss->RipValid = 0;
                    break;
            }
        }

        // Only I and UI frames have a PID byte.
        if ((kiss->AX25FrameType == AX25_I) || (kiss->AX25UControlFieldType == AX25_UCTL_UI)) {
            kiss->AX25PIDByte = kiss->Output[15];
            kiss->AX25PIDByteExists = 1;
            // Check if this PID is IL2P mappable:
            if (!CheckPID(kiss->AX25PIDByte)) {
                kiss->RipValid = 0;
            }

        } else {
            kiss->AX25PIDByte = 0;
            kiss->AX25PIDByteExists = 0;
        }

        // Extract and save the callsigns.
        kiss->AX25Callsign7Bit = 0;
        for (i = 0; i < 6; i++) {
            kiss->AX25DestCall[i] = kiss->Output[i] >> 1;
            kiss->AX25SourceCall[i] = kiss->Output[i + 7] >> 1;
            // Check if the callsign characters use 7 bits (SIXBIT encoding in IL2P not possible if so)
            if ((kiss->AX25DestCall[i] > 0x5F) || (kiss->AX25DestCall[i] < 0x20)) {
                kiss->AX25Callsign7Bit = 1;
            }
            if ((kiss->AX25SourceCall[i] > 0x5F) || (kiss->AX25SourceCall[i] < 0x20)) {
                kiss->AX25Callsign7Bit = 1;
            }
        }

        // Extract and save the SSIDs.
        kiss->AX25DestSSID = (kiss->Output[6] >> 1) & 0xF;
        kiss->AX25SourceSSID = (kiss->Output[13] >> 1) & 0xF;

        // Extract and save the Command bit.
        if (kiss->Output[6] & 0x80) {
            kiss->AX25CBit = 1;
        } else {
            kiss->AX25CBit = 0;
        }
    }
}
