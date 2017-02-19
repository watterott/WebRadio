#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "fatfs/ff.h"
#include "fatfs/diskio.h"
#include "debug.h"
#include "tools.h"
#include "main.h"
#include "mmc_io.h"
#include "mmc.h"


FATFS fatfs;
#ifndef LOADER
FIL fileobj;
char lfn[_MAX_LFN+1];
#endif


void fs_unmount(void)
{
  DEBUGOUT("MMC: unmount\n");

  f_mount(0, 0);

  return;
}


void fs_mount(void)
{
  unsigned int mount_try=3;

  if(disk_status(0) & STA_NOINIT)
  {
    DEBUGOUT("MMC: init\n");
    while((disk_initialize(0) & STA_NOINIT) && --mount_try);
  }

  DEBUGOUT("MMC: mount\n");

  if(disk_status(0) == 0)
  {
    f_mount(0, &fatfs);
  }

  return;
}


#ifndef LOADER

unsigned int fs_checkitem(FILINFO *finfo)
{
  unsigned int len;
  char *fname, c1, c2, c3, c4, c5;

  if(!(finfo->fattrib & (AM_HID|AM_SYS))) //no system and hidden files
  {
    if(finfo->fattrib & AM_DIR) //directory
    {
      return 0;
    }
    else //file
    {
#if _USE_LFN
      fname = (*finfo->lfname)?finfo->lfname:finfo->fname;
#else
      fname = finfo->fname;
#endif
      len = strlen(fname);
      if(len > 4)
      {
        c5 = toupper(fname[len-5]);
        c4 = toupper(fname[len-4]);
        c3 = toupper(fname[len-3]);
        c2 = toupper(fname[len-2]);
        c1 = toupper(fname[len-1]);
        if((c4 == '.') &&
           (((c3 == 'A') && (c2 == 'A') && (c1 == 'C')) || //AAC
            ((c3 == 'M') && (c2 == 'P') && (c1 == '3')) || //MP3
            ((c3 == 'O') && (c2 == 'G') && (c1 == 'G')) || //OGG
            ((c3 == 'W') && (c2 == 'A') && (c1 == 'V')) || //WAV
            ((c3 == 'W') && (c2 == 'M') && (c1 == 'A'))))  //WMA
        {
          return 0;
        }
        else if((c5 == '.') &&
                ((c4 == 'F') && (c3 == 'L') && (c2 == 'A') && (c1 == 'C')))   //FLAC
        {
          return 0;
        }
      }
    }
  }

  return 1;
}


unsigned int fs_isdir(const char *path, unsigned int item)
{
  FILINFO finfo;
  DIR dir;
  unsigned int i=0;

#if _USE_LFN
  finfo.lfname = lfn;
  finfo.lfsize = sizeof(lfn);
#endif

  if(f_opendir(&dir, path) == FR_OK)
  {
    while((f_readdir(&dir, &finfo) == FR_OK) && finfo.fname[0])
    {
      if(fs_checkitem(&finfo) == 0)
      {
        if(item == i)
        {
          if(finfo.fattrib & AM_DIR)
          {
            return 0;
          }
          break;
        }
        i++;
      }
    }
  }

  return 1;
}


/*
MP3 ID3 Tag v1
offset  len
 0        3        TAG -> ID3v1
 3       30        Song title
 33      30        Artist
 63      30        Album
 93       4        Year
 97      30        Comment
127       1        Genre
*/
void fs_getitemtag(const char *path, unsigned int item, char *dst, unsigned int len)
{
  char tmp[MAX_ADDR];
  unsigned int rd;

  fs_getitem(path, item, dst, len);

  if(dst[0] == '/') //directory
  {
    return;
  }

  rd = strlen(dst);
  if((toupper(dst[rd-3]) == 'M') &&
     (toupper(dst[rd-2]) == 'P') &&
     (toupper(dst[rd-1]) == '3')) //MP3 file
  {
    strcpy(tmp, path);
    strcat(tmp, "/");
    strcat(tmp, dst);
    if(f_open(&fileobj, tmp, FA_OPEN_EXISTING | FA_READ) == FR_OK)
    {
      if(f_lseek(&fileobj, fileobj.fsize-128) == FR_OK)
      {
        if(f_read(&fileobj, tmp, 128, &rd) == FR_OK)
        {
          if((rd == 128) &&
             (tmp[0] == 'T') &&
             (tmp[1] == 'A') &&
             (tmp[2] == 'G'))
          {
            strncpy(dst, tmp+3, 30);
            dst[30] = 0;
          }
        }
      }
      f_close(&fileobj);
    }
  }

  return;
}


