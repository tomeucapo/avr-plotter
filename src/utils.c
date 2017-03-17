#include <avr/io.h>
#include "global.h"

/******************************************************************
 * Rutines de retard
 */

void delay_meu(u08 delay)
{
	u08 i,j;

	for (i=0; i<delay; i++)
  	{   					// delay loop
       	     j++;
        }
}


void long_delay(u08 del)
{
	u08 i;

        for (i=0; i<del; i++)
        {   					// delay loop
       	     delay_meu(255);
        }
}

/*******************************************************************
 * Rutines per treballar amb el SIPOs (Serial-In-Parallel-Out-Regs)
 * (Connectats per defècte al PORTB)
 *******************************************************************/

#define PORT_SIPOS 	PORTB

void clk_sipo(void)               // Genera un pols pel CK
{
     sbi(PORT_SIPOS, 3);
     cbi(PORT_SIPOS, 3);
}

void sipo(unsigned int luku)
{
     unsigned int maski, i;
     maski = 0x0001;
     i = 16;
     while (i) {
          if(luku & maski) {    // Miram el que hem d'enviar si un 1 o un 0.
             sbi(PORT_SIPOS, 0);
          } else {
             cbi(PORT_SIPOS, 0);
          }
          clk_sipo();           // Enviam un click al(s) 595
          maski <<=1;
          i--;
     }

     sbi(PORT_SIPOS, 2);
     cbi(PORT_SIPOS, 2);
}

void sipos_init(void)
{
     sbi(PORT_SIPOS, 4);  // Reset del 595
     cbi(PORT_SIPOS, 1);  // Activam OE
     cbi(PORT_SIPOS, 3);  // Shf. Reg. Clk
     cbi(PORT_SIPOS, 2);  // Sto. Reg. Clk
}

void clear_sipo(void)
{
     cbi(PORT_SIPOS, 4);     // Reset 595
     sbi(PORT_SIPOS, 4);
}

