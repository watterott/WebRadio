#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "fatfs/ff.h"
#include "debug.h"
#include "tools.h"
#include "main.h"
#include "io.h"
#include "lcd.h"
#include "lcd/font_8x8.h"
#include "lcd/font_8x12.h"
#include "lcd/font_clock.h"
#include "lcd/img.h"
#include "mmc.h"
#include "vs.h"
#include "eth.h"
#include "eth/utils.h"
#include "station.h"
#include "share.h"
#include "card.h"
#include "alarm.h"
#include "settings.h"
#include "buffer.h"
#include "menu_dlg.h"
#include "menu.h"


#define SUB_STATION  (0)
//#define SUB_SHARE    (1)
#define SUB_CARD     (1)
#define SUB_ALARM    (2)
#define SUB_SETTINGS (3)
#define SUB_BACK     (4)
#define SUB_STANDBY  (5)
#define MAINITEMS    (6)
const MAINMENUITEM mainmenu[MAINITEMS] =
{
  {"Station",  {&img_station[0][0],  &img_station[1][0],  &img_station[2][0]},  station_init,  station_items,  station_getitem,  station_openitem,  station_closeitem, station_service},
//  {"Share",    {&img_share[0][0],    &img_share[1][0],    &img_share[2][0]},    share_init,    share_items,    share_getitem,    share_openitem,    share_closeitem,   share_service},
  {"Card",     {&img_card[0][0],     &img_card[1][0],     &img_card[2][0]},     card_init,     card_items,     card_getitem,     card_openitem,     card_closeitem,    card_service},
  {"Alarm",    {&img_clock[0][0],    &img_clock[1][0],    &img_clock[2][0]},    alarm_init,    alarm_items,    alarm_getitem,    alarm_openitem,    0,                 0},
  {"Settings", {&img_settings[0][0], &img_settings[1][0], &img_settings[2][0]}, settings_init, settings_items, settings_getitem, settings_openitem, 0,                 0},
  {"Back",     {&img_back[0][0],     &img_back[1][0],     &img_back[2][0]},     0,             0,              0,                0,                 0,                 0},
  {"Standby",  {&img_power[0][0],    &img_power[1][0],    &img_power[2][0]},    0,             0,              0,                standby,           0,                 0},
};

#define MODE_INFO (0) //normal info screen
#define MODE_MAIN (1) //animated main menu
#define MODE_SUB  (2) //list menu
unsigned int menu_mode=0, menu_sub=0, menu_items=0, menu_first=0, menu_last=0, menu_sel=0, menu_lastsel=0;
unsigned int menu_status=0, menu_format=0, menu_bitrate=0;
unsigned int bgcolor=0, fgcolor=0, selcolor=0, edgecolor=0;


unsigned int menu_openfile(char *file)
{
  unsigned int ret;
  char tmp[MAX_ADDR];

  if(file[0] == 0)
  {
    return 1;
  }

  strncpy(tmp, file, MAX_ADDR-1);
  tmp[MAX_ADDR-1] = 0;

  if(mainmenu[menu_sub].close)
  {
    mainmenu[menu_sub].close();
  }

  ret = 1;
  if(isdigit(tmp[0])) //station number
  {
    menu_sub = SUB_STATION;
    mainmenu[menu_sub].init();
    if(station_open(atoi(tmp)) == STATION_OPENED)
    {
      ret = 0;
    }
  }
  else //path to card file
  {
    menu_sub = SUB_CARD;
    mainmenu[menu_sub].init();
    if(card_openfile(tmp) == MENU_PLAY)
    {
      menu_setname(tmp);
      menu_setinfo("");
      ret = 0;
    }
  }
  menu_mode    = MODE_INFO;
  menu_items   = 1;
  menu_first   = 0;
  menu_last    = 0;
  menu_sel     = 0;
  menu_lastsel = 0;
  menu_drawwnd(1);

  return ret;
}


