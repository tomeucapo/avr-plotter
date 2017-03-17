/**************************************************************************
 * motors.c 
 * Modul per el control de motors pas-a-pas per ús gràfic, aquest módul
 * té una sèrie de primitives gràfiques.
 *
 * Tomeu Capó Capó 2004 (C)
 * Last Modify: 29/06/2004
 **************************************************************************/

#include <avr/io.h>
#include <avr/io8515.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "utils.h"
#include "timer.h"

char stableLimitSwitches = 0;

#define MOTOR_XMAX  (((stableLimitSwitches) & 4) >> 2)
#define MOTOR_XMIN  (((stableLimitSwitches) & 2) >> 1)
#define PAPER       ((stableLimitSwitches) & 1)

static char delayMs, delayCntr;
int Ax = 0, Ay = 0;

/**************************************************************************
 * Rutina que escaneja l'estat dels fi de cursa del plotter               *
 **************************************************************************/

void polling_fc()
{
     static char lastLimitSwitches = 0;
     char temp;

     // Agafam els 3 bits del mitg del port D 
     temp = (inp(PIND) & 0x38) >> 3;
     
     if (temp != lastLimitSwitches)
         lastLimitSwitches = temp; 
     else if (lastLimitSwitches != stableLimitSwitches)
          stableLimitSwitches = lastLimitSwitches;
}

/**************************************************************************
 * Rutines de retard, per els motors pas-a-pas                            *
 **************************************************************************/

void MotorDelayInit(void)
{
     delayMs = 6;
     delayCntr = 200;
}

void MotorDelay(void)
{
     delayCntr--;
     if (delayMs >3 && delayCntr == 0) {delayMs--; delayCntr=100;}
         delay(delayMs);
}

/**************************************************************************
 * Atura/engega els motors (Senyal de Enable)                             *
 **************************************************************************/

void motors(unsigned char posta)
{
     if (posta) {
         sbi(PORTB, 6);   // Habilita motor Y (SAA-1027) (Enable=1)
         sbi(PORTA, 4);   // Habilita motor X (L297)     (Enable=1)
     } else {
         cbi(PORTB, 6);   // Enable = 0
         cbi(PORTA, 4);
     }
}

/**************************************************************************
 * Activa/desactiva el solenoide de la plumilla                           *
 **************************************************************************/

void SolenoidActivate()
{
     unsigned char temp;
     temp = inp(PORTD) & 0xbf;      
     temp |= 0x40;                
     outp(temp, PORTD);
}

void SolenoidDeactivate()
{
     unsigned char temp;
     temp = inp(PORTD) & 0xbf;  
     outp(temp, PORTD);
}

/**************************************************************************
 * Controla els avanços/retrocessos dels motors X i Y, utilitzant         *
 * increments Ax i Ay                                                     *
 **************************************************************************/

void MotorStepXY(char Xincr, char Yincr)
{
     if (Xincr == 1)               // CW/CCW (Sentit de gir del SAA1027) Eix Y
         cbi(PORTA, 3);
     else if (Xincr == -1)
         sbi(PORTA, 3);
    
     if (Yincr == 1)               // CW/CCW (Sentit de gir del L297) Eix X
        cbi(PORTB, 5);
     else if (Yincr == -1)
        sbi(PORTB, 5);
          
     if (Xincr != 0) {     
        sbi(PORTA, 7);             // Motor Eix X (Envia un cicle de clock)
        MotorDelay();
        cbi(PORTA, 7);
        MotorDelay();
     }
     
     if (Yincr != 0) {
         cbi(PORTB, 7);            // Motor Eix Y (Envia un cicle de clock)
         delay(1);
         sbi(PORTB, 7);
         delay(1);
     }
    
     Ax+=Xincr;
     Ay+=Yincr;
}

void MotorGotoOrigin()
{
     MotorStepXY(0,0);

     MotorDelayInit();
 
     while (MOTOR_XMIN != 1)
     {
            MotorStepXY(-1, 0);
            MotorDelay();
     }

     Ax=0; Ay=0;
}

void MotorBresLine(int Bx, int By)
{
        int dX, dY, Xincr, Yincr;

        MotorDelayInit();

        dX = abs(Bx-Ax);        // Guardam els increments necessaris de X i Y de la punt final
        dY = abs(By-Ay);
        
        // Determinar les direccions a incrementar de X i Y
        if (Ax > Bx) { Xincr=-1; } else { Xincr=1; }         // Direcció de X        
        if (Ay > By) { Yincr=-1; } else { Yincr=1; }         // Direcció de Y
        
        if (dX >= dY)        
        {           
                int dPr         = dY<<1;          
                int dPru         = dPr - (dX<<1);
                int P                 = dPr - dX;  

                    for (; dX>0; dX--)            
                {
                        if (P > 0)           
                        { 
                                P+=dPru;    
                                MotorStepXY(Xincr, Yincr);
                        }
                        else               
                        {
                                P+=dPr;   
                                MotorStepXY(Xincr, 0);
                        }
                        MotorDelay();
                }                
        }
        else              
        {
                int dPr     = dX<<1;           
                int dPru    = dPr - (dY<<1);
                int P       = dPr - dY;

                for (; dY>0; dY--)        
                {
                        if (P > 0)               
                        { 
                                P+=dPru;        
                                MotorStepXY(Xincr, Yincr);
                        }
                        else                   
                        {
                                P+=dPr;       
                                MotorStepXY(0, Yincr);
                        }
                        MotorDelay();
                }                
        }                
        MotorStepXY(0, 0);
}
