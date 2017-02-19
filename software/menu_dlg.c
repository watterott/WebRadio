#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "fatfs/ff.h"
#include "tools.h"
#include "main.h"
#include "io.h"
#include "lcd.h"
#include "mmc.h"
#include "eth.h"
#include "eth/utils.h"
#include "settings.h"
#include "alarm.h"
#include "menu.h"
#include "menu_dlg.h"


#define CTRL_SELECTION (0)
#define CTRL_ACTIVE    (1)
#define CTRL_MODIFY    (2)


// "/ \ : * ? " < > |" not allowed
#define CHARACTERS (11+26+19)
const char characters[CHARACTERS] = 
{
  ' ', '0','1','2','3','4','5','6','7','8','9',
  'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
  '!','#','$','%','&','\'','`','(',')','+','-',',','.',';','=','[',']','_','/'
};


char next_char(char c, unsigned int dir) //char, direction (0=-, 1=+)
{
  unsigned int i;

  //find char
  for(i=0; i<CHARACTERS; i++)
  {
    if(c == characters[i])
    {
      break;
    }
  }

  if(dir) //++
  {
    if(i == (CHARACTERS-1))
    {
      i = 0;
    }
    else
    {
      i++;
    }
  }
  else //--
  {
    if(i == 0)
    {
      i = CHARACTERS-1;
    }
    else
    {
      i--;
    }
  }

  return characters[i];
}


unsigned int dlg_ip(const char* title, IP_Addr *value)
{
  int i, close, state, sel, sw;
  const int ctrls=6;
  CONTROL ctrl[ctrls];
  IP_Addr old_val;
  char buf[4][4];

  //prepare buffers
  itoa((*value&0x000000FF)>> 0, buf[0], 10);
  itoa((*value&0x0000FF00)>> 8, buf[1], 10);
  itoa((*value&0x00FF0000)>>16, buf[2], 10);
  itoa((*value&0xFF000000)>>24, buf[3], 10);

  //create dialog
  menu_drawdlg(title, "");
  menu_createctrl(&ctrl[0], CTRL_BUTTON, 0, 100, 80, 0, "  OK  ");
  menu_createctrl(&ctrl[1], CTRL_BUTTON, 1,  20, 80, 0, " Abort ");
  menu_createctrl(&ctrl[2], CTRL_INPUT,  0, 125, 40, 3, buf[3]); //4
  menu_createctrl(&ctrl[3], CTRL_INPUT,  0,  90, 40, 3, buf[2]); //3
  menu_createctrl(&ctrl[4], CTRL_INPUT,  0,  55, 40, 3, buf[1]); //2
  menu_createctrl(&ctrl[5], CTRL_INPUT,  0,  20, 40, 3, buf[0]); //1
  ctrl[2].p2 = ctrl[3].p2 = ctrl[4].p2 = ctrl[5].p2 = 0xFFFF; //mark whole edit on selection
  sel = 1;

  close   = 0;
  state   = CTRL_SELECTION;
  old_val = *value;
  do
  {
    sw = dlg_service();
    if(sw == 0)
    {
      continue;
    }

    switch(state)
    {
      case CTRL_SELECTION:
        switch(sw)
        {
          case SW_ENTER:
            if     (sel == 0){ close = 1; } //OK
            else if(sel == 1){ close = 2; } //Abort
            else             { state = CTRL_MODIFY; }
            break;
          case SW_UP:
            ctrl[sel].sel = 0;
            sel--;
            break;
          case SW_DOWN:
            ctrl[sel].sel = 0;
            sel++;
            break;
        }
        if     (sel < 0)     { sel = 0; }
        else if(sel >= ctrls){ sel = ctrls-1; }
        ctrl[sel].sel = 1;
        break;

      case CTRL_MODIFY:
        i = atoi(ctrl[sel].val);
        switch(sw)
        {
          case SW_ENTER:
            state = CTRL_SELECTION;
            break;
          case SW_UP:
            if(i < 0xFF){ i++; }
            else        { i = 0; }
            break;
          case SW_DOWN:
            if(i > 0x00){ i--; }
            else        { i = 0xFF; }
            break;
        }
        itoa(i, ctrl[sel].val, 10);
        break;
    }

    for(i=0; i<ctrls; i++)
    {
      menu_drawctrl(&ctrl[i]);
    }
  }while(close == 0);

  *value = ((atoi(buf[0])&0xFF)<< 0) | 
           ((atoi(buf[1])&0xFF)<< 8) |
           ((atoi(buf[2])&0xFF)<<16) |
           ((atoi(buf[3])&0xFF)<<24);
  if((close == 1) && (*value != old_val)) //1=OK, 2=ABORT
  {
    return 0;
  }

  return 1;
}


