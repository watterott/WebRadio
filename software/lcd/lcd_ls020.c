#include <stdint.h>
#include "lmi/inc/hw_types.h"
#include "lmi/inc/hw_memmap.h"
#include "lmi/driverlib/gpio.h"
#include "../io.h"
#include "../lcd.h"
#include "lcd_ls020.h"


#ifdef LCD_LS020


void lcd_draw(unsigned int color)
{
  ssi_write(color>>8);
  ssi_write(color);

  return;
}


void lcd_drawstop(void)
{
  ssi_wait();
  LCD_CS_DISABLE();

  return;
}


void lcd_drawstart(void)
{
  LCD_RS_ENABLE(); //data
  LCD_CS_ENABLE();

  return;
}


void lcd_setarea(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1)
{
  //set area
  lcd_cmd(0xEF, 0x90);
#ifdef LCD_ROTATE
# ifdef LCD_MIRROR
  lcd_cmd(0x08, (LCD_WIDTH-1)-x0);  //set x0
  lcd_cmd(0x09, (LCD_WIDTH-1)-x1);  //set x1
  lcd_cmd(0x0A, (LCD_HEIGHT-1)-y0); //set y0
  lcd_cmd(0x0B, (LCD_HEIGHT-1)-y1); //set y1
# else
  lcd_cmd(0x08, x0);                //set x0
  lcd_cmd(0x09, x1);                //set x1
  lcd_cmd(0x0A, y0);                //set y0
  lcd_cmd(0x0B, y1);                //set y1
# endif
#else
# ifdef LCD_MIRROR
  lcd_cmd(0x08, (LCD_HEIGHT-1)-y0); //set y0
  lcd_cmd(0x09, (LCD_HEIGHT-1)-y1); //set y1
  lcd_cmd(0x0A, x0);                //set x0
  lcd_cmd(0x0B, x1);                //set x1
# else
  lcd_cmd(0x08, y0);                //set y0
  lcd_cmd(0x09, y1);                //set y1
  lcd_cmd(0x0A, (LCD_WIDTH-1)-x0);  //set x0
  lcd_cmd(0x0B, (LCD_WIDTH-1)-x1);  //set x1
# endif
#endif

  //set cursor
  lcd_setcursor(x0, y0);

  return;
}


void lcd_setcursor(unsigned int x, unsigned int y)
{
  lcd_cmd(0xEF, 0x90);
#ifdef LCD_ROTATE
# ifdef LCD_MIRROR
  lcd_cmd(0x06, (LCD_WIDTH-1)-x);  //set x cursor pos
  lcd_cmd(0x07, (LCD_HEIGHT-1)-y); //set y cursor pos
# else
  lcd_cmd(0x06, x);                //set x cursor pos
  lcd_cmd(0x07, y);                //set y cursor pos
# endif
#else
# ifdef LCD_MIRROR
  lcd_cmd(0x06, (LCD_HEIGHT-1)-y); //set y cursor pos
  lcd_cmd(0x07, x);                //set x cursor pos
# else
  lcd_cmd(0x06, y);                //set y cursor pos
  lcd_cmd(0x07, (LCD_WIDTH-1)-x);  //set x cursor pos
# endif
#endif

  return;
}


void lcd_cmd(unsigned int reg, unsigned int param)
{
  LCD_RS_DISABLE(); //cmd
  LCD_CS_ENABLE();
  ssi_write(reg);
  ssi_write(param);
  ssi_wait();
  LCD_CS_DISABLE();

  return;
}


void lcd_data(unsigned int c)
{
  LCD_RS_ENABLE(); //data
  LCD_CS_ENABLE();
  ssi_write(c>>8);
  ssi_write(c);
  ssi_wait();
  LCD_CS_DISABLE();

  return;
}


void lcd_reset(void)
{
  //reset
  LCD_CS_DISABLE();
  LCD_RS_DISABLE();
  LCD_RST_ENABLE();
  delay_ms(50);
  LCD_RST_DISABLE();
  delay_ms(50);

  lcd_cmd(0xFD, 0xFD);
  lcd_cmd(0xFD, 0xFD);

  delay_ms(50);

  //init 1
  lcd_cmd(0xEF, 0x00);
  lcd_cmd(0xEE, 0x04);
  lcd_cmd(0x1B, 0x04);
  lcd_cmd(0xFE, 0xFE);
  lcd_cmd(0xFE, 0xFE);
  lcd_cmd(0xEF, 0x90);
  lcd_cmd(0x4A, 0x04);
  lcd_cmd(0x7F, 0x3F);
  lcd_cmd(0xEE, 0x04);
  lcd_cmd(0x43, 0x06);

  delay_ms(7); //important: 7ms

  //init 2
  lcd_cmd(0xEF, 0x90);
  lcd_cmd(0x09, 0x83);
  lcd_cmd(0x08, 0x00);
  lcd_cmd(0x0B, 0xAF);
  lcd_cmd(0x0A, 0x00);
  lcd_cmd(0x05, 0x00);
  lcd_cmd(0x06, 0x00);
  lcd_cmd(0x07, 0x00);
  lcd_cmd(0xEF, 0x00);
  lcd_cmd(0xEE, 0x0C);
  lcd_cmd(0xEF, 0x90);
  lcd_cmd(0x00, 0x80);
  lcd_cmd(0xEF, 0xB0);
  lcd_cmd(0x49, 0x02);
  lcd_cmd(0xEF, 0x00);
  lcd_cmd(0x7F, 0x01);
  lcd_cmd(0xE1, 0x81);
  lcd_cmd(0xE2, 0x02);
  lcd_cmd(0xE2, 0x76);
  lcd_cmd(0xE1, 0x83);

  delay_ms(50);

  //display on
  lcd_cmd(0x80, 0x01);
  lcd_cmd(0xEF, 0x90);
  lcd_cmd(0x00, 0x00);

  //display options
  lcd_cmd(0xEF, 0x90);
#ifdef LCD_ROTATE
# ifdef LCD_MIRROR
  lcd_cmd(0x01, 0xC0); //x1->x0, y1->y0
  lcd_cmd(0x05, 0x00); //0x04=rotate, 0x00=normal
# else
  lcd_cmd(0x01, 0x00); //x0->x1, y0->y1
  lcd_cmd(0x05, 0x00); //0x04=rotate, 0x00=normal
# endif
#else
# ifdef LCD_MIRROR
  lcd_cmd(0x01, 0x80); //x0->x1, y1->y0
  lcd_cmd(0x05, 0x04); //0x04=rotate, 0x00=normal
# else
  lcd_cmd(0x01, 0x40); //x1->x0, y0->y1
  lcd_cmd(0x05, 0x04); //0x04=rotate, 0x00=normal
# endif
#endif
  lcd_setarea(0, 0, (LCD_WIDTH-1), (LCD_HEIGHT-1));

  return;
}


#endif //LCD_LS020
