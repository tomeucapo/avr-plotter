#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/io8515.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "utils.h"
#include "timer.h"
#include "motors.h"
#include "lcd.h"
#include "keypad.h"
#include "serie.h"

void menu_motor(void);
void menu_plot(void);
void plot_interpreter(void);
char plotcmdParse(void);
void logoCmdHome();
void logoCmdForward(float);
float numberParse();
void buida_buffer();

#define atof(s) strtod(s, (char **)0)
#define PRINTBUF_SIZE 64

#define MAX_X 1190.453
#define MAX_Y 1000.000

#define X_FACTOR  14.1364 
#define Y_FACTOR  14.1364

// Buffers temporals per l'entrada tant pel port serie com pel teclat

unsigned char *buf;
unsigned char PrintBuf[PRINTBUF_SIZE];

// Valors actuals X Y

int currX, currY;

int main(void)
{

    // Inicialització general (Ports, Timers, UART, Teclat i LCD)
    
    cli();                
    initPorts();
    initTimers();
    MotorDelayInit();
    sei();
    
    initUART();
    initKeypad();
    initLcd();

    //***********************************************************

    cls_lcd();
    print_lcd("AVR-Plotter 1.0");
    delay(500);
    cls_lcd();
    
    putchar_serie(12);           // Enviam una senyal al PC, per saber que ens hem possat en
                                 // marxa !!
    unsigned char c;
    float x,y;

    //****************** Menu principal *************************
    
    for(;;) {
            print_lcd("1.Motor  2.ADC  \n3.LDraw  4.Plot");
            c = getchar_keypad();
            switch(c) { 
                    
                   case '1'://**************** Control del capçal i ploma ***********
                            
                            menu_motor();
                            break;
                            
                   case '2'://**************** Lectura del ADC **********************

                            while (!iskeypressed()) {
                                   setpos_lcd(1,1);print_lcd("Valor ADC = ");
                                   
                            }
                            
                            break;
                            
                   case '3'://**************** Traçat de linies *********************  
                            
                            
                            cls_lcd();
                            print_lcd("X = ");
                           
                            cursor_on();
                            
                            buida_buffer();
                            Entrada_Teclat(PrintBuf, sizeof(PrintBuf));
                            x = atof(PrintBuf);
                            
                            setpos_lcd(2,1);print_lcd("Y = "); 
                            buida_buffer();
                            Entrada_Teclat(PrintBuf, sizeof(PrintBuf));
                            y = atof(PrintBuf);
                            
                            cursor_off();
                            
                            motors(1);
                            SolenoidActivate();
                            MotorBresLine(x, y);
                            SolenoidDeactivate();
                            currX = x;currY = y;
                            motors(0);
                            
                            break;
                            
                   case '4'://**************** Habilitació del interpret PC *******
                            
                            plot_interpreter();
                            break;
            }
            cls_lcd();
    }
    
}

/**********************************************************************************
 * Menu del control del capçal (Motors X i Y)
 **********************************************************************************/

void menu_motor(void)
{ 
     cls_lcd();
     print_lcd("Posicio:");
     MotorDelayInit();
     motors(1);

     unsigned char c=0x00, penstate=0;

     while(c!='\b') {
           c = getchar_keypad();
           
           switch(c) {  
                   case '2':if (currY < MAX_Y) {           // Motor eix Y's
                                  MotorStepXY(0, 1);
                                currY++;
                            }
                            break;
                            
                   case '8':if (currY > 0) {
                                MotorStepXY(0, -1);
                                currY--;
                            }
                            break;
                            
                   case '4':if (currY < MAX_X) {           // Motor eix X's
                                MotorStepXY(1, 0);
                                currX++;
                            }
                            break;
                            
                   case '6':if (currX > 0) {
                                 MotorStepXY(-1, 0);
                               currX--;
                            }
                            break;
                            
                   case '5':penstate = penstate ^ 0xff;    // Control de la ploma
                            setpos_lcd(1,16);print_lcd(penstate ? "U" : "D");
                            
                            if (penstate) 
                                SolenoidActivate();
                            else
                                SolenoidDeactivate();
                            
           }
           setpos_lcd(2,1);
           print_lcd("X =     Y =     ");
           setpos_lcd(2,5);
           pinta_valor(currX);
           setpos_lcd(2,13);
           pinta_valor(currY);
     }
     motors(0);
}

/**********************************************************************************
 * Habilitació del intèrpret de comandes remotes per RS-232
 **********************************************************************************/

void plot_interpreter(void)
{
     cls_lcd();
     print_lcd("PC Ctrl Remot:");
     putchar_serie('%');
     
     for(;;) {
          llegeix_serie(PrintBuf, sizeof(PrintBuf));
          buf = PrintBuf;
          if (!*buf)
              break;
          if (iskeypressed()) break;
          if (plotcmdParse()) break;
     }

     motors(0);                      // Ens asseguram que els motors estan aturats
     cbi(PORTD,7);
}

/**********************************************************************************
 * Parsejador de les comandes, interpret
 **********************************************************************************/

int intNumber;

char plotcmdParse(void)
{
         register char *buf1 = buf;
        register char c;

        float newx, newy;

        c = *buf1++;

        setpos_lcd(2,1);        
        print_lcd("Cmd: ");

        sbi(PORTD, 7);
        while(c)
        {
                while(c && c==' ')
                      c = *buf1++;
                buf = buf1;
                switch (c)
                {
                        case 'U':
                                SolenoidDeactivate();
                                print_lcd("Pen Up  ");
                                break;

                        case 'D':
                                SolenoidActivate();
                                print_lcd("Pen Down");
                                break;
                                
                        case 'L':
                                newx = numberParse();
                                newy = numberParse();
                                
                                print_lcd("Linia   ");
                                motors(1);                    // Activam els motors
                                MotorBresLine(newx, newy);
                                
                                currX = newx;currY = newy;
                                motors(0);                    // Desactivam els motors
                                break;
                                
                        case 'E':
                                return 1;
                }

                putchar_serie('#');putchar_serie('\n');

                buf1 = buf;
                c = *buf1++;
        }    
        return 0;
}

float numberParse()
{
        register char *buf1 = buf;
        register char c;

        int number = 0, base=1;
        char minusSign = 0;

        c = *buf1++;
        while(c && c == ' ')        // Salta els espais 
        { c = *buf1++; };

        if (c=='-')
        {
                minusSign = 1;
                c = *buf1++;
        }

        while(c>='0' && c<='9')
        {
                c -= '0';
                number = number * 10 + c;
                c = *buf1++;
        }

        if (c == '.')
        {
                c = *buf1++;
                while(c>='0' && c<='9')
                {
                        c -= '0';
                        number = number * 10 + c;
                        base = base * 10;
                }
        }
        
        if (minusSign)
            number = -number;

        buf = buf1-1;
        intNumber = number;

        return (float)number/base;
}

void buida_buffer()
{
     unsigned char i;
     for(i=0;i<=PRINTBUF_SIZE;i++)
         PrintBuf[i] = 0;
}

