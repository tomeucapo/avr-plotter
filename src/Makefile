MCU = at90s8515
PRG = stk200
MCU_PRG = 8515
TRG = plotter
SRC = $(TRG).c stdiolite.c timer.c lcd.c serie.c coa.c
CFLAGS = -O -Wstrict-prototypes -mmcu=$(MCU) -Wa,-ahlms=$(<:.c=.lst) 
LDFLAGS = -Wl,-Map=$(TRG).map,--cref 
FORMAT = ihex
CC = avr-gcc
OBJCONV = avr-objcopy
COMPILE = $(CC) -mmcu=$(MCU) -Wa,-ahlms=$(<:.c=.lst)

all:		$(TRG)

plotter:	$(TRG).c timer.o lcd.o keypad.o serie.o coa.o motors.o 
		$(COMPILE) -o $(TRG).elf $(TRG).c timer.o motors.o lcd.o keypad.o serie.o coa.o -lm 
		
		$(OBJCONV) -O ihex $(TRG).elf $(TRG).hex
		size $(TRG).elf

lcd:		lcd.c lcd.h utils.h timer.h
		$(CC) -c lcd.c

serie:		serie.c serie.h coa.h
		$(CC) -c serie.c

timer:		timer.c timer.h
		$(CC) -c timer.c
	
motors:		motors.c motors.h timer.h utils.h
		$(CC) -c motors.c
		
coa:		coa.c coa.h 
		$(CC) -c coa.c

burn:
		avrdude -c $(PRG) -p $(MCU_PRG) -e		
		avrdude -c $(PRG) -p $(MCU_PRG) -i $(TRG).hex

clean:		
		rm -f $(TRG).elf $(TRG).hex *.o *.lst
