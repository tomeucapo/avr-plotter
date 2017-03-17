/**************************************************************************
 * keypad.c
 * Modul per el control de l'entrada del teclat de 3x3 numèric, amb la 
 * possibilitat de fer entrades numèriques/alfanumèriques. Lectura utilitzant
 * buffer (Coa FIFO).
 * 
 * Empram un decodificador 2/4 per reduïr el nombre de bits que utilitza
 * el uC.
 *
 * Tomeu Capó Capó 2004 (C)
 * Last Modify: 29/06/2004
 **************************************************************************/

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "keypad.h"
#include "coa.h"
#include "timer.h"

struct coa keyQueue;

/*************************************************************************
 * Rutina de inicialització del teclat
 *************************************************************************/

void initKeypad()
{
        unsigned char t;
        t = inp(KEYPADPORT);
        t = t & 0xe0;               // Borram els 5 bits de mes pes
        outp(t | 0x07, KEYPADPORT); // X,X,X,0,0,pup,pup,pup

        t = inp(KEYPADDR);
        t = t & 0xe0;               // Borram els 5 bits de mes pes
        outp(t | 0x18,  KEYPADDR);  // X,X,X,out,out,in,in,in
        
        coa_nova(&keyQueue);        // Cream la coa del teclat
}

/*************************************************************************
 * Mapa de caràcters del teclat
 *************************************************************************/

static unsigned char __attribute__((progmem)) keyArray[4][3] = 
        { {'1', '2', '3'}, {'4', '5', '6'}, {'7', '8', '9'}, {'\b', '0', '\n'}};

static unsigned char __attribute__((progmem)) alternateKeys[10][4] =
        { { '0', '.', 'Q', 'Z' }, { '1', ' ', '[', ']'}, {'2', 'A', 'B', 'C'}, {'3', 'D', 'E', 'F'},
          { '4', 'G', 'H', 'I' }, { '5', 'J', 'K', 'L'}, {'6', 'M', 'N', 'O'}, {'7', 'P', 'R', 'S'},
          { '8', 'T', 'U', 'V' }, { '9', 'W', 'X', 'Y'} };

/*************************************************************************
 * Funció que realitza el polling del teclat. Cridada cada mitg segon per
 * la rutina d'interrupció del TIMER0
 *************************************************************************/

void polling_teclat()
{
        static char lastKey = 0, lastAddedKey= 0;
        static unsigned char timeAfterKeyRelease = 255;

        unsigned char row, col, t;
        char newKey;

        // Escaneig de les 4 files, gràcies al decodificador 2/4 empram només dues
        // sortides del uC per definir les 4 files.

        for(row=0; row<4; row++)
        {        
                t = inp(KEYPADPORT);
                t = t & 0xe7;              // Borram el tercer i quart bit
                t = t | (row << 3);        // Possam el valor row als bits 3 i 4
        
                outp(t, KEYPADPORT);

                for(t=0; t<100; t++) {}    // Esperam un temps
                
                col = inp(KEYPADPIN);
                col &= 0x07;               // Comprovam l'estat de les 3 col.lumnes

                if (col != 0x07)           // Una tecla polsada
                   break;
        }

        if (row == 4)                      // Hem comprovat totes les files ?
        {
                lastKey = 0;

                // timeAfterKeyRelease és 0 només quan s'ha pitjat una tecla i
                // és mantè a 0 tot el temps que està pitjada.
                // Quan amollam la tecla el començam a incrementar fins a 100.
                
                if (timeAfterKeyRelease < 100)      
                        timeAfterKeyRelease++;      
                
                return;
        }

        // Si pitjam més d'una tecla
        // multiple keys might have been pressed, take the leftmost key
        
        if ((col & 0x04) == 0)      col = 2;    // Es la columna 3?
        else if ((col & 0x02) == 0) col = 1;    // Es la columna 2?
        else if ((col & 0x01) == 0) col = 0;    // Es la columna 1?

        // Agafa el caràcter corresponent del mapa 
        
        newKey = __LPM_classic__((int)(&keyArray[row][col]));

        // Si no s'havia pitjat cap tecla abans, però ara n'hi ha una de pitjada
        // ignoram aquesta tecla.
        
        if (lastKey == 0) {
                lastKey = newKey;
                return;
        }

        // Si el caràcter no és el mateix que l'anterior
        
        if (newKey != lastKey) {
                lastKey = newKey;
                return;
        }

        if (timeAfterKeyRelease == 0) {
                return;
        }

        if (lastAddedKey == newKey && timeAfterKeyRelease < 50 && newKey >= '0' && newKey <='9' && altIndex <= 3)
        {
                register char altChar = newKey;

                altIndex++;
                if (altIndex == 4)  
                    altIndex = 0;
                altChar = __LPM_classic__((int) &alternateKeys[(unsigned char)(newKey-'0')][altIndex]);
                afageix_element(&keyQueue, '\b');
                afageix_element(&keyQueue, altChar);
        } else {
                if (altIndex <= 3)
                    altIndex = 0;

                lastAddedKey = newKey;
                afageix_element(&keyQueue, newKey);
        }

        lastKey = newKey;
        timeAfterKeyRelease = 0;
}

/*****************************************************************************
 * Espera que es pitji una tecla, i ens torna el valor de la tecla
 * (Treu el primer element del buffer de teclat si n'hi ha. 
 *****************************************************************************/

char getchar_keypad()
{
        char c = -1; 
        while (c == -1)
        {
               c = treu_element(&keyQueue);
        }
        return c;
}

/*****************************************************************************
 * Llegeix una secuència de caràcters del buffer de teclat i els pinta per
 * el LCD. 
 *****************************************************************************/

void Entrada_Teclat(char *buf, unsigned char length)
{
        unsigned char n=0;
        char c;

        while(n < length)
        {
                c = getchar_keypad();
                if (c == -1 || c == 0 || c == '\r')
                        continue;

                // Si pitja la tecla # (Backspace)
        
                if (c == '\b')
                {
                        if (n > 0)
                        {
                                n--;
                                putchar_lcd(c);
                        }
                        continue;
                }
        
                // Si pitjam * (Enter)
                if (c == '\n')
                        break;
                buf[n++] = c;
                putchar_lcd(c);
        }
        buf[n] = 0;
}

/*****************************************************************************
 * Mira si el buffer de teclat està ple o buid? (S'han pitjat tecles?)
 *****************************************************************************/

unsigned char iskeypressed()
{
        return !coa_buida(&keyQueue);
}


