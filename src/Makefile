CC=avr-gcc
MMCU=atmega16

F_CLK=F_CLK=1000000UL
CFLAGS=-c -Wall -gdwarf-2 -mmcu=$(MMCU) -std=gnu99 -D$(F_CLK)
LFLAGS=-mmcu=$(MMCU)

all: fedordesk.elf fedordesk.hex


fedordesk.elf: main.o fedordesk.o
	$(CC) $(LFLAGS) main.o fedordesk.o -o fedordesk.elf

# Extract intel hex for burning
fedordesk.hex:
	avr-objcopy -j .text -j .data -O ihex fedordesk.elf fedordesk.hex

# Burning
# to avoid sudo place udev rule /etc/udev/rules.d/usbasp.rules
# SUBSYSTEM=="usb", SYSFS{idVendor}=="1781", SYSFS{idProduct}=="0c9f", GROUP="users", MODE="0666"
flash: all
	avrdude -c usbasp -p m16 -u -U flash:w:fedordesk.hex

clean:
	rm -rf *o *~ fedordesk.elf fedordesk.hex
