/*************************************************************************
 *
 * lcd.c
 * Mòdul pel control del LCD HD4480, que treballen mode de 4 bits.
 * Utilizam els 4 bits més significatius de les dades del LCD:
 * 
 * PC7 PC6 PC5 PC4 PC3 |  PC2 PC1 PC0
 * D7  D6  D5  D4  E   |  RW  RS  N/C
 *
 * Tomeu Capó Capó
 * 27/04/2004
 *************************************************************************/

#include <avr/io.h>
#include <avr/signal.h>
#include <avr/interrupt.h>
#include "lcd.h"

void delay40us(void);
void writeNibbleLcd(unsigned char ch);
void writeByteLcd(unsigned char ch);
void setLcdCmd(unsigned char ch);
void setPosLcd(unsigned char address);
void dataWriteLcd(unsigned char ch);

/*******************************************************************
 * Comandes LCD 
 * Control del display (ON/OFF)
 *         0 0 0 0 1 D C B
 *         D = display on, C = cursor on, B = cursor blink on
 */

#define DISPLAY_OFF                        0x08
#define DISPLAY_ON                        0x0f
#define CURSOR_ON                        0x0e
#define CURSOR_OFF                        0x0c

/*******************************************************************
 * Entry mode set
 *         0 0 0 0 0 1 I/D S
 *         I/D: 1=increment cursor pos, 0=decrement
 *         S: 1=display shift 0 = no display shift
 */

#define ENTRY_MODE_SET                0x06
#define CLEAR_DISPLAY                0x01
#define CURSOR_HOME                0x02
#define dd_mask                        0x80

#define RW                        2
#define EN                        1
#define RS                        3

unsigned char lcd_address = 0x00;

/*******************************************************************
 * Rutines de retard només per el LCD
 *******************************************************************/

/* Rutina de retard de 40usecs */

void delay40us()
{
     register unsigned char i, j;
     for(i=0; i<40; i++)
        for (j=0; j<8; j++)
          {}
}

void delay_lcd(unsigned int temp)
{
     register unsigned int i,j;
     for(i=0;i<=128;i++)
         for(j=0;j<=255;j++) {} 
}


/*******************************************************************
 * Per escriure els 4 bits del LCD
 *******************************************************************/

void writeNibbleLcd(unsigned char ch)
{
        unsigned char i;
        register unsigned char temp;

        ch &= 0xf0;                // Borram els 4 bits menys significatius
        temp = inp(LCDPORT);
        temp &= 0x0f;                // Borram els 4 bits més significatius
        temp |= ch;
        outp(temp, LCDPORT);
        
        asm("nop");
        asm("nop");

        cbi(LCDPORT, RW);        // Habilitam la senyal R/W = 0 (Per escriure)

        asm("nop");             // Retard de 2 cicles
        asm("nop");
        
        sbi(LCDPORT, EN);        // Habilitam el Enable = 1 


        // Retard per l'amplada de pols, nivell alt PWEH 
        for(i=0;i<40;i++) {}

        cbi(LCDPORT, EN);        // Deshabilitam el Enable = 0
        
        // LCD reacts to falling edge of E
        // Delay for address/data hold tAH, tH  (10 ns min)

        // Keep enable line low for some time  
        // Enable cycle time is 1000ns, 
        for(i=0;i<40;i++) {}
}

/*******************************************************************
 * Rutina per escriure un byte cap al LCD (nibble+nibble)
 *******************************************************************/

void writeByteLcd(unsigned char ch)
{
        writeNibbleLcd(ch);
        ch <<= 4;
        writeNibbleLcd(ch);
}

/*******************************************************************
 * Ens permet enviar una comanda al LCD
 *******************************************************************/

void setLcdCmd(unsigned char ch)
{
        cbi(LCDPORT, RS);  // RS = 0
        asm("nop");
        asm("nop");
        writeByteLcd(ch);
}

/*******************************************************************
 * Ens permet posicionar-mos per dins LCD, rutina emprada per
 * la funció setpos_lcd
 *******************************************************************/

void setPosLcd(unsigned char address)
{
        setLcdCmd(0x80 | address);
        delay40us();
}

