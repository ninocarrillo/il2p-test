#include "ax25.h"
#include "crc.h"

void InitAX25(AX25_Receiver_struct *Receiver) {
    Receiver->BitIndex = 0;
    Receiver->OneCounter = 0;
    Receiver->WordIndex = 0;
    Receiver->CRCOK = 0;
}

int AX25BuildFrame(uint8_t *input, int in_count, uint8_t *output, int bit_offset) {
    int out_count = 0;
    int one_counter = 0; // for bitstuffing
    // Start assembling data into the transmit buffer. This routine is targeted
    // to transfers via the SPI port, which sends most-significant bit first.
    // AX25 specifies least-significant bit first, so bit endian-ness is changed
    // in this transfer.
    int bit_counter = bit_offset;
    int output_index = 0;
    if (bit_counter > 0) {
        output[output_index] >>= (AX25_OUTPUT_WORD_WIDTH - bit_counter);
    }
    // append start flag
    int bits_remaining = AX25_START_FLAG_BITS;
    AX25_WORKING_WORD_TYPE working_word = AX25_START_FLAG;
    working_word <<= (AX25_WORKING_WORD_WIDTH - AX25_START_FLAG_BITS);
    while (bits_remaining--) {
        out_count++;
        output[output_index] <<= 1;
        if (working_word & AX25_WORKING_WORD_MASK) {
            output[output_index] |= 1;
        }
        working_word <<= 1;
        bit_counter++;
        if (bit_counter >= AX25_OUTPUT_WORD_WIDTH) {
            bit_counter = 0;
            output_index++;
            output[output_index] = 0;
        }
    }
    
    for (int i = 0; i < in_count + 1; i++) {
        if (i < in_count) {
            working_word = input[i];
            bits_remaining = 8;
        } else {
            working_word = CCITT16CalcCRC(input, in_count);
            bits_remaining = 16;                    
        }
        while (bits_remaining) {
            output[output_index] <<= 1; // shift output left one
            out_count++; // count the output bit
            if (one_counter == 5) { // stuff a zero
                one_counter = 0;
            } else {
                if (working_word & 1) { // is input bit a '1'?
                    output[output_index] |= 1; // place a '1' on the output bit stream
                    one_counter++;
                    bits_remaining--; // count the input bit
                    working_word >>= 1; // shift input bit stream right
                } else { // input bit is a '0'
                    one_counter = 0; // reset '1' counter
                    bits_remaining--; // count the input bit
                    working_word >>= 1; // shift the input bit stream right
                }
            }
            bit_counter++; // count output bit in word
            if (bit_counter >= AX25_OUTPUT_WORD_WIDTH) {
                bit_counter = 0;
                output_index++;
                output[output_index] = 0;
            }            
        }
    }
    
	// Handle dangling stuffed zeros... strings of 5 ones at the end of the input.
    // The lacoutput_index of this code caused me a ton of headaches in v1.1. Frames that ended
    // with 5 ones before the flag would not decode because the receiver was deleting
    // the first zero after the 5 ones, invalidating the flag.
	if (one_counter == 5) {
		one_counter = 0;
		output[output_index] <<= 1;
		out_count++;
		bit_counter++; // count output bit in word
		if (bit_counter >= AX25_OUTPUT_WORD_WIDTH) {
			bit_counter = 0;
			output_index++;
            output[output_index] = 0;
		}    
	}
    
    // append closing flag
    bits_remaining = AX25_END_FLAG_BITS;
    working_word = AX25_END_FLAG;
    while (bits_remaining--) {
        out_count++;
        output[output_index] <<= 1;
        output[output_index] |= working_word & 1;
        working_word >>= 1;
        bit_counter++;
        if (bit_counter > AX25_OUTPUT_WORD_WIDTH) {
            bit_counter = 0;
            output_index++;
            output[output_index] = 0;
        }
    }
    
    // align final word to msb
    if (bit_counter > 0) {
        output[output_index] <<= (AX25_OUTPUT_WORD_WIDTH - bit_counter);
    }
    return out_count; // Returns bits in assembled AX.25 frame.
}

