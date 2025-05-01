MCU = atmega32
F_CPU = 16000000UL
CC = avr-gcc
OBJCOPY = avr-objcopy
AVRDUDE = avrdude
PROGRAMMER = atmelice_isp
PORT = usb

CFLAGS = -Wall -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU) -I include/
LDFLAGS = -mmcu=$(MCU)

TARGET = main
SRC := $(wildcard src/*.c)
OBJS := $(patsubst src/%.c, build/%.o, $(SRC))

all: build/$(TARGET).hex

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

build/$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

build/$(TARGET).hex: build/$(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

flash: build/$(TARGET).hex
	$(AVRDUDE) -c $(PROGRAMMER) -p m32 -P $(PORT) -U flash:w:build/$(TARGET).hex:i

fuse:
	$(AVRDUDE) -c $(PROGRAMMER) -p m32 -P $(PORT) \
	  -U lfuse:w:0xFF:m \
	  -U hfuse:w:0xD9:m

clean:
	rm -f build/*