void fs_getitem(const char *path, unsigned int item, char *dst, unsigned int len)
{
  FILINFO finfo;
  DIR dir;
  char *fname;
  unsigned int i=0;

#if _USE_LFN
  finfo.lfname = lfn;
  finfo.lfsize = sizeof(lfn);
#endif

  *dst = 0;

  if(f_opendir(&dir, path) == FR_OK)
  {
    while((f_readdir(&dir, &finfo) == FR_OK) && finfo.fname[0])
    {
      if(fs_checkitem(&finfo) == 0)
      {
        if(item == i)
        {
#if _USE_LFN
          fname = (*finfo.lfname)?finfo.lfname:finfo.fname;
#else
          fname = finfo.fname;
#endif
          if(finfo.fattrib & AM_DIR)
          {
            *dst = '/';
            strncpy(dst+1, fname, len-1-1);
            dst[len-1] = 0;
          }
          else
          {
            strncpy(dst, fname, len-1);
            dst[len-1] = 0;
          }
          break;
        }
        i++;
      }
    }
  }

  return;
}


unsigned int fs_items(const char *path)
{
  FILINFO finfo;
  DIR dir;
  unsigned int i=0;

#if _USE_LFN
  finfo.lfname = lfn;
  finfo.lfsize = sizeof(lfn);
#endif

  if(disk_status(0) & STA_NOINIT)
  {
    return 0;
  }

  if(f_opendir(&dir, path) == FR_OK)
  {
    while((f_readdir(&dir, &finfo) == FR_OK) && finfo.fname[0])
    {
      if(fs_checkitem(&finfo) == 0)
      {
        i++;
      }
    }
  }

  return i;
}


void ini_stripfile(FIL *file, unsigned int pos, unsigned int len)
{
  FRESULT res;
  unsigned int rd;
  unsigned char c;

  if((pos+len) >= file->fsize)
  {
    f_lseek(file, pos);
  }
  else
  {
    for(; (pos+len)<file->fsize; pos++)
    {
      f_lseek(file, pos+len);
      res = f_read(file, &c, 1, &rd);
      if((res != FR_OK) || (rd != 1))
      {
        break;
      }
      f_lseek(file, pos);
      f_putc(c, file);
    }
  }
  f_truncate(file);

  return;
}


void ini_extendfile(FIL *file, unsigned int pos, unsigned int len)
{
  FRESULT res;
  unsigned int i, rd;
  unsigned char c;

  if(pos < file->fsize)
  {
    for(i=file->fsize-1; i>pos; i--)
    {
      f_lseek(file, i);
      res = f_read(file, &c, 1, &rd);
      if((res != FR_OK) || (rd != 1))
      {
        break;
      }
      f_lseek(file, i+len);
      f_putc(c, file);
    }
  }

  return;
}


//delete spaces/line breaks at end of file
void ini_delspace(FIL *file)
{
  FRESULT res;
  unsigned int i, rd;
  unsigned char c;

  if(file->fsize)
  {
    for(i=file->fsize-1, c=0; ((c == 0) || (c == '\r') || (c == '\n'));)
    {
      f_lseek(file, i--);
      res = f_read(file, &c, 1, &rd);
      if((res != FR_OK) || (rd != 1))
      {
        break;
      }
    }
    f_truncate(file);
  }

  return;
}


//returns values start position (can not be 0) and entry start position
unsigned int ini_searchentry(FIL *file, const char *entry, unsigned int *entry_start) //entry in upper case
{
  FRESULT res;
  unsigned int i, entry_len, entry_pos, found, rd;
  char c, buf[32]; //entry name max 32-1 characters

  entry_len = strlen(entry);
  entry_pos = file->fptr;

  if(entry_len >= file->fsize)
  {
    return 0;
  }

  found     = 0;
  i         = 0;
  do
  {
    res = f_read(file, &c, 1, &rd);
    if((res != FR_OK) || (rd != 1))
    {
      break;
    }
    switch(c)
    {
      case 0:    //file end
      case '\r': //line end
      case '\n':
        i = 0;
        entry_pos = file->fptr;
        break;

      case '=': //value start
        if(i != 0)
        {
          if(strncmp(buf, entry, entry_len) == 0)
          {
            found = file->fptr;
          }
        }
        break;

      case '#': //comment
        if(i == 0) //line start
        {
          while(1) //skip comment
          {
            res = f_read(file, &c, 1, &rd);
            if((res != FR_OK) || (rd != 1))
            {
              break;
            }
            else if((c == 0) || (c == '\r') || (c == '\n'))
            {
              break;
            }
          }
          entry_pos = file->fptr;
        }
        break;

      default:
        if(c != ' ')
        {
          if(i<31)
          {
            buf[i++] = toupper(c);
            buf[i]   = 0;
          }
        }
        break;
    }
  }while(found == 0);

  if(entry_start && found)
  {
    *entry_start = entry_pos;
  }

  return found;
}


