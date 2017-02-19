#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "debug.h"
#include "tools.h"
#include "main.h"
#include "mmc.h"
#include "eth.h"
#include "settings.h"
#include "alarm.h"
#include "menu.h"
#include "menu_dlg.h"


ALARMTIME alarmtimes[ALARMTIMES];


unsigned int alarm_check(TIME *time)
{
  unsigned int i, wday, h, m;

  wday = 1<<time->wday;
  h    = time->h;
  m    = time->m;

  for(i=0; i<ALARMTIMES; i++)
  {
    if(alarmtimes[i].action && (alarmtimes[i].wdays & wday) && (alarmtimes[i].h == h) && (alarmtimes[i].m == m))
    {
      return alarmtimes[i].action;
    }
  }

  return 0;
}


unsigned int alarm_getfile(char *dst, unsigned int nr)
{
  char entry[16];

  sprintf(entry, "ALARMFILE%i", nr);
  if(ini_getentry(SETTINGS_FILE, entry, dst, MAX_ADDR) == 0)
  {
    return 0;
  }

  return 1;
}


unsigned int alarm_getvol(void)
{
  char buf[16];

  if(ini_getentry(SETTINGS_FILE, "ALARMVOL", buf, 16) == 0)
  {
    return atoi(buf);
  }
  
  return 50;
}


unsigned int alarm_settime(unsigned int item, ALARMTIME *time)
{
  char entry[16], buf[32];

  sprintf(entry, "TIME%i", item);

  sprintf(buf, " %02i:%02i:", time->h, time->m);
  switch(time->action)
  {
    case 0: buf[0] = '!';  break; //alarm off
    //case 1: buf[0] = '+';  break; //play
    case 2: buf[0] = '-';  break; //standby
  }

  if(time->wdays & (1<<1)){ strcat(buf, "Mo"); }
  if(time->wdays & (1<<2)){ strcat(buf, "Tu"); }
  if(time->wdays & (1<<3)){ strcat(buf, "We"); }
  if(time->wdays & (1<<4)){ strcat(buf, "Th"); }
  if(time->wdays & (1<<5)){ strcat(buf, "Fr"); }
  if(time->wdays & (1<<6)){ strcat(buf, "Sa"); }
  if(time->wdays & (1<<0)){ strcat(buf, "Su"); }

  if(ini_setentry(ALARM_FILE, entry, buf) == 0)
  {
    return 0;
  }

  return 1;
}


unsigned int alarm_gettime(unsigned int item, ALARMTIME *time)
{
  char entry[16], buf[MAX_NAME];

  memset(time, 0, sizeof(ALARMTIME));

  sprintf(entry, "TIME%i", item);
  if(ini_getentry(ALARM_FILE, entry, buf, MAX_NAME) == 0)
  {
    return alarm_parsetime(buf, time);
  }

  return 1;
}


unsigned int alarm_parsetime(const char *src, ALARMTIME *time)
{
  char c1, c2;

  memset(time, 0, sizeof(ALARMTIME));

  if(strlen(src) < 5)
  {
    return 1;
  }

  while(*src && (*src == ' ')){ src++; } //skip spaces

  time->action = 1; //default
  switch(*src)
  {
    case '!': time->action = 0;  break; //alarm off
    case '+': time->action = 1;  break; //play
    case '-': time->action = 2;  break; //standby
  }

  while(*src && !isdigit(*src)){ src++; }; //skip non digits
  time->h = atoi(src);                     //get hour
  while(*src && isdigit(*src)) { src++; }; //skip digits

  while(*src && !isdigit(*src)){ src++; }; //skip non digits
  time->m = atoi(src);                     //get min
  while(*src && isdigit(*src)) { src++; }; //skip digits

  while(*src && !isalpha(*src)){ src++; }; //skip non alpha
  time->wdays = 0;
  while(*src) //get days
  {
    c1 = toupper(*src++);
    c2 = toupper(*src++);
    if((c1 == 0) || !isalpha(c1) ||
       (c2 == 0) || !isalpha(c2))
    {
      break;
    }
    else
    {
      if((c1 == 'S') && (c2 == 'U')){ time->wdays |= (1<<0); } //Sunday
      if((c1 == 'M') && (c2 == 'O')){ time->wdays |= (1<<1); } //Monday
      if((c1 == 'T') && (c2 == 'U')){ time->wdays |= (1<<2); } //Tuesday
      if((c1 == 'W') && (c2 == 'E')){ time->wdays |= (1<<3); } //Wednesday
      if((c1 == 'T') && (c2 == 'H')){ time->wdays |= (1<<4); } //Thursday
      if((c1 == 'F') && (c2 == 'R')){ time->wdays |= (1<<5); } //Friday
      if((c1 == 'S') && (c2 == 'A')){ time->wdays |= (1<<6); } //Saturday
    }
  }

  //check values
  if(time->h > 23)
  {
    time->h = 0;
  }
  if(time->m > 59)
  {
    time->m = 0;
  }

  return 0;
}


void alarm_load(void)
{
  unsigned int i;

  //reset all alarm times
  memset(alarmtimes, 0, sizeof(alarmtimes));

  for(i=0; i<ALARMTIMES; i++)
  {
    alarm_gettime(i+1, &alarmtimes[i]);
  }

  return;
}


unsigned int alarm_openitem(unsigned int item)
{
  if(item == 0) //back
  {
    return MENU_BACK;
  }
  else if(item <= ALARMTIMES)
  {
    if(dlg_alarmtime(&alarmtimes[item-1]) == 0) //time modified -> save to ini
    {
      if(alarm_settime(item, &alarmtimes[item-1]) != 0)
      {
        return MENU_ERROR;
      }
    }
  }

  return MENU_NOP;
}


void alarm_getitem(unsigned int item, char *name)
{
  if(item == 0) //back
  {
    strcpy(name, MENU_BACKTXT);
  }
  else if(item <= ALARMTIMES)
  {
    item--;
    sprintf(name, " %02i:%02i ", alarmtimes[item].h, alarmtimes[item].m);
    switch(alarmtimes[item].action)
    {
      case 0: name[0] = '!';  break; //alarm off
      //case 1: name[0] = '+';  break; //alarm: play
      case 2: name[0] = '-';  break; //alarm: standby
    }
    if(alarmtimes[item].wdays & (1<<1)){ strcat(name, "Mo"); }
    if(alarmtimes[item].wdays & (1<<2)){ strcat(name, "Tu"); }
    if(alarmtimes[item].wdays & (1<<3)){ strcat(name, "We"); }
    if(alarmtimes[item].wdays & (1<<4)){ strcat(name, "Th"); }
    if(alarmtimes[item].wdays & (1<<5)){ strcat(name, "Fr"); }
    if(alarmtimes[item].wdays & (1<<6)){ strcat(name, "Sa"); }
    if(alarmtimes[item].wdays & (1<<0)){ strcat(name, "Su"); }
  }

  return;
}


unsigned int alarm_items(void)
{
  return ALARMTIMES+1;
}


void alarm_init(void)
{
  DEBUGOUT("Alarm: init\n");

  return;
}