void menu_stopfile(void)
{
  if(mainmenu[menu_sub].close)
  {
    mainmenu[menu_sub].close();
  }

  menu_drawwnd(1);

  return;
}


unsigned int menu_sw(void)
{
  switch(menu_mode)
  {
    case MODE_INFO:
      if(menu_items == 0) //main
      {
        menu_mode    = MODE_MAIN;
        menu_items   = 0;
        menu_sel     = menu_sub;
        menu_lastsel = menu_sel;
      }
      else                //sub
      {
        if(mainmenu[menu_sub].close)
        {
          mainmenu[menu_sub].close();
        }
        menu_mode = MODE_SUB;
        if(menu_last == 0)
        {
          menu_items   = mainmenu[menu_sub].items();
          menu_first   = menu_sel;
          menu_last    = (menu_items >= MENU_LINES)?(MENU_LINES-1):(menu_items-1);
          menu_sel     = 0;
          menu_lastsel = 0;
        }
      }
      break;

    case MODE_MAIN:
      if(mainmenu[menu_sel].items) //sub
      {
        menu_mode  = MODE_SUB;
        menu_sub   = menu_sel;
        if(mainmenu[menu_sub].init)
        {
          mainmenu[menu_sub].init();
        }
        menu_items   = mainmenu[menu_sub].items();
        menu_first   = 0;
        menu_last    = (menu_items >= MENU_LINES)?(MENU_LINES-1):(menu_items-1);
        menu_sel     = 0;
        menu_lastsel = 0;
      }
      else                         //open item
      {
        if(mainmenu[menu_sel].open) //open and stay in main mode
        {
          mainmenu[menu_sel].open(0);
          menu_lastsel = (menu_sel==(MAINITEMS-1))?0:(menu_sel+1);
        }
        else //back
        {
          menu_mode  = MODE_INFO;
          menu_items = 0;
        }
      }
      break;

    case MODE_SUB:
      if(mainmenu[menu_sub].open)
      {
        switch(mainmenu[menu_sub].open(menu_sel))
        {
          case MENU_NOP:
            break;
          case MENU_PLAY:
            menu_mode = MODE_INFO;
            break;
          case MENU_UPDATE:
          case MENU_ERROR:
            menu_items   = mainmenu[menu_sub].items();
            menu_first   = 0;
            menu_last    = (menu_items>=MENU_LINES)?(MENU_LINES-1):(menu_items-1);
            menu_sel     = 0;
            menu_lastsel = 0;
            break;
          case MENU_BACK:
            menu_mode    = MODE_MAIN;
            menu_items   = 0;
            menu_sel     = menu_sub;
            menu_lastsel = menu_sel;
            break;
        }
      }
      break;
  }

  return 1;
}


unsigned int menu_swlong(void)
{
  switch(menu_mode)
  {
    case MODE_INFO:
      break;

    case MODE_MAIN:
      menu_mode  = MODE_INFO;
      menu_items = 0;
      break;

    case MODE_SUB:
      menu_mode    = MODE_MAIN;
      menu_items   = 0;
      menu_sel     = menu_sub;
      menu_lastsel = menu_sel;
      break;
  }

  return 1;
}


void menu_up(void)
{
  switch(menu_mode)
  {
    case MODE_INFO:
      vs_setvolume(vs_getvolume()+2);
      menu_drawvol();
      break;

    case MODE_MAIN:
      if(menu_sel < (MAINITEMS-1))
      {
        menu_sel++;
      }
      else
      {
        menu_sel = 0;
      }
      menu_drawwndmain(0);
      break;

    case MODE_SUB:
      if(menu_sel < menu_last)
      {
        menu_sel++;
      }
      else if(menu_sel < (menu_items-1))
      {
        menu_sel++;
        menu_first++;
        menu_last++;
      }
      break;
  }

  return;
}


