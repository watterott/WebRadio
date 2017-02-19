#ifndef _LCD_LS020_H_
#define _LCD_LS020_H_


#ifdef LCD_LS020

//----- PROTOTYPES -----
void                                   lcd_draw(unsigned int color);
void                                   lcd_drawstop(void);
void                                   lcd_drawstart(void);
void                                   lcd_setarea(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1);
void                                   lcd_setcursor(unsigned int x, unsigned int y);
void                                   lcd_cmd(unsigned int reg, unsigned int param);
void                                   lcd_data(unsigned int c);
void                                   lcd_reset(void);

#endif


#endif //_LCD_LS020_H_
