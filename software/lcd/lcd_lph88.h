#ifndef _LCD_LPH88_H_
#define _LCD_LPH88_H_


#ifdef LCD_LPH88

//----- PROTOTYPES -----
void                                   lcd_draw(unsigned int color);
void                                   lcd_drawstop(void);
void                                   lcd_drawstart(void);
void                                   lcd_setarea(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1);
void                                   lcd_setcursor(unsigned int x, unsigned int y);
void                                   lcd_cmd(unsigned int reg, unsigned int param);
void                                   lcd_data(unsigned int c);
void                                   lcd_reg(unsigned int c);
void                                   lcd_reset(void);

#endif


#endif //_LCD_LPH88_H_
