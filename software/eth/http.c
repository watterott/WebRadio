#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../debug.h"
#include "../tools.h"
#include "../main.h"
#include "../io.h"
#include "../lcd.h"
#include "../mmc.h"
#include "../eth.h"
#include "../menu.h"
#include "../station.h"
#include "../share.h"
#include "../card.h"
#include "../alarm.h"
#include "../settings.h"
#include "utils.h"
#include "http_files.h"
#include "http.h"


HTTP_Table http_table[TCP_ENTRIES];


void http_station(char *rx, unsigned int rx_len)
{
  unsigned int i, save=0, add=0, del=0, up=0, down=0, play=0, stop=0;
  char *name, *addr, *item;

  name=0; addr=0; item=0;
  for(; rx_len!=0;)
  {
         if(strncmpi(rx, "name=", 5) == 0)
    { rx += 5; rx_len -= 5; name = rx; i = url_decode(rx, rx, rx_len); rx += i; rx_len -= i; }
    else if(strncmpi(rx, "addr=", 5) == 0)
    { rx += 5; rx_len -= 5; addr = rx; i = url_decode(rx, rx, rx_len); rx += i; rx_len -= i; }
    else if(strncmpi(rx, "item=", 5) == 0)
    { rx += 5; rx_len -= 5; item = rx; i = url_decode(rx, rx, rx_len); rx += i; rx_len -= i; }
    else if(strncmpi(rx, "save=", 5) == 0)
    { rx += 5; rx_len -= 5; save = 1; }
    else if(strncmpi(rx, "add=", 4)  == 0)
    { rx += 4; rx_len -= 4; add  = 1; }
    else if(strncmpi(rx, "del=", 4)  == 0)
    { rx += 4; rx_len -= 4; del  = 1; }
    else if(strncmpi(rx, "up=", 3)   == 0)
    { rx += 3; rx_len -= 3; up   = 1; }
    else if(strncmpi(rx, "down=", 5) == 0)
    { rx += 5; rx_len -= 5; down = 1; }
    else if(strncmpi(rx, "play=", 5) == 0)
    { rx += 5; rx_len -= 5; play = 1; }
    else if(strncmpi(rx, "stop=", 5) == 0)
    { rx += 5; rx_len -= 5; stop = 1; }
    else
    { rx++;    rx_len--; }
  }

  if(save && name && addr && item)
  {
    if((strlen(name) > 0) && (strlen(addr) > 3) && (strlen(item) > 0))
    {
      i = atoi(item);
      station_setitemaddr(i, addr);
      station_setitem(i, name);
      menu_drawwnd(1);
    }
  }
  else if(add && name && addr)
  {
    if((strlen(name) > 0) && (strlen(addr) > 3))
    {
      i = station_items(); //returns item count + 1 (first menu entry "<< back <<")
      station_setitemaddr(i, addr);
      station_setitem(i, name);
      station_setitems(i);
      menu_drawwnd(1);
    }
  }
  else if(del && item)
  {
    if(strlen(item) > 0)
    {
      i = atoi(item);
      station_delitem(i);
      menu_drawwnd(1);
    }
  }
  else if(up && item)
  {
    if(strlen(item) > 0)
    {
      i = atoi(item);
      station_moveitem(i, 1);
      menu_drawwnd(1);
    }
  }
  else if(down && item)
  {
    if(strlen(item) > 0)
    {
      i = atoi(item);
      station_moveitem(i, 0);
      menu_drawwnd(1);
    }
  }
  else if(play && item)
  {
    if(strlen(item) > 0)
    {
      standby_off();
      menu_openfile(item);
    }
  }
  else if(stop)
  {
    if(!standby_isactive())
    {
      menu_stopfile();
    }
  }

  return;
}


void http_alarm(char *rx, unsigned int rx_len)
{
  unsigned int i, save;
  char *time, *item;
  ALARMTIME t;

  save=0;
  time=0; item=0;
  for(; rx_len!=0;)
  {
         if(strncmpi(rx, "time=", 5) == 0)
    { rx += 5; rx_len -= 5; time = rx; i = url_decode(rx, rx, rx_len); rx += i; rx_len -= i; }
    else if(strncmpi(rx, "item=", 5) == 0)
    { rx += 5; rx_len -= 5; item = rx; i = url_decode(rx, rx, rx_len); rx += i; rx_len -= i; }
    else if(strncmpi(rx, "save=", 5) == 0)
    { rx += 5; rx_len -= 5; save = 1; }
    else
    { rx++;    rx_len--; }
  }

  if(save && time && item)
  {
    if((strlen(time) > 3) || (strlen(item) > 0))
    {
      i = atoi(item);
      alarm_parsetime(time, &t);
      alarm_settime(i, &t);
      alarm_load();
      menu_drawwnd(1);
    }
  }

  return;
}


