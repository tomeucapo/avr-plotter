/********************************************************************
 * Configuració del teclat i de les files i columnes:
 * 
 *   PB4 PB3
 *    0   0     Fila 1,2,3
 *    0   1     Fila 4,5,6
 *    1   0     Fila 7,8,9
 *    1   1     Fila *,0,#
 *
 *   PB0  Entrada columna 1,4,7,*
 *   PB1  Entrada columna 2,5,8,0
 *   PB2  Entrada columna 3,6,9,#
 ********************************************************************/

#ifndef KEYPAD_H
#define KEYPAD_H

#define KEYPADPORT  PORTB
#define KEYPADDR    DDRB
#define KEYPADPIN   PINB

void initKeypad(void);

static unsigned char altIndex = 100; 

#define keypad_numeric_mode() (altIndex = 100)
#define keypad_alpha_mode() (altIndex = 0)

char getchar_keypad(void);  
void Entrada_Teclat(char *buf, unsigned char length);  
unsigned char iskeypressed(void);

extern void putchar_lcd(unsigned char ch);		// Escriu un caràcter per LCD

#endif 
