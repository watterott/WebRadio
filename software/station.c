#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fatfs/ff.h"
#include "debug.h"
#include "tools.h"
#include "main.h"
#include "io.h"
#include "mmc.h"
#include "vs.h"
#include "eth.h"
#include "eth/utils.h"
#include "eth/rtsp.h"
#include "eth/shoutcast.h"
#include "menu.h"
#include "buffer.h"
#include "station.h"


unsigned int station_item=0;
unsigned int station_status=STATION_CLOSED;
long station_timeout=0;
unsigned int station_try=0;
unsigned int station_bufmin=0, station_bufplay=0, station_bufstart=0;


void station_setbitrate(unsigned int bitrate) //bitrate kbit/s
{
  unsigned int size;

  DEBUGOUT("Station: %i kb/s\n", bitrate);

  if(bitrate >= 8)
  {
    menu_setbitrate(bitrate);
  }

  if(bitrate < 8)
  {
    bitrate = 128;
  }
  station_bufmin   = bitrate/8*512;
  station_bufplay  = station_bufmin*4;
  station_bufstart = station_bufmin*8;

  size = buf_size();
  if(station_bufstart >= size)
  {
    station_bufmin   = size*10UL/100UL; //10% go into buffer mode
    station_bufplay  = size*50UL/100UL; //50% go back to play mode
    station_bufstart = size*80UL/100UL; //80% 1st play buffer
  }

  return;
}


void station_close(void)
{
  if(station_status != STATION_CLOSED)
  {
    shoutcast_close();
    rtsp_close();
    vs_stop();
    station_status = STATION_CLOSED;
    menu_setstatus(MENU_STATE_STOP);
    DEBUGOUT("Station: closed\n");
  }

  return;
}


unsigned int station_open(unsigned int item)
{
  unsigned int r;
  char proto[8];

  menu_drawpopup("Open Station...");

  station_setbitrate(0);
  station_getitem(item, gbuf.menu.name);
  menu_setinfo("");

  if(gbuf.station.addr[0] == 0)
  {
    if(station_getitemaddr(item, gbuf.station.addr) != 0)
    {
      station_init();
      return STATION_ERROR;
    }
  }

  DEBUGOUT("Station: %i %s\n", station_try, gbuf.station.addr);

  //get ip and mac
  atoaddr(gbuf.station.addr, proto, 0, 0, gbuf.station.host, &gbuf.station.port, gbuf.station.file);
  gbuf.station.ip  = atoip(gbuf.station.host);
  gbuf.station.mac = arp_getmac(gbuf.station.ip);
  if(gbuf.station.mac == 0ULL)
  {
    station_init();
    return STATION_ERROR;
  }

  station_item    = item;
  station_status  = STATION_OPEN;
  station_timeout = getontime()+STATION_TIMEOUT;

  vs_start();

  r = STATION_ERROR;
  if(strcmp(proto, "http") == 0) //shoutcast / icecast
  {
    r = shoutcast_open();
  }
  else if(strcmp(proto, "rtsp") == 0) //rtsp
  {
    r = rtsp_open();
  }

  if(r == STATION_OPENED) //opened -> play station
  {
    station_timeout = getontime()+STATION_TIMEOUT;
    menu_setstatus(MENU_STATE_BUF);
    r = STATION_OPENED;
  }
  else if(r == STATION_ADDRMOVED) //addr moved -> open new addr
  {
    station_timeout = getontime();
    r = STATION_OPEN;
  }
  else //if((r == STATION_ERROR) || (r == STATION_ERRTIMEOUT)) //error -> close station
  {
    station_closeitem(); //also clears addr
    r = STATION_ERROR;
  }

  return r;
}


void station_service(void)
{
  buf_service();

  switch(station_status)
  {
    case STATION_OPENED:
      station_timeout = getontime()+STATION_TIMEOUT;
      if(buf_len() < station_bufmin) //buffer
      {
        station_status = STATION_BUFFER;
        vs_pause();
        menu_setstatus(MENU_STATE_BUF);
        DEBUGOUT("Station: buffer\n");
      }
      else
      {
        vs_play();
      }
      break;

    case STATION_BUFFER:
      if(buf_len() > station_bufplay)
      {
        station_status = STATION_OPENED;
        vs_play();
        menu_setstatus(MENU_STATE_PLAY);
        DEBUGOUT("Station: play\n");
      }
      if(getdeltatime(station_timeout) > 0)
      {
        station_close();
        station_status = STATION_OPEN;
        station_timeout = getontime()+1;
      }
      break;

    case STATION_OPEN:
      if(buf_len() > station_bufstart)
      {
        station_status = STATION_OPENED;
        vs_play();
        menu_setstatus(MENU_STATE_PLAY);
        DEBUGOUT("Station: play\n");
      }
      if(getdeltatime(station_timeout) > 0)
      {
        station_close();
        if(station_try)
        {
          station_try--;
          station_open(station_item);
          menu_drawwnd(1);
        }
      }
      break;

    case STATION_CLOSED:
      break;
  }

  return;
}


void station_closeitem(void)
{
  station_close();
  station_init();

  return;
}


unsigned int station_openitem(unsigned int item)
{
  if(item == 0) //back
  {
    return MENU_BACK;
  }
  else //play item
  {
    station_try = STATION_TRY;
    if(station_open(item) != STATION_ERROR)
    {
      return MENU_PLAY;
    }
  }

  return MENU_ERROR;
}


