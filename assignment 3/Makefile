# Makefile för ID1206 HT25 - Laboration A3


CC = gcc
CFLAGS = -Wall
LDFLAGS_Q8 = -pthread

all: Q_6_A3 Q_7_A3 Q_8_A3

clean:
	rm -f Q_6_A3 Q_7_A3 Q_8_A3 *.o

Q_6_A3: Q_6_A3.c
	$(CC) -o $@ $< $(CFLAGS)

run_q6: Q_6_A3
	echo "--- Kör Q_6_A3 med N=128 ---"
	/usr/bin/time --verbose ./Q_6_A3 128


Q_7_A3: Q_7_A3.c
	$(CC) $< -o $@ $(CFLAGS)

run_q7: Q_7_A3
	echo "--- Kör Q_7_A3 med N=4096 ---"
	./Q_7_A3 4096
	echo ""
	echo "--- Kör Q_7_A3 med N=8192 ---"
	./Q_7_A3 8192
	echo ""
	echo "--- Kör Q_7_A3 med N=16384 ---"
	./Q_7_A3 16384
	echo ""
	echo "--- Kör Q_7_A3 (time -v) med N=16384 ---"
	/usr/bin/time -v ./Q_7_A3 16384



Q_8_A3: Q_8_A3.c
	$(CC) $(LDFLAGS_Q8) -o $@ $< $(CFLAGS)

run_q8: Q_8_A3
	echo "--- Kör Q_8_A3 med N=64 och P=20 ---"
	./Q_8_A3 64 20
	echo ""
	echo "--- Kör Q_8_A3 med N=64 och P=200 ---"
	./Q_8_A3 64 200
	echo ""
	echo "--- Kör Q_8_A3 med N=64 och P=2000 ---"
	./Q_8_A3 64 2000
