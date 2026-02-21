# il2p-test
IL2P encoder and decoder implementations with random error testing. Repository contains example Reed Solomon encoder and decoder, including Galois Field arithmetic functions. IL2P implementation conforms to draft spec 0.6.
# Requirements
gcc or other c compiler stdlib and stdio
# Compiling with gcc
Recommend make a directory for the compiled binary, to easily exclude the binary from git commits.
```
mkdir bin
gcc -o bin/rs-test src/*.c
```
# Usage
```
il2p-test <header restriction> <sync tolerance> <payload length> <low ber> <high ber> <steps> <runs> <seed>
```
## Arguments
### header restriction:
* 0 - No restriction (equal distribution of translatable and non-translatable headers).
* 1 - Only random translatable headers.
* 2 - Only random non-translatable headers.
* 3 - Only random translatable UI headers.
IL2P "translatable headers" means the octets in the header portion of the packet are selected so their values can be translated to the available IL2P packet header fields.
### sync tolerance
Integer number of bits in error allowable as the decoder attempts to "match" the 24-bit IL2P header. Setting 0 requires an exact match, which causes the decoder to ignore some otherwise decodable packets in the noisy bitstream. Setting 1 or 2 typically results in higher valid decode counts, but also causes a small amount of packet loss in noiseless conditions, as the decoder finds loose syncword matches in other parts of the bitstream.
### payload length
Integer number of bytes of payload after the header. Valid values are 0 to 1023.
### low ber
Float number, Bit Error Rate setting for the first runs of test trials. Must be more than zero.
### high ber
Float number, Bit Error Rate setting for the last runs of test trials. Must be less than one.
### steps
Integer number of Bit Error Rate set points between **low ber** and **high ber**. Bit Error Rate set points are calculated in a power sequence, so the results are suitable for display on a logarithmic axis.
### runs
Integer number of trials to perform at each Bit Error Rate set point. Must be greater than zero and less than 1 billion.
### seed
Seed value for random number generator for repeatable tests.
# Invoke Example with Arguments
```
bin/il2p-test 0 0 50 1e-6 1e-1 11 1000 0
```
# Example Output
```
bin/il2p-test 0 0 50 1e-6 1e-1 11 1000 0 
Starting 11000 trials.
[======================================================================]

IL2P+CRC Observations
Trials at Each Bit Error Rate, 1000
      BER,  Success,  Hdr Rej,  Pld Rej,  CRC Rej,  No Dtct,    False
1.000e-06,     1000,        0,        0,        0,        0,        0
3.162e-06,     1000,        0,        0,        0,        0,        0
1.000e-05,      999,        0,        0,        0,        1,        0
3.162e-05,      999,        0,        0,        0,        1,        0
1.000e-04,     1000,        0,        0,        0,        0,        0
3.162e-04,      990,        0,        0,        0,       10,        0
1.000e-03,      970,        2,        0,        1,       27,        0
3.162e-03,      879,       46,        2,        0,       73,        0
1.000e-02,      506,      240,       32,        4,      218,        0
3.162e-02,        0,      371,       73,        0,      556,        0
1.000e-01,        0,       86,        8,        0,      906,        0

AX.25 Observations
Trials at Each Bit Error Rate, 1000
      BER,  Success,  No Dtct,    False
1.000e-06,     1000,        0,        0
3.162e-06,      997,        3,        0
1.000e-05,      994,        6,        0
3.162e-05,      979,       21,        0
1.000e-04,      955,       45,        0
3.162e-04,      852,      148,        0
1.000e-03,      603,      397,        0
3.162e-03,      199,      801,        0
1.000e-02,        6,      994,        0
3.162e-02,        0,     1000,        0
1.000e-01,        0,     1000,        0

Measured Bit Error Rates
  Tgt BER,  Meas BER
1.000e-06, 8.287e-07
3.162e-06, 4.139e-06
1.000e-05, 1.247e-05
3.162e-05, 3.479e-05
1.000e-04, 9.367e-05
3.162e-04, 3.173e-04
1.000e-03, 9.897e-04
3.162e-03, 3.223e-03
1.000e-02, 1.019e-02
3.162e-02, 3.190e-02
1.000e-01, 9.995e-02
Done.
```
# Explanation of Program Output
The program first validates the supplied arguments, then begins running trials. A progress bar is displayed while trials are running. Completion time depends largely on payload length and run count, and can take seconds or days. Numeric output is provided with comma delimiting for easy import into spreadsheet programs.
## Column Explanations
### Success
The decoder output matched the pre-encoded data exactly.
### Hdr Rej
The decoder rejected the packet because Reed-Solomon decoding of the header failed.
### Pld Rej
The decoder rejected the packet because Reed-Solomon decoding of the payload failed.
### CRC Rej
The decoder rejected the packet because the calculated CRC of the decoded packet did not match the encoded trailing CRC.
### No Dtct
The decoder did not detect a packet, even though the program generated a packet for the trial.
### False
The decoder generated a packet that passed all Reed-Solomon and CRC validity checks, but does not match the original data.
## Measured Bit Error Rates
These columns compare the requested Bit Error Rate to the actual Bit Error Rate generated by the pseudorandom number generator. Should be closer matches at higher run counts. Low Bit Error Rates tend to vary more than high Bit Error Rates.