void station_delitem(unsigned int item)
{
  unsigned int i, items;
  char entry[16], newentry[16];

  //delete entry
  sprintf(entry, "FILE%i", item);
  i  = ini_delentry(STATION_FILE, entry);
  sprintf(entry, "TITLE%i", item);
  i |= ini_delentry(STATION_FILE, entry);

  //rename other entries
  for(items=(station_items()-1); (i == 0) && (item<items); item++)
  {
    sprintf(entry, "FILE%i", item+1);
    sprintf(newentry, "FILE%i", item);
    i |= ini_renentry(STATION_FILE, entry, newentry);
    sprintf(entry, "TITLE%i", item+1);
    sprintf(newentry, "TITLE%i", item);
    i |= ini_renentry(STATION_FILE, entry, newentry);
  }

  if(i == 0)
  {
    //set items
    station_setitems(station_items()-2);
  }

  return;
}


void station_moveitem(unsigned int item, unsigned int direction)
{
  char entry[16], newentry[16];

  if(direction)
  {
    if(item > 1)
    {
      sprintf(entry, "FILE%i", item);
      ini_renentry(STATION_FILE, entry, "FILEXX");
      sprintf(entry, "TITLE%i", item);
      ini_renentry(STATION_FILE, entry, "TITLEXX");

      sprintf(entry, "FILE%i", item-1);
      sprintf(newentry, "FILE%i", item);
      ini_renentry(STATION_FILE, entry, newentry);
      sprintf(entry, "TITLE%i", item-1);
      sprintf(newentry, "TITLE%i", item);
      ini_renentry(STATION_FILE, entry, newentry);

      sprintf(newentry, "FILE%i", item-1);
      ini_renentry(STATION_FILE, "FILEXX", newentry);
      sprintf(newentry, "TITLE%i", item-1);
      ini_renentry(STATION_FILE, "TITLEXX", newentry);
    }
  }
  else
  {
    if(item < (station_items()-1))
    {
      sprintf(entry, "FILE%i", item);
      ini_renentry(STATION_FILE, entry, "FILEXX");
      sprintf(entry, "TITLE%i", item);
      ini_renentry(STATION_FILE, entry, "TITLEXX");

      sprintf(entry, "FILE%i", item+1);
      sprintf(newentry, "FILE%i", item);
      ini_renentry(STATION_FILE, entry, newentry);
      sprintf(entry, "TITLE%i", item+1);
      sprintf(newentry, "TITLE%i", item);
      ini_renentry(STATION_FILE, entry, newentry);

      sprintf(newentry, "FILE%i", item+1);
      ini_renentry(STATION_FILE, "FILEXX", newentry);
      sprintf(newentry, "TITLE%i", item+1);
      ini_renentry(STATION_FILE, "TITLEXX", newentry);
    }
  }

  return;
}


void station_setitemaddr(unsigned int item, const char *addr)
{
  char entry[16];

  if(item)
  {
    sprintf(entry, "FILE%i", item);
    ini_setentry(STATION_FILE, entry, addr);
  }

  return;
}


unsigned int station_getitemaddr(unsigned int item, char *addr)
{
  char entry[16];

  *addr = 0;

  if(item)
  {
    sprintf(entry, "FILE%i", item);
    if(ini_getentry(STATION_FILE, entry, addr, MAX_ADDR) == 0)
    {
      return 0;
    }
  }

  return 1;
}


void station_setitem(unsigned int item, const char *name)
{
  char entry[16];

  if(item)
  {
    sprintf(entry, "TITLE%i", item);
    ini_setentry(STATION_FILE, entry, name);
  }

  return;
}


void station_getitem(unsigned int item, char *name)
{
  char entry[16];

  if(item == 0) //back
  {
    strcpy(name, MENU_BACKTXT);
  }
  else
  {
    sprintf(entry, "TITLE%i", item);
    if(ini_getentry(STATION_FILE, entry, name, MAX_NAME) != 0)
    {
      sprintf(entry, "FILE%i", item);
      ini_getentry(STATION_FILE, entry, name, MAX_NAME);
    }
  }

  return;
}


void station_setitems(unsigned int items)
{
  char value[16];

  sprintf(value, "%i", items);
  ini_setentry(STATION_FILE, "NUMBEROFENTRIES", value);

  return;
}


unsigned int station_items(void)
{
  char value[16];

  if(ini_getentry(STATION_FILE, "NUMBEROFENTRIES", value, 16) == 0)
  {
    return atoi(value)+1; //add first entry ("<< back <<")
  }

  return 1;
}


void station_init(void)
{
  DEBUGOUT("Station: init\n");

  station_item     = 0;
  station_status   = STATION_CLOSED;
  station_try      = STATION_TRY;

  gbuf.station.name[0] = 0;
  gbuf.station.info[0] = 0;
  gbuf.station.addr[0] = 0;

  gbuf.station.name[0] = 0;
  gbuf.station.info[0] = 0;
  gbuf.station.addr[0] = 0;
  gbuf.station.name[MAX_NAME-1] = 0;
  gbuf.station.info[MAX_INFO-1] = 0;
  gbuf.station.addr[MAX_ADDR-1] = 0;

  menu_setstatus(MENU_STATE_STOP);

  return;
}
