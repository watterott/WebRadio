#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "fatfs/ff.h"
#include "../tools.h"
#include "../main.h"
#include "../io.h"
#include "../vs.h"
#include "../eth.h"
#include "../menu.h"
#include "../buffer.h"
#include "../station.h"
#include "utils.h"
#include "shoutcast.h"


volatile unsigned int shoutcast_status=SHOUTCAST_CLOSED;
unsigned int shoutcast_localport=0;


void shoutcast_close(void)
{
  long timeout;

  if(shoutcast_status != SHOUTCAST_CLOSED)
  {
    shoutcast_status = SHOUTCAST_CLOSE;

    timeout = getontime()+3; //3 seconds
    while(shoutcast_status != SHOUTCAST_CLOSED)
    {
      eth_service();
      if(getdeltatime(timeout) > 0)
      {
        shoutcast_status = SHOUTCAST_CLOSED;
        break;
      }
    }
  }

  return;
}


unsigned int shoutcast_open(void)
{
  long timeout;
  unsigned int idx, trying, status;

  shoutcast_status = SHOUTCAST_OPEN;

  //calc next local port
  switch(shoutcast_localport)
  {
    case SHOUTCAST_CLIENTPORT1: shoutcast_localport = SHOUTCAST_CLIENTPORT2; break;
    case SHOUTCAST_CLIENTPORT2: shoutcast_localport = SHOUTCAST_CLIENTPORT3; break;
    default:                    shoutcast_localport = SHOUTCAST_CLIENTPORT1; break;
  }

  if(gbuf.station.port == 0)
  {
    gbuf.station.port = SHOUTCAST_SERVERPORT;
  }

  idx     = tcp_open(TCP_ENTRIES, gbuf.station.mac, gbuf.station.ip, gbuf.station.port, shoutcast_localport);
  timeout = getontime()+SHOUTCAST_TIMEOUT;
  trying  = SHOUTCAST_TRY;
  for(;;)
  {
    eth_service();

    status = shoutcast_status;
    if((status == SHOUTCAST_CLOSED)     ||
       (status == SHOUTCAST_OPENED)     ||
       (status == SHOUTCAST_ADDRMOVED)  ||
       (status == SHOUTCAST_SERVERFULL) ||
       (status == SHOUTCAST_ERROR))
    {
      break;
    }
    if(keys_sw() || (ir_cmd() == SW_ENTER))
    {
        shoutcast_status = SHOUTCAST_ERROR;
        tcp_abort(idx);
        break;
    }
    if(getdeltatime(timeout) > 0)
    {
      timeout = getontime()+SHOUTCAST_TIMEOUT;
      if(--trying)
      {
        shoutcast_status = SHOUTCAST_OPEN;
        idx = tcp_open(idx, gbuf.station.mac, gbuf.station.ip, gbuf.station.port, shoutcast_localport);
      }
      else
      {
        shoutcast_status = SHOUTCAST_ERRTIMEOUT;
        tcp_abort(idx);
        break;
      }
    }
  }

  if(shoutcast_status == SHOUTCAST_OPENED)
  {
    return STATION_OPENED;
  }

  status = shoutcast_status;
  shoutcast_status = SHOUTCAST_CLOSED;
  switch(status)
  {
    case SHOUTCAST_ADDRMOVED:  return STATION_ADDRMOVED;  break;
    case SHOUTCAST_ERRTIMEOUT: return STATION_ERRTIMEOUT; break;
  }

  return STATION_ERROR;
}


