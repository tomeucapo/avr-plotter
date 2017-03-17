/**************************************************************************
 * timer.c 
 * Modul general per la gestió dels timers. Ens inicialitza els ports del
 * uC, i el Timer 0.
 * 
 * Aqui també configuram la rutina d'interrupció que va associada amb el
 * TIMER0 perque ens faci una sèrie de tasques de reconeixement cada cert
 * temps. Com per exemple el polling del teclat i de l'estat dels fi de
 * cursa.
 *
 * Tomeu Capó Capó 2004 (C)
 * Last Modify: 29/06/2004
 **************************************************************************/

#include <avr/io.h>
#include <avr/signal.h>
#include "timer.h"

void initPorts()
{                            // ********** Dispositius ********
     PORTA=0xA2;DDRA=0xFB;   // Motor X i ADC
     PORTB=0x00;DDRB=0xFF;   // Motor Y i Teclat
     PORTC=0xFF;DDRC=0xFF;   // LCD i LED Verd
     PORTD=0x00;DDRD=0xC0;   // RS-232, uSwitches i LED Vermell
}

// Timer ajustat per emprar CK/64 = 8Mhz/64 (un cada 8usecs)
// 8us x 125 = 1ms
// 256 - 125 = 131
// 

#define TxVal 131        

void initTimers()
{
     outp(0x82, TIMSK);               // Inicialitzam el Timer 0 i les interrupcions del Timer
     outp(0x03, TCCR0);
     outp(TxVal, TCNT0);
}

unsigned volatile short MilliSec = 0; 
unsigned short oMilliSec         = 0;

unsigned char KeyPollMS = 0;
unsigned char Seconds        = 0;

/**************************************************************************
 * Rutina d'interrupció del TIMER0, es dispara quan el temporitzador ha
 * arribat al final.
 **************************************************************************/

SIGNAL(SIG_OVERFLOW0)
{
        outp(TxVal, TCNT0);    // Tornam a carregar el valor del temporitzador

        MilliSec++;
        oMilliSec++;

        //***************** Cada 20ms ***************************
        
        KeyPollMS ++;
        if (KeyPollMS == 20) {
            KeyPollMS = 0;

            polling_teclat();           // Escanejar el teclat (Polling).
            polling_fc();               // Escaneig del estat dels finals de carrera.
        }

        //**************** Cada 1 segon *************************
        
        if (oMilliSec == 1000) {
            oMilliSec = 0;
            Seconds++;
        
            // LED d'estat del core (LED-Verd)
                
            if(bit_is_set(PINC, 0)) cbi(PORTC, 0); else sbi(PORTC, 0);
                
            if (Seconds == 60)
                Seconds = 0;
        }
}

/**************************************************************************
 * Rutina bàsica de retard utilitzant el TIMER0, utilizant la variable
 * de comptatge MilliSec
 **************************************************************************/

void delay(unsigned short milliSec)
{
        unsigned short totalMilliSec = MilliSec + milliSec;

        do { } while(MilliSec != totalMilliSec);
}