void menu_down(void)
{
  switch(menu_mode)
  {
    case MODE_INFO:
      vs_setvolume((vs_getvolume()<=2)?1:(vs_getvolume()-2));
      menu_drawvol();
      break;

    case MODE_MAIN:
      if(menu_sel > 0)
      {
        menu_sel--;
      }
      else
      {
        menu_sel = (MAINITEMS-1);
      }
      menu_drawwndmain(0);
      break;

    case MODE_SUB:
      if(menu_sel > menu_first)
      {
        menu_sel--;
      }
      else if(menu_sel > 0)
      {
        menu_sel--;
        menu_first--;
        menu_last--;
      }
      break;
  }

  return;
}


void menu_steps(int steps)
{
  if(steps != 0)
  {
    if(menu_mode == MODE_INFO)
    {
      vs_setvolume(vs_getvolume()+steps);
      menu_drawvol();
    }
    else
    {
      if(steps > 0)
      {
        if(steps > MENU_LINES)
        {
          steps = MENU_LINES;
        }
        while(steps--)
        {
          menu_up();
        }
      }
      else
      {
        if(steps < -MENU_LINES)
        {
          steps = -MENU_LINES;
        }
        while(steps++)
        {
          menu_down();
        }
      }
    }
  }

  return;
}


void menu_service(unsigned int draw)
{
  unsigned int redraw=0;
  static unsigned int menu_standbytimer=0;

  menu_steps(keys_steps());

  //rotary encoder
  switch(keys_sw())
  {
    case SW_PRESSED:
      redraw |= menu_sw();
      break;
    case SW_PRESSEDLONG:
      redraw |= menu_swlong();
      break;
  }

  //ir remote control
  switch(ir_cmd())
  {
    case SW_VOLP:
      switch(menu_mode)
      {
        case MODE_INFO: vs_setvolume(vs_getvolume()+4); menu_drawvol();                         break;
        case MODE_MAIN: menu_steps(+1);                                                         break;
        case MODE_SUB:  menu_steps(+MENU_LINES);                                                break;
      }
      break;
    case SW_VOLM:
      switch(menu_mode)
      {
        case MODE_INFO: vs_setvolume((vs_getvolume()<=4)?1:(vs_getvolume()-4)); menu_drawvol(); break;
        case MODE_MAIN: menu_steps(-1);                                                         break;
        case MODE_SUB:  menu_steps(-MENU_LINES);                                                break;
      }
      break;
    case SW_UP:
      menu_steps(-1);
      break;
    case SW_DOWN:
      menu_steps(+1);
      break;
    case SW_ENTER:
      redraw |= menu_sw();
      break;
    case SW_POWER:
      if(mainmenu[menu_sub].close)
      {
        mainmenu[menu_sub].close();
      }
      standby(0);
      redraw = 1;
      break;
  }

  if(redraw == 0)
  {
    menu_drawwnd(0);
    if(draw & DRAWALL)
    {
      redraw = 1;
    }
    else
    {
      if(draw & SEC_CHANGED)
      {
        if(menu_status == MENU_STATE_STOP)
        {
          if(++menu_standbytimer > STANDBY_TIME)
          {
            standby(0);
            redraw = 1;
          }
        }
        menu_drawclock(draw);
      }
      if(draw & DAY_CHANGED)
      {
        menu_drawdate();
      }
    }
  }
  if(redraw)
  {
    menu_standbytimer = 0;
    menu_drawwnd(1);
  }

  if(mainmenu[menu_sub].service)
  {
    mainmenu[menu_sub].service();
  }

  return;
}


void menu_alarm(void)
{
  unsigned int i;

  DEBUGOUT("Alarm\n");

  //set alarm volume
  i = alarm_getvol();
  if(i)
  {
    vs_setvolume(i);
    menu_drawvol();
  }

  //open alarm file
  if(menu_status == MENU_STATE_STOP)
  {
    menu_drawpopup("Alarm");
    for(i=1; i<=ALARM_FILEITEMS; i++) //open alarm file
    {
      if(alarm_getfile(gbuf.menu.file, i) == 0)
      {
        if(menu_openfile(gbuf.menu.file) == 0)
        {
          break;
        }
      }
    }
  }

  return;
}


