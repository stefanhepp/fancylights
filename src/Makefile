OPTIONS=-mmcu=atmega48p -Wall -Os
DEPS=uart.c uart.h command.h
SOURCES=uart.c

all: fancycontrol keypad projector

.PHONY: fancycontrol keypad projector
fancycontrol: fancycontrol.hex
keypad: keypad.hex
projector: projector.hex

%.elf: %.c ${DEPS}
	avr-gcc ${OPTIONS} -o $@ $< ${SOURCES}

%.hex: %.elf
	avr-objcopy -j .text -j .data -O ihex $^ $@

.PHONY: clean
clean:
	rm -f *.elf *.hex *.o
