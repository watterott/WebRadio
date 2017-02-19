#include <stdint.h>
#include <stdlib.h>
#include "lmi/inc/hw_types.h"
#include "lmi/inc/hw_memmap.h"
#include "lmi/driverlib/gpio.h"
#include "lmi/driverlib/interrupt.h"
#include "lmi/driverlib/ssi.h"
#include "fatfs/ff.h"
#include "debug.h"
#include "tools.h"
#include "main.h"
#include "io.h"
#include "eth.h"
#include "menu.h"
#include "buffer.h"
#include "vs_patch.h"
#include "vs.h"


unsigned int vs_playing=0;
int vs_vol=0, vs_sbamp=0, vs_sbfreq=0, vs_stamp=0, vs_stfreq=0;
VSBUFFER vs_buf;
volatile unsigned int vs_bufhead=0, vs_buftail=0;


void vs_requesthandler(void)
{
  unsigned int len, tail;

  GPIOPinIntClear(GPIO_PORTA_BASE, GPIO_PIN_1);

  len = vs_buflen();
  if(len != 0)
  {
    if(len > 16)
    {
      len = 16;
    }
    vs_ssi_writewait(); //ssi transmit fifo full?
    VS_DCS_DISABLE();
    tail = vs_buftail;
    VS_DCS_ENABLE();
    for(; len!=0; len--)
    {
      vs_ssi_write(vs_buf.b8[tail]);
      if(++tail == VS_BUFSIZE)
      {
        tail = 0;
      }
    }
    vs_buftail = tail;
  }
  else
  {
    vs_pause();
  }

  return;
}


unsigned char vs_bufgetc(void)
{
  unsigned char c;
  unsigned int head, tail;

  head = vs_bufhead;
  tail = vs_buftail;

  if(head != tail)
  {
    c = vs_buf.b8[tail++];
    if(tail == VS_BUFSIZE)
    {
      tail = 0;
    }
    vs_buftail = tail;
  }
  else
  {
    c = 0;
  }

  return c;
}


void vs_bufputs(const unsigned char *s, unsigned int len)
{
  unsigned int head;

  head = vs_bufhead;
  while(len--)
  {
    vs_buf.b8[head++] = *s++;
    if(head == VS_BUFSIZE)
    {
      head = 0;
    }
  }
  vs_bufhead = head;

  return;
}


unsigned int vs_buffree(void)
{
  unsigned int head, tail;

  head = vs_bufhead;
  tail = vs_buftail;

  if(head > tail)
  {
    return (VS_BUFSIZE-(head-tail))-1;
  }
  else if(head < tail)
  {
    return (tail-head)-1;
  }

  return (VS_BUFSIZE-1);
}


unsigned int vs_buflen(void)
{
  unsigned int head, tail;

  head = vs_bufhead;
  tail = vs_buftail;

  if(head > tail)
  {
    return (head-tail);
  }
  else if(head < tail)
  {
    return (VS_BUFSIZE-(tail-head));
  }

  return 0;
}


void vs_bufsethead(unsigned int head)
{
  vs_bufhead = head;

  return;
}


void vs_bufreset(void)
{
  vs_bufhead = 0;
  vs_buftail = 0;

  return;
}


int vs_gettreblefreq(void)
{
  return vs_stfreq*1000;
}


void vs_settreblefreq(int freq) //1000 - 15000Hz
{
  freq /= 1000;

  if(freq < 1) //< 1
  {
    freq = 1;
  }
  else if(freq > 15) //> 15
  {
    freq = 15;
  }
  vs_stfreq = freq;
  vs_write_bass();

  return;
}


int vs_gettrebleamp(void)
{
  return vs_stamp;
}


void vs_settrebleamp(int amp) //-8 - 7dB
{
  if(amp < -8) //< -8
  {
    amp = -8;
  }
  else if(amp > 7) //> 7
  {
    amp = 7;
  }
  vs_stamp = amp;
  vs_write_bass();

  return;
}


int vs_getbassfreq(void)
{
  return vs_sbfreq*10;
}


void vs_setbassfreq(int freq) //20 - 150Hz
{
  freq /= 10;

  if(freq < 2) //< 2
  {
    freq = 2;
  }
  else if(freq > 15) //> 15
  {
    freq = 15;
  }
  vs_sbfreq = freq;
  vs_write_bass();

  return;
}


int vs_getbassamp(void)
{
  return vs_sbamp;
}


void vs_setbassamp(int amp) //0 - 15dB
{
  if(amp < 0) //< 0
  {
    amp = 0;
  }
  else if(amp > 15) //> 15
  {
    amp = 15;
  }
  vs_sbamp = amp;
  vs_write_bass();

  return;
}


int vs_getvolume(void)
{
  return vs_vol;
}


