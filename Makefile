signals: signal.c
	gcc signal.c -o signalab 

all: signal1 signal2 timer

signal1: signal1.c
	gcc -o signal1 signal1.c

signal2: signal2.c
	gcc -o signal2 signal2.c

timer: timer.c
	gcc -o timer timer.c

clean:
	rm -f signal1 signal2 timer