unsigned int dlg_rgb(const char* title, unsigned int *value)
{
  int i, close, state, sel, sw;
  const int ctrls=5;
  CONTROL ctrl[ctrls];
  unsigned int old_val;
  char buf_r[4], buf_g[4], buf_b[4];

  //prepare buffers
  itoa(GET_RED(*value),   buf_r, 10);
  itoa(GET_GREEN(*value), buf_g, 10);
  itoa(GET_BLUE(*value),  buf_b, 10);

  //create dialog
  menu_drawdlg(title, "");
  menu_createctrl(&ctrl[0], CTRL_BUTTON, 0, 100, 100, 0, "  OK  ");
  menu_createctrl(&ctrl[1], CTRL_BUTTON, 1,  20, 100, 0, " Abort ");
  menu_createctrl(&ctrl[2], CTRL_TEXT,   0, 112-4,50, 0, "Blue");
  menu_createctrl(&ctrl[2], CTRL_INPUT,  0, 112,  70, 3, buf_b); //B
  menu_createctrl(&ctrl[3], CTRL_TEXT,   0,  72-8,50, 0, "Green");
  menu_createctrl(&ctrl[3], CTRL_INPUT,  0,  72,  70, 3, buf_g); //G
  menu_createctrl(&ctrl[4], CTRL_TEXT,   0,  32-0,50, 0, "Red");
  menu_createctrl(&ctrl[4], CTRL_INPUT,  0,  32,  70, 3, buf_r); //R
  lcd_fillrect(60, 20, 111, 42, *value);
  ctrl[2].p2 = ctrl[3].p2 = ctrl[4].p2 = 0xFFFF; //mark whole edit on selection
  sel = 1;

  close   = 0;
  state   = CTRL_SELECTION;
  old_val = *value;
  do
  {
    sw = dlg_service();
    if(sw == 0)
    {
      continue;
    }

    switch(state)
    {
      case CTRL_SELECTION:
        switch(sw)
        {
          case SW_ENTER:
            if     (sel == 0){ close = 1; } //OK
            else if(sel == 1){ close = 2; } //Abort
            else             { state = CTRL_MODIFY; }
            break;
          case SW_UP:
            ctrl[sel].sel = 0;
            sel--;
            break;
          case SW_DOWN:
            ctrl[sel].sel = 0;
            sel++;
            break;
        }
        if     (sel < 0)     { sel = 0; }
        else if(sel >= ctrls){ sel = ctrls-1; }
        ctrl[sel].sel = 1;
        break;

      case CTRL_MODIFY:
        i = atoi(ctrl[sel].val);
        switch(sw)
        {
          case SW_ENTER:
            state = CTRL_SELECTION;
            break;
          case SW_UP:
            if(i < 0xFF){ i++; }
            else        { i = 0; }
            break;
          case SW_DOWN:
            if(i > 0x00){ i--; }
            else        { i = 0xFF; }
            break;
        }
        itoa(i, ctrl[sel].val, 10);
        *value = RGB(atoi(buf_r), atoi(buf_g), atoi(buf_b));
        lcd_fillrect(60, 20, 111, 42, *value);
        break;
    }

    for(i=0; i<ctrls; i++)
    {
      menu_drawctrl(&ctrl[i]);
    }
  }while(close == 0);

  if((close == 1) && (*value != old_val)) //1=OK, 2=ABORT
  {
    return 0;
  }

  return 1;
}