#define ITEM_LEFT (0)
#define ITEM_TOP  (11)
void menu_drawwndsub(unsigned int redraw)
{
  unsigned int i, x, y;
  static unsigned int last_first=0;
  char tmp[MAX_NAME];

  if(redraw == 0)
  {
    if(menu_sel == menu_lastsel)
    {
      return;
    }
  }

  //clear last selection
  i = ITEM_TOP + ((menu_lastsel-last_first)*MENU_LINEHEIGHT);
  lcd_drawrect(ITEM_LEFT, i, LCD_WIDTH-5, i+MENU_LINEHEIGHT, bgcolor);

  //selection
  i = ITEM_TOP + ((menu_sel-menu_first)*MENU_LINEHEIGHT);
  lcd_drawrect(ITEM_LEFT, i, LCD_WIDTH-5, i+MENU_LINEHEIGHT, selcolor);

  //draw items
  if((menu_first != last_first) || redraw)
  {
    last_first = menu_first;
    for(i=0; (i < MENU_LINES) && ((menu_first+i) < menu_items); i++)
    {
      mainmenu[menu_sub].get(menu_first+i, tmp);
      x = lcd_puts(ITEM_LEFT+2, ITEM_TOP+2+(i*MENU_LINEHEIGHT), tmp, NORMALFONT, 1, fgcolor, bgcolor);
      if(x < (LCD_WIDTH-5))
      {
        y = ITEM_TOP+2+(i*MENU_LINEHEIGHT);
        lcd_fillrect(x, y, LCD_WIDTH-5-1, y+12-1, bgcolor); //font height = 12
      }
    }
  }

  //scrollbar
  i = ITEM_TOP+((menu_sel*(LCD_HEIGHT-1-8-ITEM_TOP)) / (menu_items-1));
  lcd_fillrect(LCD_WIDTH-4, ITEM_TOP, LCD_WIDTH-1, LCD_HEIGHT-1, edgecolor);
  lcd_fillrect(LCD_WIDTH-4, i, LCD_WIDTH-1, i+8, fgcolor);

  menu_lastsel = menu_sel;

  return;
}


