#include <stdio.h>
#include <stdlib.h>
#include "il2p.h"
#include "kiss.h"
#include "kiss-frame-handlers.h"
#include "crc.h"
#include "vector_errors.h"

#define MAX_BUFFER 1300

int GenRandomCallsignChar(void) {
	// Generate a random capital letter or numeric digit
	// There are 10 acceptable digits
	// 26 acceptable capital letters
	// And include space as a valid character
	// 37 total acceptable characters
	int rand_char = rand() % 38;
	if (rand_char < 10) {
		// Make this one an ASCII digit
		rand_char += 48;
	} else if (rand_char < 36) {
		// Make this one an ASCII captial letter
		rand_char += 55;
	} else {
		// Make this one a space
		rand_char = 32;
	}
	return rand_char;
}

int GenRandomAX25UIHeader(unsigned char *buffer) {
	int buf_i = 0;
	// Generate the destination callsign
	for (int i = 0; i < 6; i++) {
		buffer[buf_i++] = GenRandomCallsignChar() << 1;
	}
	// Add a random SSID
	buffer[buf_i] = rand() % 256;
	// Clear the address extension bit and command bit
	buffer[buf_i] &= 0x7E;
	// Set the reserved bits
	buffer[buf_i++] |= 0x60;
	// Generate the source callsign
	for (int i = 0; i < 6; i++) {
		buffer[buf_i++] = GenRandomCallsignChar() << 1;
	}
	// Add a random SSID
	buffer[buf_i] = rand() % 256;
	// Set the reserved bits, command bit, address extension bit
	buffer[buf_i++] |= 0xE1;
	// Set Control byte
	buffer[buf_i++] = 0x03;
	// Set the PID byte
	buffer[buf_i++] = 0xF0;
	return buf_i;
}


int GenRandomBytes(unsigned char *buffer, int count) {
	int i;
	for (i = 0; i < count; i++) {
		buffer[i] = rand() & 0xFF;
	}
	return i;
}

int GenRandomHeader(unsigned char *buffer) {
	unsigned char pids[14] = {0x10, 0x01, 0x06, 0x07, 0x08, 0xC3, 0xC4, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xF0 };
	// Make the callsign and SSID octets:
	for (int i = 0; i < 14; i++) {
		buffer[i] = GenRandomCallsignChar() << 1;
	}
	// Set the address extension bit and reserved bits:
	buffer[13] |= 0x61;
	buffer[6] |= 0x60;
	// set control byte
	buffer[14] = 3;
	// Generate a random IL2P encodable PID
	buffer[15] = pids[rand() % 15];
	return 16;
}