void http_settings(char *rx, unsigned int rx_len)
{
  unsigned int item, found, len;
  int i;
  char buf[16];
  MAC_Addr mac;
  IP_Addr ip;
  unsigned int rgb;

  for(; rx_len!=0;)
  {
    len = strlen("restartwebradio=");
    if(strncmpi(rx, "restartwebradio=", len) == 0)
    {
      rx += len; rx_len -= len;
      cpu_reset();
    }

    for(item=0, found=0; item<SETTINGSITEMS; item++)
    {
      if(settingsmenu[item].ini[0] == 0)
      {
        continue;
      }
      len = sprintf(buf, "%s=", settingsmenu[item].ini);
      if(strncmpi(rx, buf, len) == 0)
      {
        rx += len; rx_len -= len;
        len = url_decode(rx, rx, rx_len);
        i   = 0;
        switch(settingsmenu[item].format)
        {
          case F_NR:  //p1-p2, p3=step size
            i = atoi(rx);
                 if(i < settingsmenu[item].p1){ i = settingsmenu[item].p1; }
            else if(i > settingsmenu[item].p2){ i = settingsmenu[item].p2; }
            itoa(i, buf, 10);
            ini_setentry(SETTINGS_FILE, settingsmenu[item].ini, buf);
            if(settingsmenu[item].set){ settingsmenu[item].set((void*)(int)i); }
            break;

          case F_OR:  //p1 or p2
            i = atoi(rx);
            if((i != settingsmenu[item].p1) && 
               (i != settingsmenu[item].p2)){ i = settingsmenu[item].p1; }
            itoa(i, buf, 10);
            ini_setentry(SETTINGS_FILE, settingsmenu[item].ini, buf);
            if(settingsmenu[item].set){ settingsmenu[item].set((void*)(int)i); }
            break;

          case F_STR: //p1=max len
            if((settingsmenu[item].p1 != 0) &&
              (strlen(rx) > (unsigned)settingsmenu[item].p1))
            {
              rx[settingsmenu[item].p1] = 0;
            }
            ini_setentry(SETTINGS_FILE, settingsmenu[item].ini, rx);
            if(settingsmenu[item].set){ settingsmenu[item].set(rx); }
            break;

          case F_MAC:
            mac = atomac(rx);
            ini_setentry(SETTINGS_FILE, settingsmenu[item].ini, mactoa(mac));
            //if(settingsmenu[item].set){ settingsmenu[item].set((void*)(MAC_Addr)mac); }
            break;

          case F_IP:
            ip = atoip(rx);
            ini_setentry(SETTINGS_FILE, settingsmenu[item].ini, iptoa(ip));
            if(settingsmenu[item].set){ settingsmenu[item].set((void*)(IP_Addr)atoip(rx)); }
            break;

          case F_RGB:
            rgb = atorgb(rx);
            sprintf(buf, "%03i,%03i,%03i", GET_RED(rgb), GET_GREEN(rgb), GET_BLUE(rgb));
            ini_setentry(SETTINGS_FILE, settingsmenu[item].ini, buf);
            if(settingsmenu[item].set){ settingsmenu[item].set((void*)(unsigned int)rgb); }
            break;
        }
        rx += len; rx_len -= len;
        found = 1;
        break;
      }
    }
    if(found == 0)
    {
      rx++; rx_len--;
    }
  }

  menu_drawwnd(1);

  return;
}