//--- Filter Ogg headers ---
//It is one whole stream. So we only need the Ogg header at stream start.
//Otherwise there are sometimes problems with duplicate Ogg headers when a new song starts in the stream.
#define WORKINGBUF (64)
void shoutcast_putogg(const unsigned char *s, unsigned int len)
{
  static unsigned int state=0;
  unsigned char c, buf[WORKINGBUF];
  unsigned int buflen=0;
  long timeout;

  timeout = getontime()+2;

  //OggS 0x00 0x0x (0x02=1st, 0x04=last)
  while(len--)
  {
    c = *s++;
    if(c == 'O')
    {
      state = 1;
    }
    else
    {
      switch(state)
      {
        case 1: if(c == 'g') { state=2; }
                else         { state=0; } break;
        case 2: if(c == 'g') { state=3; }
                else         { state=0; } break;
        case 3: if(c == 'S') { state=4; }
                else         { state=0; } break;
        case 4: if(c == 0x00){ state=5; }
                else         { state=0; } break;
        case 5:
          state=0;
          if(c & 0x06) //0x04 || 0x02
          {
            c = 0x00; //0x00 modify header
          }
          break;
      }
    }
    buf[buflen++] = c;
    if(buflen == WORKINGBUF)
    {
      buflen = 0;
      while(WORKINGBUF > buf_free()) //wait for free buffer
      {
        buf_service();
        if(getdeltatime(timeout) > 0)
        {
          shoutcast_close();
          return;
        }
      }
      buf_puts(buf, WORKINGBUF);
    }
  }
  buf_puts(buf, buflen);

  return;
}


void shoutcast_putdata(const unsigned char *s, unsigned int len)
{
  unsigned int f;
  long timeout;

  timeout = getontime()+2;
  while(len)
  {
    buf_service();

    f = buf_free();
    if(f < len)
    {
      buf_puts(s, f);
      s   += f;
      len -= f;
      if(getdeltatime(timeout) > 0)
      {
        shoutcast_close();
        break;
      }
    }
    else
    {
      buf_puts(s, len);
      break;
    }
  }

  return;
}


