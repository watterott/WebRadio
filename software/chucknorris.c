#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fatfs/ff.h"
#include "tools.h"
#include "main.h"
#include "io.h"
#include "lcd.h"
#include "mmc.h"
#include "vs.h"
#include "chucknorris.h"


void chucknorris_rfact(void)
{
  char value[16];
  unsigned int entries, i, j;

  if(ini_getentry(CHUCKNORRIS_FILE, "NUMBEROFENTRIES", value, 16) == 0)
  {
    entries = atou(value);

    if(entries)
    {
      if(getontime()&1)
      {
        srand(getontime()+rand());
      }
      else
      {
        srand(getontime()-rand());
      }
    
      i = rand();
      while(i >= entries)
      {
        j = rand();
        while(j > i)
        {
          j /= 10;
        }
        i -= j;
      }
  
      chucknorris_fact(i);
    }
  }

  return;
}


void chucknorris_fact(unsigned int nr)
{
  char tmp[16];
  unsigned int i;

  vs_bufreset();
  if(ini_getentry(CHUCKNORRIS_FILE, "TITLE", (char*)vs_buf.b8, VS_BUFSIZE) == 0)
  {
    lcd_clear(RGB(0,0,0));
    lcd_putline(20, 0, (char*)vs_buf.b8, NORMALFONT, 1, RGB(0,0,0), RGB(255,255,255));
    sprintf(tmp, "%i", nr);
    ini_getentry(CHUCKNORRIS_FILE, tmp, (char*)vs_buf.b8, VS_BUFSIZE);
    lcd_putlinebr(3, 15, (char*)vs_buf.b8, NORMALFONT, 1, RGB(255,255,255), RGB(0,0,0));

    for(i=8000/20; i!=0; i--)
    {
      delay_ms(20);
      if(keys_sw() || ir_cmd()) { break; }
    }
  }

  return;
}