unsigned int dlg_or(const char* title, int *value, int p1, int p2)
{
  int i, close, state, sel, sw, old_val;
  const int ctrls=3;
  CONTROL ctrl[ctrls];
  char buf[32];

  //prepare buffer
  memset(buf, 0, sizeof(buf));
  itoa(*value, buf, 10);

  //create dialog
  menu_drawdlg(title, "");
  menu_createctrl(&ctrl[0], CTRL_BUTTON, 0, 100, 80,  0, "  OK  ");
  menu_createctrl(&ctrl[1], CTRL_BUTTON, 1,  20, 80,  0, " Abort ");
  menu_createctrl(&ctrl[2], CTRL_INPUT,  0,  20, 40, 16, buf);
  ctrl[2].p2 = 0xFFFF; //mark whole edit on modify
  sel = 1;

  close   = 0;
  state   = CTRL_SELECTION;
  old_val = *value;
  do
  {
    sw = dlg_service();
    if(sw == 0)
    {
      continue;
    }

    switch(state)
    {
      case CTRL_SELECTION:
        switch(sw)
        {
          case SW_ENTER:
            if     (sel == 0){ close = 1; } //OK
            else if(sel == 1){ close = 2; } //Abort
            else             { state = CTRL_MODIFY; }
            break;
          case SW_UP:
            ctrl[sel].sel = 0;
            sel--;
            break;
          case SW_DOWN:
            ctrl[sel].sel = 0;
            sel++;
            break;
        }
        if(sel < 0)          { sel = 0; }
        else if(sel >= ctrls){ sel = ctrls-1; }
        ctrl[sel].sel = 1;
        break;

      case CTRL_MODIFY:
        switch(sw)
        {
          case SW_ENTER:
            state = CTRL_SELECTION;
            break;
          case SW_UP:
          case SW_VOLP:
            if(*value == p1){ *value = p2; }
            else            { *value = p1; }
            break;
          case SW_DOWN:
          case SW_VOLM:
            if(*value == p1){ *value = p2; }
            else            { *value = p1; }
            break;
        }
        itoa(*value, buf, 10);
        break;
    }

    for(i=0; i<ctrls; i++)
    {
      menu_drawctrl(&ctrl[i]);
    }
  }while(close == 0);

  if((close == 1) && (*value != old_val)) //1=OK, 2=ABORT
  {
    return 0;
  }

  return 1;
}


unsigned int dlg_nr(const char* title, int *value, int min, int max, int step)
{
  int i, close, state, sel, sw, old_val;
  const int ctrls=3;
  CONTROL ctrl[ctrls];
  char buf[32];

  //prepare buffer
  memset(buf, 0, sizeof(buf));
  itoa(*value, buf, 10);

  //create dialog
  menu_drawdlg(title, "");
  menu_createctrl(&ctrl[0], CTRL_BUTTON, 0, 100, 80,  0, "  OK  ");
  menu_createctrl(&ctrl[1], CTRL_BUTTON, 1,  20, 80,  0, " Abort ");
  menu_createctrl(&ctrl[2], CTRL_INPUT,  0,  20, 40, 16, buf);
  ctrl[2].p2 = 0xFFFF; //mark whole edit on selection
  sel = 1;

  close   = 0;
  state   = CTRL_SELECTION;
  old_val = *value;
  do
  {
    sw = dlg_service();
    if(sw == 0)
    {
      continue;
    }

    switch(state)
    {
      case CTRL_SELECTION:
        switch(sw)
        {
          case SW_ENTER:
            if     (sel == 0){ close = 1; } //OK
            else if(sel == 1){ close = 2; } //Abort
            else             { state = CTRL_MODIFY; }
            break;
          case SW_UP:
            ctrl[sel].sel = 0;
            sel--;
            break;
          case SW_DOWN:
            ctrl[sel].sel = 0;
            sel++;
            break;
        }
        if     (sel < 0)     { sel = 0; }
        else if(sel >= ctrls){ sel = ctrls-1; }
        ctrl[sel].sel = 1;
        break;

      case CTRL_MODIFY:
        switch(sw)
        {
          case SW_ENTER:
            state = CTRL_SELECTION;
            break;
          case SW_UP:
            if(*value < max){ *value = *value + step; }
            else            { *value = min; }
            break;
          case SW_DOWN:
            if(*value > min){ *value = *value - step; }
            else            { *value = max; }
            break;
        }
        if(*value > max){ *value = max; }
        if(*value < min){ *value = min; }
        itoa(*value, buf, 10);
        break;
    }

    for(i=0; i<ctrls; i++)
    {
      menu_drawctrl(&ctrl[i]);
    }
  }while(close == 0);

  if((close == 1) && (*value != old_val)) //1=OK, 2=ABORT
  {
    return 0;
  }

  return 1;
}


