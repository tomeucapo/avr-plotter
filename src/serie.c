/**************************************************************************
 * serie.c 
 * Modul per treballar amb la UART del Atmel. Igualment que hem fet amb el
 * teclat aqui tindrem un buffer per la recepci� de car�cters del port serie
 *
 * Tomeu Cap� Cap� 2004 (C)
 * Last Modify: 29/06/2004
 **************************************************************************/


#include <avr/io.h> 
#include <avr/signal.h>
#include <avr/interrupt.h>

#include "serie.h"
#include "coa.h"

struct coa coa_pserie;

/**************************************************************************
 * Rutina d'inicialitzaci� de la UART
 **************************************************************************/

void initUART(void)
{
	cli();                 // Deshabilitar interrupcions
	SetBaud9600;           // Configuram la UART a 9600Bps

	UCR = 0x98;            // Habilitam la transimissi� TX (TXEN)
	                       // Habilitam la recepci� RS (RXEN)
			       // Habilitam la interrupci� de la recepci� (RXCIE)
	
	sei();                 // Habilitar interrupcions

	coa_nova(&coa_pserie); // Cream una coa FIFO per el port serie
}

/**************************************************************************
 * Rutina per enviar car�cters al port serie
 **************************************************************************/

void putchar_serie(unsigned char c)
{
	loop_until_bit_is_set(USR, UDRE);
	outp(c, UDR);
}

/**************************************************************************
 * Rutina que ens va llegint els car�cters que hi ha dins el nostre buffer
 * del port serie.
 **************************************************************************/

unsigned char getchar_serie(void)
{
	char c = -1; 
	while (c == -1) {
   	       c = treu_element(&coa_pserie);
	}
	return c;
}

/**************************************************************************
 * Rutina que ens va llegint una seq��ncia de car�cters de manera "cuinada"
 * comprovant car�cters de control \n, \b ... I ens fa echo cap al
 * port serie.
 **************************************************************************/

void llegeix_serie(unsigned char *buf, unsigned char length)
{
	unsigned char n=0;
        char c;

	while(n < length)
	{
		c = getchar_serie();
		if (c == -1 || c == 0 || c == '\r')
			continue;

		if (c == '\b' && n > 0)           
		{
			n--;
			putchar_serie(c);
		}
		if (c == '\n')
			break;
		buf[n++] = c;
		putchar_serie(c);
	} 
	
	buf[n] = 0;
}

/**************************************************************************
 * Rutina d'interrupci� de recepci� de la UART. Tot el que rebem ho ficam
 * dins el buffer.
 **************************************************************************/

SIGNAL(SIG_UART_RECV)
{
       afageix_element(&coa_pserie, inp(UDR));
}

// Hi ha qualque car�cter en la coa del port serie?

unsigned char iscom(void)
{
       return !coa_buida(&coa_pserie);
}


