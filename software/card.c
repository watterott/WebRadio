#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "fatfs/ff.h"
#include "debug.h"
#include "tools.h"
#include "main.h"
#include "mmc.h"
#include "vs.h"
#include "menu.h"
#include "eth.h"
#include "settings.h"
#include "buffer.h"
#include "card.h"

//0=normal, 1=one-by-one, 2=random, 3=repeat
unsigned int card_status=CARD_CLOSED, card_item=0, card_playmode=0;
long card_timeout=0;


void card_service(void)
{
  unsigned int rd;

  if(card_status != CARD_PLAY)
  {
    return;
  }

  while(vs_buffree() >= CARD_READBUF)
  {
    if(f_read(&gbuf.card.fsrc, gbuf.card.buf, CARD_READBUF, &rd) == FR_OK)
    {
      if(rd)
      {
        vs_bufputs(gbuf.card.buf, rd);
        card_timeout = getontime()+CARD_TIMEOUT;
      }
      else
      {
        break;
      }
    }
    else
    {
      break;
    }
  }

  if(vs_buflen() <= 1) //song end
  {
    DEBUGOUT("Card: buf <= 1\n");
    card_closeitem();
    card_nextitem();
  }
  else if(getdeltatime(card_timeout) > 0) //time out
  {
    DEBUGOUT("Card: timeout\n");
    card_closeitem();
    card_nextitem();
  }
  else
  {
    vs_play();
  }

  return;
}


void card_closeitem(void)
{
  if(card_status != CARD_CLOSED)
  {
    card_status = CARD_CLOSED;
    vs_stop();
    f_close(&gbuf.card.fsrc);
    strshrinkpath(gbuf.card.file);
    menu_setstatus(MENU_STATE_STOP);
    DEBUGOUT("Card: closed\n");
  }

  return;
}


unsigned int card_openfile(const char *file)
{
  if(file != gbuf.card.file)
  {
    strncpy(gbuf.card.file, file, MAX_ADDR-1);
  }

  if(f_open(&gbuf.card.fsrc, gbuf.card.file, FA_OPEN_EXISTING | FA_READ) == FR_OK)
  {
    card_status = CARD_PLAY;
    vs_start();
    menu_setstatus(MENU_STATE_PLAY);
    return MENU_PLAY;
  }

  return MENU_ERROR;
}


unsigned int card_nextitem(void)
{
  unsigned int item, items;

  item  = card_item;
  items = card_items();

  switch(card_playmode)
  {
    case 0: //normal
      break;

    case 1: //1=one-by-one
      for(item++; item <= items; item++)
      {
        if(fs_isdir(gbuf.card.file, item-1) != 0) //open dir
        {
          break;
        }
      }
      if(item <= item)
      {
        card_openitem(item);
      }
      break;

    case 2: //2=random
      break;

    case 3: //3=repeat
      card_openitem(item);
      break;
  }

  menu_drawwnd(1);

  card_item = item;

  return 0;
}


unsigned int card_openitem(unsigned int item)
{
  char tmp[MAX_ADDR];
  unsigned int len;

  if(item == 0) //back
  {
    if(gbuf.card.file[0] == 0)
    {
      return MENU_BACK;
    }
    else //up dir
    {
      strshrinkpath(gbuf.card.file);
    }
  }
  else
  {
    card_item = item;
    item--;
    if(fs_isdir(gbuf.card.file, item) == 0) //open dir
    {
      fs_getitem(gbuf.card.file, item, tmp, MAX_ADDR);
      len = strlen(gbuf.card.file);
      strncat(gbuf.card.file, tmp, MAX_ADDR-1-len);
    }
    else //play item
    {
      fs_getitem(gbuf.card.file, item, gbuf.menu.name, MAX_NAME);
      fs_getitemtag(gbuf.card.file, item, gbuf.menu.info, MAX_INFO);
      fs_getitem(gbuf.card.file, item, tmp, MAX_ADDR);
      len = strlen(gbuf.card.file);
      strncat(gbuf.card.file, "/", MAX_ADDR-1-len);
      strncat(gbuf.card.file, tmp, MAX_ADDR-1-len-1);
      DEBUGOUT("Card: %s\n", gbuf.card.file);

      if(card_openfile(gbuf.card.file) == MENU_PLAY)
      {
        return MENU_PLAY;
      }
      else
      {
        strshrinkpath(gbuf.card.file);
        return MENU_ERROR;
      }
    }
  }

  return MENU_UPDATE;
}


void card_getitem(unsigned int item, char *name)
{
  if(item == 0) //back
  {
    strcpy(name, MENU_BACKTXT);
  }
  else
  {
    fs_getitem(gbuf.card.file, item-1, name, MAX_NAME); //fs_getitemtag
  }

  return;
}


unsigned int card_items(void)
{
  return fs_items(gbuf.card.file)+1;
}


void card_init(void)
{
  DEBUGOUT("Card: init\n");

  card_status   = CARD_CLOSED;
  card_item     = 0;
  card_playmode = settings_getplaymode();

  gbuf.card.name[0] = 0;
  gbuf.card.info[0] = 0;
  gbuf.card.file[0] = 0;
  gbuf.card.name[MAX_NAME-1] = 0;
  gbuf.card.info[MAX_INFO-1] = 0;
  gbuf.card.file[MAX_ADDR-1] = 0;

  menu_setstatus(MENU_STATE_STOP);

  return;
}
