prog1: main.c
	gcc main.c -o prog1

prog2: main2.c
	gcc main2.c -o prog2

my3proc: main3.c
	gcc -o my3proc main3.c

clean:
	rm -f my3proc