unsigned int http_sendfile(unsigned int idx, const char *name, unsigned char *tx)
{
  unsigned int len=0, i;

  if(name) //start transfer -> add http header
  {
    http_table[idx].status = HTTP_SEND;
    http_table[idx].file   = http_fid(name, &http_table[idx].fparam);
    http_table[idx].ftype  = http_ftype(http_table[idx].file);
    http_table[idx].flen   = http_flen(http_table[idx].file, http_table[idx].fparam);
    http_table[idx].fpos   = 0;
    http_table[idx].fparse = 0;

    if(http_table[idx].flen == 0) //file not found
    {
      http_table[idx].status = HTTP_CLOSED;
      len = sprintf((char*)tx, HTTP_404_HEADER"Error 404 Not found\r\n\r\n");
      tcp_send(idx, len, 0);
      tcp_close(idx);
      return len;
    }
    else //file found -> send http header
    {
      switch(http_table[idx].ftype)
      {
        case HTML_FILE:
          len = sprintf((char*)tx, HTTP_HTML_HEADER"%i\r\n\r\n", http_table[idx].flen);
          tx += len;
          break;
        case XML_FILE:
          len = sprintf((char*)tx, HTTP_XML_HEADER"%i\r\n\r\n", http_table[idx].flen);
          tx += len;
          break;
        case JS_FILE:
          len = sprintf((char*)tx, HTTP_JS_HEADER"%i\r\n\r\n", http_table[idx].flen);
          tx += len;
          break;
        case CSS_FILE:
          len = sprintf((char*)tx, HTTP_CSS_HEADER"%i\r\n\r\n", http_table[idx].flen);
          tx += len;
          break;
        case TXT_FILE:
          len = sprintf((char*)tx, HTTP_TXT_HEADER"%i\r\n\r\n", http_table[idx].flen);
          tx += len;
          break;
        case ICON_FILE:
          len = sprintf((char*)tx, HTTP_ICON_HEADER"%i\r\n\r\n", http_table[idx].flen);
          tx += len;
          break;
        case GIF_FILE:
          len = sprintf((char*)tx, HTTP_GIF_HEADER"%i\r\n\r\n", http_table[idx].flen);
          tx += len;
          break;
        case JPEG_FILE:
          len = sprintf((char*)tx, HTTP_JPEG_HEADER"%i\r\n\r\n", http_table[idx].flen);
          tx += len;
          break;
      }
    }
  }

  if(http_table[idx].flen) //file found
  {
    switch(http_table[idx].ftype)
    {
      //dynamic content
      case HTML_FILE:
      case XML_FILE:
        i = http_fparse((char*)tx, http_table[idx].file, &http_table[idx].fparse, (ETH_MTUSIZE-IP_HEADERLEN-TCP_HEADERLEN-MAX_ADDR-100), http_table[idx].fparam);
        http_table[idx].fpos += i;
        len += i;
        break;
      //static content
      case JS_FILE:
      case CSS_FILE:
      case TXT_FILE:
      case ICON_FILE:
      case GIF_FILE:
      case JPEG_FILE:
        i = http_fdata(tx, http_table[idx].file, http_table[idx].fpos, (ETH_MTUSIZE-IP_HEADERLEN-TCP_HEADERLEN-MAX_ADDR-100));
        http_table[idx].fpos += i;
        len += i;
        break;
    }
    tcp_send(idx, len, 0);
    if((http_table[idx].fpos >= http_table[idx].flen) || (len == 0))
    {
      http_close(idx);
      tcp_close(idx);
    }
  }

  return len;
}


void http_close(unsigned int idx)
{
  http_table[idx].status = HTTP_CLOSED;
  http_table[idx].flen   = 0;

  return;
}


void http_tcpapp(unsigned int idx, char *rx, unsigned int rx_len, unsigned char *tx)
{
  unsigned int tx_len;

  DEBUGOUT("HTTP: TCP app\n");

  switch(http_table[idx].status)
  {
    case HTTP_CLOSED: //new connection
      if(rx_len)
      {
        if(strncmpi(rx, "GET", 3) == 0)
        {
          rx     += 3+1;
          rx_len -= 3+1;
          http_sendfile(idx, rx, tx);
        }
        else if(strncmpi(rx, "POST", 4) == 0)
        {
          rx     += 4+1;
          rx_len -= 4+1;
          if(strncmpi(rx, "/station", 8) == 0)
          {
            http_table[idx].status = HTTP_STATION;
            rx = (char*)http_skiphd(rx, &rx_len);
          }
          else if(strncmpi(rx, "/alarm", 6) == 0)
          {
            http_table[idx].status = HTTP_ALARM;
            rx = http_skiphd(rx, &rx_len);
          }
          else if(strncmpi(rx, "/settings", 9) == 0)
          {
            http_table[idx].status = HTTP_SETTINGS;
            rx = http_skiphd(rx, &rx_len);
          }
          else
          {
            http_sendfile(idx, rx, tx);
          }
        }
        else
        {
          tx_len = sprintf((char*)tx, HTTP_400_HEADER"Error 400 Bad request\r\n\r\n");
          tcp_send(idx, tx_len, 0);
          tcp_close(idx);
        }
      }
      break;

    case HTTP_SEND:
      http_sendfile(idx, 0, tx);
      break;
  }

  //parse station, alarm or settings
  switch(http_table[idx].status)
  {
    case HTTP_STATION:
      if(rx_len)
      {
        http_station(rx, rx_len);
        http_sendfile(idx, "/station", tx);
      }
      else
      {
        tcp_send(idx, 0, 0);
      }
      break;

    case HTTP_ALARM:
      if(rx_len)
      {
        http_alarm(rx, rx_len);
        http_sendfile(idx, "/alarm", tx);
      }
      else
      {
        tcp_send(idx, 0, 0);
      }
      break;

    case HTTP_SETTINGS:
      if(rx_len)
      {
        http_settings(rx, rx_len);
        http_sendfile(idx, "/settings", tx);
      }
      else
      {
        tcp_send(idx, 0, 0);
      }
      break;
  }

  return;
}