int main(int arg_count, char* arg_values[]) {
	
	if (arg_count < 6) {
		printf("Not enough arguments.\r\n");
		printf("Usage:\r\nil2p-test <header restriction> <payload length> <max errors> <runs> <seed>\r\n");
		printf("\r\nExample: il2p-test 3 0 4 1000000 0");
		printf("\r\n\n     header restricion:");
		printf("\r\n              0 - No restriction (equal distribution of translatable and non-translatable headers. (slow)");
		printf("\r\n              1 - Only random translatable headers. (slow)");
		printf("\r\n              2 - Only random non-translatable headers. (fast)");
		printf("\r\n              3 - Only random translatable UI headers. (fast)");
		printf("\r\n\n     payload length:");
		printf("\r\n              Integer number representing packet payload byte count. For a header-only packet,");
		printf("\r\n              set this to 0. Maximum payload length is 1023. ");
		printf("\r\n\n     max errors:");
		printf("\r\n              Maximum number of byte errors per packet.");
		printf("\r\n\n     runs:");
		printf("\r\n              Integer number of random test cases to perform at each error count. The program");
		printf("\r\n              will generate a random packet of specified length for each run, and corrupt");
		printf("\r\n              the packet with a precise number of random errors in random locations.");
		printf("\r\n              Error count will span from zero to max errors.");
		printf("\r\n\n     seed:");
		printf("\r\n              Integer number used to seed random number generator, for test repeatability.");
		printf("\r\n");

		return(-1);
	}

	int header_restriction = atoi(arg_values[1]);
	int payload_length = atoi(arg_values[2]);
	int max_errors = atoi(arg_values[3]);
	int run_count = atoi(arg_values[4]);
	int seed = atoi(arg_values[5]);
	srand(seed);
	
	// Validate arguments
	
	if ((header_restriction < 0) || (header_restriction > 4)) {
		printf("\r\nHeader restriction %i is invalid. Must be an integer between 0 and 3.\r\n", header_restriction);
		return(-1);
	}
	if (payload_length < 0) {
		printf("\r\nPayload length %i is too small. Must be greater than zero.\r\n", payload_length);
		return(-1);
	}
	
	if (payload_length > 1023) {
		printf("\r\nPayload length %i is too large. Must be less than 1023.\r\n", payload_length);
		return(-1);
	}

	if (payload_length == 0) {
		if (max_errors > 15) {
			printf("\r\nMax error count %i is too large for zero length payload. \r\nMust be less than or equal to 15.\r\n", max_errors);
			return(-1);
		}
	} else {
		if (max_errors > (payload_length + 13)) {
			printf("\r\nMax error count %i is too large for specified payload size. \r\nMust be less than or equal to %i.\r\n", max_errors, payload_length + 13);
		}
	}

	if (max_errors < 1) {
		printf("\r\nMax error count %i is too small. Must be greater than zero.\r\n", max_errors);
		return(-1);
	}


	printf("\r\nSize of int variable is %li bits.", sizeof(int)*8);
	

	IL2P_TRX_struct il2p_trx;
	InitIL2P(&il2p_trx);
	KISS_struct kiss;

	int decoder_reject_header[MAX_BUFFER];
	int decoder_reject_payload[MAX_BUFFER];
	int decoder_reject_crc[MAX_BUFFER];
	int decoder_reject[MAX_BUFFER];
	int decoder_no_detect[MAX_BUFFER];
	int decoder_accept[MAX_BUFFER];
	int actual_successes[MAX_BUFFER];
	int undetected_failures[MAX_BUFFER];


	int ax25_source_packet[MAX_BUFFER];
	uint8_t il2p_encoded_packet[MAX_BUFFER];
	uint8_t il2p_decoded_packet[MAX_BUFFER];
	uint8_t corrupt_packet[MAX_BUFFER];
	int error_vector[MAX_BUFFER];

	for (int i = 0; i <= MAX_BUFFER; i++) {
		decoder_reject_header[i] = 0;
		decoder_reject_payload[i] = 0;
		decoder_reject_crc[i] = 0;
		decoder_accept[i] = 0;
		actual_successes[i] = 0;
		undetected_failures[i] = 0;
		decoder_no_detect[i] = 0;
	}


	printf("\r\nStarting %i trials.", (max_errors + 1) * run_count);
	int master_count = 1;
	int prog_bar_segs = 40;
	int print_interval = ((max_errors + 1) * run_count) / prog_bar_segs;
	printf("\r\n");
	for (int i = 0; i <= prog_bar_segs; i++) {
		printf(" ");
	}
	printf("]\r[");
	
	for (int error_count = 0; error_count <= max_errors; error_count++) {
		for (int run_number = 1; run_number <= run_count; run_number++) {
			if ((master_count++ % print_interval) == 0 ) {
				printf("=");
				fflush(stdout);
			}

			int translatable = 0;
			if (header_restriction == 0) {
				// No restriction, flip a coin to determine if this should be a translatable header.
				if (rand() & 0x10) {
					translatable = 1;
				}
			} else if (header_restriction == 1) {
				// Must be translatable
				translatable = 1;
			}  else if (header_restriction == 3) {
				translatable = 2;
			}
			if (translatable == 2) {
				kiss.OutputCount = GenRandomAX25UIHeader(kiss.Output);
				RipAX25Header(&kiss);
			} else {
				int rip_result = -1;
				while (rip_result != translatable) {
					kiss.OutputCount = GenRandomHeader(kiss.Output);
					RipAX25Header(&kiss);
					rip_result = kiss.RipValid;
				}
			}

			// if (kiss.RipValid) {
			// 	printf("\r\nValid rip.");
			// } else {
			// 	printf("\r\nInvalid rip.");
			// }
			// Create packet payload.
			kiss.OutputCount += GenRandomBytes(&kiss.Output[kiss.OutputCount], payload_length);

			// printf("\r\nRandom packet generated: ");
			// for (int i = 0; i < kiss.OutputCount; i++) {
			// 	printf(" %2x", kiss.Output[i]);
			// }

			int encode_CRC = CCITT16CalcCRC(kiss.Output, kiss.OutputCount);
			// printf(" CRC: %4x", encode_CRC);
			// Perform IL2P Encoding.
			int il2p_tx_count = IL2PBuildPacket(&kiss, il2p_encoded_packet, &il2p_trx);
			printf("\r\nIL2P Packet Size: %i", il2p_tx_count);
			// printf("\r\nIL2P Encoded Packet: ");
			// fflush(stdout);
			// for (int i = 0; i < il2p_tx_count; i++) {
			// 	printf(" %2x", il2p_encoded_packet[i]);

			// }

			// Generate an error vector.
			GenErrorVector(error_vector, 0xFF, il2p_tx_count, error_count);

			// Make a noisy packet.
			for (int i = 0; i < il2p_tx_count; i++) {
				corrupt_packet[i] = error_vector[i] ^ il2p_encoded_packet[i];
			}

			int il2p_rx_count = 0;
			IL2PReceive(&il2p_trx, corrupt_packet, il2p_tx_count, il2p_decoded_packet);
			switch(il2p_trx.Result) {
			case 0:
				// No packet detection.
				decoder_no_detect[error_count]++;
				break;
			case 1:
				// Decoder indicates success
				decoder_accept[error_count]++;
				break;
			case -1:
				// Decoder indicates header RS decode unsuccessful
				decoder_reject_header[error_count]++;
				break;
			case -2:
			case -3:
				// Decoder indicates payload decode unsuccessful
				decoder_reject_payload[error_count]++;
				break;
			case -4:
				// Decoder indicates CRC mismatch
				decoder_reject_crc[error_count]++;
			}


			// compare the decoded packet to the original packet
			if (CompareVectors(il2p_decoded_packet, kiss.Output, kiss.OutputCount)) {
				// The packets differ.
				if (il2p_trx.Result > 0) {
					// But decoder indicated success
					undetected_failures[error_count]++;
				}
			} else {
				// The packets are the same
				actual_successes[error_count]++;
			}


			// if (il2p_trx.Result == 1) {
			// 	printf("\r\nPacket received.");
			// 	il2p_rx_count = il2p_trx.RxByteCount;
			// }
			// for (int i = 0; i < il2p_rx_count; i++) {
			// 	printf(" %2x", il2p_decoded_packet[i]);

			// }
			// int decode_CRC = CCITT16CalcCRC(il2p_decoded_packet, il2p_rx_count);
			// printf(" CRC: %4x", decode_CRC);
		
		}
	}


	printf("\r\nDecode Success by Error Count:");
	for (int i = 0; i <= max_errors; i++) {
		printf("\r\n%i, %i", i, actual_successes[i]);
	}
	printf("\r\nDecoder Indicated Success by Error Count:");
	for (int i = 0; i <= max_errors; i++) {
		printf("\r\n%i, %i", i, decoder_accept[i]);
	}
	printf("\r\nDecoder Rejected for Header by Error Count:");
	for (int i = 0; i <= max_errors; i++) {
		printf("\r\n%i, %i", i, decoder_reject_header[i]);
	}
	printf("\r\nDecoder Rejected for Payload by Error Count:");
	for (int i = 0; i <= max_errors; i++) {
		printf("\r\n%i, %i", i, decoder_reject_payload[i]);
	}
	printf("\r\nDecoder Rejected for CRC by Error Count:");
	for (int i = 0; i <= max_errors; i++) {
		printf("\r\n%i, %i", i, decoder_reject_crc[i]);
	}
	printf("\r\nDecoder Detection Failures by Error Count:");
	for (int i = 0; i <= max_errors; i++) {
		printf("\r\n%i, %i", i, decoder_no_detect[i]);
	}
	printf("\r\nUndetected Failures by Error Count:");
	for (int i = 0; i <= max_errors; i++) {
		printf("\r\n%i, %i", i, undetected_failures[i]);
	}

	printf("\r\nDone.\r\n");
}