unsigned int dlg_str(const char* title, const char *value, unsigned int len, char *buf, unsigned int buf_len)
{
  int i, close, state, sel, sw;
  const int ctrls=3;
  CONTROL ctrl[ctrls];

  if(len == 0)
  {
    len = buf_len-1; //-1 -> "<" symbol at start
  }

  //prepare buffer
  memset(buf, 0, buf_len);
  buf[0] = '<';
  buf[1] = 0;
  strtoupper((buf+1), value);

  //create dialog
  menu_drawdlg(title, "");
  menu_createctrl(&ctrl[0], CTRL_BUTTON, 0, 100, 80,  0, "  OK  ");
  menu_createctrl(&ctrl[1], CTRL_BUTTON, 1,  20, 80,  0, " Abort ");
  menu_createctrl(&ctrl[2], CTRL_INPUT,  0,   5, 40, 20, buf);
  sel = 1;

  close = 0;
  state = CTRL_SELECTION;
  do
  {
    sw = dlg_service();
    if(sw == 0)
    {
      continue;
    }

    switch(state)
    {
      case CTRL_SELECTION:
        switch(sw)
        {
          case SW_ENTER:
            if     (sel == 0){ close = 1; } //OK
            else if(sel == 1){ close = 2; } //Abort
            else             { state = CTRL_ACTIVE; }
            break;
          case SW_UP:
            ctrl[sel].sel = 0;
            sel--;
            break;
          case SW_DOWN:
            ctrl[sel].sel = 0;
            sel++;
            break;
        }
        if     (sel < 0)     { sel = 0; }
        else if(sel >= ctrls){ sel = ctrls-1; }
        ctrl[sel].sel = 1;
        break;

      case CTRL_ACTIVE:
        switch(sw)
        {
          case SW_ENTER:
            if(ctrl[sel].p2 == 0) //first char
            {
              state = CTRL_SELECTION;
            }
            else
            {
              state = CTRL_MODIFY;
            }
            break;
          case SW_UP:
            if(ctrl[sel].p2 < len)
            {
              ctrl[sel].p2++;
              if(ctrl[sel].p2 >= (ctrl[sel].p1+20))
              {
                ctrl[sel].p1++;
              }
              if(ctrl[sel].val[ctrl[sel].p2] == 0) //extend string
              {
                ctrl[sel].val[ctrl[sel].p2]   = ' ';
                ctrl[sel].val[ctrl[sel].p2+1] = 0;
              }
            }
            break;
          case SW_DOWN:
            if(ctrl[sel].p2 > 0)
            {
              ctrl[sel].p2--;
              if(ctrl[sel].p2 < ctrl[sel].p1)
              {
                ctrl[sel].p1--;
              }
            }
            break;
        }
        break;

      case CTRL_MODIFY:
        switch(sw)
        {
          case SW_ENTER:
            state = CTRL_ACTIVE;
            break;
          case SW_UP:
            ctrl[sel].val[ctrl[sel].p2] = next_char(ctrl[sel].val[ctrl[sel].p2], 1); //++
            break;
          case SW_DOWN:
            ctrl[sel].val[ctrl[sel].p2] = next_char(ctrl[sel].val[ctrl[sel].p2], 0); //--
            break;
        }
        break;
    }

    for(i=0; i<ctrls; i++)
    {
      menu_drawctrl(&ctrl[i]);
    }
  }while(close == 0);

  //remove < symbol at beginning
  buf[0] = ' ';
  strrmvspace(buf, buf);

  if(close == 1) //1=OK, 2=ABORT
  {
    return 0;
  }

  return 1;
}


