#include <stdint.h>
#include <stdlib.h>
#include "fatfs/ff.h"
#include "tools.h"
#include "main.h"
#include "io.h"
#include "vs.h"
#include "eth.h"
#include "buffer.h"


BUFFER gbuf;


void buf_service(void)
{
  unsigned int len;

  if(fm_size())
  {
    len = fm_len();
    while(len)
    {
      if(vs_buffree() < 32)
      {
        break;
      }
      if(len < 32)
      {
        fm_gets(gbuf.card.buf, len);
        vs_bufputs(gbuf.card.buf, len);
        break;
      }
      else
      {
        fm_gets(gbuf.card.buf, 32);
        vs_bufputs(gbuf.card.buf, 32);
        len -= 32;
      }
    }
  }

  return;
}


void buf_puts(const unsigned char *s, unsigned int len)
{
  if(fm_size())
  {
    return fm_puts(s, len);
  }

  return vs_bufputs(s, len);
}


unsigned int buf_size(void)
{
  if(fm_size())
  {
    return fm_size();
  }

  return VS_BUFSIZE;
}


unsigned int buf_free(void)
{
  if(fm_size())
  {
    return fm_free();
  }

  return vs_buffree();
}


unsigned int buf_len(void)
{
  unsigned int len;

  len = vs_buflen();
  if(fm_size())
  {
    len += fm_len();
  }

  return len;
}


void buf_reset(void)
{
  fm_reset();
  vs_bufreset();

  return;
}