#define IMG_TOP    (45)
#define IMG_LEFT   (0)
#define IMG_MIDDLE ((LCD_WIDTH/2)-(32/2))
#define IMG_RIGHT  (LCD_WIDTH-32)
void menu_drawwndmain(unsigned int redraw)
{
  int x[4], add;
  unsigned int state[4], item[4];
  unsigned int i, stop;

  if(redraw == 0)
  {
    if(menu_sel == menu_lastsel)
    {
      return;
    }
  }
  else
  {
    if(menu_sel == menu_lastsel)
    {
      menu_lastsel = (menu_sel==(MAINITEMS-1))?0:(menu_sel+1);
    }
  }

  x[0]     = IMG_LEFT;
  x[1]     = IMG_MIDDLE;
  x[2]     = IMG_RIGHT;
  state[0] = 2;
  state[1] = 0;
  state[2] = 2;
  state[3] = 2;

  item[1] = menu_lastsel;

  if(menu_lastsel == 0)
  {
    item[0] = MAINITEMS-1;
  }
  else
  {
    item[0] = menu_lastsel-1;
  }

  if(menu_lastsel == (MAINITEMS-1))
  {
    item[2] = 0;
  }
  else
  {
    item[2] = menu_lastsel+1;
  }

  if(((menu_sel > menu_lastsel) && ((menu_sel-menu_lastsel) == 1)) ||
     ((menu_sel == 0) && ((menu_lastsel == (MAINITEMS-1))) ))          //move ->
  {
    if(item[2] == (MAINITEMS-1))
    {
      item[3] = 0;
    }
    else
    {
      item[3] = item[2]+1;
    }
    x[3] = IMG_RIGHT+IMG_MIDDLE;
    add  = -2;
  }
  else                                                                 //move <-
  {
    if(item[0] == 0)
    {
      item[3] = MAINITEMS-1;
    }
    else
    {
      item[3] = item[0]-1;
    }
    x[3] = -IMG_MIDDLE;
    add  = +2;
  }

  stop = 0;
  while(stop == 0)
  {
    for(i=0; i<4; i++)
    {
      x[i] += add;
      switch(x[i])
      {
        case IMG_MIDDLE/3:              state[i]=2;         break;
        case IMG_MIDDLE/2:              state[i]=1;         break;
        case IMG_MIDDLE-10:             state[i]=0;         break;
        case IMG_MIDDLE:                state[i]=0; stop=1; break;
        case IMG_MIDDLE+10:             state[i]=0;         break;
        case IMG_RIGHT-(IMG_MIDDLE/2):  state[i]=1;         break;
        case IMG_RIGHT-(IMG_MIDDLE/3):  state[i]=2;         break;
      }
      lcd_drawimg32(x[i], IMG_TOP, mainmenu[item[i]].img[state[i]], fgcolor, bgcolor);
    }
  }

  i = (LCD_WIDTH/2)-(strlen(mainmenu[menu_sel].name)*NORMALFONT_WIDTH/2);
  lcd_putline(i, IMG_TOP+32+15, mainmenu[menu_sel].name, NORMALFONT, 1, fgcolor, bgcolor);

  menu_lastsel = menu_sel;

  return;
}


void menu_drawclock(unsigned int draw)
{
  char *clock;

  if(menu_mode == MODE_INFO)
  {
    clock = getclock();
    if(draw & HOUR_CHANGED)
    {
      lcd_puts(((LCD_WIDTH/2)-((8*TIMEFONT_WIDTH)/2))+(0*TIMEFONT_WIDTH), LCD_HEIGHT-24, clock+0, TIMEFONT, 1, bgcolor, fgcolor); //00:00:00
    }
    else if(draw & MIN_CHANGED)
    {
      lcd_puts(((LCD_WIDTH/2)-((8*TIMEFONT_WIDTH)/2))+(3*TIMEFONT_WIDTH), LCD_HEIGHT-24, clock+3, TIMEFONT, 1, bgcolor, fgcolor); //---00:00
    }
    else if(draw & SEC_CHANGED)
    {
      lcd_puts(((LCD_WIDTH/2)-((8*TIMEFONT_WIDTH)/2))+(6*TIMEFONT_WIDTH), LCD_HEIGHT-24, clock+6, TIMEFONT, 1, bgcolor, fgcolor); //------00
    }
  }

  return;
}


void menu_drawdate(void)
{
  if(menu_mode == MODE_INFO)
  {
    lcd_puts(((LCD_WIDTH/2)-((13*NORMALFONT_WIDTH)/2)), LCD_HEIGHT-42, getdate(), NORMALFONT, 1, bgcolor, fgcolor);
  }

  return;
}


void menu_drawvol(void)
{
  unsigned int x;

  if(menu_mode == MODE_INFO)
  {
    x = (vs_getvolume()/4); //100/4 = 25
    lcd_fillrect(LCD_WIDTH-1-5-25,   2, LCD_WIDTH-1-5-25+x, 8, fgcolor);
    lcd_fillrect(LCD_WIDTH-1-5-25+x, 2, LCD_WIDTH-1-5,      8, bgcolor);
  }

  return;
}


