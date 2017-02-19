#ifndef _LCD_L2F50_H_
#define _LCD_L2F50_H_


#ifdef LCD_L2F50

//----- PROTOTYPES -----
void                                   lcd_draw(unsigned int color);
void                                   lcd_drawstop(void);
void                                   lcd_drawstart(void);
void                                   lcd_setarea(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1);
void                                   lcd_setcursor(unsigned int x, unsigned int y);
void                                   lcd_cmd(unsigned int cmd);
void                                   lcd_data(unsigned int c);
void                                   lcd_reset(void);

#endif


#endif //_LCD_L2F50_H_