// void AX25Receivetooutput_indexISS(AX25_Receiver_struct *Receiver, UInt16_Buffer_struct *Input, UART_struct *UART) {
    // int16_t i, j;
    // for (i = 0; i < Input->WordCount; i++) { // step through each input word
        // for (j = 0; j < 16; j++) { //step through each bit, MSB first
            /*bit un-stuff*/
            // if (Input->StartAddr[i] & 0x8000) { // one bit
                // Receiver->Woroutput_indexingWord8 |= 0x80;
                // Receiver->OneCounter++;
                // Receiver->BitIndex++;
                // if (Receiver->OneCounter > 6){
                    /*abort frame for invalid bit sequence*/
                    // Receiver->BitIndex = 0;
                    // Receiver->WordIndex = 0;
                // } else if (Receiver->BitIndex > 7) { // 8 valid bits received, record byte
                    // Receiver->BitIndex = 0;
                    // Receiver->Buffer[Receiver->WordIndex] = Receiver->Woroutput_indexingWord8;
                    // Receiver->WordIndex++;
                    // if (Receiver->WordIndex >= AX25_MAX_RX_BUFFER) {
                        /*Frame exceeds size of buffer*/
                        // Receiver->WordIndex = 0;
                        // Receiver->OneCounter = 0;
                    // }                        
                // } else {
                    // Receiver->Woroutput_indexingWord8 >>= 1;
                // }                    
            // } else { //zero bit
                // if (Receiver->OneCounter < 5) {
                    // Receiver->Woroutput_indexingWord8 &= 0x7F;
                    // Receiver->BitIndex++;
                    // if (Receiver->BitIndex > 7) {
                        // Receiver->BitIndex = 0;
                        // Receiver->Buffer[Receiver->WordIndex] = Receiver->Woroutput_indexingWord8;
                        // Receiver->WordIndex++;
                        // if (Receiver->WordIndex >= AX25_MAX_RX_BUFFER) {
                            /*Frame exceeds size of buffer*/
                            // Receiver->WordIndex = 0;
                        // }
                    // } else {
                        // Receiver->Woroutput_indexingWord8 >>= 1;
                    // }
                // } else if (Receiver->OneCounter == 5) {
                    /*ignore stuffed zero*/
                // } else if (Receiver->OneCounter == 6) { // Flag frame end
                    // if ((Receiver->WordIndex >= MIN_AX25_FRAME_LENGTH) && (Receiver->BitIndex == 7)) {

                            // Receiver->WordIndex -= 2;
                            // if (CCITT16Checoutput_indexCRC(Receiver->Buffer, Receiver->WordIndex)) {
                                // Receiver->CRC = (uint16_t)Receiver->Buffer[Receiver->WordIndex];
                                // Receiver->CRC |= ((uint16_t)Receiver->Buffer[Receiver->WordIndex + 1]) << 8;
                                // Receiver->CRCOoutput_index = 1;
                                // if (Receiver->CRC != Receiver->SisterCRC) {
                                    // Sendoutput_indexISS(UART, 0, 0, Receiver->Buffer, Receiver->WordIndex);
                                    // Receiver->Duplicate = 0;
                                    // Receiver->RxByteCount = Receiver->WordIndex;
                                    // int16_t i;
                                    // for (i = 0; i < 7; i++) {
                                        // Receiver->LastHeardDestCallsign[i] = Receiver->Buffer[i];
                                    // }
                                // } else {
                                    // Receiver->Duplicate = 1;
                                // }

                            // } else {
                                // Receiver->CRCBad = 1;
                            // }

                    // }
                    // Receiver->WordIndex = 0;
                    // Receiver->BitIndex = 0;
                // } else { // one counter > 6, therefore frame is invalid
                    // Receiver->WordIndex = 0;
                    // Receiver->BitIndex = 0;
                // }
                // Receiver->OneCounter = 0;
            // }
            // Input->StartAddr[i] <<= 1;
        // }
    // } 
// }