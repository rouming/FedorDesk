CC=avr-gcc
CFLAGS=-c -Wall -mmcu=atmega16

all: fedordesk

fedordesk: main.o fedordesk.o
	$(CC) main.o fedordesk.o -o fedordesk

clean:
	rm -rf *o *~ fedordesk