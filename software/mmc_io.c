#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "lmi/inc/hw_types.h"
#include "lmi/inc/hw_memmap.h"
#include "lmi/driverlib/gpio.h"
#include "fatfs/ff.h"
#include "fatfs/diskio.h"
#include "tools.h"
#include "main.h"
#include "io.h"
#include "mmc_io.h"


volatile DSTATUS Stat=STA_NOINIT;
volatile unsigned int Timer1=0, Timer2=0; //100Hz decrement timer
BYTE CardType=0; //b0:MMC, b1:SDC, b2:Block addressing


void xmit_spi(BYTE dat)
{
  ssi_readwrite(dat);

  return;
}


BYTE rcvr_spi(void)
{
  DWORD dat;

  dat = ssi_readwrite(0xFF);

  return (BYTE)dat;
}


void rcvr_spi_m(BYTE *dst)
{
  *dst = rcvr_spi();

  return;
}


BYTE wait_ready(void)
{
  BYTE res;

  Timer2 = 50; // Wait for ready in timeout of 500ms
  rcvr_spi();
  do
  {
    res = rcvr_spi();
  }while((res != 0xFF) && Timer2);

  return res;
}


void release_spi(void)
{
  MMC_DESELECT();
  rcvr_spi();
  
  return;
}


void power_on(void)
{
  //MMC_POWERON();
  MMC_DESELECT();

  return;
}


void power_off(void)
{
  MMC_SELECT(); //Wait for card ready
  wait_ready();
  release_spi();

  //MMC_POWEROFF();
  Stat |= STA_NOINIT; //Set STA_NOINIT

  return;
}


BYTE chk_power(void)
{
  return 1; //Socket power state: 0=off, 1=on
}


BYTE rcvr_datablock(BYTE *buff, UINT btr)
{
  BYTE token;

  Timer1 = 20; //Wait for data packet in timeout of 200ms
  do
  {
    token = rcvr_spi();
  }while((token == 0xFF) && Timer1);

  //If not valid data token, retutn with error
  if(token != 0xFE)
  {
    return 0;
  }

  //Receive the data block into buffer
  do
  {
    rcvr_spi_m(buff++);
    rcvr_spi_m(buff++);
    rcvr_spi_m(buff++);
    rcvr_spi_m(buff++);
  }while(btr -= 4);

  //Discard CRC
  rcvr_spi();
  rcvr_spi();

  return 1;
}


BYTE xmit_datablock(const BYTE *buff, BYTE token)
{
  BYTE resp, wc;

  if(wait_ready() != 0xFF)
  {
    return 0;
  }

  xmit_spi(token); //Xmit data token
  if(token != 0xFD)
  {
    wc = 0;
    //Xmit the 512 byte data block to MMC
    do
    {
      xmit_spi(*buff++);
      xmit_spi(*buff++);
    }while(--wc);
    //CRC (Dummy)
    xmit_spi(0xFF);
    xmit_spi(0xFF);
    //Receive data response
    resp = rcvr_spi();
    //If not accepted, return with error
    if((resp & 0x1F) != 0x05)
    {
      return 0;
    }
  }

  return 1;
}


BYTE send_cmd(BYTE cmd, DWORD arg)
{
  BYTE n, res;

  //ACMD<n> is the command sequence of CMD55-CMD<n>
  if(cmd & 0x80)
  {
    cmd &= 0x7F;
    res = send_cmd(CMD55, 0);
    if(res > 1)
    {
      return res;
    }
  }

  //Select the card and wait for ready
  MMC_DESELECT();
  MMC_SELECT();
  if(wait_ready() != 0xFF)
  {
    MMC_DESELECT();
    return 0xFF;
  }

  //Send command packet
  xmit_spi(cmd);               //Start + Command index
  xmit_spi((BYTE)(arg >> 24)); //Argument[31..24]
  xmit_spi((BYTE)(arg >> 16)); //Argument[23..16]
  xmit_spi((BYTE)(arg >> 8));  //Argument[15..8]
  xmit_spi((BYTE)arg);         //Argument[7..0]
  n = 0x01;                    //Dummy CRC + Stop
  if(cmd == CMD0){ n = 0x95; } //Valid CRC for CMD0(0)
  if(cmd == CMD8){ n = 0x87; } //Valid CRC for CMD8(0x1AA)
  xmit_spi(n);

  //Receive command response
  if(cmd == CMD12){ rcvr_spi(); } //skip a stuff byte when stop reading
  n = 10; //Wait for a valid response in timeout of 10 attempts
  do
  {
    res = rcvr_spi();
  }while((res & 0x80) && --n);

  return res;
}


