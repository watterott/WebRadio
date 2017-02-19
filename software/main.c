/*---------------------------------------------------
 * WebRadio (c) Andreas Watterott (www.watterott.net)
 *---------------------------------------------------
 * For more information, see readme.txt
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "lmi/inc/hw_types.h"
#include "lmi/inc/hw_memmap.h"
#include "lmi/driverlib/sysctl.h"
#include "lmi/driverlib/gpio.h"
#include "lmi/driverlib/interrupt.h"
#include "lmi/driverlib/systick.h"
#include "fatfs/ff.h"
#include "fatfs/diskio.h"
#include "debug.h"
#include "tools.h"
#include "main.h"
#include "io.h"
#include "lcd.h"
#include "lcd/font_8x8.h"
#include "lcd/font_8x12.h"
#include "lcd/font_clock.h"
#include "mmc_io.h"
#include "mmc.h"
#include "vs.h"
#include "eth.h"
#include "eth/ntp.h"
#include "eth/ssdp.h"
#include "buffer.h"
#include "menu.h"
#include "alarm.h"
#include "settings.h"
#include "chucknorris.h"


volatile unsigned int status=0, standby_active=0;
volatile long on_time=0;
volatile unsigned long sec_time=0;
TIME timedate;
char date_str[14] = {'T','h',' ','0','1','.','0','1','.','1','9','7','0',0}; //Th 01.01.1970
char clock_str[9] = {'0','0',':','0','0',':','0','0',0}; //00:00:00

const char day_tab[7][3] = 
{
  {"Su"},{"Mo"},{"Tu"},{"We"},{"Th"},{"Fr"},{"Sa"} //English
  //{"So"},{"Mo"},{"Di"},{"Mi"},{"Do"},{"Fr"},{"Sa"} //German
};

const char clock_tab[60][3] = 
{
  {"00"},{"01"},{"02"},{"03"},{"04"},{"05"},{"06"},{"07"},{"08"},{"09"},
  {"10"},{"11"},{"12"},{"13"},{"14"},{"15"},{"16"},{"17"},{"18"},{"19"},
  {"20"},{"21"},{"22"},{"23"},{"24"},{"25"},{"26"},{"27"},{"28"},{"29"},
  {"30"},{"31"},{"32"},{"33"},{"34"},{"35"},{"36"},{"37"},{"38"},{"39"},
  {"40"},{"41"},{"42"},{"43"},{"44"},{"45"},{"46"},{"47"},{"48"},{"49"},
  {"50"},{"51"},{"52"},{"53"},{"54"},{"55"},{"56"},{"57"},{"58"},{"59"}
};


void systick(void) //100 Hz
{
  static unsigned long sec=1;
  unsigned int s, a;

  disk_timerproc();
  keys_timerservice();

  //1 Hz
  if(--sec == 0)
  {
    sec = 100;
    on_time++;
    sec_time++;

    s  = status;
    s |= SEC_CHANGED;
    if(++timedate.s == 60)
    {
      timedate.s = 0;
      s |= MIN_CHANGED;
      if(++timedate.m == 60)
      {
        timedate.m = 0;
        s |= HOUR_CHANGED;
        if(++timedate.h == 24)
        {
          timedate.h = 0;
          s |= DAY_CHANGED;
          settime(sec_time); //set date
        }
        clock_str[0] = clock_tab[timedate.h][0];
        clock_str[1] = clock_tab[timedate.h][1];
      }
      clock_str[3] = clock_tab[timedate.m][0];
      clock_str[4] = clock_tab[timedate.m][1];

      //alarm check
      a = alarm_check(&timedate);
      if(a == 1)
      {
        s |= ALARM_PLAY;
      }
      else if(a == 2)
      {
        s |= ALARM_STANDBY;
      }
    }
    clock_str[6] = clock_tab[timedate.s][0];
    clock_str[7] = clock_tab[timedate.s][1];
    status = s;
  }

  return;
}


char* getclock(void)
{
  return clock_str;
}


char* getdate(void)
{
  return date_str;
}


void gettime(TIME* t)
{
  IntMasterDisable();
  t->year  = timedate.year;
  t->month = timedate.month;
  t->day   = timedate.day;
  t->wday  = timedate.wday;
  t->h     = timedate.h;
  t->m     = timedate.m;
  t->s     = timedate.s;
  IntMasterEnable();

  return;
}


void settime(unsigned long s)
{
  TIME t;

  IntMasterDisable();
  SysTickIntDisable();
  sec_time = s;
  sectotime(s-1, &t);
  timedate.year  = t.year;
  timedate.month = t.month;
  timedate.day   = t.day;
  timedate.wday  = t.wday;
  timedate.h     = t.h;
  timedate.m     = t.m;
  timedate.s     = t.s;
  SysTickIntEnable();
  IntMasterEnable();

  date_str[0] = day_tab[timedate.wday][0];
  date_str[1] = day_tab[timedate.wday][1];
  date_str[3] = clock_tab[timedate.day][0];
  date_str[4] = clock_tab[timedate.day][1];
  date_str[6] = clock_tab[timedate.month][0];
  date_str[7] = clock_tab[timedate.month][1];
  itoa(timedate.year, &date_str[9], 10);

  clock_str[0] = clock_tab[timedate.h][0];
  clock_str[1] = clock_tab[timedate.h][1];
  clock_str[3] = clock_tab[timedate.m][0];
  clock_str[4] = clock_tab[timedate.m][1];
  clock_str[6] = clock_tab[timedate.s][0];
  clock_str[7] = clock_tab[timedate.s][1];

  DEBUGOUT("Set time: %s %s\n", getclock(), getdate());

  return;
}


long getdeltatime(long t)
{
  long s;

  s = on_time;

  return (s-t);
}


long getontime(void)
{
  long s;

  s = on_time;

  return s;
}


unsigned int standby_isactive(void)
{
  return standby_active;
}


void standby_off(void)
{
  standby_active = 0;

  return;
}


unsigned int standby(unsigned int param)
{
  unsigned int i, alarm=0;
  unsigned long t;
  char tmp[32];

  DEBUGOUT("Standby\n");

  standby_active = 1;

  vs_stop();
  vs_setvolume(0); //0 -> analog power off
  delay_ms(10);
  USB_OFF();

  //draw clock
  lcd_clear(RGB(0,0,0));
  tmp[0] = clock_str[0];
  tmp[1] = clock_str[1];
  tmp[2] = clock_str[2];
  tmp[3] = clock_str[3];
  tmp[4] = clock_str[4];
  tmp[5] = 0;
  lcd_puts((LCD_WIDTH/2)-((5*(TIMEFONT_WIDTH*2))/2), (LCD_HEIGHT/2)-((TIMEFONT_HEIGHT*2)/2), tmp, TIMEFONT, 2, RGB(255,255,255), RGB(0,0,0));

  //try to get time from ntp
#ifndef DEBUG
  t = ntp_gettime();
  if(t){ settime(t); }
#endif

  cpu_speed(1); //low speed & reduce LED power%

  while(standby_active == 1)
  {
    eth_service();

    IntMasterDisable();
    i = status;
    status &= ~i;
    IntMasterEnable();

    if(i & MIN_CHANGED)
    {
      if(i & ALARM_PLAY)
      {
        alarm = 1;
        break;
      }

      tmp[0] = clock_str[0];
      tmp[1] = clock_str[1];
      tmp[3] = clock_str[3];
      tmp[4] = clock_str[4];
      lcd_puts((LCD_WIDTH/2)-((5*(TIMEFONT_WIDTH*2))/2), (LCD_HEIGHT/2)-((TIMEFONT_HEIGHT*2)/2), tmp, TIMEFONT, 2, RGB(255,255,255), RGB(0,0,0));
    }

    if(keys_sw() || (ir_cmd() == SW_POWER))
    {
      if(alarm == 0)
      {
        pwm_led(80);
        daytime(tmp, &timedate);
        lcd_puts(10, 10, tmp, NORMALFONT, 1, RGB(255,255,255), RGB(0,0,0));

        chucknorris_rfact();
        pwm_led(100);
      }
      break;
    }
  }

  USB_ON(); //speaker on

  cpu_speed(0); //high speed & LED 100%

  //clear cmds
  keys_sw();
  keys_steps();
  ir_cmd();

  standby_active = 0;

  if(alarm != 0)
  {
    menu_alarm();
  }

  return 0;
}


int main()
{
  unsigned int i;
  unsigned long l;

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

  //low speed and enable interrupts
  DEBUGOUT("Low speed and IRQs on...\n");
  cpu_speed(1);

  //init mmc & mount filesystem
  DEBUGOUT("Init Memory Card...\n");
  fs_mount();

  //init lcd
  DEBUGOUT("Init LCD...\n");
  lcd_init();

  //show start-up screen
  lcd_clear(DEFAULT_BGCOLOR);
  lcd_fillrect( 0, 0, LCD_WIDTH-1, 10, DEFAULT_EDGECOLOR);
  lcd_puts(30, 2, APPNAME" v"APPVERSION, SMALLFONT, 1, DEFAULT_BGCOLOR, DEFAULT_EDGECOLOR);
  lcd_fillrect( 0, LCD_HEIGHT-1-13, LCD_WIDTH-1, LCD_HEIGHT-1, DEFAULT_EDGECOLOR);
  lcd_puts(20, LCD_HEIGHT-1-10, "www.watterott.net", SMALLFONT, 1, DEFAULT_BGCOLOR, DEFAULT_EDGECOLOR);
  lcd_puts(10, 20, "HW:"LM3S_NAME","LCD_NAME, SMALLFONT, 1, DEFAULT_EDGECOLOR, DEFAULT_BGCOLOR);

  if(l) //l = reset cause
  {
    i = lcd_puts(10,  35, "Reset:", SMALLFONT, 1, DEFAULT_EDGECOLOR, DEFAULT_BGCOLOR) + 4;
    if(l & SYSCTL_CAUSE_LDO) { i = lcd_puts(i, 35, "LDO", SMALLFONT, 1, DEFAULT_EDGECOLOR, DEFAULT_BGCOLOR) + 4; }
    if(l & SYSCTL_CAUSE_SW)  { i = lcd_puts(i, 35, "SW",  SMALLFONT, 1, DEFAULT_EDGECOLOR, DEFAULT_BGCOLOR) + 4; }
    if(l & SYSCTL_CAUSE_WDOG){ i = lcd_puts(i, 35, "WD",  SMALLFONT, 1, DEFAULT_EDGECOLOR, DEFAULT_BGCOLOR) + 4; }
    if(l & SYSCTL_CAUSE_BOR) { i = lcd_puts(i, 35, "BOR", SMALLFONT, 1, DEFAULT_EDGECOLOR, DEFAULT_BGCOLOR) + 4; }
    if(l & SYSCTL_CAUSE_POR) { i = lcd_puts(i, 35, "POR", SMALLFONT, 1, DEFAULT_EDGECOLOR, DEFAULT_BGCOLOR) + 4; }
    if(l & SYSCTL_CAUSE_EXT) { i = lcd_puts(i, 35, "EXT", SMALLFONT, 1, DEFAULT_EDGECOLOR, DEFAULT_BGCOLOR) + 4; }
    i = 35+15; //msg y start
  }
  else
  {
    i = 35; //msg y start
  }

  //show mmc state
  if(disk_status(0) == 0)
  {
    lcd_puts(10,  i, "Memory Card: OK", SMALLFONT, 1, DEFAULT_FGCOLOR, DEFAULT_BGCOLOR);
  }
  else
  {
    lcd_puts(10,  i, "Memory Card: Error", SMALLFONT, 1, DEFAULT_FGCOLOR, DEFAULT_BGCOLOR);
  }
  i += 10;

  //init fram
  lcd_puts(10,  i, "Init F-RAM...", SMALLFONT, 1, DEFAULT_FGCOLOR, DEFAULT_BGCOLOR);
  l = fm_init();
  if(l)
  {
    char tmp[8];
    sprintf(tmp, "%ikb", (unsigned int)(unsigned long)(l/1024UL));
    lcd_puts(120,  i, tmp, SMALLFONT, 1, DEFAULT_FGCOLOR, DEFAULT_BGCOLOR);
  }
  i += 10;

  //init ethernet
  lcd_puts(10,  i, "Init Ethernet...", SMALLFONT, 1, DEFAULT_FGCOLOR, DEFAULT_BGCOLOR); i += 10;
  eth_init();

  //load settings
  lcd_puts(10, i, "Load Settings...", SMALLFONT, 1, DEFAULT_FGCOLOR, DEFAULT_BGCOLOR); i += 10;
  settings_read();

  //advertise UPnP device
  lcd_puts(10, i, "SSDP: Advertise...", SMALLFONT, 1, DEFAULT_FGCOLOR, DEFAULT_BGCOLOR); i += 10;
  ssdp_advertise();

  //set clock
  lcd_puts(10, i, "NTP: Get Time...", SMALLFONT, 1, DEFAULT_FGCOLOR, DEFAULT_BGCOLOR); i += 10;
#ifdef DEBUG
  settime(1);
#else
  l = ntp_gettime();
  if(l)
  {
    settime(l);
  }
  else
  {
    settime(timetosec(0, 0, 0, COMPILE_DAY, COMPILE_MONTH, COMPILE_YEAR));
  }
#endif

  //cpu high speed
  cpu_speed(0);

  //init menu
  menu_init();

  //usb power on
  USB_ON();

  //check alarm
  i = alarm_check(&timedate);
  if(i == 1)
  {
    menu_alarm();
  }

  DEBUGOUT("Ready...\n");

  for(;;)
  {
    eth_service();

    IntMasterDisable();
    i = status;
    status &= ~i;
    IntMasterEnable();

#ifdef DEBUG
//    if(i & SEC_CHANGED)
//    {
//      DEBUGOUT("buf: %i / rx %i\n", buf_free(), eth_rxfree());
//    }
#endif
    if(i & MIN_CHANGED)
    {
      if(i & ALARM_PLAY)
      {
        menu_alarm();
      }
      else if(i & ALARM_STANDBY)
      {
        standby(0);
        i |= DRAWALL;
      }
    }
    menu_service(i);
  }
}