void menu_drawstatus(void)
{
  char c, buf[8];

  switch(menu_status)
  {
    case MENU_STATE_STOP: c = 0xFE; break;
    case MENU_STATE_BUF:  c = 0xFD; break;
    case MENU_STATE_PLAY: c = 0xFC; break;
    default:              c = ' ';  break;
  }
  lcd_putc(LCD_WIDTH-1-5-38, 1, c, SMALLFONT, 1, bgcolor, edgecolor);

  switch(menu_format)
  {
    case FORMAT_WAV:  strcpy(buf, "WAV"); break;
    case FORMAT_MP3:  strcpy(buf, "MP3"); break;
    case FORMAT_AAC:  strcpy(buf, "AAC"); break;
    case FORMAT_OGG:  strcpy(buf, "OGG"); break;
    case FORMAT_WMA:  strcpy(buf, "WMA"); break;
    case FORMAT_FLAC: strcpy(buf, "FLA"); break;
    default:          strcpy(buf, "   ");  break;
  }
  lcd_puts(LCD_WIDTH-1-5-65, 2, buf, SMALLFONT, 1, bgcolor, edgecolor);

  if(menu_bitrate)
  {
    itoa(menu_bitrate, buf, 10);
  }
  else
  {
    strcpy(buf, "   ");
  }
  lcd_puts(LCD_WIDTH-1-5-93, 2, buf, SMALLFONT, 1, bgcolor, edgecolor);

  return;
}


void menu_setinfo(const char *info)
{
  DEBUGOUT("Menu: info: %s\n", info);

  strncpy(gbuf.menu.info, info, MAX_INFO-1);
  if(menu_mode == MODE_INFO)
  {
    menu_drawwndinfo(1);
  }

  return;
}


void menu_setname(const char *name)
{
  DEBUGOUT("Menu: name: %s\n", name);

  strncpy(gbuf.menu.name, name, MAX_NAME-1);
  if(menu_mode == MODE_INFO)
  {
    menu_drawwndinfo(1);
  }

  return;
}


void menu_setbitrate(unsigned int bitrate)
{
  menu_bitrate = bitrate;
  if(menu_mode == MODE_INFO)
  {
    menu_drawstatus();
  }

  return;
}


void menu_setformat(unsigned int format)
{
  menu_format = format;
  if(menu_mode == MODE_INFO)
  {
    menu_drawstatus();
  }

  return;
}


void menu_setstatus(unsigned int status)
{
  menu_status = status;

  if(status == MENU_STATE_STOP)
  {
    menu_format  = FORMAT_UNKNOWN;
    menu_bitrate = 0;
  }

  if(menu_mode == MODE_INFO)
  {
    menu_drawstatus();
  }

  return;
}


void menu_drawwndinfo(unsigned int redraw)
{
  if(redraw)
  {
    menu_drawstatus();
    menu_drawvol();
    menu_drawdate();
    menu_drawclock(HOUR_CHANGED);
  
    lcd_putline(5, 18, gbuf.menu.name, NORMALFONT, 1, fgcolor, bgcolor);
    lcd_putlinebr(5, 40, gbuf.menu.info, SMALLFONT, 1, fgcolor, bgcolor);
  }

  return;
}


