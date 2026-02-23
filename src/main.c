#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "il2p.h"
#include "kiss.h"
#include "kiss-frame-handlers.h"
#include "crc.h"
#include "vector_errors.h"
#include "ax25.h"

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
	for (int i = 0; i < arg_count; i++) {
		printf("%s ", arg_values[i]);
	}
	
	if (arg_count < 10) {
		printf("Not enough arguments.\r\n");
		printf("Usage:\r\nil2p-test <header restriction> <sync tolerance> <payload length> <bits per symbol> <low ser> <high ser> <steps> <runs> <seed>\r\n");
		printf("\r\nExample: il2p-test 0 0 50 1e-6 1e-1 10 1000 0");
		printf("\r\n\n     header restriction:");
		printf("\r\n              0 - No restriction.");
		printf("\r\n                  Equally distributed translatable and non-translatable headers.");
		printf("\r\n              1 - Only random translatable headers.");
		printf("\r\n              2 - Only random non-translatable headers.");
		printf("\r\n              3 - Only random translatable UI headers.");
		printf("\r\n\n     sync tolerance:");
		printf("\r\n              Integer number of bits in error allowable for syncword match.");
		printf("\r\n\n     payload length:");
		printf("\r\n              Integer number representing packet payload byte count. For a");
		printf("\r\n              header-only packet, set this to 0. Maximum payload length is 1023.");
		printf("\r\n\n     bits per symbol:");
		printf("\r\n              Integer number of bits per modulator symbol, in the range 1 to 8.");
		printf("\r\n\n     low ser:");
		printf("\r\n              Float number, starting symbol error rate for trials.");
		printf("\r\n\n     high ser:");
		printf("\r\n              Float number, ending symbol error rate for trials.");
		printf("\r\n\n     steps:");
		printf("\r\n              Integer number of bit error rate sample steps, exponentially");
		printf("\r\n              distributed.");
		printf("\r\n\n     runs:");
		printf("\r\n              Integer number of random test cases at each error count. The");
		printf("\r\n              program will generate a random packet of specified length for each");
		printf("\r\n              run, and corrupt the packet with bit errors according to specified");
		printf("\r\n              bit error rate.");
		printf("\r\n\n     seed:");
		printf("\r\n              Integer number used to seed random number generator, for test");
		printf("\r\n              repeatability.");
		printf("\r\n");

		return(-1);
	}

	int header_restriction = atoi(arg_values[1]);
	int sync_tolerance = atoi(arg_values[2]);
	int payload_length = atoi(arg_values[3]);
	int bits_per_symbol = atof(arg_values[4]);
	double low_ser = atof(arg_values[5]);
	double high_ser = atof(arg_values[6]);
	int steps = atoi(arg_values[7]);
	int run_count = atoi(arg_values[8]);
	int seed = atoi(arg_values[9]);
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

	if (bits_per_symbol < 1) {
		printf("Bits per symbol %i is too small. Must be greater than zero.\r\n", bits_per_symbol);
		return(-1);
	}

	if (bits_per_symbol > 8) {
		printf("Bits per symbol %i is too large. Must be less than 8.\r\n", bits_per_symbol);
		return(-1);
	}

	if (low_ser <= 0) {
		printf("Low ser %f is too small. Must be greater than zero.", low_ser);
		return(-1);
	}

	if (high_ser >= 1) {
		printf("High ser %f is too large. Must be less than 1.", high_ser);
		return(-1);
	}

	if (steps <= 0) {
		printf("Step count %i is too small. Must be greater than zero.", steps);
		return(-1);
	}

	if (run_count > 999999999) {
		printf("Run count %i is too large. Must be less than 1 billion.", run_count);
		return(-1);
	}

	

	IL2P_TRX_struct il2p_trx;
	InitIL2P(&il2p_trx);
	il2p_trx.SyncTolerance = sync_tolerance;

	AX25_Receiver_struct ax25_rx;
	InitAX25(&ax25_rx);

	KISS_struct kiss;

	int il2p_reject_header[MAX_BUFFER];
	int il2p_reject_payload[MAX_BUFFER];
	int il2p_reject_crc[MAX_BUFFER];
	int il2p_no_detect[MAX_BUFFER];
	int il2p_accept[MAX_BUFFER];
	int il2p_success[MAX_BUFFER];
	int il2p_undetected[MAX_BUFFER];
	int ax25_no_detect[MAX_BUFFER];
	int ax25_accept[MAX_BUFFER];
	int ax25_success[MAX_BUFFER];
	int ax25_undetected[MAX_BUFFER];
	double tgt_ser_record[MAX_BUFFER];
	double meas_ser_record[MAX_BUFFER];

	for (int i = 0; i <= MAX_BUFFER; i++) {
		il2p_reject_header[i] = 0;
		il2p_reject_payload[i] = 0;
		il2p_reject_crc[i] = 0;
		il2p_accept[i] = 0;
		il2p_success[i] = 0;
		il2p_undetected[i] = 0;
		il2p_no_detect[i] = 0;
		ax25_no_detect[i] = 0;
		ax25_accept[i] = 0;
		ax25_success[i] = 0;
		ax25_undetected[i] = 0;
		meas_ser_record[i] = 0;
	}

	int ax25_source_packet[MAX_BUFFER];
	uint8_t il2p_encoded_packet[MAX_BUFFER];
	uint8_t il2p_decoded_packet[MAX_BUFFER];
	uint8_t ax25_encoded_packet[MAX_BUFFER];
	uint8_t ax25_decoded_packet[MAX_BUFFER];
	uint8_t corrupt_il2p_packet[MAX_BUFFER];
	uint8_t corrupt_ax25_packet[MAX_BUFFER];
	int il2p_error_vector[MAX_BUFFER];
	int ax25_error_vector[MAX_BUFFER];

	double ser_base = pow(high_ser/low_ser, 1/(double)(steps-1));
	//printf("\r\n symbol_error_rate step base: %f", ser_base);

	printf("\r\nStarting %li trials.", (long int)steps * (long int)run_count);
	long int master_count = 1;
	int prog_bar_segs = 70;
	long int print_interval = ((long int)steps * (long int)run_count) / (long int)prog_bar_segs;
	printf("\r\n");
	for (int i = 0; i <= prog_bar_segs; i++) {
		printf(" ");
	}
	printf("]\r[");
	fflush(stdout);

	double symbol_error_rate = low_ser;
	
	for (int ser_index = 0; ser_index < steps; ser_index++) {
		long int symbol_error_sum = 0;
		long int symbol_sum = 0;
		for (int run_number = 1; run_number <= run_count; run_number++) {
			if ((master_count++ % (long int)print_interval) == 0 ) {
				printf("=");
				fflush(stdout);
			}

			int translatable = 0;
			// If the header is not translatable, reduce the maximum payload length to make room for the header in the payload.
			int adj_payload_length = payload_length - 16;
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
			if (kiss.RipValid) {
				// Resture full length payload.
				adj_payload_length = payload_length;
			}

			// Create packet payload.
			kiss.OutputCount += GenRandomBytes(&kiss.Output[kiss.OutputCount], adj_payload_length);

			// Encode packet in IL2P format
			int il2p_tx_count = IL2PBuildPacket(&kiss, il2p_encoded_packet, &il2p_trx);

			// Generate an error vector.
			symbol_error_sum += GenSERErrorVector(il2p_error_vector, 8, il2p_tx_count, bits_per_symbol, symbol_error_rate);
			symbol_sum += (il2p_tx_count * 8) / bits_per_symbol;

			// Add noise to IL2P packet.
			for (int i = 0; i < il2p_tx_count; i++) {
				corrupt_il2p_packet[i] = il2p_error_vector[i] ^ il2p_encoded_packet[i];
			}

			IL2PReceive(&il2p_trx, corrupt_il2p_packet, il2p_tx_count, il2p_decoded_packet);
			switch(il2p_trx.Result) {
			case 0:
				// No packet detection.
				il2p_no_detect[ser_index]++;
				break;
			case 1:
				// Decoder indicates success
				il2p_accept[ser_index]++;
				break;
			case -1:
				// Decoder indicates header RS decode unsuccessful
				il2p_reject_header[ser_index]++;
				break;
			case -2:
			case -3:
				// Decoder indicates payload decode unsuccessful
				il2p_reject_payload[ser_index]++;
				break;
			case -4:
				// Decoder indicates CRC mismatch
				il2p_reject_crc[ser_index]++;
			}

			// compare the decoded packet to the original packet
			if (CompareVectors(il2p_decoded_packet, kiss.Output, kiss.OutputCount)) {
				// The packets differ.
				if (il2p_trx.Result > 0) {
					// But decoder indicated success
					il2p_undetected[ser_index]++;
				}
			} else {
				// The packets are the same
				il2p_success[ser_index]++;
			}
			
			// Perform AX.25 Encoding.
			int ax25_tx_bit_count = AX25BuildFrame(kiss.Output, kiss.OutputCount, ax25_encoded_packet, 0);
			int ax25_tx_byte_count = ax25_tx_bit_count / 8;
			if (ax25_tx_bit_count - (ax25_tx_byte_count * 8) > 0) {
				ax25_tx_byte_count++;
			}

			// Generate an error vector.
			symbol_error_sum += GenSERErrorVector(ax25_error_vector, 8, ax25_tx_byte_count, bits_per_symbol, symbol_error_rate);
			symbol_sum += (ax25_tx_byte_count * 8) / bits_per_symbol;

			// Add noise to AX25 packet.
			for (int i = 0; i < ax25_tx_byte_count; i++) {
				corrupt_ax25_packet[i] = ax25_error_vector[i] ^ ax25_encoded_packet[i];
			}

			AX25Receive(&ax25_rx, corrupt_ax25_packet, ax25_tx_byte_count + 4, ax25_decoded_packet);
			switch(ax25_rx.Result) {
			case 0:
				// No packet detection
				ax25_no_detect[ser_index]++;
				break;
			case 1:
				// Decoder indicates success
				ax25_accept[ser_index]++;
				break;
			default:
				break;
			}

			// compare the decoded packet to the original packet
			if (CompareVectors(ax25_decoded_packet, kiss.Output, kiss.OutputCount)) {
				// The packets differ.
				if (ax25_rx.Result > 0) {
					// But decoder indicated success
					ax25_undetected[ser_index]++;
				}
			} else {
				// The packets are the same
				ax25_success[ser_index]++;
			}

		
		}
		tgt_ser_record[ser_index] = symbol_error_rate;
		meas_ser_record[ser_index] = (double)symbol_error_sum / (double)symbol_sum;
		symbol_error_rate *= ser_base;
	}

	printf("\r\n\nIL2P+CRC Observations");
	printf("\r\nRuns, %i", run_count);
	printf("\r\n      SER,  ");
	printf("Success,  ");
	printf("Hdr Rej,  ");
	printf("Pld Rej,  ");
	printf("CRC Rej,  ");
	printf("No Dtct,  ");
	printf("  False");
	for (int i = 0; i < steps; i++) {
		printf("\r\n%3.3e,%9i,%9i,%9i,%9i,%9i,%9i", tgt_ser_record[i], il2p_success[i], il2p_reject_header[i], il2p_reject_payload[i], il2p_reject_crc[i], il2p_no_detect[i], il2p_undetected[i]);
	}

	printf("\r\n\nAX.25 Observations");
	printf("\r\nRuns, %i", run_count);
	printf("\r\n      SER,  ");
	printf("Success,  ");
	printf("No Dtct,  ");
	printf("  False");
	for (int i = 0; i < steps; i++) {
		printf("\r\n%3.3e,%9i,%9i,%9i", tgt_ser_record[i], ax25_success[i], ax25_no_detect[i], ax25_undetected[i]);
	}

	printf("\r\n\nMeasured Symbol Error Rates");
	printf("\r\nBits Per Symbol, %i", bits_per_symbol);
	printf("\r\n  Tgt SER,  ");
	printf("Meas SER");
	for (int i = 0; i < steps; i++) {
		printf("\r\n%3.3e, %3.3e", tgt_ser_record[i], meas_ser_record[i]);
	}

	printf("\r\nDone.\r\n");

}
