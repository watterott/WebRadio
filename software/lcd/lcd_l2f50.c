#include <stdint.h>
#include "lmi/inc/hw_types.h"
#include "lmi/inc/hw_memmap.h"
#include "lmi/driverlib/gpio.h"
#include "../io.h"
#include "../lcd.h"
#include "lcd_l2f50.h"


#ifdef LCD_L2F50


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
  lcd_cmd(0x5C);
  LCD_RS_DISABLE(); //data
  LCD_CS_ENABLE();

  return;
}


void lcd_setarea(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1)
{
#ifdef LCD_MIRROR
  lcd_cmd(0x15);                    //column address set 
  lcd_data(0x08+(LCD_HEIGHT-1)-y1); //start column
  lcd_data(0x01);                   //start column
  lcd_data(0x08+(LCD_HEIGHT-1)-y0); //end column
  lcd_data(0x01);                   //end column
  lcd_cmd(0x75); //page address set 
  lcd_data(x0);  //start page
  lcd_data(x1);  //end page
#else
  lcd_cmd(0x15);     //column address set 
  lcd_data(0x08+y0); //start column
  lcd_data(0x01);    //start column
  lcd_data(0x08+y1); //end column
  lcd_data(0x01);    //end column
  lcd_cmd(0x75); //page address set 
  lcd_data(x0);  //start page
  lcd_data(x1);  //end page
#endif

  return;
}


void lcd_setcursor(unsigned int x, unsigned int y)
{
#ifdef LCD_MIRROR
  lcd_cmd(0x15);                   //column address set 
  lcd_data(0x08+(LCD_HEIGHT-1)-y); //start column
  lcd_data(0x01);                  //start column
  lcd_data(0x08+(LCD_HEIGHT-1)-y); //end column
  lcd_data(0x01);                  //end column
  lcd_cmd(0x75); //page address set 
  lcd_data(x);   //start page
  lcd_data(x);   //end page
#else
  lcd_cmd(0x15);    //column address set 
  lcd_data(0x08+y); //start column
  lcd_data(0x01);   //start column
  lcd_data(0x08+y); //end column
  lcd_data(0x01);   //end column
  lcd_cmd(0x75); //page address set 
  lcd_data(x);   //start page
  lcd_data(x);   //end page
#endif

  return;
}


void lcd_cmd(unsigned int c)
{
  LCD_RS_ENABLE(); //cmd
  LCD_CS_ENABLE();
  ssi_write(c);
  ssi_write(0x00);
  ssi_wait();
  LCD_CS_DISABLE();

  return;
}


void lcd_data(unsigned int c)
{
  LCD_RS_DISABLE(); //data
  LCD_CS_ENABLE();
  ssi_write(c);
  ssi_write(0x00);
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

  lcd_cmd(0xAE); //display off

  lcd_cmd(0xBC);  //data control
#ifdef LCD_MIRROR
  lcd_data(0x2A); //565 mode, 0x2A=normal, 0x2B=180
#else
  lcd_data(0x2B); //565 mode, 0x2A=normal, 0x2B=180
#endif

  lcd_cmd(0xCA);  //display control 
  lcd_data(0x4C); //P1
  lcd_data(0x01); //P2
  lcd_data(0x53); //P3
  lcd_data(0x00); //P4
  lcd_data(0x02); //P5
  lcd_data(0xB4); //P6
  lcd_data(0xB0); //P7
  lcd_data(0x02); //P8
  lcd_data(0x00); //P9

  lcd_cmd(0x94); //sleep out

  delay_ms(5);

  lcd_cmd(0xAF); //display on

  delay_ms(5);

  lcd_setarea(0, 0, (LCD_WIDTH-1), (LCD_HEIGHT-1));

  return;
}


#endif //LCD_L2F50