void menu_drawwnd(unsigned int redraw)
{
  unsigned int i;

  if(standby_isactive())
  {
    return;
  }

  //draw background
  if(redraw)
  {
    //title bar
    lcd_fillrect(0, 0, LCD_WIDTH-1, 10, edgecolor);

    //text background
    lcd_fillrect(0, 11, LCD_WIDTH-1, LCD_HEIGHT-1, bgcolor);

    //text
    switch(menu_mode)
    {
      case MODE_INFO:
        lcd_puts(4, 2, APPNAME, SMALLFONT, 1, bgcolor, edgecolor);
        lcd_drawline(0, 35, LCD_WIDTH-1, 35, edgecolor);
        lcd_fillrect(0, LCD_HEIGHT-46, LCD_WIDTH-1, LCD_HEIGHT-1, fgcolor);
        break;
      case MODE_MAIN:
        lcd_puts( 4, 2, APPNAME, SMALLFONT, 1, bgcolor, edgecolor);
        break;
      case MODE_SUB:
        if(gbuf.menu.file[0] != 0)
        {
          for(i=0; (strlen(gbuf.menu.file+i)*SMALLFONT_WIDTH+5) > (LCD_WIDTH-1); i++);
          lcd_puts(4, 2, gbuf.menu.file+i, SMALLFONT, 1, bgcolor, edgecolor);
        }
        else
        {
          lcd_puts(4, 2, mainmenu[menu_sub].name, SMALLFONT, 1, bgcolor, edgecolor);
        }
        break;
    }
  }

  //draw text, controls...
  switch(menu_mode)
  {
    case MODE_INFO: menu_drawwndinfo(redraw); break;
    case MODE_MAIN: menu_drawwndmain(redraw); break;
    case MODE_SUB:  menu_drawwndsub(redraw);  break;
  }

  return;
}


void menu_drawctrl(CONTROL *ctrl)
{
  unsigned int i, x;
  char *ptr;

  switch(ctrl->type)
  {
    case CTRL_TEXT:
      x = lcd_puts(ctrl->x1+2, ctrl->y1+2, ctrl->val, NORMALFONT, 1, fgcolor, bgcolor);
      lcd_fillrect(x, ctrl->y1+2, ctrl->x2-2, ctrl->y2-2, bgcolor);
      break;

    case CTRL_BUTTON:
      lcd_fillrect(ctrl->x1, ctrl->y1, ctrl->x2, ctrl->y2, edgecolor);
      lcd_puts(ctrl->x1+2, ctrl->y1+2, ctrl->val, NORMALFONT, 1, fgcolor, edgecolor);
      if(ctrl->sel)
      {
        lcd_drawrect(ctrl->x1, ctrl->y1, ctrl->x2, ctrl->y2, selcolor);
      }
      else
      {
        lcd_drawrect(ctrl->x1, ctrl->y1, ctrl->x2, ctrl->y2, edgecolor);
      }
      break;

    case CTRL_CHECKBOX:
      lcd_fillrect(ctrl->x1, ctrl->y1, ctrl->x2, ctrl->y2, edgecolor);
      if(ctrl->p1) //checked
      {
        lcd_puts(ctrl->x1+2, ctrl->y1+2, ctrl->val, NORMALFONT, 1, selcolor, edgecolor);
      }
      else
      {
        lcd_puts(ctrl->x1+2, ctrl->y1+2, ctrl->val, NORMALFONT, 1, bgcolor, edgecolor);
      }
      if(ctrl->sel)
      {
        lcd_drawrect(ctrl->x1, ctrl->y1, ctrl->x2, ctrl->y2, selcolor);
      }
      else
      {
        lcd_drawrect(ctrl->x1, ctrl->y1, ctrl->x2, ctrl->y2, edgecolor);
      }
      break;

    case CTRL_INPUT:
      ptr = ctrl->val + ctrl->p1;
      for(i=0, x=ctrl->x1+2; (i<ctrl->len) && *ptr; i++)
      {
        if(ctrl->sel && (ctrl->p2 == 0xFFFF)) //select all
        {
          x = lcd_putc(x, ctrl->y1+2, *ptr++, NORMALFONT, 1, fgcolor, edgecolor);
        }
        else if(ctrl->sel && ((ctrl->p1+i) == ctrl->p2))
        {
          x = lcd_putc(x, ctrl->y1+2, *ptr++, NORMALFONT, 1, fgcolor, edgecolor);
        }
        else
        {
          x = lcd_putc(x, ctrl->y1+2, *ptr++, NORMALFONT, 1, fgcolor, bgcolor);
        }
      }
      lcd_fillrect(x, ctrl->y1+2, ctrl->x2-2, ctrl->y2-2, bgcolor);
      if(ctrl->sel)
      {
        lcd_drawrect(ctrl->x1, ctrl->y1, ctrl->x2, ctrl->y2, selcolor);
      }
      else
      {
        lcd_drawrect(ctrl->x1, ctrl->y1, ctrl->x2, ctrl->y2, edgecolor);
      }
      break;
  }

  return;
}