void vs_setvolume(int vol) //0 - 100%
{
  if(vol <= 0) //<= 0
  {
    vs_write_reg(VS_VOL, 0xFFFF); //analog power off
  }
  else if(vol > 100) //> 100
  {
    vs_vol = 100;
    if(vs_playing)
    {
      vs_write_volume();
    }
  }
  else //1 - 99
  {
    vs_vol = vol;
    if(vs_playing)
    {
      vs_write_volume();
    }
  }

  menu_drawvol();

  return;
}


unsigned int vs_request(void)
{
  return VS_DREQ_READ(); //1=ready, 0=buf full
}


void vs_data(unsigned int c)
{
  VS_DCS_ENABLE();

  vs_ssi_readwrite(c);

  VS_DCS_DISABLE();

  return;
}


void vs_write_bass(void)
{
  vs_write_reg(VS_BASS, ((vs_stamp&0x0f)<<12)|((vs_stfreq&0x0f)<<8)|((vs_sbamp&0x0f)<<4)|((vs_sbfreq&0x0f)<<0));

  return;
}


void vs_write_volume(void)
{
  int vol;

  vol = 100-vs_vol;
  vs_write_reg(VS_VOL, (vol<<8)|(vol<<0));

  return;
}


void vs_write_plugin(const unsigned short *plugin, unsigned int len)
{
  unsigned int i;
  unsigned short addr, n, val;

  for(i=0; i<len;)
  {
    addr = plugin[i++];
    n    = plugin[i++];
    if(n & 0x8000U) //RLE run, replicate n samples
    {
      n  &= 0x7FFF;
      val = plugin[i++];
      while(n--) 
      {
        vs_write_reg(addr, val);
      }
    }
    else //copy run, copy n sample
    {
      while(n--)
      {
        val = plugin[i++];
        vs_write_reg(addr, val);
      }
    }
  }

  return;
}


unsigned int vs_read_ram(unsigned int addr)
{
  unsigned long ret;

  vs_write_reg(VS_WRAMADDR, addr);
  ret = vs_read_reg(VS_WRAM);

  return ret;
}


void vs_write_reg(unsigned int reg, unsigned int data)
{
  unsigned int timeout;

  IntMasterDisable();

  vs_ssi_wait(); //wait for transfer complete
  VS_DCS_DISABLE();
  VS_CS_ENABLE();
  vs_ssi_readwrite(VS_WRITE);
  vs_ssi_readwrite(reg);
  vs_ssi_readwrite(data>>8);
  vs_ssi_readwrite(data);
  VS_CS_DISABLE();

  //execution -> DREQ low
  switch(reg)
  {
    case VS_MODE:     timeout = 20000; break;
    case VS_STATUS:   timeout =   100; break;
    case VS_BASS:     timeout =  1000; break;
    case VS_CLOCKF:   timeout = 20000; break;
    case VS_WRAM:     timeout =   100; break;
    case VS_WRAMADDR: timeout =   100; break;
    case VS_VOL:      timeout =  1000; break;
    default:          timeout =  1000; break;
  }
  for(; timeout!=0; timeout--)
  {
    if(vs_request())
    {
      break;
    }
  }

  IntMasterEnable();

  return;
}


unsigned int vs_read_reg(unsigned int reg)
{
  unsigned int ret, timeout;

  IntMasterDisable();

  vs_ssi_wait(); //wait for transfer complete
  VS_DCS_DISABLE();
  VS_CS_ENABLE();
  vs_ssi_readwrite(VS_READ);
  vs_ssi_readwrite(reg);
  ret  = vs_ssi_readwrite(0xff)<<8;
  ret |= vs_ssi_readwrite(0xff);
  VS_CS_DISABLE();

  //execution -> DREQ low
  switch(reg)
  {
    case VS_MODE:     timeout = 20000; break;
    case VS_STATUS:   timeout =   100; break;
    case VS_BASS:     timeout =  1000; break;
    case VS_CLOCKF:   timeout = 20000; break;
    case VS_WRAM:     timeout =   100; break;
    case VS_WRAMADDR: timeout =   100; break;
    case VS_VOL:      timeout =  1000; break;
    default:          timeout =  1000; break;
  }
  for(; timeout!=0; timeout--)
  {
    if(vs_request())
    {
      break;
    }
  }

  IntMasterEnable();

  return ret;
}


void vs_pause(void)
{
  GPIOPinIntDisable(GPIO_PORTA_BASE, GPIO_PIN_1); //disable dreq irq
  GPIOPinIntClear(GPIO_PORTA_BASE, GPIO_PIN_1);

  return;
}


void vs_play(void)
{
  if(vs_request() && vs_playing)
  {
    GPIOPinIntEnable(GPIO_PORTA_BASE, GPIO_PIN_1); //enable dreq irq
  }

  return;
}


