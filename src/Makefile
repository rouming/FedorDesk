#
# Makefile for AVR ATMega16
# Good example of makefiles for AVR
# can be taken from
# http://www.obdev.at/products/vusb/index.html
# e.g. this: examples/hid-mouse/firmware/Makefile
#
CC=avr-gcc
MMCU=atmega16

F_CLK=F_CLK=1000000UL
F_CPU=F_CPU=1000000UL
CFLAGS=-c -O2 -Wall -gdwarf-2 -mmcu=$(MMCU) -std=gnu99 -D$(F_CLK) -D$(F_CPU)
LFLAGS=-mmcu=$(MMCU)

all: fedordesk.elf fedordesk.hex


fedordesk.elf: main.o fedordesk.o
	$(CC) $(LFLAGS) main.o fedordesk.o -o fedordesk.elf

# Extract intel hex for flash burning
fedordesk.hex:
	avr-objcopy -j .text -j .data -O ihex fedordesk.elf fedordesk.hex

# Flash burning
# to avoid sudo place udev rule for USBASP as /etc/udev/rules.d/usbasp.rules:
# SUBSYSTEMS=="usb", ATTRS{idVendor}=="16c0", ATTRS{idProduct}=="05dc", GROUP="users", MODE="0666"
flash: all
	avrdude -c usbasp -p m16 -u -U flash:w:fedordesk.hex

clean:
	rm -rf *o *~ fedordesk.elf fedordesk.hex
