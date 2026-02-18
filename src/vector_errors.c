#include <stdlib.h>

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

void GenBERErrorVector(int *buffer, int bits_per_word, int size, float ber) {
	int bit_count = size * bits_per_word;
	int byte_index = 0;
	int bit_index = 0;
	int work = 0;
	for (int i = 0; i < bit_count; i++) {
		if (((double)rand() / (double)RAND_MAX) < ber) {
			work |= 1;
		}
		bit_index++;
		if (bit_index >= bits_per_word) {
			bit_index = 0;
			buffer[byte_index++] = work;
			work = 0;
		}
		work <<= 1;
	}
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