/*******************************************************************
 * Rutina per escriure dades al LCD
 *******************************************************************/

void dataWriteLcd(unsigned char ch)
{
        sbi(LCDPORT, RS);  // RS = 1
        asm("nop");
        asm("nop");

        writeByteLcd(ch);
        delay40us();

}

/*******************************************************************
 * Rutina general d'inicialització del LCD
 *******************************************************************/
void initLcd()
{
        outp(0x00, LCDPORT);  // Buida el port a on està connectat el LCD
        outp(0xff, LCDDR);    // I configuram el port a on està connectat el LCD.

        // Esperam després de la posta en marxa
        delay_lcd(100);
        writeNibbleLcd(0x30);           // Pas 1
        // delay 4.1 ms
        delay_lcd(5);
        writeNibbleLcd(0x30);           // Pas 2
        //delay 100us
        delay_lcd(1);
        writeNibbleLcd(0x30);           // Pas 3

        writeNibbleLcd(0x20);           // Passam el LCD a mode de 4 bits

        // Function set  0 0 0 1 DL N F 0 0 
        //  DL=0 is 4 bit,   DL=0 is 8 bit
        //  N=1  is 2 lines, N=0  is 1 line
        //  F=0  is 5x7 font F=1  is 5x10 font
        
        writeByteLcd(0x28);             // DL = 0, N=1 , F=0
        delay40us();

        writeByteLcd(DISPLAY_OFF);      // Display OFF
        delay40us();

        writeByteLcd(CURSOR_OFF);
        delay40us();

        writeByteLcd(CLEAR_DISPLAY);
        delay_lcd(2);

        writeByteLcd(ENTRY_MODE_SET);
        delay40us();

        cls_lcd();
}

/*******************************************************************
 * Rutines de impresió de caràcters, posicionament i control del
 * cursor.
 *******************************************************************/

/* Ens imprimeix un caràcter per pantalla, emulant els caràcters de control \b \n ... */

void putchar_lcd(unsigned char ch)
{
        if (ch == '\b') {              // És el \b (Backspace) ?
           
           if (lcd_address > 0) {      // Si ho és i hi ha algun caràcter anterior, el borra!
               lcd_address--;
               setPosLcd(lcd_address);
               dataWriteLcd(' ');
               setPosLcd(lcd_address);
           }
           
        } else if (ch == '\n') {       // És el \n (salt de linia) ?
          
          if (lcd_address < 0x40) {    // Estic a la primera línia?
              lcd_address = 0x40;      // No, Botar a la següent linia
              setPosLcd(lcd_address);
          } else 
              cls_lcd();               // Si, borram la pantalla
          
        } else {                       // Si és un altre caràcter que no sigui de control, el pintam!
            
            dataWriteLcd(ch);
            lcd_address++;
            
            if (lcd_address == 0x28)
                lcd_address = 0x40;
            if (lcd_address == 0x68)
                lcd_address = 0x00;
        }
}

/* Ens borra el LCD */

void cls_lcd()
{
        setLcdCmd(CLEAR_DISPLAY);
        delay_lcd(2);
        lcd_address = 0x00;
}

/* Ens habilita el cursor */

void cursor_on()
{
        setLcdCmd(CURSOR_ON);
        delay40us();
}

/* Ens deshabilita el cursor */

void cursor_off()
{
        setLcdCmd(CURSOR_OFF);
        delay40us();
}

/* Ens permet posicionar-nos per LCD */

void setpos_lcd(unsigned char row, unsigned char col)
{
        unsigned char address = col - 1;

        if (row > 1)
            address += 0x40;
        setPosLcd(address);
        lcd_address = address;
}

/* Imprimir una cadena de caràcters per l'LCD */

void print_lcd(unsigned char *cad)
{
     unsigned char i=0;

     while(i<=strlen(cad)-1) {
          putchar_lcd(cad[i]);
          i++;
     }
}

/* Imprimir un valor numèric enter per pantalla */

void pinta_valor(unsigned int valor)
{
     unsigned char val_str[5];
     itoa(valor, val_str, 10);
     print_lcd(val_str);
}

