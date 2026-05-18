all: il2p-test

il2p-test:
	gcc src/ax25.c src/crc.c src/gf2.c src/il2p.c src/intmath.c src/kiss-frame-handlers.c src/lfsr.c src/rs2.c src/main.c src/vector_errors.c -o il2p-test
clean:
	rm -f ./bin/*
	rm -f ./il2p-test
	rm -f ./il2p-test.exe
