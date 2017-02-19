#include <stdint.h>
#include "lmi/inc/hw_types.h"
#include "lmi/inc/hw_memmap.h"
#include "lmi/driverlib/gpio.h"
#include "../io.h"
#include "../lcd.h"
#include "lcd_lph88.h"


#ifdef LCD_LPH88


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
  lcd_reg(0x22);
  LCD_CS_ENABLE();
  ssi_write(0x76);

  return;
}


void lcd_setarea(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1)
{
  //set area
#ifdef LCD_MIRROR
  lcd_cmd(0x16, (((LCD_HEIGHT-1)-y0)<<8)|(((LCD_HEIGHT-1)-y1))); //set y
  lcd_cmd(0x17, (((LCD_WIDTH-1)-x0)<<8)|(((LCD_WIDTH-1)-x1)));  //set x
#else
  lcd_cmd(0x16, (y1<<8)|(y0)); //set y
  lcd_cmd(0x17, (x1<<8)|(x0)); //set x
#endif

  //set cursor
  lcd_setcursor(x0, y0);

  return;
}


void lcd_setcursor(unsigned int x, unsigned int y)
{
#ifdef LCD_MIRROR
  lcd_cmd(0x21, ((((LCD_WIDTH-1)-x)<<8)|((LCD_HEIGHT-1)-y))); //set cursor pos
#else
  lcd_cmd(0x21, ((x<<8)|y)); //set cursor pos
#endif

  return;
}


void lcd_cmd(unsigned int reg, unsigned int param)
{
  lcd_reg(reg);
  lcd_data(param);

  return;
}


void lcd_data(unsigned int c)
{
  LCD_CS_ENABLE();
  ssi_write(0x76); //instruction or RAM data
  ssi_write(c>>8);
  ssi_write(c);
  ssi_wait();
  LCD_CS_DISABLE();

  return;
}


void lcd_reg(unsigned int c)
{
  LCD_CS_ENABLE();
  ssi_write(0x74); //index register
  ssi_write(0x00);
  ssi_write(c);
  ssi_wait();
  LCD_CS_DISABLE();

  return;
}

/*&H01 , &H0215% , &H0F , &H0000% , &H0B , &H0000%
Data &H11 , &H0000% , &H06 , &H0000% , &H05 , &H0030% , &H14 , &HAF00% , &H15 , &H0000% , &H16 , &H8300%
Data &H17 , &HAF00% , &H20 , &H0000% , 

&H30 , &H0000% , &H31 , &H0400%
Data &H32 , &H0207% , &H33 , &H0700% , &H34 , &H0005% , &H35 , &H0703% , &H36 , &H0707% , &H37 , &H0007%
Data &H3A , &H1200% , &H3B , &H0009% , 

*/
void lcd_reset(void)
{
  //reset
  LCD_CS_DISABLE();
  LCD_RS_DISABLE();
  LCD_RST_ENABLE();
  delay_ms(50);
  LCD_RST_DISABLE();
  delay_ms(50);

  lcd_cmd(0x00, 0x0001); //start oscillation
  delay_ms(50);
  lcd_cmd(0x07, 0x0000); //display off
  delay_ms(10);

  //power on sequence
  lcd_cmd(0x02, 0x0400); //lcd drive control
  lcd_cmd(0x04, 0x0000); //power control 2  //step 1
  lcd_cmd(0x0C, 0x0001); //power control 3
  lcd_cmd(0x0D, 0x0616); //power control 4
  lcd_cmd(0x0E, 0x1010); //power control 5
  lcd_cmd(0x03, 0x000C); //power control 1
  delay_ms(40);
  lcd_cmd(0x0E, 0x2D1F); //power control 5  //step 3
  delay_ms(40);
  lcd_cmd(0x0D, 0x0616); //power control 4  //step 4
  delay_ms(100);
  lcd_cmd(0x01, 0x0015); //driver output control
  lcd_cmd(0x0F, 0x0000); //gate scan starting pos
  lcd_cmd(0x0B, 0x0000); //frame cycle control 

  //display options
#ifdef LCD_MIRROR
  lcd_cmd(0x05, 0x0008); //Entry mode --
#else
  lcd_cmd(0x05, 0x0038); //Entry mode ++
#endif
  lcd_setarea(0, 0, (LCD_WIDTH-1), (LCD_HEIGHT-1));

  //display on sequence (bit2 = reversed colors)
  lcd_cmd(0x07, 0x0005); //display control: D0
  lcd_cmd(0x07, 0x0025); //display control: GON
  lcd_cmd(0x07, 0x0027); //display control: D1
  lcd_cmd(0x07, 0x0037); //display control: DTE

  delay_ms(10);

  return;
}


#endif //LCD_LPH88