void shoutcast_tcpapp(unsigned int idx, const unsigned char *rx, unsigned int rx_len, unsigned char *tx)
{
  unsigned int tx_len, i;
  char buf[32], *ptr;
  static unsigned int parse_header=0, format=0, format_header=0, skip=0;

  switch(shoutcast_status)
  {
    case SHOUTCAST_OPENED:
      //send ack
      tcp_send(idx, 0, 0);
      //save audio data
      if(format == FORMAT_OGG) //ogg stream
      {
        shoutcast_putogg(rx, rx_len);
      }
      else
      {
        shoutcast_putdata(rx, rx_len);
      }
      break;

    case SHOUTCAST_HEADER:
      if(rx_len)
      {
        //send ack
        tcp_send(idx, 0, 0);
        //search http header
        if(parse_header == 0)
        {
          i = http_response((const char*)rx);
          if(i == 200) //200 OK
          {
            parse_header = 1;
            menu_drawpopup("Station: OK");
          }
          else
          {
            switch(i)
            {
              case 301: //301 Moved Permanently
              case 302: //302 Moved Temporarily
              case 303: //303 See Other
                if(http_hdparam(gbuf.station.addr, MAX_ADDR-1, (const char*)rx, "LOCATION:") == 0)
                {
                  shoutcast_status = SHOUTCAST_ADDRMOVED;
                }
                else
                {
                  shoutcast_status = SHOUTCAST_ERROR;
                }
                menu_drawpopup("Station: Addr moved");
                break;
              case 400: //400 Server full
                shoutcast_status = SHOUTCAST_SERVERFULL;
                menu_drawpopup("Station: Server full");
                break;
              case 404: //404 Resource not found
                shoutcast_status = SHOUTCAST_ERROR;
                menu_drawpopup("Station: Not found");
                break;
              case 500: //500 Internal Server Error
                shoutcast_status = SHOUTCAST_ERROR;
                menu_drawpopup("Station: Server error");
                break;
              default: //Error
                shoutcast_status = SHOUTCAST_ERROR;
                menu_drawpopup("Station: Error");
                break;
            }
            parse_header = 0;
            rx_len       = 0;
            tcp_abort(idx);
            delay_ms(1000); //for reading popup
          }
        }

        //get stream infos and search http header end
        if(parse_header == 1)
        {
          //get stream bitrate
          if(http_hdparam(buf, 32-1, (const char*)rx, "ICY-BR:") == 0)
          {
            i = atoi(buf);
            if(fm_size() == 0)
            {
              if(i >= 128) //bitrate >= 128
              {
                skip = 64; //skip first 64 frames
              }
              else if(i >= 64) //bitrate >= 64
              {
                skip = 32; //skip first 32 frames
              }
              else if(i >= 16)
              {
                skip = 16; //skip first 16 frames
              }
              else if(i >= 8)
              {
                skip = 0; //skip nothing
              }
            }
            station_setbitrate(i);
          }
          //get stream format
          if(http_hdparam(buf, 32-1, (const char*)rx, "CONTENT-TYPE:") == 0)
          {
            ptr = buf;
            if(strncmpi(buf, "AUDIO/", 6) == 0)
            {
              ptr += 6;
            }
            else if(strncmpi(buf, "APPLICATION/", 12) == 0)
            {
              ptr += 12;
            }

            if((strncmpi(ptr, "X-WAV", 5) == 0) ||
               (strncmpi(ptr, "WAV",   3) == 0))           //WAV
            {
              format = FORMAT_WAV;
            }
            else if((strncmpi(ptr, "MPEG", 4) == 0) ||
                    (strncmpi(ptr, "MP3",  3) == 0))       //MP3
            {
              format = FORMAT_MP3;
            }
            else if(strncmpi(ptr, "AAC", 3) == 0)          //AAC
            {
              format = FORMAT_AAC;
            }
            else if(strncmpi(ptr, "OGG", 3) == 0)          //OGG
            {
              format = FORMAT_OGG;
            }
            else if((strncmpi(ptr, "X-MS-WMA", 8) == 0) ||
                    (strncmpi(ptr, "WMA",      3) == 0))   //WMA
            {
              format = FORMAT_WMA;
            }
            else if(strncmpi(ptr, "FLAC", 4) == 0)         //FLAC
            {
              format = FORMAT_FLAC;
            }
            menu_setformat(format);
          }
          //get stream name
          http_hdparam(gbuf.menu.info, MAX_INFO-1, (const char*)rx, "ICY-NAME:");
          //search http header end
          for(; rx_len!=0; rx_len--, rx++)
          {
            if(rx_len >= 4)
            {
              if((rx[0] == '\r') && (rx[1] == '\n') && (rx[2] == '\r') && (rx[3] == '\n'))
              {
                rx     += 4;
                rx_len -= 4;
                parse_header = 2;
                break;
              }
            }
          }
        }

        //skip the first x frames
        if(parse_header >= (2+skip))
        {
          switch(format)
          {
            case FORMAT_WAV:
              for(; rx_len!=0; rx_len--, rx++)
              {
                if((rx_len >= 3) &&
                   (rx[0]== 'W') && 
                   (rx[1]== 'A') &&
                   (rx[2]== 'V')) //WAVE sync: WAV
                {
                  break;
                }
              }
              break;
            case FORMAT_MP3:
              for(; rx_len!=0; rx_len--, rx++)
              {
                if((rx_len >= 2)         &&
                   (rx[0]       == 0xFF) && 
                  ((rx[1]&0xE0) == 0xE0)) //MPEG sync: 0xFFE (11x 1bits)
                {
                  break;
                }
              }
              break;
            case FORMAT_AAC:
             for(; rx_len!=0; rx_len--, rx++)
              {
                if(rx_len >= 2)
                {
                  if((rx[0]       == 0xFF) && 
                    ((rx[1]&0xF0) == 0xF0))        //AAC ADTS sync: 0xFFF
                  {
                    break;
                  }
                  else if((rx[0]       == 0x56) && 
                         ((rx[1]&0xE0) == 0xE0))   //AAC LATM/LOAS sync: 0x56E 
                  {
                    break;
                  }
                  else if((rx[0] == 0x4D) && 
                          (rx[1] == 0xE1))         //AAC LATM/LOAS sync: 0x4DE1
                  {
                    break;
                  }
                }
                if(rx_len >= 3)
                {
                  if((rx[0] == 'A') && 
                     (rx[1] == 'D') &&
                     (rx[2] == 'I'))               //AAC ADIF sync: ADI
                  {
                    break;
                  }
                }
              }
              break;
            case FORMAT_OGG:
              for(; rx_len!=0; rx_len--, rx++)
              {
                if((rx_len >= 3)  &&
                   (rx[0] == 'O') && 
                   (rx[1] == 'g') &&
                   (rx[2] == 'g')) //Ogg sync: Ogg
                {
                  break;
                }
              }
              break;
            case FORMAT_WMA:
              for(; rx_len!=0; rx_len--, rx++)
              {
                if((rx_len >= 3)  &&
                   (rx[0] == 0x82) && 
                   (rx[1] == 0x00) &&
                   (rx[2] == 0x00)) //WMA sync: 0x82 0x00 0x00
                {
                  break;
                }
              }
              break;
            case FORMAT_FLAC:
              for(; rx_len!=0; rx_len--, rx++)
              {
                if((rx_len >= 2)         &&
                   (rx[0]       == 0xFF) && 
                  ((rx[1]&0xF8) == 0xF8)) //FLAC sync: 0xFFF8 (13x 1bits)
                {
                  break;
                }
              }
              break;
          }
          if(rx_len)
          {
            buf_puts(rx, rx_len);
            rx_len = 0;
            shoutcast_status = SHOUTCAST_OPENED;
          }
        }
        else if(parse_header >= 2) //get format header (some formats start with a special frame with decoding infos)
        {
          parse_header++;
          switch(format)
          {
            case FORMAT_OGG:
              if(format_header < 4000) //Ogg header is around 4-5 kByte
              {
                buf_puts(rx, rx_len);
                format_header += rx_len;
                rx_len         = 0;
              }
              else if(format_header < 10000)
              {
                for(i=0; i<rx_len; i++)
                {
                  if(((rx_len-i) >= 3) &&
                      (rx[i+0] == 'O') && 
                      (rx[i+1] == 'g') &&
                      (rx[i+2] == 'g')) //Ogg sync: Ogg
                  {
                    format_header = 10000;
                    break;
                  }
                }
                buf_puts(rx, i);
                format_header += i;
                rx_len        -= i;
              }
              break;
            case FORMAT_WMA:
              if(format_header < 4000) //WMA header is around 5 kByte
              {
                buf_puts(rx, rx_len);
                format_header += rx_len;
                rx_len         = 0;
              }
              else if(format_header < 10000)
              {
                for(i=0; i<rx_len; i++)
                {
                  if(((rx_len-i) >= 3) &&
                      (rx[i+0] == 0x82) && 
                      (rx[i+1] == 0x00) &&
                      (rx[i+2] == 0x00)) ///WMA sync: 0x82 0x00 0x00
                  {
                    format_header = 10000;
                    break;
                  }
                }
                buf_puts(rx, i);
                format_header += i;
                rx_len        -= i;
              }
              break;
          }
        }
      }
      break;

    case SHOUTCAST_OPEN:
      shoutcast_status = SHOUTCAST_HEADER;
      parse_header     = 0;
      format           = FORMAT_UNKNOWN;
      format_header    = 0;
      if(fm_size() == 0) //no F-RAM
      {
        skip = 32; //default skip
      }
      else
      {
        skip = 0;
      }
      station_setbitrate(0); //set default bitrate / buffer limits
      menu_setformat(FORMAT_UNKNOWN);
      tx_len = sprintf((char*)tx, "GET %s HTTP/1.0\r\n"
                                  "Host: %s\r\n"
                                  "User-Agent: "APPNAME"\r\n"
                                  "Icy-MetaData: 0\r\n"
                                  "Connection: Keep-Alive\r\n"
                                  "\r\n",
                                  gbuf.station.file, gbuf.station.host);
      tcp_send(idx, tx_len, 0);
      break;

    case SHOUTCAST_CLOSE:
      shoutcast_status = SHOUTCAST_CLOSED;
      tcp_abort(idx);
      tcp_send(idx, 0, 0);
      break;

    case SHOUTCAST_CLOSED:
      break;
  }

  return;
}
