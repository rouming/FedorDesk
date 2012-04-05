CC=avr-gcc
MMCU=atmega16

F_CLK=F_CLK=1000000UL
CFLAGS=-c -Wall -gdwarf-2 -mmcu=$(MMCU) -std=gnu99 -D$(F_CLK)
LFLAGS=-mmcu=$(MMCU)

all: fedordesk.elf

fedordesk.elf: main.o fedordesk.o
	$(CC) $(LFLAGS) main.o fedordesk.o -o fedordesk.elf

clean:
	rm -rf *o *~ fedordesk.elf
