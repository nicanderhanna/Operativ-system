# Operativ-system
For the course ID1206 HT25 Operating Systems (50423)

compile Q_6_A3
gcc -o Q_6_A3 Q_6_A3.c -Wall
/usr/bin/time --verbose ./Q_6_A3 128

compile Q_7_A3
gcc Q_7_A3.c -o Q_7_A3
Run Q_7_A3
./Q_7_A3 4096
./Q_7_A3 8192
./Q_7_A3 16384
/usr/bin/time -v ./Q_7_A3.out 16384

compile Q_8_A3
gcc -pthread -o Q_8_A3 Q_8_A3.c

Run Q_8_A3
./Q_8_A3 64 20
./Q_8_A3 64 200
./Q_8_A3 64 2000
