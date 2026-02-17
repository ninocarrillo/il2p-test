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

void GenErrorVector(int *buffer, int mask, int size, int count) {
	int error_locs[size];
	// Clear error buffers
	for (int i = 0; i < size; i++) {
		buffer[i] = 0;
		error_locs[i] = 0;
	}
	// Generate count unique error locations in range 0:(size-1)
	int error_index = 0;
	while(error_index < count) {
		int candidate_location = rand() % size;
		int is_unique = 1;
		int i = 0;
		while (is_unique && (i < error_index)) {
			if (error_locs[i++] == candidate_location) {
				is_unique = 0;
			}
		}
		if (is_unique) {
			error_locs[error_index++] = candidate_location;
		}
	}
	for (int i = 0; i < count; i++) {
		int x = 0;
		while (x == 0) {
			x = rand() & mask;
		}
		buffer[error_locs[i]] = x;
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