void menu_createctrl(CONTROL *ctrl, unsigned int type, unsigned int sel, unsigned int x, unsigned int y, unsigned int p, char *value)
{
  unsigned int len=0;

  memset(ctrl, 0, sizeof(CONTROL));

  switch(type)
  {
    case CTRL_TEXT:
      break;
    case CTRL_BUTTON:
      break;
    case CTRL_CHECKBOX:
      ctrl->p1 = (p!=0)?1:0;
      break;
    case CTRL_INPUT:
      len = p;
      break;
  }

  if(len == 0)
  {
    len = strlen(value);
  }

  ctrl->type = type;
  ctrl->x1   = x;
  ctrl->y1   = y;
  ctrl->x2   = x + 3 + (NORMALFONT_WIDTH*len);
  ctrl->y2   = y + 3 + NORMALFONT_HEIGHT;
  ctrl->val  = value;
  ctrl->len  = len;
  ctrl->sel  = sel;

  menu_drawctrl(ctrl);

  return;
}


void menu_drawdlg(const char *title, const char *msg)
{
  lcd_fillrect(0, 0, LCD_WIDTH-1, NORMALFONT_HEIGHT+2, edgecolor);
  lcd_puts(2, 1, title, NORMALFONT, 1, bgcolor, edgecolor);

  lcd_fillrect(0, NORMALFONT_HEIGHT+2, LCD_WIDTH-1, LCD_HEIGHT-1, bgcolor);
  lcd_putlinebr(2, 20, msg, NORMALFONT, 1, fgcolor, bgcolor);

  return;
}


void menu_drawpopup(const char *msg)
{
  lcd_drawrect(4, (LCD_HEIGHT/2)-11, LCD_WIDTH-1-4, (LCD_HEIGHT/2)+11, edgecolor);
  lcd_fillrect(5, (LCD_HEIGHT/2)-10, LCD_WIDTH-1-5, (LCD_HEIGHT/2)+10, fgcolor);
  lcd_puts(10, (LCD_HEIGHT/2)- 4, msg, SMALLFONT, 1, bgcolor, fgcolor);

  return;
}


void menu_setedgecolor(unsigned int c) { edgecolor = c; }
void menu_setselcolor(unsigned int c)  { selcolor  = c; }
void menu_setfgcolor(unsigned int c)   { fgcolor   = c; }
void menu_setbgcolor(unsigned int c)   { bgcolor   = c; }


unsigned int menu_getedgecolor(void)   { return edgecolor; }
unsigned int menu_getselcolor(void)    { return selcolor;  }
unsigned int menu_getfgcolor(void)     { return fgcolor;   }
unsigned int menu_getbgcolor(void)     { return bgcolor;   }


void menu_init(void)
{
  TIME t;

  DEBUGOUT("Menu: init\n");

  gbuf.menu.name[0]          = 0;
  gbuf.menu.info[0]          = 0;
  gbuf.menu.file[0]          = 0;
  gbuf.menu.name[MAX_NAME-1] = 0;
  gbuf.menu.info[MAX_INFO-1] = 0;
  gbuf.menu.file[MAX_ADDR-1] = 0;

  menu_setstatus(MENU_STATE_STOP);
  gettime(&t);
  daytime(gbuf.menu.name, &t);
  menu_drawwnd(1);

  //auto start
  if(ini_getentry(SETTINGS_FILE, "AUTOSTART", gbuf.menu.file, MAX_ADDR) == 0)
  {
    menu_openfile(gbuf.menu.file);
  }

  return;
}