DSTATUS disk_initialize(BYTE drv)
{
  BYTE n, cmd, ty, ocr[4], init_try;

  if(drv)              { return STA_NOINIT; } //Supports only single drive
  if(Stat & STA_NODISK){ return Stat;       } //No card in the socket

  ssi_off();      //SCK, SI = low (and released from ssi)

/*
  MMC_SELECT();   //CS = low
  MMC_POWEROFF(); //sd power off
  delay_ms(100);
  MMC_POWERON();  //sd power on
  delay_ms(1);
  MMC_DESELECT(); //CS = high
  delay_ms(50);
*/

  //80 dummy clocks
  MMC_DESELECT();
  MMC_SI_HIGH();
  MMC_SCK_LOW();
  for(n=80; n; n--)
  {
    volatile unsigned int i;
    MMC_SCK_HIGH();
    for(i=10000; i; i--);
    MMC_SCK_LOW();
    for(i=10000; i; i--);
  }

  ssi_on(); //SCK, SI = ssi
  ssi_speed(250000); //ssi speed 250 kHz

  init_try = 3;
  do
  {
    ty = 0;
    if(send_cmd(CMD0, 0) == 1) //Enter Idle state
    {
      Timer1 = 100; //Initialization timeout of 1000ms
      if(send_cmd(CMD8, 0x1AA) == 1) //SD v2 ?
      {
        for(n=0; n < 4; n++){ ocr[n] = rcvr_spi(); } //Get trailing return value of R7 resp
        //if((ocr[2] == 0x01) && (ocr[3] == 0xAA)) //The card can work at vdd range of 2.7-3.6V
        //{
          while(Timer1 && send_cmd(ACMD41, 1UL<<30)); // Wait for leaving idle state (ACMD41 with HCS bit)
          if(Timer1 && (send_cmd(CMD58, 0) == 0)) //Check CCS bit in the OCR
          {
            for(n=0; n < 4; n++){ ocr[n] = rcvr_spi(); }
            ty = (ocr[0]&0x40) ? CT_SD2|CT_BLOCK : CT_SD2; //SD v2
          }
        //}
      }
      else //SD v1 or MMC v3
      {
        if(send_cmd(ACMD41, 0) <= 1)
        {
          ty = CT_SD1; cmd = ACMD41; //SD v1
        }
        else
        {
          ty = CT_MMC; cmd = CMD1; //MMC v3
        }
        while(Timer1 && send_cmd(cmd, 0)); //Wait for leaving idle state
        if(!Timer1 || send_cmd(CMD16, 512) != 0) //Set R/W block length to 512 
        {
          ty = 0;
        }
      }
    }
  }while((ty == 0) && --init_try);

  CardType = ty;
  release_spi();

  //ssi speed up
  ssi_speed(0); //0 = default speed

  if(ty) //Initialization succeded
  {
    Stat &= ~STA_NOINIT; //Clear STA_NOINIT
  }
  else //Initialization failed
  {
    power_off();
  }

  return Stat;
}


DSTATUS disk_status(BYTE drv)
{
  if(drv){ return STA_NOINIT; } //Supports only single drive

  return Stat;
}


DRESULT disk_read(BYTE drv, BYTE *buff, DWORD sector, BYTE count)
{
  if(drv || !count)    { return RES_PARERR; }
  if(Stat & STA_NOINIT){ return RES_NOTRDY; }

  if(!(CardType & CT_BLOCK)){ sector *= 512; } //Convert to byte address if needed

  if(count == 1) //Single block read
  {
    if((send_cmd(CMD17, sector) == 0) && rcvr_datablock(buff, 512))
    {
      count = 0;
    }
  }
  else //Multiple block read
  {
    if(send_cmd(CMD18, sector) == 0)
    {
      do
      {
        if(!rcvr_datablock(buff, 512))
        {
          break;
        }
        buff += 512;
      }while(--count);
      send_cmd(CMD12, 0); //STOP_TRANSMISSION
    }
  }
  release_spi();

  return count ? RES_ERROR : RES_OK;
}


DRESULT disk_write(BYTE drv, const BYTE *buff, DWORD sector, BYTE count)
{
  if(drv || !count)     { return RES_PARERR; }
  if(Stat & STA_NOINIT) { return RES_NOTRDY; }
  if(Stat & STA_PROTECT){ return RES_WRPRT;  }

  if(!(CardType & CT_BLOCK)){ sector *= 512; }//Convert to byte address if needed

  if(count == 1) //Single block write
  {
    if((send_cmd(CMD24, sector) == 0) && xmit_datablock(buff, 0xFE))
    {
      count = 0;
    }
  }
  else //Multiple block write
  {
    if(CardType & CT_SDC)
    {
      send_cmd(ACMD23, count);
    }
    if(send_cmd(CMD25, sector) == 0)
    {
      do
      {
        if(!xmit_datablock(buff, 0xFC))
        {
          break;
        }
        buff += 512;
      }while(--count);
      if(!xmit_datablock(0, 0xFD)) //STOP_TRAN token
      {
        count = 1;
      }
    }
  }
  release_spi();

  return count ? RES_ERROR : RES_OK;
}


