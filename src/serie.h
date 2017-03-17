#ifndef SERIE_H
#define SERIE_H

#define SetBaud2400		outp(207, UBRR);    // Valor ajustat per 8Mhz
#define SetBaud4800		outp(103, UBRR);    
#define SetBaud9600		outp(51,  UBRR);    
#define SetBaud19200    	outp(29,  UBRR);    // Valor ajustat per 9Mhz

void initUART(void);
void putchar_serie(unsigned char c);   
unsigned char getchar_serie(void);              
void llegeix_serie(unsigned char *buf, unsigned char length); 
unsigned char iscom(void);                   


#endif
