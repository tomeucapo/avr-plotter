#ifndef LCD_H
#define LCD_H

#define LCDPORT		PORTC
#define LCDDR		DDRC

void initLcd(void);					
void putchar_lcd(unsigned char ch);	
void cursor_on(void);
void cursor_off(void);
void setpos_lcd(unsigned char row, unsigned char col);
void cls_lcd(void);		
void print_lcd(unsigned char *);

#endif //LCD_H