unsigned int dlg_alarmtime(ALARMTIME *time)
{
  int i, close, state, sel, sw;
  const int ctrls=12;
  CONTROL ctrl[ctrls];
  ALARMTIME t;
  char buf[2][4];

  //prepare buffers
  memcpy(&t, time, sizeof(ALARMTIME));
  sprintf(buf[0], "%02i", t.h); //hour
  sprintf(buf[1], "%02i", t.m); //min

  //create dialog
  menu_drawdlg("Alarm time", "");
  menu_createctrl(&ctrl[ 0], CTRL_BUTTON,    0, 100, 100, 0,                "  OK  ");
  menu_createctrl(&ctrl[ 1], CTRL_BUTTON,    1,  20, 100, 0,                " Abort ");
  menu_createctrl(&ctrl[ 2], CTRL_CHECKBOX,  0,  65,  75, (t.action),       "  On  "); //on
  menu_createctrl(&ctrl[ 3], CTRL_CHECKBOX,  0, 144,  50, (t.wdays&(1<<0)), "Su"); //Su 0
  menu_createctrl(&ctrl[ 4], CTRL_CHECKBOX,  0, 122,  50, (t.wdays&(1<<6)), "Sa"); //Sa 6
  menu_createctrl(&ctrl[ 5], CTRL_CHECKBOX,  0, 100,  50, (t.wdays&(1<<5)), "Fr"); //Fr 5
  menu_createctrl(&ctrl[ 6], CTRL_CHECKBOX,  0,  78,  50, (t.wdays&(1<<4)), "Th"); //Th 4
  menu_createctrl(&ctrl[ 7], CTRL_CHECKBOX,  0,  56,  50, (t.wdays&(1<<3)), "We"); //We 3
  menu_createctrl(&ctrl[ 8], CTRL_CHECKBOX,  0,  34,  50, (t.wdays&(1<<2)), "Tu"); //Tu 2
  menu_createctrl(&ctrl[ 9], CTRL_CHECKBOX,  0,  12,  50, (t.wdays&(1<<1)), "Mo"); //Mo 1
  menu_createctrl(&ctrl[10], CTRL_INPUT,     0,  92,  25, 2,                buf[1]); //min
  menu_createctrl(&ctrl[11], CTRL_INPUT,     0,  67,  25, 2,                buf[0]); //hour
  ctrl[10].p2 = ctrl[11].p2 = 0xFFFF; //mark whole edit on selection
  sel = 1;

  close = 0;
  state = CTRL_SELECTION;
  do
  {
    sw = dlg_service();
    if(sw == 0)
    {
      continue;
    }

    switch(state)
    {
      case CTRL_SELECTION:
        switch(sw)
        {
          case SW_ENTER:
            if     (sel == 0) { close = 1; } //OK
            else if(sel == 1) { close = 2; } //Abort
            else if(sel >= 10){ state = CTRL_MODIFY; } //hour & min
            else              { ctrl[sel].p1 = 1-ctrl[sel].p1; } //checkboxes
            break;
          case SW_UP:
            ctrl[sel].sel = 0;
            sel--;
            break;
          case SW_DOWN:
            ctrl[sel].sel = 0;
            sel++;
            break;
        }
        if     (sel < 0)     { sel = 0; }
        else if(sel >= ctrls){ sel = ctrls-1; }
        ctrl[sel].sel = 1;
        break;

      case CTRL_MODIFY:
        i = atoi(ctrl[sel].val);
        switch(sw)
        {
          case SW_ENTER:
            state = CTRL_SELECTION;
            break;
          case SW_UP:
            if(sel == 11) //hour
            {
              if(i < 23){ i++; }
              else      { i = 0; }
            }
            else //min
            {
              if(i < 59){ i++; }
              else      { i = 0; }
            }
            break;
          case SW_DOWN:
            if(sel == 11) //hour
            {
              if(i > 0){ i--; }
              else     { i = 23; }
            }
            else //min
            {
              if(i > 0){ i--; }
              else     { i = 59; }
            }
            break;
        }
        sprintf(ctrl[sel].val, "%02i", i);
        break;
    }

    for(i=0; i<ctrls; i++)
    {
      menu_drawctrl(&ctrl[i]);
    }
  }while(close == 0);


  //week days
  t.wdays  = (ctrl[3].p1<<0) | //Su
             (ctrl[9].p1<<1) | //Mo
             (ctrl[8].p1<<2) | //Tu
             (ctrl[7].p1<<3) | //We
             (ctrl[6].p1<<4) | //Th
             (ctrl[5].p1<<5) | //Fr
             (ctrl[4].p1<<6);  //Sa
  t.h      = atoi(ctrl[11].val); //hour
  t.m      = atoi(ctrl[10].val); //min
  t.action = ctrl[2].p1; //on

  if(close == 1) //1=OK, 2=ABORT
  {
    if((t.wdays  != time->wdays) ||
       (t.h      != time->h)     ||
       (t.m      != time->m)     ||
       (t.action != time->action))
    {
      memcpy(time, &t, sizeof(ALARMTIME));
      return 0;
    }
  }

  return 1;
}


unsigned int dlg_rawir(unsigned int i)
{
  unsigned int curr_addr;
  int data;
  char tmp[32];

  menu_drawdlg("Raw IR Data", "Press a key on the\nremote control...");

  curr_addr = ir_getaddr();
  ir_setaddr(IR_ALLADDR);

  do
  {
    data = ir_rawdata();
    if(data & 0x8000)
    {
      sprintf(tmp, " Addr  %i\n Cmd   %i", ((data&0x07C0)>>6), (data&0x3F));
      menu_drawdlg("Raw IR Data", tmp);
    }
  }while(!keys_sw());

  ir_setaddr(curr_addr);

  return 0;
}


unsigned int dlg_msg(const char* title, const char *msg)
{
  menu_drawdlg(title, msg);

  while(dlg_service() != SW_ENTER);

  return 0;
}


int dlg_service(void)
{
  int sw;

  //no ehternet in dialogs eth_service();

  sw = keys_steps();
  if(sw > 0)
  {
    sw = SW_UP;
  }
  else if(sw < 0)
  {
    sw = SW_DOWN;
  }
  else
  {
    sw = keys_sw();
    if((sw == SW_PRESSED) || (sw == SW_PRESSEDLONG))
    {
      sw = SW_ENTER;
    }
    else
    {
      sw = ir_cmd();
      if     (sw == SW_VOLP){ sw = SW_UP; }
      else if(sw == SW_VOLM){ sw = SW_DOWN; }
    }
  }

  return sw;
}
