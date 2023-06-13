all: simulator

hw7.tar: cashier.c
	tar cf $@ $<

simulator: cashier.c main.c
	gcc -Wall -Wextra -std=c11 -g cashier.c main.c -o $@ 
	./simulator

clean:
	rm -rf simulator
	rm -f hw7.tar

.PHONY: clean
