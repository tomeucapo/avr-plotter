/************************************************************
 *
 * coa.c
 * Mòdul per la gestió d'una coa FIFO utilizada per el buffer
 * del port serie i del teclat.
 *
 */

#include "coa.h"

void coa_nova(struct coa *q)
{
     q->capsalera = 0;
     q->darrer = 0;
}

void afageix_element(struct coa *q, char c)
{
     q->buffer[q->capsalera++] = c;

     if (q->capsalera == QUEUE_SIZE)
	 q->capsalera = 0;

     if (q->capsalera == q->darrer) {
	 q->capsalera--;
	 if (q->capsalera == -1)
             q->capsalera = QUEUE_SIZE-1;
     }
}

signed char treu_element(struct coa *q)
{
     register char c;

     if (q->capsalera == q->darrer)
	 return -1;

     c = q->buffer[q->darrer];
     q->darrer++;

     if (q->darrer == QUEUE_SIZE)
	 q->darrer = 0;

     return c;
}

unsigned char coa_buida(struct coa *q)
{
     return (q->capsalera == q->darrer);
}
