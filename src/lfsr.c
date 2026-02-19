#include "lfsr.h"

void UnScramble(uint8_t *target, int in_bit_count, LFSR_struct *LFSR) {
    long int output;
    int j = 0; // Input word index.
    int k = 0; // Input bit-in-word index.
    for (int i = 0; i < in_bit_count; i++) {
        if (target[k] & TGTINMASK8) {
            LFSR->ShiftRegister ^= LFSR->Polynomial;
        }
        if (LFSR->Invert) {
            output = (~LFSR->ShiftRegister) & LFSR->FeedbackMask;
        } else {
            output = LFSR->ShiftRegister & LFSR->FeedbackMask;
        }
        target[k] <<= 1;
        if (output) {
            target[k] |= 1;
        }
        j++;
        if (j == TGTWIDTH8) {
            j = 0;
            k++;
        }
        LFSR->ShiftRegister >>= 1;
    }
}



int Scramble(uint8_t *target, int in_bit_count, int target_width, LFSR_struct *LFSR, int prime, int purge) {
    int SR_save = 0;
    long int feedback, input;
    int j = 0; // Input word index.
    int k = 0; // Input bit-in-word index.
    int m = 0; // Output word index.
    int n = 0; // Output bit-in-word index.    
    int q;
    int SR_write_trigger = 0;
    int target_mask, input_mask;
    if (target_width < TGTWIDTH8) {
        target_mask = (1 << target_width) - 1;
        input_mask = 1 << (target_width - 1);
    } else {
        target_mask = TGTMASK8;
        input_mask = TGTINMASK8;
    }
    if (purge) {
        q = in_bit_count + LFSR->BitDelay;
    } else {
        q = in_bit_count;
    }
    if (prime) {
        LFSR->BitsInProgress = 0;
    } else {
        LFSR->BitsInProgress = LFSR->BitDelay;
    }
    for (int i = 0; i < q; i++) {
        if (i < in_bit_count) {
            if (LFSR->Invert) {
                input = (~target[j]) & input_mask;
            } else {
                input = target[j] & input_mask;
            }
            target[j] <<= 1;
            k++;
        } else { // Flushing registers.
            if (purge) {
                purge = 0;
                SR_save = LFSR->ShiftRegister;
                SR_write_trigger = 1;
            }
            input = 0;
        }
        
        if (input) {
            LFSR->ShiftRegister |= LFSR->InputMask;
        }
        feedback = LFSR->ShiftRegister & LFSR->FeedbackMask;
        if (feedback) {
            LFSR->ShiftRegister ^= LFSR->Polynomial;
        }
        // Delay output until data present.
        if (LFSR->BitsInProgress >= LFSR->BitDelay) {
            if (j ^ m) {
                // Advance the target output bitstream iff the 
                // input bitstream has already incremented to the
                // next word (accounts for bit delay).
                target[m] <<= 1;
            }            
            if (LFSR->ShiftRegister & LFSR->OutputMask) {
                target[m] |= 1;
            }
            n++;
            if (n == target_width) { // Complete output word has been written.
                target[m] &= target_mask;
                m++;
                n = 0;
            }
        } else {
            LFSR->BitsInProgress++;
        }
        if (k == target_width) { // Complete input word has been read.
            j++;        
            k = 0;
        }
        LFSR->ShiftRegister >>= 1;
    }
    if (SR_write_trigger) {
        LFSR->ShiftRegister = SR_save;
        LFSR->BitsInProgress-= (q - in_bit_count);        
    }
    return (m * target_width) + n;
}


void InitLFSR(long int Polynomial, LFSR_struct *LFSR) {
    LFSR->Polynomial = Polynomial;
    LFSR->TapCount = 0;
    // Index lowest power taps in lowest order positions.
    for (int i = 0; i < LFSR_MAXPOWER; i++) {
        if ((LFSR->Polynomial >> i) & 1) {
            LFSR->Tap[LFSR->TapCount++] = i;
        }
    }
    LFSR->InputMask = 1 << (LFSR->Tap[LFSR->TapCount - 1]);
    LFSR->OutputMask = 1 << LFSR->Tap[1];
    LFSR->FeedbackMask = 1;
    LFSR->BitDelay = LFSR->Tap[LFSR->TapCount - 1] - LFSR->Tap[1];
    LFSR->BitsInProgress = 0;
    LFSR->Initialized = 1;
    LFSR->Order = 1 << LFSR->Tap[LFSR->TapCount - 1];
    LFSR->ShiftRegister = -1;
}