#if _USE_IOCTL != 0
DRESULT disk_ioctl(BYTE drv, BYTE ctrl, void *buff)
{
  DRESULT res;
  BYTE n, csd[16], *ptr = buff;
  WORD csize;

  if(drv){ return RES_PARERR; }

  res = RES_ERROR;

  if(ctrl == CTRL_POWER)
  {
    switch(ptr[0])
    {
      case 0: //Sub control code (POWER_OFF)
        if(chk_power())
        {
          power_off();
        }
        res = RES_OK;
        break;
      case 1: //Sub control code (POWER_ON)
        power_on();
        res = RES_OK;
        break;
      case 2: //Sub control code (POWER_GET)
        ptr[1] = (BYTE)chk_power();
        res = RES_OK;
        break;
      default :
        res = RES_PARERR;
    }
  }
  else
  {
    if(Stat & STA_NOINIT){ return RES_NOTRDY; }
    switch (ctrl)
    {
      case CTRL_SYNC: //Make sure that no pending write process
        MMC_SELECT();
        if(wait_ready() == 0xFF)
        {
          res = RES_OK;
        }
        else
        {
          release_spi();
        }
        break;

      case GET_SECTOR_COUNT: //Get number of sectors on the disk (DWORD)
        if((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16))
        {
          if((csd[0] >> 6) == 1) //SDC ver 2.00
          {
            csize = csd[9] + ((WORD)csd[8] << 8) + 1;
            *(DWORD*)buff = (DWORD)csize << 10;
          }
          else //SDC ver 1.XX or MMC
          {
            n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
            csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
            *(DWORD*)buff = (DWORD)csize << (n - 9);
          }
          res = RES_OK;
        }
        break;

      case GET_SECTOR_SIZE: //Get R/W sector size (WORD)
        *(WORD*)buff = 512;
        res = RES_OK;
        break;

      case GET_BLOCK_SIZE: //Get erase block size in unit of sector (DWORD)
        if(CardType & CT_SD2) //SDC ver 2.00
        {
          if(send_cmd(ACMD13, 0) == 0) //Read SD status
          {
            rcvr_spi();
            if(rcvr_datablock(csd, 16)) //Read partial block
            {
              for(n=64 - 16; n; n--){ rcvr_spi(); } //Purge trailing data
              *(DWORD*)buff = 16UL << (csd[10] >> 4);
              res = RES_OK;
            }
          }
        }
        else //SDC ver 1.XX or MMC
        {
          if((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) //Read CSD
          {
            if(CardType & CT_SD1) //SDC ver 1.XX
            {
              *(DWORD*)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
            }
            else //MMC
            {
              *(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
            }
            res = RES_OK;
          }
        }
        break;

      case MMC_GET_TYPE: //Get card type flags (1 byte)
        *ptr = CardType;
        res = RES_OK;
        break;

      case MMC_GET_CSD: //Receive CSD as a data block (16 bytes)
        if((send_cmd(CMD9, 0) == 0) && rcvr_datablock(ptr, 16))
        {
          res = RES_OK;
        }
        break;

      case MMC_GET_CID: //Receive CID as a data block (16 bytes)
        if((send_cmd(CMD10, 0) == 0) && rcvr_datablock(ptr, 16))
        {
          res = RES_OK;
        }
        break;

      case MMC_GET_OCR: //Receive OCR as an R3 resp (4 bytes)
        if(send_cmd(CMD58, 0) == 0)
        {
          for(n=4; n; n--){ *ptr++ = rcvr_spi(); }
          res = RES_OK;
        }
        break;

      case MMC_GET_SDSTAT: //Receive SD status as a data block (64 bytes)
        if(send_cmd(ACMD13, 0) == 0)
        {
          rcvr_spi();
          if(rcvr_datablock(ptr, 64))
          {
            res = RES_OK;
          }
        }
        break;

      default:
        res = RES_PARERR;
    }

    release_spi();
  }

  return res;
}
#endif //_USE_IOCTL != 0


void disk_timerproc(void) //This function must be called in period of 10ms
{
  unsigned int n;

  n = Timer1;
  if(n){ Timer1 = --n; }
  n = Timer2;
  if(n){ Timer2 = --n; }

  return;
}


DWORD get_fattime(void) //User Provided Timer Function for FatFs module
{
  TIME t;

#ifdef LOADER
  memset(&t, 0, sizeof(t));
#else
  gettime(&t);
#endif

  return  (((DWORD)(t.year-1980)) << 25) | // Year
          (((DWORD)      t.month) << 21) | // Month
          (((DWORD)        t.day) << 16) | // Day
          (((DWORD)          t.h) << 11) | // Hour
          (((DWORD)          t.m) <<  5) | // Min
          (((DWORD)          t.s) >>  1);  // Sec
}