unsigned int ini_getentry(const char *filename, const char *entry, char *value, unsigned int len) //entry in upper case
{
  FRESULT res;
  unsigned int found, rd;
  char c, *ptr;

  ptr  = value;
  *ptr = 0;

  res = f_open(&fileobj, filename, FA_OPEN_EXISTING | FA_READ);
  if(res != FR_OK)
  {
    return 1;
  }

  found = ini_searchentry(&fileobj, entry, 0);
  if(found == 0)
  {
    f_close(&fileobj);
    return 2;
  }

  res = f_lseek(&fileobj, found);
  if(res != FR_OK)
  {
    f_close(&fileobj);
    return 3;
  }

  //read value
  if(len)
  {
    len--; //null at end
    while(len)
    {
      res = f_read(&fileobj, &c, 1, &rd);
      if((res != FR_OK) || (rd != 1))
      {
        break;
      }
      else if((c == 0) || (c == '\r') || (c == '\n'))
      {
        break;
      }
      *ptr++ = c;
      len--;
    }
    *ptr = 0;
  }

  f_close(&fileobj);

  //remove space at start and end of string
  strrmvspace(value, value);

  return 0;
}


unsigned int ini_setentry(const char *filename, const char *entry, const char *value) //entry in upper case
{
  FRESULT res;
  unsigned int i, len, found, rd;
  char c;

  res = f_open(&fileobj, filename, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
  if(res != FR_OK)
  {
    return 1;
  }

  found = ini_searchentry(&fileobj, entry, 0);
  if(found != 0) //entry found
  {
    res = f_lseek(&fileobj, found);
    if(res != FR_OK)
    {
      f_close(&fileobj);
      return 2;
    }
    //calc current value size
    for(i=0; 1; i++)
    {
      res = f_read(&fileobj, &c, 1, &rd);
      if((res != FR_OK) || (rd != 1))
      {
        break;
      }
      if((c == 0) || (c == '\n') || (c == '\r'))
      {
        break;
      }
    }
    //write new value
    len = strlen(value); //new value len
    if(i > len) //new value size is smaller
    {
      ini_stripfile(&fileobj, found, (i-len)); //strip file: old-new
    }
    else if(i < len) //new value size is bigger
    {
      ini_extendfile(&fileobj, found, (len-i)); //extend file: new-old
    }
    //write value to entry
    f_lseek(&fileobj, found);
    f_puts(value, &fileobj);
  }
  else
  {
    //del spaces/line breaks at file end
    ini_delspace(&fileobj);
    //write new value
    f_lseek(&fileobj, fileobj.fsize);
    f_puts("\r\n", &fileobj);
    f_puts(entry, &fileobj);
    f_puts("=", &fileobj);
    f_puts(value, &fileobj);
    f_puts("\r\n", &fileobj);
  }

  f_close(&fileobj);

  return 0;
}


unsigned int ini_delentry(const char *filename, const char *entry) //entry in upper case
{
  FRESULT res;
  unsigned int i, entry_start, found, rd;
  char c;

  res = f_open(&fileobj, filename, FA_OPEN_EXISTING | FA_READ | FA_WRITE);
  if(res != FR_OK)
  {
    return 1;
  }

  found = ini_searchentry(&fileobj, entry, &entry_start);
  if(found == 0)
  {
    f_close(&fileobj);
    return 2;
  }

  res = f_lseek(&fileobj, entry_start);
  if(res != FR_OK)
  {
    f_close(&fileobj);
    return 3;
  }

  //calc current entry size
  for(i=0; ; i++) //line len
  {
    res = f_read(&fileobj, &c, 1, &rd);
    if((res != FR_OK) || (rd != 1))
    {
      break;
    }
    if((c == 0) || (c == '\n') || (c == '\r'))
    {
      break;
    }
  }
  for(; ((c == '\n') || (c == '\r')); i++) //skip line break chars
  {
    res = f_read(&fileobj, &c, 1, &rd);
    if((res != FR_OK) || (rd != 1))
    {
      break;
    }
  }
  //remove complete line
  ini_stripfile(&fileobj, entry_start, i);

  f_close(&fileobj);

  return 0;
}


unsigned int ini_renentry(const char *filename, const char *entry, const char *newentry) //entry and newentry in upper case
{
  FRESULT res;
  unsigned int i, len, entry_start, found, rd;
  char c;

  res = f_open(&fileobj, filename, FA_OPEN_EXISTING | FA_READ | FA_WRITE);
  if(res != FR_OK)
  {
    return 1;
  }

  found = ini_searchentry(&fileobj, entry, &entry_start);
  if(found == 0)
  {
    f_close(&fileobj);
    return 2;
  }

  res = f_lseek(&fileobj, entry_start);
  if(res != FR_OK)
  {
    f_close(&fileobj);
    return 3;
  }

  //calc current entry size
  for(i=0; ; i++) //line len
  {
    res = f_read(&fileobj, &c, 1, &rd);
    if((res != FR_OK) || (rd != 1))
    {
      break;
    }
    if((c == '=') || (c == 0) || (c == '\n') || (c == '\r'))
    {
      break;
    }
  }

  len = strlen(newentry); //new entry len
  if(i > len) //new entry size is smaller
  {
    ini_stripfile(&fileobj, entry_start, (i-len)); //strip file: old-new
  }
  else if(i < len) //new entry size is bigger
  {
    ini_extendfile(&fileobj, entry_start, (len-i)); //extend file: new-old
  }

  //write new entry name
  f_lseek(&fileobj, entry_start);
  f_puts(newentry, &fileobj);

  f_close(&fileobj);

  return 0;
}

#endif
