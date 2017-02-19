#include <stdint.h>
#include "lmi/inc/hw_types.h"
#include "lmi/inc/hw_memmap.h"
#include "lmi/driverlib/gpio.h"
#include "../io.h"
#include "../lcd.h"
#include "lcd_mi0283qt2.h"


#ifdef LCD_MI0283QT2


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
  LCD_CS_ENABLE();
  ssi_write(LCD_REGISTER);
  ssi_write(0x22);
  ssi_wait();
  LCD_CS_DISABLE();

  LCD_CS_ENABLE();
  ssi_write(LCD_DATA);

  return;
}


void lcd_setarea(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1)
{
  lcd_cmd(0x03, (x0>>0)); //set x0
  lcd_cmd(0x02, (x0>>8)); //set x0
  lcd_cmd(0x05, (x1>>0)); //set x1
  lcd_cmd(0x04, (x1>>8)); //set x1
  lcd_cmd(0x07, (y0>>0)); //set y0
  lcd_cmd(0x06, (y0>>8)); //set y0
  lcd_cmd(0x09, (y1>>0)); //set y1
  lcd_cmd(0x08, (y1>>8)); //set y1

  return;
}


void lcd_setcursor(unsigned int x, unsigned int y)
{
  lcd_setarea(x, y, x, y);

  return;
}


void lcd_cmd(unsigned int reg, unsigned int param)
{
  LCD_CS_ENABLE();
  ssi_write(LCD_REGISTER);
  ssi_write(reg);
  ssi_wait();
  LCD_CS_DISABLE();

  LCD_CS_ENABLE();
  ssi_write(LCD_DATA);
  ssi_write(param);
  ssi_wait();
  LCD_CS_DISABLE();

  return;
}


void lcd_data(unsigned int c)
{
  LCD_CS_ENABLE();
  ssi_write(LCD_DATA);
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
  LCD_RST_ENABLE();
  delay_ms(50);
  LCD_RST_DISABLE();
  delay_ms(50);

  //driving ability
  lcd_cmd(0xEA, 0x0000);
  lcd_cmd(0xEB, 0x0020);
  lcd_cmd(0xEC, 0x000C);
  lcd_cmd(0xED, 0x00C4);
  lcd_cmd(0xE8, 0x0040);
  lcd_cmd(0xE9, 0x0038);
  lcd_cmd(0xF1, 0x0001);
  lcd_cmd(0xF2, 0x0010);
  lcd_cmd(0x27, 0x00A3);

  //power voltage
  lcd_cmd(0x1B, 0x001B);
  lcd_cmd(0x1A, 0x0001);
  lcd_cmd(0x24, 0x002F);
  lcd_cmd(0x25, 0x0057);

  //VCOM offset
  lcd_cmd(0x23, 0x008D); //for flicker adjust

  //power on
  lcd_cmd(0x18, 0x0036);
  lcd_cmd(0x19, 0x0001); //start osc
  lcd_cmd(0x01, 0x0000); //wakeup
  lcd_cmd(0x1F, 0x0088);
  delay_ms(5);
  lcd_cmd(0x1F, 0x0080);
  delay_ms(5);
  lcd_cmd(0x1F, 0x0090);
  delay_ms(5);
  lcd_cmd(0x1F, 0x00D0);
  delay_ms(5);

  //color selection
  lcd_cmd(0x17, 0x0005); //0x0005=65k, 0x0006=262k

  //panel characteristic
  lcd_cmd(0x36, 0x0000);

  //display on
  lcd_cmd(0x28, 0x0038);
  delay_ms(40);
  lcd_cmd(0x28, 0x003C);

  //display options
#ifdef LCD_MIRROR
  lcd_cmd(0x16, 0x0068); //MY=0 MX=1 MV=1 ML=0 BGR=1
#else
  lcd_cmd(0x16, 0x00A8); //MY=1 MX=0 MV=1 ML=0 BGR=1
#endif

  lcd_setarea(0, 0, (LCD_WIDTH-1), (LCD_HEIGHT-1));

  return;
}


#endif //LCD_MI0283QT2
