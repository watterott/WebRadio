#ifndef _LCD_MI0283QT2_H_
#define _LCD_MI0283QT2_H_


#ifdef LCD_MI0283QT2

//----- DEFINES -----
#define LCD_ID                         (0)
#define LCD_DATA                       ((0x72)|(LCD_ID<<2))
#define LCD_REGISTER                   ((0x70)|(LCD_ID<<2))


//----- PROTOTYPES -----
void                                   lcd_draw(unsigned int color);
void                                   lcd_drawstop(void);
void                                   lcd_drawstart(void);
void                                   lcd_setarea(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1);
void                                   lcd_setcursor(unsigned int x, unsigned int y);
void                                   lcd_cmd(unsigned int reg, unsigned int param);
void                                   lcd_data(unsigned int c);
void                                   lcd_reset(void);

#endif //LCD_MI0283QT2


#endif //_LCD_MI0283QT2_H_
