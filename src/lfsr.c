#include "lfsr.h"

// Unscrambles one word, MSB first. Uses Galois configuration.
void Unscramble16(uint16_t *target,
                    int16_t in_bit_count,
                    int16_t target_width,
                    LFSR_struct *LFSR) {
    int16_t i;
    int16_t j = 0; // Input word index.
    int16_t k = 0; // Input bit-in-word index.
    uint32_t output, input;
    uint16_t target_mask, input_mask;
    if (target_width < TGTWIDTH16) {
        target_mask = (1 << target_width) - 1;
        input_mask = 1 << (target_width - 1);
    } else {
        target_mask = TGTMASK16;
        input_mask = TGTINMASK16;
    }
    if (!LFSR->Initialized) {
        InitLFSR(LFSR->Polynomial, LFSR);
    }
   for (i = 0; i < in_bit_count; i++) {
        input = target[k] & input_mask;
        target[k] <<= 1;
        if (input) {
            LFSR->ShiftRegister ^= LFSR->Polynomial;
        }
        if (LFSR->Invert) {
            output = (~LFSR->ShiftRegister) & LFSR->FeedbackMask;
        } else {
            output = LFSR->ShiftRegister & LFSR->FeedbackMask;
        }
        //output = LFSR->ShiftRegister & output_mask;
        if (output) {
            target[k] |= 1;
        }
        j++;
        if (j == target_width) {
            target[k] &= target_mask;
            k++;
            j = 0;
        }
        LFSR->ShiftRegister >>= 1;
    }
}

void Unscramble8(uint8_t *target,
                    int16_t in_bit_count,
                    LFSR_struct *LFSR) {
    uint32_t output;
    int16_t i;
    int16_t j = 0; // Input word index.
    int16_t k = 0; // Input bit-in-word index.
    uint8_t input;

    if (!LFSR->Initialized) {
        InitLFSR(LFSR->Polynomial, LFSR);
    }
   for (i = 0; i < in_bit_count; i++) {
        input = target[k] & TGTINMASK8;
        target[k] <<= 1;
        if (input) {
            LFSR->ShiftRegister ^= LFSR->Polynomial;
        }
        if (LFSR->Invert) {
            output = (~LFSR->ShiftRegister) & LFSR->FeedbackMask;
        } else {
            output = LFSR->ShiftRegister & LFSR->FeedbackMask;
        }
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


int16_t Scramble16(uint16_t *target,
                    int16_t in_bit_count,
                    int16_t target_width,
                    LFSR_struct *LFSR,
                    uint16_t prime,
                    uint16_t purge) {
    uint32_t SR_save = 0;
    uint32_t feedback, input;
    int16_t i;
    int16_t j = 0; // Input word index.
    int16_t k = 0; // Input bit-in-word index.
    int16_t m = 0; // Output word index.
    int16_t n = 0; // Output bit-in-word index.    
    int16_t q;
    uint16_t target_mask, input_mask;
    uint16_t SR_write_trigger = 0;
    if (target_width < TGTWIDTH16) {
        target_mask = (1 << target_width) - 1;
        input_mask = 1 << (target_width - 1);
    } else {
        target_mask = TGTMASK16;
        input_mask = TGTINMASK16;
    }
    if (!LFSR->Initialized) {
        InitLFSR(LFSR->Polynomial, LFSR);
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
    for (i = 0; i < q; i++) {
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
    if (n > 0) {
        target[m] <<= (16 - n);
    }
    return (m * target_width) + n;
}

int16_t Scramble8(uint8_t *target,
                    int16_t in_bit_count,
                    int16_t target_width,
                    LFSR_struct *LFSR,
                    uint16_t prime,
                    uint16_t purge) {
    uint32_t SR_save = 0;
    uint32_t feedback, input;
    int16_t i;
    int16_t j = 0; // Input word index.
    int16_t k = 0; // Input bit-in-word index.
    int16_t m = 0; // Output word index.
    int16_t n = 0; // Output bit-in-word index.    
    int16_t q;
    uint16_t SR_write_trigger = 0;
    uint8_t target_mask, input_mask;
    if (target_width < TGTWIDTH8) {
        target_mask = (1 << target_width) - 1;
        input_mask = 1 << (target_width - 1);
    } else {
        target_mask = TGTMASK8;
        input_mask = TGTINMASK8;
    }
    if (!LFSR->Initialized) {
        InitLFSR(LFSR->Polynomial, LFSR);
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
    for (i = 0; i < q; i++) {
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


void InitLFSR(uint32_t Polynomial, LFSR_struct *LFSR) {
    int16_t i;
    LFSR->Polynomial = Polynomial;
    LFSR->TapCount = 0;
    // Index lowest power taps in lowest order positions.
    for (i = 0; i < MAXPOWER; i++) {
        if ((LFSR->Polynomial >> i) & 1) {
            LFSR->Tap[LFSR->TapCount++] = i;
        }
    }
    LFSR->InputMask = (uint32_t)1 << (LFSR->Tap[LFSR->TapCount - 1]);
    LFSR->OutputMask = (uint32_t)1 << LFSR->Tap[1];
    LFSR->FeedbackMask = 1;
    LFSR->BitDelay = LFSR->Tap[LFSR->TapCount - 1] - LFSR->Tap[1];
    LFSR->BitsInProgress = 0;
    LFSR->Initialized = 1;
    LFSR->Order = 1 << LFSR->Tap[LFSR->TapCount - 1];
    LFSR->ShiftRegister = 0xFFFF;
}


void StepLFSR(LFSR_struct *LFSR, int16_t step_count) {
    for (int i = 0; i < step_count; i++) {
        LFSR->ShiftRegister |= LFSR->InputMask;
        if(LFSR->ShiftRegister & LFSR->FeedbackMask) {
            LFSR->ShiftRegister ^= LFSR->Polynomial;
        }
        LFSR->ShiftRegister >>= 1;
    }
}
