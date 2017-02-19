/*-------------------------------------------------
 * Loader (c) Andreas Watterott (www.watterott.net)
 *-------------------------------------------------
 * For more information, see readme.txt
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "lmi/inc/hw_types.h"
#include "lmi/inc/hw_memmap.h"
#include "lmi/driverlib/sysctl.h"
#include "lmi/driverlib/gpio.h"
#include "lmi/driverlib/systick.h"
#include "lmi/driverlib/uart.h"
#include "lmi/driverlib/flash.h"
#include "fatfs/ff.h"
#include "../debug.h"
#include "../tools.h"
#include "main.h"
#include "../io.h"
#include "../lcd.h"
#include "../mmc_io.h"
#include "../mmc.h"


#define ITEMS                          (4)
#define ITEM_X                         (15)
#define ITEM_Y                         (36)
#define ITEM_HEIGHT                    (20)


FIL fileobj;
unsigned char flashbuf[FLASHBUF];


void systick(void) //100 Hz
{
  disk_timerproc();
  keys_timerservice();

  return;
}


long backup_app(const char* fname)
{
  unsigned long i;
  unsigned int wr;
  long err=1;

  lcd_putline(ITEM_X, ITEM_Y, "Open File...", SMALLFONT, 1, RGB(255,255,0), RGB(0,0,0));

  FlashUsecSet(8); //cpu speed = 8 MHz

  if(f_open(&fileobj, fname, FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
  {
    err = 0;
    lcd_putline(ITEM_X, ITEM_Y, "Save Flash...", SMALLFONT, 1, RGB(255,255,0), RGB(0,0,0));
    for(i=APPSTARTADDR; (i>=APPSTARTADDR) && (i<FLASHSIZE); i+=1024)
    {
      if(f_write(&fileobj, (unsigned char*)i, FLASHBUF, &wr) != FR_OK)
      {
        err = 2;
        break;
      }
    }
    f_close(&fileobj);
  }
  else
  {
    lcd_putline(ITEM_X, ITEM_Y, "ERROR: Cannot Open File", SMALLFONT, 1, RGB(255,255,0), RGB(0,0,0));
    delay_ms(1000);
  }

  return err;
}


long flash_app(const char* fname)
{
  unsigned long i;
  unsigned int rd;
  long err=1;

  lcd_putline(ITEM_X, ITEM_Y, "Open File...", SMALLFONT, 1, RGB(255,255,0), RGB(0,0,0));

  FlashUsecSet(8); //cpu speed = 8 MHz

  if(f_open(&fileobj, fname, FA_OPEN_EXISTING | FA_READ) == FR_OK)
  {
    err = 0;
    if(err == 0)
    {
      lcd_putline(ITEM_X, ITEM_Y, "Erase Flash...", SMALLFONT, 1, RGB(255,255,0), RGB(0,0,0));
      for(i=APPSTARTADDR; (i>=APPSTARTADDR) && (i<FLASHSIZE); i+=1024)
      {
        if(FlashErase(i) != 0)
        {
          err = 2;
          break;
        }
      }
    }

    if(err == 0)
    {
      lcd_putline(ITEM_X, ITEM_Y, "Flash App...", SMALLFONT, 1, RGB(255,255,0), RGB(0,0,0));
      for(i=APPSTARTADDR; (i>=APPSTARTADDR) && (i<FLASHSIZE);)
      {
        if(f_read(&fileobj, flashbuf, FLASHBUF, &rd) == FR_OK)
        {
          if(rd < FLASHBUF)
          {
            memset((flashbuf+(rd-1)), 0xff, (FLASHBUF-rd));
            rd = 0;
          }
          if(FlashProgram((unsigned long*)flashbuf, i, FLASHBUF) != 0)
          {
            err = 3;
            break;
          }
          i += FLASHBUF;
          if(rd == 0)
          {
            break;
          }
        }
        else
        {
          break;
        }
      }
    }
    f_close(&fileobj);
  }
  else
  {
    lcd_putline(ITEM_X, ITEM_Y, "ERROR: Cannot Open File", SMALLFONT, 1, RGB(255,255,0), RGB(0,0,0));
    delay_ms(1000);
  }

  return err;
}


void start_app(void)
{
  DEBUGOUT("Start App...\n");

  //disable interrupts
  __asm("cpsid   i\n");

  //disable systick
  SysTickDisable();
  SysTickIntDisable();
  SysTickIntUnregister();
  delay_ms(1);

  __asm("ldr     r0, =%0\n"         //load app start address

        "ldr     r1, =0xe000ed08\n" //set vector table addr to the beginning of the app
        "str     r0, [r1]\n"

        "ldr     r1, [r0]\n"        //load stack ptr from the app's vector table
        "mov     sp, r1\n"

        "ldr     r0, [r0, #4]\n"    //load the initial PC from the app's vector table and
        "bx      r0\n"              //branch to the app's entry point
        :
        : "i" (APPSTARTADDR));

  return;
}


int main()
{
  unsigned long l;
  int i, item;

  //init debug output
  debug_init();

  DEBUGOUT("\n"APPNAME" v"APPVERSION" ("__DATE__" "__TIME__")\n");
  DEBUGOUT("Hardware: "LM3S_NAME", "LCD_NAME"\n");

  //get reset cause
  l = SysCtlResetCauseGet();
  if(l)
  {
    SysCtlResetCauseClear(l);
    DEBUGOUT("Reset: %i\n", l);
  }

  //init brown-out reset
  DEBUGOUT("Init BOR...\n");
  init_bor(1);

  //init pins
  DEBUGOUT("Init Pins...\n");
  init_pins();

  //init systick
  DEBUGOUT("Init SysTick...\n");
  SysTickDisable();
  SysTickPeriodSet(SysCtlClockGet() / 100); //100 Hz
  SysTickIntRegister(systick);
  SysTickIntEnable();
  SysTickEnable();

  //init peripherals
  DEBUGOUT("Init Peripherals...\n");
  init_periph();

  //read switch state
  i = ENC_SW_READ(); 

  //low speed and enable interrupts
  DEBUGOUT("Low speed and IRQs on...\n");
  cpu_speed(1);

#ifdef DEBUG
  if(1)
#else
  if(!ENC_SW_READ() && !i) //read switch state twice
#endif
  {
    //init mmc & mount filesystem
    DEBUGOUT("Init Memory Card...\n");
    fs_mount();

    //init lcd
    DEBUGOUT("Init LCD...\n");
    lcd_init();
    lcd_clear(RGB(0,0,0));
    lcd_fillrect( 0, 0, LCD_WIDTH-1, 10, RGB(180,150,0));
    lcd_puts(38,  2, APPNAME" v"APPVERSION, SMALLFONT, 1, RGB(0,0,0), RGB(180,150,0));
    lcd_fillrect( 0, LCD_HEIGHT-1-10, LCD_WIDTH-1, LCD_HEIGHT-1, RGB(180,150,0));
    lcd_puts(20, LCD_HEIGHT-1-8, "www.watterott.net", SMALLFONT, 1, RGB(0,0,0), RGB(180,150,0));
    lcd_puts(10, 18, "HW:"LM3S_NAME","LCD_NAME, SMALLFONT, 1, RGB(140,140,140), RGB(0,0,0));
    lcd_putline(ITEM_X, ITEM_Y, "Start Loader...", SMALLFONT, 1, RGB(255,255,0), RGB(0,0,0));

    //menu
    lcd_fillrect(ITEM_X-10, ITEM_Y, LCD_WIDTH-1, LCD_HEIGHT-1-11, RGB(0,0,0));
    lcd_putlinebr(ITEM_X, ITEM_Y+(ITEM_HEIGHT*0), "Start Application", SMALLFONT, 1, RGB(255,255,0), RGB(0,0,0));
    lcd_putlinebr(ITEM_X, ITEM_Y+(ITEM_HEIGHT*1), "Flash "FIRMWARE_FILE, SMALLFONT, 1, RGB(255,255,0), RGB(0,0,0));
    lcd_putlinebr(ITEM_X, ITEM_Y+(ITEM_HEIGHT*2), "Flash "FIRMWARE_BAKFILE, SMALLFONT, 1, RGB(255,255,0), RGB(0,0,0));
    lcd_putlinebr(ITEM_X, ITEM_Y+(ITEM_HEIGHT*3), "Backup Firmware to   "FIRMWARE_BAKFILE, SMALLFONT, 1, RGB(255,255,0), RGB(0,0,0));
    delay_ms(200);
    keys_sw(); //clear keys
    item = 0;
    for(i=-1; keys_sw()==0;)
    {
      if(i)
      {
             if((i > 0) && (item < (ITEMS-1))) { item++; }
        else if((i < 0) && (item > 0))         { item--; }
        lcd_fillrect(ITEM_X-10, ITEM_Y, ITEM_X, ITEM_Y+(ITEMS*ITEM_HEIGHT), RGB(0,0,0));
        lcd_putc(ITEM_X-10, ITEM_Y+(item*ITEM_HEIGHT)-1, 0xFC, SMALLFONT, 1, RGB(255,160,0), RGB(0,0,0)); //play icon
      }
      i = keys_steps();
    }
    lcd_fillrect(ITEM_X-10, ITEM_Y, LCD_WIDTH-1, LCD_HEIGHT-1-11, RGB(0,0,0));

    switch(item)
    {
      case 1: flash_app(FIRMWARE_FILE);     break;
      case 2: flash_app(FIRMWARE_BAKFILE);  break;
      case 3: backup_app(FIRMWARE_BAKFILE); break;
    }

    //unmount file system
    fs_unmount();

    lcd_putline(ITEM_X, ITEM_Y, "Start App...", SMALLFONT, 1, RGB(255,255,0), RGB(0,0,0));
  }

  //start application
  start_app();

  DEBUGOUT("ERROR: No App \n");

  lcd_putline(ITEM_X, ITEM_Y, "ERROR", SMALLFONT, 1, RGB(255,255,0), RGB(0,0,0));

  while(1);
}