void vs_stopstream(void)
{
  unsigned int i, timeout;

  vs_pause();

  vs_ssi_wait(); //wait for transfer complete
  VS_DCS_DISABLE();
  VS_CS_DISABLE();

  //cancel playback
  vs_write_reg(VS_MODE, SM_SDINEW | SM_CANCEL);
  for(timeout=100; (timeout!=0) && (vs_read_reg(VS_MODE) & SM_CANCEL); timeout--)
  {
    VS_DCS_ENABLE();
    for(i=32; i!=0; i--)
    {
      vs_ssi_write(vs_bufgetc());
    }
    vs_ssi_wait();
    VS_DCS_DISABLE();
  }

  //flush buffer
  if(vs_read_reg(VS_HDAT0) || vs_read_reg(VS_HDAT1))
  {
    for(timeout=12288/32; timeout!=0; timeout--) //for FLAC 12288 otherwise 2052
    {
      VS_DCS_ENABLE();
      for(i=32; i!=0; i--)
      {
        vs_ssi_write(0x00);
      }
      vs_ssi_wait();
      VS_DCS_DISABLE();
    }
  }

  //reset
  if(vs_read_reg(VS_HDAT0) || vs_read_reg(VS_HDAT1))
  {
    vs_write_reg(VS_MODE, SM_SDINEW | SM_RESET); //soft-reset
    delay_ms(10);
    if(vs_read_reg(VS_HDAT0) || vs_read_reg(VS_HDAT1))
    {
      vs_setvolume(0); //0 -> analog power off
      vs_reset(); //hard-reset
    }
  }

  return;
}


void vs_stop(void)
{
  DEBUGOUT("VS: stop\n");

  if(vs_playing)
  {
    vs_stopstream();
  }

  buf_reset();

  vs_playing = 0;

  return;
}


void vs_start(void)
{
  DEBUGOUT("VS: start\n");

  vs_playing = 1;

  vs_pause();
  vs_setvolume(vs_vol);

  buf_reset();

  return;
}


void vs_reset(void)
{
  unsigned long i;

  DEBUGOUT("VS: reset\n");

  //ssi speed down
  vs_ssi_wait(); //wait for transfer complete
  vs_ssi_speed(2000000); //2 MHz

  //hard reset
  VS_CS_DISABLE();
  VS_DCS_DISABLE();
  VS_RST_ENABLE();
  delay_ms(5);
  VS_RST_DISABLE();
  delay_ms(10);

  //set registers
  vs_write_reg(VS_MODE, SM_SDINEW);

  //get VS version, set clock multiplier and load patch
  i = (vs_read_reg(VS_STATUS)&0xF0)>>4;
  if(i == 4)                                                         //VS1053
  {
    DEBUGOUT("VS: VS1053\n");
    vs_write_reg(VS_CLOCKF, 0x1800|VS1053_SC_MUL_4X);
    DEBUGOUT("VS: load VS1053B patch\n");                            //VS1053B
    vs_write_plugin(vs1053b_patch, VS1053B_PATCHLEN);
  }
  else if(i == 5)                                                    //VS1033
  {
    DEBUGOUT("VS: VS1033\n");
    vs_write_reg(VS_CLOCKF, 0x1800|VS1033_SC_MUL_4X);
    i = vs_read_ram(0x1942); //extra parameter (0x1940) -> version (0x1942)
    if(i < 3)                                                        //VS1033C
    {
      DEBUGOUT("VS: load VS1033C patch\n");
      vs_write_plugin(vs1033c_patch, VS1033C_PATCHLEN);
    }
    else                                                             //VS1033D
    {
      DEBUGOUT("VS: load VS1033D patch\n");
      vs_write_plugin(vs1033d_patch, VS1033D_PATCHLEN);
    }
  }

  //ssi speed up
  vs_ssi_speed(0); //0 = default speed

  return;
}


void vs_init(void)
{
  DEBUGOUT("VS: init\n");

  vs_playing = 0;

  //reset vs buffer
  vs_bufreset();

  //reset vs
  vs_reset();

  //set volume, bass, treble
  vs_setvolume(DEFAULT_VOLUME);
  vs_setbassfreq(DEFAULT_BASSFREQ);
  vs_setbassamp(DEFAULT_BASSAMP);
  vs_settreblefreq(DEFAULT_TREBLEFREQ); 
  vs_settrebleamp(DEFAULT_TREBLEAMP);

  //init pin interrupt
  GPIOIntTypeSet(GPIO_PORTA_BASE, GPIO_PIN_1, GPIO_HIGH_LEVEL);
  GPIOPortIntRegister(GPIO_PORTA_BASE, vs_requesthandler);

  return;
}
