#include <stdlib.h>
#include <stdint.h>

void GenRandomMessage(int *buffer, int mask, int size) {
	for (int i = 0; i < size; i++) {
		buffer[i] = rand() & mask;
	}
}
void CopyMessage(int *in, int *out, int size) {
	for (int i = 0; i < size; i++) {
		out[i] = in[i];
	}
}

int GenBERErrorVector(int *buffer, int bits_per_word, int size, double ber) {
	int bit_count = size * bits_per_word;
	int byte_index = 0;
	int bit_index = 0;
	int work = 0;
	int bit_error_count = 0;
	for (int i = 0; i < bit_count; i++) {
		if (((double)rand() / (double)RAND_MAX) < ber) {
			work |= 1;
			bit_error_count++;
		}
		bit_index++;
		if (bit_index >= bits_per_word) {
			bit_index = 0;
			buffer[byte_index++] = work;
			work = 0;
		}
		work <<= 1;
	}
	return bit_error_count;
}

int GenSERErrorVector(int *buffer, int bits_per_word, int word_count, int bits_per_symbol, double ser) {
	int symbol_count = (word_count * bits_per_word) / bits_per_symbol;
	if ((symbol_count * bits_per_symbol) < (word_count * bits_per_word)) {
		symbol_count++;
	}
	int error_pattern = (1<<bits_per_symbol) - 1;
	int word_mask = (1<<bits_per_word) - 1;
	int word_index = 0;
	int bit_index = 0;
	long int work = 0;
	int symbol_error_count = 0;
	for (int i = 0; i < symbol_count; i++) {
		if (((double)rand() / (double)RAND_MAX) < ser) {
			work |= error_pattern;
			symbol_error_count++;
		}
		bit_index+= bits_per_symbol;
		while (bit_index >= bits_per_word) {
			bit_index -= bits_per_symbol;
			buffer[word_index++] = (work>>(bit_index - bits_per_word)) & word_mask;
			work>>= (bit_index - bits_per_word);
		}
		work <<= bits_per_symbol;
	}
	return symbol_error_count;
}

void CombineVectors(int *in1, int *in2, int *out, int count) {
	for (int i = 0; i < count; i++) {
		out[i] = in1[i] ^ in2[i];
	}
}


int CompareVectors(uint8_t *a, uint8_t *b, int size) {
	int errors = 0;
	for (int i = 0; i < size; i++) {
		if (a[i] ^ b[i]) {
			errors++;
		}
	}
	return errors;
}
